/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "hal.h"
#include "serial.h"
#include <string.h>
#include <math.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear.h"
#include "nuklear_rawfb.h"
#include "app_ui.h"

NK_API struct nk_context* nk_rawfb_get_ctx(struct rawfb_context* rawfb);

volatile struct limine_hhdm_request hhdm_req = { .id = LIMINE_HHDM_REQUEST, .revision = 0 };
volatile struct limine_framebuffer_request framebuffer_request = { .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0 };
volatile struct limine_module_request module_request = { .id = LIMINE_MODULE_REQUEST, .revision = 0 };
volatile struct limine_memmap_request memmap_req = { .id = LIMINE_MEMMAP_REQUEST, .revision = 0 };

uint64_t hhdm_offset = 0;
struct limine_framebuffer *primary_fb = NULL;
static int g_boot_phase = 1;

void init_sse(void) {
    uint64_t cr0, cr4;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1ULL << 2); cr0 |= (1ULL << 1);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (3ULL << 9);
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));
}

void draw_rtech_logo(struct nk_context *ctx, int screen_w, int screen_h) {
    struct nk_command_buffer *canvas = nk_window_get_canvas(ctx);
    struct nk_rect bounds = nk_rect((float)screen_w/2.0f - 100.0f, (float)screen_h/2.0f - 50.0f, 200.0f, 100.0f);
    nk_fill_rect(canvas, bounds, 0, nk_rgb(0, 120, 215));
    nk_draw_text(canvas, nk_rect(bounds.x + 40.0f, bounds.y + 35.0f, 120.0f, 30.0f), "R-TECH", 6,
                ctx->style.font, nk_rgb(30, 30, 30), nk_rgb(255, 255, 255));
}

void* g_back_buffer = NULL;

typedef struct {
    void* pixels;
    int width;
    int height;
    int channels;
    char current_path[256];
} wallpaper_state_t;

static wallpaper_state_t g_wallpaper = {NULL, 0, 0, 0, ""};
static bool g_wallpaper_loaded = false;

extern const char* registry_get(const char* key);

void wallpaper_render(void) {
    const char* path = registry_get("HKCU\\ControlPanel\\Desktop\\Wallpaper");
    if (!path) path = "/system/wallpapers/pawel-czerwinski.jpg";

    if (strcmp(g_wallpaper.current_path, path) != 0) {
        serial_printf("[WALLPAPER] Requesting wallpaper change to: %s\n", path);
        size_t max_sz = 8 * 1024 * 1024; // 8MB limit
        void* file_buf = malloc(max_sz);
        if (file_buf) {
            int bytes_read = vfs_read(path, file_buf, max_sz);
            if (bytes_read > 0) {
                int w = 0, h = 0, channels = 0;
                extern unsigned char *stbi_load_from_memory(unsigned char const *buffer, int len, int *x, int *y, int *channels_in_file, int desired_channels);
                void* decoded = stbi_load_from_memory(file_buf, bytes_read, &w, &h, &channels, 4);
                if (decoded) {
                    if (g_wallpaper.pixels) {
                        extern void stbi_image_free(void *retval_from_stbi_load);
                        stbi_image_free(g_wallpaper.pixels);
                    }
                    g_wallpaper.pixels = decoded;
                    g_wallpaper.width = w;
                    g_wallpaper.height = h;
                    g_wallpaper.channels = 4;
                    strncpy(g_wallpaper.current_path, path, sizeof(g_wallpaper.current_path) - 1);
                    g_wallpaper.current_path[sizeof(g_wallpaper.current_path) - 1] = '\0';
                    g_wallpaper_loaded = true;
                    serial_printf("[WALLPAPER] Decoding success: %dx%d\n", w, h);
                } else {
                    serial_printf("[WALLPAPER] STBI load failed\n");
                }
            } else {
                serial_printf("[WALLPAPER] VFS read failed for path: %s\n", path);
            }
            free(file_buf);
        } else {
            serial_printf("[WALLPAPER] Allocation failure for file buffer\n");
        }
    }

    if (g_wallpaper_loaded && g_wallpaper.pixels && g_back_buffer && primary_fb) {
        int sw = g_wallpaper.width;
        int sh = g_wallpaper.height;
        int dw = (int)primary_fb->width;
        int dh = (int)primary_fb->height;
        uint32_t* dst = (uint32_t*)g_back_buffer;
        uint8_t r_shift = (uint8_t)primary_fb->red_mask_shift;
        uint8_t g_shift = (uint8_t)primary_fb->green_mask_shift;
        uint8_t b_shift = (uint8_t)primary_fb->blue_mask_shift;

        for (int y = 0; y < dh; y++) {
            int src_y = (y * sh) / dh;
            if (src_y >= sh) src_y = sh - 1;
            uint8_t* src_row = (uint8_t*)g_wallpaper.pixels + (src_y * sw * 4);
            uint32_t* dst_row = dst + (y * (primary_fb->pitch / 4));

            for (int x = 0; x < dw; x++) {
                int src_x = (x * sw) / dw;
                if (src_x >= sw) src_x = sw - 1;
                uint8_t* src_pixel = src_row + (src_x * 4);
                dst_row[x] = ((uint32_t)src_pixel[0] << r_shift) |
                             ((uint32_t)src_pixel[1] << g_shift) |
                             ((uint32_t)src_pixel[2] << b_shift);
            }
        }
    } else {
        if (g_back_buffer && primary_fb) {
            memset(g_back_buffer, 20, primary_fb->height * primary_fb->pitch);
        }
    }
}

