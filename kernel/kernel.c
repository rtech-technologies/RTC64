/* Modified by Sovereign: MEATY High-Power Kernel with Interrupts, APIC, PMM, and Preemptive base */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "pro_os.h"
#include "limine.h"
#include "app_ui.h"
#include "nk_software_renderer.h"
#include "serial.h"

volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST,
    .revision = 0
};

uint64_t hhdm_offset = 0;

static float font_get_width(nk_handle handle, float height, const char *text, int len) {
    (void)handle; (void)height; (void)text;
    return (float)len * 8.0f;
}

static const uint8_t cursor_bitmap[12] = {
    0b10000000, 0b11000000, 0b11100000, 0b11110000,
    0b11111000, 0b11111100, 0b11111110, 0b11110000,
    0b11011000, 0b10001100, 0b00001100, 0b00000000
};

void draw_cursor(tgx_canvas_t *canvas, int x, int y) {
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 8; j++) {
            if (cursor_bitmap[i] & (0x80 >> j)) {
                tgx_blit_rect(canvas, x + j, y + i, 1, 1, 0x00FFFF);
            }
        }
    }
}

void init_sse(void) {
    uint64_t cr0, cr4;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2);
    cr0 |= (1 << 1);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (3 << 9);
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));
}

extern void timer_handler(struct cpu_state* state);
extern void pmm_init(struct limine_memmap_response* map);

void kernel_main(void) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        while (1) { __asm__("hlt"); }
    }

    if (hhdm_request.response != NULL) {
        hhdm_offset = hhdm_request.response->offset;
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    tgx_canvas_t canvas = { (uint32_t*)fb->address, fb->width, fb->height, fb->pitch };

    serial_init();
    init_sse();

    if (memmap_request.response != NULL) {
        pmm_init(memmap_request.response);
    }

    gdt_init();
    idt_init();
    apic_init();
    irq_install_handler(32, timer_handler);
    __asm__ volatile("sti");

    serial_printf("[BOOT] Sovereign HIGH-POWER Kernel starting...\n");
    
    static uint8_t kernel_heap[16 * 1024 * 1024];
    hal_malloc_init(kernel_heap, sizeof(kernel_heap));

    hal_storage_init();
    hal_input_init();
    scheduler_init();
    vfs_init();
    pci_scan();
    hal_storage_finish_init();
    vfs_refresh_mounts();

    extern void system_shell_init(void);
    extern void system_shell_task(void);
    system_shell_init();
    scheduler_add_task("System Shell", system_shell_task);

    hal_usb_init();

    struct nk_context ctx;
    struct nk_user_font font;
    font.userdata = nk_handle_ptr(0);
    font.height = 8.0f;
    font.width = font_get_width;

    nk_init_default(&ctx, &font);
    ui_init_style(&ctx);

    struct app_state app;
    memset(&app, 0, sizeof(app));
    app.current_state = STATE_LOGIN;

    int cursor_x = fb->width / 2;
    int cursor_y = fb->height / 2;

    while (1) {
        tgx_clear(&canvas, 0x001010);
        hal_usb_poll();

        input_event_t ev;
        nk_input_begin(&ctx);
        while (hal_input_pop_event(&ev)) {
            if (ev.type == INPUT_TYPE_MOUSE) {
                cursor_x = ev.mouse.x;
                cursor_y = ev.mouse.y;
                nk_input_motion(&ctx, cursor_x, cursor_y);
                nk_input_button(&ctx, NK_BUTTON_LEFT, cursor_x, cursor_y, (ev.mouse.buttons & 1));
            }
        }
        nk_input_end(&ctx);

        scheduler_run();
        ui_render(&ctx, &app, fb->width, fb->height);

        struct nk_sw_fb sw_fb = { fb->address, fb->width, fb->height, fb->pitch };
        nk_sw_render(&sw_fb, &ctx);

        draw_cursor(&canvas, cursor_x, cursor_y);
        __asm__("pause");
    }
}
