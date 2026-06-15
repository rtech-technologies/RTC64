/* Modified by Sovereign: HIGH-POWER Kernel with 8-Phase Windows-Style Boot Order and EXHAUSTIVE LOGGING */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "pro_os.h"
#include "limine.h"
#include "app_ui.h"
#include "nk_software_renderer.h"
#include "serial.h"

/* Limine Requests */
volatile struct limine_framebuffer_request framebuffer_request = { .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0 };
static volatile struct limine_hhdm_request hhdm_request = { .id = LIMINE_HHDM_REQUEST, .revision = 0 };
static volatile struct limine_memmap_request memmap_request = { .id = LIMINE_MEMMAP_REQUEST, .revision = 0 };

uint64_t hhdm_offset = 0;
extern void timer_handler(struct cpu_state* state);

/* Environment Manager Data */
struct nk_context nk_ctx;
struct app_state os_app;
struct limine_framebuffer *primary_fb;

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
    serial_printf("[SSE] Activating Streaming SIMD Extensions (SSE2)...\n");
    uint64_t cr0, cr4;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2);
    cr0 |= (1 << 1);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (3 << 9);
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));
    serial_printf("[SSE] SSE Control Registers updated. XMM operations enabled.\n");
}

/* STEP 8: The Graphics Subsystem and Input Loop Launch (Environment Manager) */
void environment_manager_entry(void) {
    serial_printf("[USER] Environment Manager session pivot successful. PID: 1\n");
    tgx_canvas_t canvas = { (uint32_t*)primary_fb->address, primary_fb->width, primary_fb->height, primary_fb->pitch };
    int cursor_x = primary_fb->width / 2;
    int cursor_y = primary_fb->height / 2;

    serial_printf("[USER] Initializing Nuklear GUI State engine...\n");
    while (1) {
        tgx_clear(&canvas, 0x001010);
        hal_usb_poll();

        input_event_t ev;
        nk_input_begin(&nk_ctx);
        while (hal_input_pop_event(&ev)) {
            if (ev.type == INPUT_TYPE_MOUSE) {
                cursor_x = ev.mouse.x;
                cursor_y = ev.mouse.y;
                nk_input_motion(&nk_ctx, cursor_x, cursor_y);
                nk_input_button(&nk_ctx, NK_BUTTON_LEFT, cursor_x, cursor_y, (ev.mouse.buttons & 1));
            }
        }
        nk_input_end(&nk_ctx);

        ui_render(&nk_ctx, &os_app, primary_fb->width, primary_fb->height);
        struct nk_sw_fb sw_fb = { primary_fb->address, primary_fb->width, primary_fb->height, primary_fb->pitch };
        nk_sw_render(&sw_fb, &nk_ctx);

        draw_cursor(&canvas, cursor_x, cursor_y);
        __asm__("pause");
    }
}

extern void* pmm_alloc_blocks(size_t count);

