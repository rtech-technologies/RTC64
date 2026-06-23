/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "pro_os.h"
#include "limine.h"
#include "app_ui.h"
#define NK_IMPLEMENTATION
#include "nuklear.h"
#include "nuklear_rawfb.h"
#include "serial.h"

__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);
__attribute__((used, section(".limine_requests"))) volatile struct limine_framebuffer_request framebuffer_request = { .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0 };
__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = { .id = LIMINE_HHDM_REQUEST, .revision = 0 };
__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = { .id = LIMINE_MEMMAP_REQUEST, .revision = 0 };
__attribute__((used, section(".limine_requests"))) volatile struct limine_module_request module_request = { .id = LIMINE_MODULE_REQUEST, .revision = 0 };
__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

uint64_t hhdm_offset = 0;
struct limine_framebuffer *primary_fb;
struct app_state os_app;

extern int main(void);
extern void debug_shell_init(void);
extern void debug_shell_task(void* arg);

void init_sse(void) {
    uint64_t cr0, cr4;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2); cr0 |= (1 << 1);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (3 << 9);
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));
}

void init_pat(void) {
    /* Set PAT0 to WB, PAT1 to WC */
    uint64_t pat = 0x0000000000000106ULL; /* PAT0=WB (06), PAT1=WC (01) */
    uint32_t low = (uint32_t)pat;
    uint32_t high = (uint32_t)(pat >> 32);
    __asm__ volatile("wrmsr" : : "c"(0x277), "a"(low), "d"(high));
}

static int g_mouse_x = 400;
static int g_mouse_y = 300;

const char cursor_arrow[12][12] = {
    "X___________",
    "XX__________",
    "XXX_________",
    "XXXX________",
    "XXXXX_______",
    "XXXXXX______",
    "XXXXXXX_____",
    "XXXXXXXX____",
    "XXXXX_______",
    "XX__XX______",
    "X____XX_____",
    "______XX____"
};

void draw_software_cursor(uint32_t *buffer, int mx, int my, int width, int height) {
    /* Draw shadow/border */
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            if (cursor_arrow[y][x] == 'X') {
                for (int dy = -1; dy <= 1; dy++) {
                    for (int dx = -1; dx <= 1; dx++) {
                        int nx = mx + x + dx;
                        int ny = my + y + dy;
                        if (nx >= 0 && nx < width && ny >= 0 && ny < height) {
                            buffer[ny * width + nx] = 0xFF000000;
                        }
                    }
                }
            }
        }
    }
    /* Draw white arrow */
    for (int y = 0; y < 12; y++) {
        for (int x = 0; x < 12; x++) {
            if (cursor_arrow[y][x] == 'X') {
                if ((mx + x) >= 0 && (mx + x) < width && (my + y) >= 0 && (my + y) < height) {
                    buffer[(my + y) * width + (mx + x)] = 0xFFFFFFFF;
                }
            }
        }
    }
}

