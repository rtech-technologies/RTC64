/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "hal.h"
#include "serial.h"
#include <string.h>
#include "nuklear.h"
#include "nuklear_rawfb.h"
#include "app_ui.h"

NK_API struct nk_context* nk_rawfb_get_ctx(struct rawfb_context* rawfb);

/* --- Limine Requests --- */
static volatile struct limine_hhdm_request hhdm_req = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

volatile struct limine_module_request module_request = {
    .id = LIMINE_MODULE_REQUEST,
    .revision = 0
};

/* --- Global OS State --- */
uint64_t hhdm_offset = 0;
struct limine_framebuffer *primary_fb = NULL;

/* --- Executive Subsystems --- */
void init_sse(void) {
    uint64_t cr0, cr4;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1ULL << 2);
    cr0 |= (1ULL << 1);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));

    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (3ULL << 9);
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));
}

void environment_manager_entry(void* arg) {
    (void)arg;
    serial_printf("[ENV] Environment Manager Started.\n");

    if (!primary_fb) {
        serial_printf("[ENV] FATAL: No Framebuffer.\n");
        while(1) { scheduler_yield(); }
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
    struct rawfb_context *rawfb = nk_rawfb_init((void*)((uint64_t)primary_fb->address + hhdm_offset),
                          font_tex_mem,
                          (unsigned int)primary_fb->width, (unsigned int)primary_fb->height,
                          (unsigned int)primary_fb->pitch, pl);

    struct nk_context *ctx = nk_rawfb_get_ctx(rawfb);
    ui_init_style(ctx);

    static struct app_state app;
    memset(&app, 0, sizeof(app));
    app.current_state = STATE_DESKTOP;

    while(1) {
        input_event_t ev;
        nk_input_begin(ctx);
        while (hal_input_pop_event(&ev)) {
            if (ev.type == INPUT_TYPE_MOUSE) {
                nk_input_motion(ctx, (float)ev.mouse.x, (float)ev.mouse.y);
                nk_input_button(ctx, NK_BUTTON_LEFT, ev.mouse.x, ev.mouse.y, (int)(ev.mouse.buttons & 1));
            }
        }
        nk_input_end(ctx);

        ui_render(ctx, &app, (int)primary_fb->width, (int)primary_fb->height);

        nk_rawfb_render(rawfb, nk_rgb(30,30,30), 1);
        scheduler_yield();
    }
}

void kernel_main(void) {
    serial_init();
    serial_printf("[BOOT] Phase 0: System Genesis\n");

    if (hhdm_req.response) {
        hhdm_offset = hhdm_req.response->offset;
    }

    if (framebuffer_request.response && framebuffer_request.response->framebuffer_count > 0) {
        primary_fb = framebuffer_request.response->framebuffers[0];
    }

    gdt_init();
    idt_init();

    pmm_init(NULL);
    hal_malloc_init(pmm_alloc_blocks(1024), 1024 * 4096);

    apic_init();
    init_sse();

    scheduler_init();

    vfs_init();
    hal_storage_init();
    pci_scan();
    hal_usb_init();
    hal_ps2_init();

    serial_printf("[BOOT] Spawning High-Priority Input Tasks...\n");
    scheduler_spawn("KBD", kbd_task, NULL);
    scheduler_spawn("MOUSE", mouse_task, NULL);
    scheduler_spawn("Compliance", comprec_task, NULL);

    serial_printf("[BOOT] Final Handoff to Environment Manager...\n");
    scheduler_spawn("Environment Manager", environment_manager_entry, NULL);

    scheduler_run();
}
