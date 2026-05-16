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

#include "drivers/font_8x8.h"

void panic(const char *msg) {
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    tgx_canvas_t canvas = { (uint32_t*)fb->address, fb->width, fb->height, fb->pitch };

    /* Red screen of death */
    tgx_clear(&canvas, 0xFF0000);

    /* Draw panic icon / drawing */
    /* Let's draw a simple "X" or a warning box */
    int cx = fb->width / 2;
    int cy = fb->height / 2;

    tgx_blit_rect(&canvas, cx - 50, cy - 80, 100, 10, 0xFFFFFF); // Top bar
    tgx_blit_rect(&canvas, cx - 50, cy + 30, 100, 10, 0xFFFFFF); // Bottom bar
    tgx_blit_rect(&canvas, cx - 50, cy - 80, 10, 120, 0xFFFFFF); // Left bar
    tgx_blit_rect(&canvas, cx + 40, cy - 80, 10, 120, 0xFFFFFF); // Right bar

    /* Drawing an exclamation mark (!) */
    tgx_blit_rect(&canvas, cx - 5, cy - 50, 10, 40, 0xFFFFFF);
    tgx_blit_rect(&canvas, cx - 5, cy, 10, 10, 0xFFFFFF);

    /* Render Text Message */
    const char *header = "CORE SYSTEM PANIC";
    int h_len = strlen(header);
    int m_len = strlen(msg);

    /* Manual text drawing using font_8x8_data */
    int tx = cx - (h_len * 8) / 2;
    int ty = cy + 60;

    for (int i = 0; i < h_len; i++) {
        uint8_t c = (uint8_t)header[i];
        if (c < 32 || c > 126) continue;
        int idx = c - 32;
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                if (font_8x8_data[idx][row] & (1 << col)) {
                    canvas.pixels[(ty + row) * (canvas.pitch / 4) + (tx + i * 8 + col)] = 0xFFFFFF;
                }
            }
        }
    }

    tx = cx - (m_len * 8) / 2;
    ty = cy + 80;
    for (int i = 0; i < m_len; i++) {
        uint8_t c = (uint8_t)msg[i];
        if (c < 32 || c > 126) continue;
        int idx = c - 32;
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                if (font_8x8_data[idx][row] & (1 << col)) {
                    canvas.pixels[(ty + row) * (canvas.pitch / 4) + (tx + i * 8 + col)] = 0xFFFFFF;
                }
            }
        }
    }

    hcf();
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
            panic("USER TRIGGERED PANIC TEST");
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
