#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "pro_os.h"
#include "limine.h"
#include "app_ui.h"
#include "nk_software_renderer.h"
#include "serial.h"

// Tell the bootloader we want a graphical framebuffer
volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

uint64_t hhdm_offset = 0;

static float font_get_width(nk_handle handle, float height, const char *text, int len) {
    (void)handle; (void)height; (void)text;
    return (float)len * 8.0f;
}

// SSE and GDT Initialization
static void init_cpu_features(void) {
    uint64_t cr0, cr4;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1ULL << 2); // EM
    cr0 |= (1ULL << 1);  // MP
    __asm__ volatile ("mov %0, %%cr0" :: "r"(cr0));

    __asm__ volatile ("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1ULL << 9);  // OSFXSR
    cr4 |= (1ULL << 10); // OSXMMEXCPT
    __asm__ volatile ("mov %0, %%cr4" :: "r"(cr4));
}

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct gdt_entry gdt[3];
static struct gdt_ptr gdtp;

static void init_gdt(void) {
    gdt[0] = (struct gdt_entry){0, 0, 0, 0, 0, 0}; // Null
    gdt[1] = (struct gdt_entry){0, 0, 0, 0x9A, 0x20, 0}; // Code (64-bit)
    gdt[2] = (struct gdt_entry){0, 0, 0, 0x92, 0x00, 0}; // Data

    gdtp.limit = sizeof(gdt) - 1;
    gdtp.base = (uint64_t)&gdt;

    __asm__ volatile (
        "lgdt %0\n\t"
        "push $0x08\n\t"
        "lea 1f(%%rip), %%rax\n\t"
        "push %%rax\n\t"
        "lretq\n\t"
        "1:\n\t"
        "mov $0x10, %%ax\n\t"
        "mov %%ax, %%ds\n\t"
        "mov %%ax, %%es\n\t"
        "mov %%ax, %%fs\n\t"
        "mov %%ax, %%gs\n\t"
        "mov %%ax, %%ss\n\t"
        : : "m"(gdtp) : "rax", "memory"
    );
}

#include "idt.h"

// The true, freestanding entry point
void kernel_main(void) {
    // --- Phase 0: Immediate Logging ---
    serial_init();
    serial_write("[PHASE 0] Sovereign OS Kernel Booting...\n");

    // --- Phase 1: Processor Prep ---
    serial_write("[PHASE 1] Initializing CPU features (SSE, GDT, IDT)...\n");
    init_cpu_features();
    init_gdt();
    idt_init();

    // Initial Proof of Life & Check Blindness
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        serial_write("[ERROR] No graphical framebuffer available!\n");
        while (1) { __asm__("hlt"); }
    }
    serial_write("[INFO] Graphical framebuffer acquired.\n");

    if (hhdm_request.response != NULL) {
        hhdm_offset = hhdm_request.response->offset;
        serial_write("[INFO] HHDM Offset integrated.\n");
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    tgx_canvas_t canvas = { (uint32_t*)fb->address, fb->width, fb->height, fb->pitch };

    // --- Phase 2: Memory Sovereignty ---
    serial_write("[PHASE 2] Initializing Kernel Heap (TLSF, 16MB)...\n");
    // Allocate 16MB for the kernel heap
    static uint8_t kernel_heap[16 * 1024 * 1024];
    hal_malloc_init(kernel_heap, sizeof(kernel_heap));

    // --- Phase 3: Hardware Discovery ---
    serial_write("[PHASE 3] Starting hardware discovery...\n");
    hal_storage_init();
    hal_input_init();

    extern void pci_scan(void);
    pci_scan();

    // --- Phase 4: Logical Services ---
    serial_write("[PHASE 4] Initializing Logical Services (VFS, Scheduler)...\n");
    vfs_init();
    scheduler_init();

    // --- Phase 5: Peripheral Activation ---
    serial_write("[PHASE 5] Activating USB Stack...\n");
    hal_usb_init();

    // --- Phase 6: UI Subsystem ---
    serial_write("[PHASE 6] Initializing Nuklear UI...\n");
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
    chell_init(&app.chell);
    lab_init(&app.lab);
    installer_init(&app.installer);

    int cursor_x = fb->width / 2;
    int cursor_y = fb->height / 2;

    // --- Phase 7: Main Executive Loop ---
    serial_write("[PHASE 7] Entering Main Executive Loop.\n");
    while (1) {
        tgx_clear(&canvas, 0x001010); // Dark Teal Background

        /* Event Polling */
        hal_usb_poll();

        input_event_t ev;
        nk_input_begin(&ctx);
        while (hal_input_pop_event(&ev)) {
            if (ev.type == INPUT_TYPE_MOUSE) {
                cursor_x += ev.mouse.x;
                cursor_y += ev.mouse.y;
                if (cursor_x < 0) cursor_x = 0;
                if (cursor_y < 0) cursor_y = 0;
                if (cursor_x >= (int)fb->width) cursor_x = fb->width - 1;
                if (cursor_y >= (int)fb->height) cursor_y = fb->height - 1;

                nk_input_motion(&ctx, cursor_x, cursor_y);
                nk_input_button(&ctx, NK_BUTTON_LEFT, cursor_x, cursor_y, (ev.mouse.buttons & 1));

                app.lab.last_x = cursor_x;
                app.lab.last_y = cursor_y;
            } else if (ev.type == INPUT_TYPE_KEYBOARD) {
                if (ev.kbd.down) {
                    app.lab.last_key = ev.kbd.key;
                    // Extremely basic HID to ASCII for Chell testing
                    char c = 0;
                    if (ev.kbd.key >= 0x04 && ev.kbd.key <= 0x1D) c = 'a' + (ev.kbd.key - 0x04);
                    else if (ev.kbd.key >= 0x1E && ev.kbd.key <= 0x27) c = (ev.kbd.key == 0x27) ? '0' : '1' + (ev.kbd.key - 0x1E);
                    else if (ev.kbd.key == 0x28) c = '\n'; // Enter
                    else if (ev.kbd.key == 0x2C) c = ' ';  // Space
                    else if (ev.kbd.key == 0x2A) c = '\b'; // Backspace

                    if (c) nk_input_char(&ctx, c);
                }
            }
        }
        nk_input_end(&ctx);

        /* Logic Update */
        scheduler_run();

        /* UI Render */
        ui_render(&ctx, &app, fb->width, fb->height);

        struct nk_sw_fb sw_fb = { fb->address, fb->width, fb->height, fb->pitch };
        nk_sw_render(&sw_fb, &ctx);

        /* FB Flush / Hardware Cursor Draw */
        tgx_blit_rect(&canvas, cursor_x, cursor_y, 5, 5, 0x00FFFF);
        tgx_blit_rect(&canvas, cursor_x+1, cursor_y+1, 3, 3, 0xFFFFFF);

        __asm__("pause");
    }
}