void environment_manager_entry(void* arg) {
    (void)arg;
    serial_printf("[PHASE 7] Environment Manager session pivot successful.\n");
    comprec_log("WINDOW", "Graphical Environment Started");
    vga_disable_log();

    uint32_t width = (uint32_t)primary_fb->width;
    uint32_t height = (uint32_t)primary_fb->height;
    uint32_t fb_size = width * height * 4;
    uint32_t *shadow_fb = (uint32_t*)malloc(fb_size);
    memset(shadow_fb, 0, fb_size);

    struct rawfb_pl pl = {4, 16, 8, 0, 24, 0, 0, 0, 0};
    void* nuklear_mem = malloc(2*1024*1024);
    struct rawfb_context* rawfb = nk_rawfb_init(shadow_fb, nuklear_mem, width, height, width * 4, pl);
    struct nk_context* ctx = (struct nk_context*)rawfb;

    /* Font Initialization and Baking */
    struct nk_font_atlas atlas;
    nk_font_atlas_init_default(&atlas);
    nk_font_atlas_begin(&atlas);
    struct nk_font *default_font = nk_font_atlas_add_default(&atlas, 13.0f, NULL);

    const void *font_image;
    int font_width, font_height;
    font_image = nk_font_atlas_bake(&atlas, &font_width, &font_height, NK_FONT_ATLAS_RGBA32);
    nk_rawfb_font_bake(rawfb, font_image, font_width, font_height);

    nk_font_atlas_end(&atlas, nk_handle_id(0), NULL);
    nk_style_set_font(ctx, &default_font->handle);

    ui_init_style(ctx);
    chell_init(&os_app);

    while (1) {
        hal_usb_poll();
        input_event_t ev;
        nk_input_begin(ctx);

        while (hal_input_pop_event(&ev)) {
            if (ev.type == INPUT_TYPE_MOUSE) {
                g_mouse_x += ev.mouse.x;
                g_mouse_y += ev.mouse.y;
                if (g_mouse_x < 0) g_mouse_x = 0;
                if (g_mouse_y < 0) g_mouse_y = 0;
                if (g_mouse_x >= (int)width) g_mouse_x = width - 1;
                if (g_mouse_y >= (int)height) g_mouse_y = height - 1;

                nk_input_motion(ctx, (float)g_mouse_x, (float)g_mouse_y);
                nk_input_button(ctx, NK_BUTTON_LEFT, g_mouse_x, g_mouse_y, (int)(ev.mouse.buttons & 1));
            } else if (ev.type == INPUT_TYPE_KEYBOARD) {
                if (ev.kbd.down) {
                    if (ev.kbd.key >= 32 && ev.kbd.key <= 126) nk_input_unicode(ctx, (nk_rune)ev.kbd.key);
                    if (ev.kbd.key == 0x0A || ev.kbd.key == 0x0D) nk_input_key(ctx, NK_KEY_ENTER, 1);
                    if (ev.kbd.key == 0x08) nk_input_key(ctx, NK_KEY_BACKSPACE, 1);
                } else {
                    if (ev.kbd.key == 0x0A || ev.kbd.key == 0x0D) nk_input_key(ctx, NK_KEY_ENTER, 0);
                    if (ev.kbd.key == 0x08) nk_input_key(ctx, NK_KEY_BACKSPACE, 0);
                }
            }
        }
        nk_input_end(ctx);

        ui_render(ctx, &os_app, (int)width, (int)height);
        nk_rawfb_render(rawfb, nk_rgba(30,30,30,255), 1);

        /* Draw Software Cursor */
        draw_software_cursor(shadow_fb, g_mouse_x, g_mouse_y, (int)width, (int)height);

        /* Blit to physical framebuffer */
        memcpy((void*)primary_fb->address, shadow_fb, fb_size);

        scheduler_yield();
    }
}

void session_manager_task(void* arg) {
    (void)arg;
    serial_printf("[SMSS] Initializing Session 0...\n");
    memset(&os_app, 0, sizeof(os_app));
    os_app.current_state = STATE_DESKTOP;
    os_app.show_terminal = 1;
    scheduler_add_task("COMPREC", (void*)comprec_task, NULL, 1, 1);
    main();
    scheduler_spawn("Environment Manager", environment_manager_entry, NULL);
    while(1) { scheduler_yield(); }
}

void kernel_main(void) {
    serial_init();
    serial_printf("[PHASE 0] Entering Bare-Metal Isolation.\n");
    if (!framebuffer_request.response || !hhdm_request.response || !memmap_request.response) while(1) __asm__("hlt");
    hhdm_offset = hhdm_request.response->offset;
    primary_fb = framebuffer_request.response->framebuffers[0];
    init_sse();
    init_pat();
    pmm_init(memmap_request.response);
    void* heap_phys = pmm_alloc_blocks(4096);
    hal_malloc_init((void*)((uint64_t)heap_phys + hhdm_offset), 4096 * 4096);
    gdt_init(); idt_init();
    apic_init(); irq_install_handler(32, (void*)timer_handler);
    hal_input_init(); cm_orchestrate_drivers();
    scheduler_init();
    vfs_init(); vfs_refresh_mounts();
    hal_storage_finish_init();
    vfs_refresh_mounts();
    debug_shell_init();
    scheduler_add_task("Debug Shell", (void*)debug_shell_task, NULL, 1, 1);
    scheduler_add_task("SMSS", session_manager_task, NULL, 1, 1);
    apic_timer_unmask();
    __asm__ volatile("sti");
    while (1) { scheduler_run(); __asm__ volatile("hlt"); }
}