void environment_manager_entry(void* arg) {
    (void)arg;
    serial_printf("[EM] Environment Manager started.\n");
    if (!primary_fb) {
        serial_printf("[EM] Error: Primary framebuffer missing.\n");
        while(1) scheduler_yield();
    }

    struct rawfb_pl pl;
    pl.bytesPerPixel = primary_fb->bpp / 8;
    pl.rshift = (unsigned char)primary_fb->red_mask_shift;
    pl.gshift = (unsigned char)primary_fb->green_mask_shift;
    pl.bshift = (unsigned char)primary_fb->blue_mask_shift;
    pl.ashift = 0;
    pl.rloss = 8 - (unsigned char)primary_fb->red_mask_size;
    pl.gloss = 8 - (unsigned char)primary_fb->green_mask_size;
    pl.bloss = 8 - (unsigned char)primary_fb->blue_mask_size;
    pl.aloss = 8;

    void* font_tex_mem = malloc(2 * 1024 * 1024);
    if (!font_tex_mem) kpanic("FONT_ALLOC_FAILED");

    /* Double Buffering: Allocate back buffer */
    size_t fb_size = primary_fb->height * primary_fb->pitch;
    g_back_buffer = malloc(fb_size);
    if (!g_back_buffer) kpanic("BACK_BUFFER_ALLOC_FAILED");

    /* Limine FB address is virtual, but we render to backbuffer */
    serial_printf("[EM] FB Address: %p (Virtual), BackBuffer: %p\n", (void*)primary_fb->address, g_back_buffer);

    struct rawfb_context *rawfb = nk_rawfb_init(g_back_buffer,
                          font_tex_mem, (unsigned int)primary_fb->width, (unsigned int)primary_fb->height, (unsigned int)primary_fb->pitch, pl);

    if (!rawfb) kpanic("NK_RAWFB_INIT_FAULT");
    struct nk_context *ctx = nk_rawfb_get_ctx(rawfb);
    ui_init_style(ctx);
    ui_icon_init();
    nk_style_show_cursor(ctx);

    static struct app_state app;
    memset(&app, 0, sizeof(app));
    strncpy(app.username, "Administrator", sizeof(app.username) - 1);
    app.current_state = STATE_LOGIN;
    app.installed = 0;
    strcpy(app.explorer_path, "/");

    int mx, my;
    uint64_t start_time = hal_get_uptime_ms();

    while(1) {
        if (g_boot_phase == 1 && (hal_get_uptime_ms() - start_time) > 3000) g_boot_phase = 2;

        input_event_t ev;
        nk_input_begin(ctx);
        while (hal_input_pop_event(&ev)) {
            if (ev.type == INPUT_TYPE_MOUSE) {
                hal_input_get_mouse_abs(&mx, &my);
                if (mx < 0) mx = 0;
                if (mx >= (int)primary_fb->width) mx = (int)primary_fb->width - 1;
                if (my < 0) my = 0;
                if (my >= (int)primary_fb->height) my = (int)primary_fb->height - 1;

                nk_input_motion(ctx, (float)mx, (float)my);
                nk_input_button(ctx, NK_BUTTON_LEFT, mx, my, (ev.mouse.buttons & 1));
                nk_input_button(ctx, NK_BUTTON_RIGHT, mx, my, (ev.mouse.buttons & 2));
                if (ev.mouse.scroll != 0) nk_input_scroll(ctx, nk_vec2(0, (float)ev.mouse.scroll));
            }
        }
        nk_input_end(ctx);

        if (g_boot_phase == 1) {
            if (nk_begin(ctx, "Boot", nk_rect(0, 0, (float)primary_fb->width, (float)primary_fb->height), NK_WINDOW_NO_SCROLLBAR)) {
                draw_rtech_logo(ctx, (int)primary_fb->width, (int)primary_fb->height);
            }
            nk_end(ctx);
            nk_rawfb_render(rawfb, nk_rgb(20, 20, 20), 1);
        } else {
            wallpaper_render();
            ui_render(ctx, &app, (int)primary_fb->width, (int)primary_fb->height);
            nk_rawfb_render(rawfb, nk_rgb(20, 20, 20), 0);
        }

        /* Flush back-buffer to primary framebuffer */
        memcpy((void*)primary_fb->address, g_back_buffer, fb_size);

        scheduler_yield();
    }
}

