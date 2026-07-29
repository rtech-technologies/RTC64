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
volatile struct limine_kernel_file_request kernel_file_req = { .id = LIMINE_KERNEL_FILE_REQUEST, .revision = 0 };

int g_safe_mode = 0;
int g_debug_mode = 0;
int g_nosmp = 0;
int g_exhaustive_logging = 0;

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

static uint32_t g_cursor_backup[32 * 32];
static int g_old_mx = -1, g_old_my = -1;
static bool g_cursor_saved = false;

void cursor_restore(void) {
    if (g_cursor_saved && g_old_mx >= 0 && g_old_my >= 0 && g_back_buffer && primary_fb) {
        int dw = (int)primary_fb->width;
        int dh = (int)primary_fb->height;
        uint32_t* dst = (uint32_t*)g_back_buffer;

        for (int y = 0; y < 32; y++) {
            int dst_y = g_old_my + y;
            if (dst_y >= dh) break;
            uint32_t* dst_row = dst + (dst_y * (primary_fb->pitch / 4));
            uint32_t* src_row = g_cursor_backup + (y * 32);

            for (int x = 0; x < 32; x++) {
                int dst_x = g_old_mx + x;
                if (dst_x >= dw) break;
                dst_row[dst_x] = src_row[x];
            }
        }
    }
}

void cursor_backup(int mx, int my) {
    if (g_back_buffer && primary_fb) {
        int dw = (int)primary_fb->width;
        int dh = (int)primary_fb->height;
        uint32_t* src = (uint32_t*)g_back_buffer;

        for (int y = 0; y < 32; y++) {
            int src_y = my + y;
            if (src_y >= dh) break;
            uint32_t* src_row = src + (src_y * (primary_fb->pitch / 4));
            uint32_t* dst_row = g_cursor_backup + (y * 32);

            for (int x = 0; x < 32; x++) {
                int src_x = mx + x;
                if (src_x >= dw) break;
                dst_row[x] = src_row[src_x];
            }
        }
        g_old_mx = mx;
        g_old_my = my;
        g_cursor_saved = true;
    }
}

