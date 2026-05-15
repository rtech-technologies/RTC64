#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
// No NK_IMPLEMENTATION here
#include "nuklear.h"
#include "app_ui.h"
#include "services.h"
#include "nk_software_renderer.h"
#include "usbd_core.h"
#include "usbh_core.h"
#include "hal.h"

service_table_t g_services = { (void*)1, (void*)1, (void*)1, (void*)1 };

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

void _start(void) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];

    struct nk_context ctx;
    struct nk_user_font font;
    font.userdata = nk_handle_ptr(0);
    font.height = 8.0f;
    font.width = font_get_width;

    nk_init_default(&ctx, &font);
    ui_init_style(&ctx);

    /* Initialize Hardware Abstraction Layer */
    hal_input_init();
    hal_storage_init();

    /* Expansion point drivers */
    void hal_nvme_init(void);
    void hal_sata_init(void);
    void hal_satapi_init(void);
    hal_nvme_init();
    hal_sata_init();
    hal_satapi_init();

    hal_usb_init();

    struct app_state app;
    app.current_state = STATE_LOGIN;
    app.progress = 0;
    app.install_started = 0;

    while (1) {
        ui_render(&ctx, &app, fb->width, fb->height);

        struct nk_sw_fb sw_fb = { fb->address, fb->width, fb->height, fb->pitch };
        nk_sw_render(&sw_fb, &ctx);

        // Simple delay
        for (volatile int i = 0; i < 10000000; i++);
    }
}