void hal_get_screen_size(int *w, int *h) {
    if (primary_fb) {
        if (w) *w = (int)primary_fb->width;
        if (h) *h = (int)primary_fb->height;
    } else {
        if (w) *w = 800;
        if (h) *h = 600;
    }
}

void kernel_main(void) {
    serial_init();
    serial_printf("[BOOT] Stage 0: Initialized.\n");

    if (hhdm_req.response) hhdm_offset = hhdm_req.response->offset;
    if (framebuffer_request.response && framebuffer_request.response->framebuffer_count > 0)
        primary_fb = framebuffer_request.response->framebuffers[0];

    gdt_init();
    idt_init();
    msr_init();

    if (memmap_req.response) {
        /* Enable SSE early so low-level optimized routines may use XMM
         * instructions during early boot (e.g., optimized memset/memcpy). */
        init_sse();
        pmm_init(memmap_req.response);
    } else {
        kpanic("MISSING_MEMMAP");
    }

    void* phys_heap = pmm_alloc_blocks(8192); // 32MB Heap
    if (!phys_heap) kpanic("HEAP_GENESIS_FAULT");
    void* virt_heap = (void*)((uint64_t)phys_heap + hhdm_offset);
    hal_malloc_init(virt_heap, 8192 * 4096);

    apic_init();

    scheduler_init();

    vfs_init();
    hal_storage_init();
    linux_compat_init();
    virtio_net_linux_init();
    extern int e1000_init(void);
    extern int rtl8139_init(void);
    extern int iwlwifi_init(void);
    e1000_init();
    rtl8139_init();
    iwlwifi_init();
    pci_scan();
    extern void hal_storage_finish_init(void);
    hal_storage_finish_init();

    extern uint64_t xhci_mmio_base;
    extern uint64_t ehci_mmio_base;
    bool has_usb = (xhci_mmio_base != 0 || ehci_mmio_base != 0);

    if (has_usb) {
        hal_usb_init();
    } else {
        hal_ps2_init();
    }

    vfs_refresh_mounts();
    extern void registry_init(void);
    registry_init();

    if (has_usb) {
        scheduler_spawn_kernel("USB", usb_task, NULL);
    } else {
        scheduler_spawn_kernel("KBD", kbd_task, NULL);
        scheduler_spawn_kernel("MOUSE", mouse_task, NULL);
    }

    scheduler_spawn_kernel("Compliance", comprec_task, NULL);
    scheduler_spawn_kernel("Environment Manager", environment_manager_entry, NULL);

    scheduler_run();
}