void cursor_draw(int mx, int my) {
    if (g_back_buffer && primary_fb) {
        int dw = (int)primary_fb->width;
        int dh = (int)primary_fb->height;
        uint32_t* dst = (uint32_t*)g_back_buffer;

        static const char* cursor_mask[] = {
            "X               ",
            "XX              ",
            "X.X             ",
            "X..X            ",
            "X...X           ",
            "X....X          ",
            "X.....X         ",
            "X......X        ",
            "X.......X       ",
            "X........X      ",
            "X.........X     ",
            "X......XXXXX    ",
            "X...X..X        ",
            "X..X X..X       ",
            "X.X   X..X      ",
            "XX     X..X     ",
            "        X..X    ",
            "         XX     ",
            "                "
        };

        uint8_t r_shift = (uint8_t)primary_fb->red_mask_shift;
        uint8_t g_shift = (uint8_t)primary_fb->green_mask_shift;
        uint8_t b_shift = (uint8_t)primary_fb->blue_mask_shift;

        uint32_t white = (255 << r_shift) | (255 << g_shift) | (255 << b_shift);
        uint32_t black = 0;

        for (int y = 0; y < 19; y++) {
            int dst_y = my + y;
            if (dst_y >= dh) break;
            uint32_t* dst_row = dst + (dst_y * (primary_fb->pitch / 4));
            const char* row_mask = cursor_mask[y];

            for (int x = 0; x < 16; x++) {
                int dst_x = mx + x;
                if (dst_x >= dw) break;

                char c = row_mask[x];
                if (c == 'X') {
                    dst_row[dst_x] = black;
                } else if (c == '.') {
                    dst_row[dst_x] = white;
                }
            }
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

    /* Load custom TTF font from physical storage using our new your_os_fopen/your_os_fread block layers */
    size_t max_font_sz = 256 * 1024; // 256KB limit
    void* ttf_buffer = malloc(max_font_sz);
    if (ttf_buffer) {
        extern FILE* your_os_fopen(const char* filename, const char* mode);
        extern size_t your_os_fread(void* ptr, size_t size, size_t nmemb, FILE* stream);
        FILE* font_fp = your_os_fopen("/system/fonts/adwaita.ttf", "rb");
        if (font_fp) {
            size_t font_bytes = your_os_fread(ttf_buffer, 1, max_font_sz, font_fp);
            fclose(font_fp);
            if (font_bytes > 0) {
                serial_printf("[EM] Successfully loaded TTF font from FAT32 partition: %d bytes\n", (int)font_bytes);
            }
        }
        free(ttf_buffer);
    }

    struct rawfb_context *rawfb = nk_rawfb_init(g_back_buffer,
                          font_tex_mem, (unsigned int)primary_fb->width, (unsigned int)primary_fb->height, (unsigned int)primary_fb->pitch, pl);

    if (!rawfb) kpanic("NK_RAWFB_INIT_FAULT");
    struct nk_context *ctx = nk_rawfb_get_ctx(rawfb);
    ui_init_style(ctx);
    ui_icon_init();
    nk_style_hide_cursor(ctx); /* Disable default software cursor */

    static struct app_state app;
    memset(&app, 0, sizeof(app));
    strncpy(app.username, "Administrator", sizeof(app.username) - 1);
    app.current_state = STATE_LOGIN;
    app.installed = 0;
    strcpy(app.explorer_path, "/");

    int mx = 100, my = 100;
    uint64_t start_time = hal_get_uptime_ms();

    while(1) {
        if (g_boot_phase == 1 && (hal_get_uptime_ms() - start_time) > 3000) g_boot_phase = 2;

        /* 1. Erase the old cursor from the shadow buffer */
        if (g_boot_phase == 2) {
            cursor_restore();
        }

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

        /* 2. Render desktop widgets and windows */
        if (g_boot_phase == 1) {
            if (nk_begin(ctx, "Boot", nk_rect(0, 0, (float)primary_fb->width, (float)primary_fb->height), NK_WINDOW_NO_SCROLLBAR)) {
                draw_rtech_logo(ctx, (int)primary_fb->width, (int)primary_fb->height);
            }
            nk_end(ctx);
            nk_rawfb_render(rawfb, nk_rgb(20, 20, 20), 1);
        } else {
            wallpaper_render();
            ui_render(ctx, &app, (int)primary_fb->width, (int)primary_fb->height);

            extern int g_safe_mode;
            if (g_safe_mode) {
                struct nk_color orange_color = nk_rgb(255, 120, 0);
                if (nk_begin(ctx, "SAFE_MODE_OVERLAY", nk_rect((float)primary_fb->width / 2.0f - 100.0f, 5.0f, 200.0f, 35.0f),
                             NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BORDER)) {
                    nk_layout_row_dynamic(ctx, 22, 1);
                    nk_label_colored(ctx, "SAFE MODE", NK_TEXT_CENTERED, orange_color);
                }
                nk_end(ctx);
            }

            nk_rawfb_render(rawfb, nk_rgb(20, 20, 20), 0);
        }

        /* 3. Backup background and blit hardware-style cursor arrow */
        if (g_boot_phase == 2) {
            cursor_backup(mx, my);
            cursor_draw(mx, my);
        }

        /* 4. Complete the Shadow Framebuffer rendering loop */
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
    serial_printf("[PHASE 0] Step 1: The Bootloader Handoff and Registry Mapping.\n");

    if (kernel_file_req.response && kernel_file_req.response->kernel_file) {
        const char *cmdline = kernel_file_req.response->kernel_file->cmdline;
        if (cmdline) {
            serial_printf("[BOOT] Command Line: %s\n", cmdline);
            if (strstr(cmdline, "safe-mode")) {
                g_safe_mode = 1;
                serial_printf("[BOOT] safe-mode flag is set.\n");
            }
            if (strstr(cmdline, "debug")) {
                g_debug_mode = 1;
                serial_printf("[BOOT] debug flag is set.\n");
            }
            if (strstr(cmdline, "nosmp")) {
                g_nosmp = 1;
                serial_printf("[BOOT] nosmp flag is set.\n");
            }
            if (strstr(cmdline, "exhaustive_logging")) {
                g_exhaustive_logging = 1;
                serial_printf("[BOOT] exhaustive_logging flag is set.\n");
            }
        }
    }

    if (hhdm_req.response) hhdm_offset = hhdm_req.response->offset;
    if (framebuffer_request.response && framebuffer_request.response->framebuffer_count > 0)
        primary_fb = framebuffer_request.response->framebuffers[0];

    serial_printf("[PHASE 0] Step 4: The Hardware Architecture Frame Setup.\n");
    gdt_init();
    idt_init();
    msr_init();

    serial_printf("[PHASE 0] Step 2: The Core Memory Matrix Allocation.\n");
    if (memmap_req.response) {
        /* Enable SSE early so low-level optimized routines may use XMM
         * instructions during early boot (e.g., optimized memset/memcpy). */
        init_sse();
        pmm_init(memmap_req.response);
    } else {
        kpanic("MISSING_MEMMAP");
    }

    serial_printf("[PHASE 0] Step 3: The Critical Kernel Heap Genesis.\n");
    void* phys_heap = pmm_alloc_blocks(8192); // 32MB Heap
    if (!phys_heap) kpanic("HEAP_GENESIS_FAULT");
    void* virt_heap = (void*)((uint64_t)phys_heap + hhdm_offset);
    hal_malloc_init(virt_heap, 8192 * 4096);

    serial_printf("[PHASE 0] Step 5: The Entropy and Security Activation.\n");
    apic_init();

    serial_printf("[PHASE 0] Step 7: Subsystem Threading.\n");
    scheduler_init();

    serial_printf("[PHASE 0] Step 6: Probing I/O Matrix and Driver Orchestration.\n");
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

    if (!g_safe_mode) {
        scheduler_spawn_kernel("Compliance", comprec_task, NULL);
    } else {
        serial_printf("[BOOT] SAFE MODE active: Compliance service skipped.\n");
    }

    serial_printf("[PHASE 1] Step 8: The Graphics Subsystem and Input Loop Launch.\n");
    scheduler_spawn_kernel("Environment Manager", environment_manager_entry, NULL);

    scheduler_run();
}
