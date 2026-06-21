/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "pro_os.h"
#include "limine.h"
#include "app_ui.h"
#include "nuklear_rawfb.h"
#include "serial.h"

// Repaired Limine configuration initialization macro structures
__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);
__attribute__((used, section(".limine_requests"))) volatile struct limine_framebuffer_request framebuffer_request = { .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0 };
__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = { .id = LIMINE_HHDM_REQUEST, .revision = 0 };
__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = { .id = LIMINE_MEMMAP_REQUEST, .revision = 0 };
__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;
__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

uint64_t hhdm_offset = 0;
struct limine_framebuffer *primary_fb;
struct app_state os_app;

static float font_get_width(nk_handle handle, float height, const char *text, int len) {
    (void)handle; (void)height; (void)text; return (float)len * 8.0f;
}

void init_sse(void) {
    uint64_t cr0, cr4;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(1 << 2); cr0 |= (1 << 1);
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (3 << 9);
    __asm__ volatile("mov %0, %%cr4" : : "r"(cr4));
}

void environment_manager_entry(void* arg) {
    (void)arg;
    serial_printf("[PHASE 7] Environment Manager session pivot successful.\n");

    void* virt_fb_addr = (void*)((uint64_t)primary_fb->address + hhdm_offset);
    struct rawfb_pl pl = {4, 16, 8, 0, 24, 0, 0, 0, 0};

    /* industrial fix: use kernel heap allocator (malloc is mapped to it) */
    struct rawfb_context* rawfb = nk_rawfb_init(virt_fb_addr, malloc(1024*1024), (unsigned int)primary_fb->width, (unsigned int)primary_fb->height, (unsigned int)primary_fb->pitch, pl);
    struct nk_context* ctx = (struct nk_context*)rawfb;

    struct nk_user_font font;
    font.userdata = nk_handle_ptr(0);
    font.height = 8.0f;
    font.width = font_get_width;
    nk_init_default(ctx, &font);
    ui_init_style(ctx);
    chell_init(&os_app);

    while (1) {
        hal_usb_poll();
        input_event_t ev;
        nk_input_begin(ctx);

        /* Protection for shared event polling states */
        __asm__ volatile("cli");
        int has_event = hal_input_pop_event(&ev);
        __asm__ volatile("sti");

        if (has_event) {
            if (ev.type == INPUT_TYPE_MOUSE) {
                nk_input_motion(ctx, (float)ev.mouse.x, (float)ev.mouse.y);
                nk_input_button(ctx, NK_BUTTON_LEFT, (int)ev.mouse.x, (int)ev.mouse.y, (int)(ev.mouse.buttons & 1));
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

        ui_render(ctx, &os_app, (int)primary_fb->width, (int)primary_fb->height);
        nk_rawfb_render(rawfb, nk_rgba(30,30,30,255), 1);
        scheduler_yield();
    }
}

void session_manager_task(void* arg) {
    (void)arg;
    serial_printf("[SMSS] Initializing Session 0...\n");
    memset(&os_app, 0, sizeof(os_app));
    os_app.current_state = STATE_DESKTOP;
    os_app.show_terminal = 1;

    /* PHASE 6: Service Control Manager */
    serial_printf("[PHASE 6] Starting core background services (SCM)...\n");
    scheduler_add_task("COMPREC", (void*)comprec_task, NULL, 1, 1);

    /* PHASE 7: Environment Manager */
    serial_printf("[PHASE 7] Spawning User-mode Environment Manager...\n");
    scheduler_spawn("Environment Manager", environment_manager_entry, NULL);

    while(1) { scheduler_yield(); }
}

void kernel_main(void) {
    /* REMOVED destructive inline __asm__ stack pointer overwrite */
    serial_init();
    serial_printf("[PHASE 0] Entering Bare-Metal Isolation.\n");

    /* Step 1: Bootloader Handoff */
    serial_printf("[STEP 1] Bootloader Handoff (Limine).\n");
    if (!framebuffer_request.response || !hhdm_request.response || !memmap_request.response) while(1) __asm__("hlt");
    hhdm_offset = hhdm_request.response->offset;
    primary_fb = framebuffer_request.response->framebuffers[0];
    init_sse();

    /* Step 2: Memory Matrix */
    serial_printf("[STEP 2] Building Physical Memory Manager (PMM).\n");
    pmm_init(memmap_request.response);

    /* Step 3: Kernel Heap Genesis */
    serial_printf("[STEP 3] Initializing Executive Pool (Heap).\n");
    void* heap_phys = pmm_alloc_blocks(4096);
    hal_malloc_init((void*)((uint64_t)heap_phys + hhdm_offset), 4096 * 4096);

    /* Step 4: Architecture Frame Setup */
    serial_printf("[STEP 4] Constructing Security Boundaries (GDT/IDT).\n");
    gdt_init(); idt_init();

    /* Step 5: Entropy and Security Activation */
    serial_printf("[STEP 5] Calibrating Local APIC and Security Entropy.\n");
    apic_init(); irq_install_handler(32, (void*)timer_handler);

    /* Step 6: Hardware Peripheral I/O Probe */
    serial_printf("[STEP 6] Probing I/O Matrix and Driver Orchestration.\n");
    hal_input_init(); pci_scan(); cm_orchestrate_drivers();

    /* Step 7: Subsystem Threading */
    serial_printf("[STEP 7] Initializing Subsystem Threading (Scheduler).\n");
    scheduler_init();

    /* PHASE 1: Kernel Initialization */
    serial_printf("[PHASE 1] Kernel Initialization. Spawning SMSS.\n");

    /* PHASE 2: Namespace Initialization */
    serial_printf("[PHASE 2] Establishing VFS and System Namespace.\n");
    vfs_init(); vfs_refresh_mounts();

    /* PHASE 3: Security Monitor (SRM) */
    serial_printf("[PHASE 3] Activating Security Reference Monitor (SRM).\n");

    /* PHASE 4: Power & I/O Manager */
    serial_printf("[PHASE 4] Finalizing Storage Stacks and IRP Stability.\n");
    hal_storage_init(); hal_storage_finish_init();

    /* PHASE 5: Session Manager (smss.exe) */
    serial_printf("[PHASE 5] Spawning Session Manager (smss.exe)...\n");
    system_shell_init();

    /* Secure queueing: populate scheduler before interrupts enabled */
    scheduler_add_task("System Shell", (void*)system_shell_task, NULL, 1, 1);
    scheduler_add_task("SMSS", session_manager_task, NULL, 1, 1);

    serial_printf("[PHASE 1] Enabling STI. Multitasking active.\n");
    apic_timer_unmask();
    __asm__ volatile("sti");

    while (1) { scheduler_run(); __asm__ volatile("hlt"); }
}
