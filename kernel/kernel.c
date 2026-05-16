#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "pro_os.h"
#include "app_ui.h"
#include "nk_software_renderer.h"

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

static float font_get_width(nk_handle handle, float height, const char *text, int len) {
    (void)handle; (void)height; (void)text;
    return (float)len * 8.0f;
}

static void hcf(void) {
    __asm__ ("cli");
    for (;;) __asm__ ("hlt");
}

struct panic_framebuffer {
    uint64_t address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
};

struct panic_framebuffer* get_kernel_framebuffer(void) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        return NULL;
    }
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    static struct panic_framebuffer pfb;
    pfb.address = (uint64_t)fb->address;
    pfb.width = fb->width;
    pfb.height = fb->height;
    pfb.pitch = fb->pitch;
    return &pfb;
}

/* Global Cursor Position */
static int cursor_x = 0;
static int cursor_y = 0;

void _start(void) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    /* Initialize "Pro" Subsystems */
    static uint8_t kernel_heap[1024 * 1024 * 4]; // 4MB Heap
    tlsf_create_with_pool(kernel_heap, sizeof(kernel_heap));

    scheduler_init();
    vfs_init();
    void pci_scan(void);
    pci_scan();
    hal_input_init();
    hal_storage_init();
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

    tgx_canvas_t canvas = { (uint32_t*)fb->address, fb->width, fb->height, fb->pitch };

    while (1) {
        /* 1. Poll Hardware */
        hal_usb_poll();

        /* 2. Update Input */
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

        /* Test Panic Trigger (e.g., if cursor is at top-left corner) */
        if (cursor_x < 5 && cursor_y < 5 && cursor_x > 0) {
            kpanic("USER TRIGGERED PANIC TEST");
        }

        /* 3. Run Scheduler */
        scheduler_run();

        /* 4. Render UI */
        ui_render(&ctx, &app, fb->width, fb->height);

        struct nk_sw_fb sw_fb = { fb->address, fb->width, fb->height, fb->pitch };
        nk_sw_render(&sw_fb, &ctx);

        /* 5. Draw Global Cursor (High Priority) */
        tgx_blit_rect(&canvas, cursor_x, cursor_y, 8, 8, 0xFFFFFFFF);

        // Direct delay
        for (volatile int i = 0; i < 5000000; i++);
    }
}
