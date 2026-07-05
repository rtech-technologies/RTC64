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

static void* g_back_buffer = NULL;

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
        } else {
            ui_render(ctx, &app, (int)primary_fb->width, (int)primary_fb->height);
        }

        nk_rawfb_render(rawfb, nk_rgb(20, 20, 20), 1);

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
    pci_scan();
    hal_usb_init();
    hal_ps2_init();
    serial_printf("[BOOT] Checking for Rescue Mode (Hold F1)...\n");
    /* Minimal probe: if F1 (0x3B) is pressed, drop to shell */
    if (inb(0x60) == 0x3B) {
        serial_printf("[RESCUE] Manual override detected! Launching Emergency Shell.\n");
        debug_shell_task(NULL);
    }
    vfs_refresh_mounts();

    scheduler_spawn_kernel("KBD", kbd_task, NULL);
    scheduler_spawn_kernel("MOUSE", mouse_task, NULL);
    scheduler_spawn_kernel("Compliance", comprec_task, NULL);
    scheduler_spawn_kernel("Environment Manager", environment_manager_entry, NULL);

    scheduler_run();
}
