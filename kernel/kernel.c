#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "pro_os.h"
#include "limine.h"
#include "app_ui.h"
#include "nk_software_renderer.h"

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

// The true, freestanding entry point
void kernel_main(void) {
    // 1. Initial Proof of Life & Check Blindness
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        while (1) { __asm__("hlt"); }
    }

    if (hhdm_request.response != NULL) {
        hhdm_offset = hhdm_request.response->offset;
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    tgx_canvas_t canvas = { (uint32_t*)fb->address, fb->width, fb->height, fb->pitch };

    // 2. System Bootstrap
    // Allocate 16MB for the kernel heap
    static uint8_t kernel_heap[16 * 1024 * 1024];
    hal_malloc_init(kernel_heap, sizeof(kernel_heap));

    hal_storage_init();
    hal_input_init();
    scheduler_init();
    vfs_init();
    user_init();
    hal_usb_init();

    // 3. UI Initialization
    struct nk_context ctx;
    struct nk_user_font font;
    font.userdata = nk_handle_ptr(0);
    font.height = 8.0f;
    font.width = font_get_width;

    nk_init_default(&ctx, &font);
    ui_init_style(&ctx);

    struct app_state app;
    memset(&app, 0, sizeof(app));
    // Start with the Installer first as requested
    app.current_state = STATE_INSTALLER;

    int cursor_x = fb->width / 2;
    int cursor_y = fb->height / 2;

    // 4. Main Executive Loop
    while (1) {
        tgx_clear(&canvas, 0x001010); // Dark Teal Background
        hal_usb_poll();

        input_event_t ev;
        nk_input_begin(&ctx);
        while (hal_input_pop_event(&ev)) {
            if (ev.type == INPUT_TYPE_MOUSE) {
                cursor_x = ev.mouse.x;
                cursor_y = ev.mouse.y;
                nk_input_motion(&ctx, cursor_x, cursor_y);
                nk_input_button(&ctx, NK_BUTTON_LEFT, cursor_x, cursor_y, (ev.mouse.buttons & 1));
            } else if (ev.type == INPUT_TYPE_KEYBOARD) {
                // Route keyboard characters or action events to Nuklear
                if (ev.kbd.down) {
                    uint32_t key = ev.kbd.key;
                    if (key >= 32 && key <= 126) {
                        nk_input_char(&ctx, (char)key);
                    } else if (key == 8) { // Backspace
                        nk_input_key(&ctx, NK_KEY_BACKSPACE, 1);
                        nk_input_key(&ctx, NK_KEY_BACKSPACE, 0);
                    } else if (key == 13) { // Enter
                        nk_input_key(&ctx, NK_KEY_ENTER, 1);
                        nk_input_key(&ctx, NK_KEY_ENTER, 0);
                    } else if (key == 38) { // Up
                        nk_input_key(&ctx, NK_KEY_UP, 1);
                        nk_input_key(&ctx, NK_KEY_UP, 0);
                    } else if (key == 40) { // Down
                        nk_input_key(&ctx, NK_KEY_DOWN, 1);
                        nk_input_key(&ctx, NK_KEY_DOWN, 0);
                    }
                }
            }
        }
        nk_input_end(&ctx);

        // Run the scheduler to handle background tasks
        scheduler_run();

        ui_render(&ctx, &app, fb->width, fb->height);

        struct nk_sw_fb sw_fb = { fb->address, fb->width, fb->height, fb->pitch };
        nk_sw_render(&sw_fb, &ctx);

        // Draw Hardware Cursor
        tgx_blit_rect(&canvas, cursor_x, cursor_y, 4, 4, 0xFFFFFF);

        // Logical flow delay using scheduler-aware mechanics
        __asm__("pause");
    }
}