void kernel_main(void) {
    /* PHASE 0: The Bare-Metal Isolation Layer */
    __asm__ volatile("cli");
    serial_init();
    serial_printf("\n\n#################################################################\n");
    serial_printf("# Sovereign RTC64 HIGH-POWER Executive Initialization Sequence #\n");
    serial_printf("#################################################################\n\n");
    serial_printf("[PHASE 0] Entering Bare-Metal Isolation Layer. Interrupts disabled.\n");

    /* STEP 1: The Bootloader Handoff and Registry Mapping */
    serial_printf("[STEP 1] Executing Winload-style handoff from Limine...\n");
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        serial_printf("[FATAL] Video output device not found. Halting.\n");
        while (1) { __asm__("hlt"); }
    }
    if (hhdm_request.response != NULL) {
        hhdm_offset = hhdm_request.response->offset;
        serial_printf("[STEP 1] HHDM Mapping established at: %p\n", (void*)hhdm_offset);
    }
    primary_fb = framebuffer_request.response->framebuffers[0];
    serial_printf("[STEP 1] Framebuffer registered: %dx%d @ %p\n", primary_fb->width, primary_fb->height, primary_fb->address);
    init_sse();

    /* STEP 2: The Core Memory Matrix Allocation */
    serial_printf("[STEP 2] Building Physical Memory Matrix...\n");
    if (memmap_request.response != NULL) {
        pmm_init(memmap_request.response);
    } else {
        serial_printf("[FATAL] Architectural memory map unavailable.\n");
        while(1) { __asm__("hlt"); }
    }

    /* STEP 3: The Critical Kernel Heap Genesis */
    serial_printf("[STEP 3] Establishing Kernel Heap Genesis (Executive Pool)...\n");
    // Allocate 16MB for the kernel heap from PMM to ensure it's in the Direct Map range
    size_t heap_pages = (16 * 1024 * 1024) / 4096;
    void* heap_phys = pmm_alloc_blocks(heap_pages);
    if (!heap_phys) {
        serial_printf("[FATAL] Failed to allocate 16MB for Executive Heap.\n");
        while(1) { __asm__("hlt"); }
    }
    void* kernel_heap = (void*)((uint64_t)heap_phys + hhdm_offset);
    hal_malloc_init(kernel_heap, heap_pages * 4096);
    serial_printf("[STEP 3] Executive Heap (16MB) allocated at: Phys %p -> Virt %p\n", heap_phys, kernel_heap);

    /* STEP 4: The Hardware Architecture Frame Setup */
    serial_printf("[STEP 4] Constructing Global Descriptor and Interrupt Tables...\n");
    gdt_init();
    idt_init();
    serial_printf("[STEP 4] CPU Exception Gateways and architectural frames loaded.\n");

    /* PHASE 1: The Executive Subsystem Onboarding */
    serial_printf("[PHASE 1] Transitioning to Executive Subsystem Onboarding.\n");
    /* STEP 5: The Entropy and Security Activation */
    serial_printf("[STEP 5] Clock genesis: Activating Local APIC and Timer interrupts...\n");
    apic_init();
    irq_install_handler(32, timer_handler);
    __asm__ volatile("sti");
    serial_printf("[STEP 5] STI executed. System interrupts are now ACTIVE.\n");

    /* STEP 6: The Hardware Peripheral I/O Probe */
    serial_printf("[STEP 6] Probing I/O Matrix and initializing peripheral drivers...\n");
    hal_input_init();
    scheduler_init();
    vfs_init();
    pci_scan();
    hal_storage_init();
    hal_storage_finish_init();
    vfs_refresh_mounts();
    hal_usb_init();
    serial_printf("[STEP 6] I/O Manager initialized. Hardware start-drivers loaded.\n");

    /* USER SPACE: The Environment Management Hand-off */
    serial_printf("[USER] Performing Session Manager Pivot (smss.exe equivalent)...\n");
    /* STEP 7: The Session Manager Pivot (smss.exe Equivalent) */
    serial_printf("[STEP 7] Spawning PID 1 (Environment Manager Task)...\n");
    struct nk_user_font font;
    font.userdata = nk_handle_ptr(0);
    font.height = 8.0f;
    font.width = font_get_width;
    nk_init_default(&nk_ctx, &font);
    ui_init_style(&nk_ctx);
    memset(&os_app, 0, sizeof(os_app));
    os_app.current_state = STATE_LOGIN;

    /* Modified by Sovereign: Launch persistent System Shell and Environment Manager */
    extern void system_shell_init(void);
    extern void system_shell_task(void);
    system_shell_init();
    scheduler_add_task("System Shell", system_shell_task);

    scheduler_add_task("Environment Manager", environment_manager_entry);

    serial_printf("[USER] Hand-off complete. Relinquishing core control to scheduler.\n");
    /* Hand off to preemptive scheduler loop */
    while (1) {
        scheduler_run();
    }
}
