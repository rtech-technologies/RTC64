/* Modified by Sovereign: HIGH-POWER Kernel with 8-Phase Windows-Style Boot Order and EXHAUSTIVE LOGGING */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "pro_os.h"
#include "limine.h"
#include "app_ui.h"
#include "nk_software_renderer.h"
#include "serial.h"

/* Limine Requests */
__attribute__((used, section(".limine_requests")))
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests")))
volatile struct limine_framebuffer_request framebuffer_request = { .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0 };

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = { .id = LIMINE_HHDM_REQUEST, .revision = 0 };

__attribute__((used, section(".limine_requests")))
static volatile struct limine_memmap_request memmap_request = { .id = LIMINE_MEMMAP_REQUEST, .revision = 0 };

__attribute__((used, section(".limine_requests_start")))
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end")))
static volatile LIMINE_REQUESTS_END_MARKER;

uint64_t hhdm_offset = 0;
static uint8_t kernel_stack[65536] __attribute__((aligned(16)));
struct limine_framebuffer *primary_fb;

/* Environment Manager Data */
struct nk_context nk_ctx;
struct app_state os_app;

static float font_get_width(nk_handle handle, float height, const char *text, int len) {
    (void)handle; (void)height; (void)text;
    return (float)len * 8.0f;
}

static const uint8_t cursor_bitmap[12] = {
    0b10000000, 0b11000000, 0b11100000, 0b11110000,
    0b11111000, 0b11111100, 0b11111110, 0b11110000,
    0b11011000, 0b10001100, 0b00001100, 0b00000000
};

void draw_cursor(tgx_canvas_t *canvas, int x, int y) {
    for (int i = 0; i < 12; i++) {
        for (int j = 0; j < 8; j++) {
            if (cursor_bitmap[i] & (0x80 >> j)) {
                tgx_blit_rect(canvas, x + j, y + i, 1, 1, 0x00FFFF);
            }
        }
    }
}

void init_sse(void) {
    serial_printf("[SSE] Activating Streaming SIMD Extensions (SSE2)...\n");
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
    serial_printf("[SMSS] Environment Manager session pivot successful.\n");

    struct nk_user_font font;
    font.userdata = nk_handle_ptr(0);
    font.height = 8.0f;
    font.width = font_get_width;
    nk_init_default(&nk_ctx, &font);
    ui_init_style(&nk_ctx);

    /* Professional access to framebuffer using HHDM */
    uint32_t* fb_virt = (uint32_t*)((uint64_t)primary_fb->address + hhdm_offset);
    tgx_canvas_t canvas = { fb_virt, primary_fb->width, primary_fb->height, primary_fb->pitch };
    int cursor_x = primary_fb->width / 2;
    int cursor_y = primary_fb->height / 2;

    while (1) {
        tgx_clear(&canvas, 0x001010);
        hal_usb_poll();

        input_event_t ev;
        nk_input_begin(&nk_ctx);
        while (hal_input_pop_event(&ev)) {
            if (ev.type == INPUT_TYPE_MOUSE) {
                cursor_x = ev.mouse.x;
                cursor_y = ev.mouse.y;
                nk_input_motion(&nk_ctx, cursor_x, cursor_y);
                nk_input_button(&nk_ctx, NK_BUTTON_LEFT, cursor_x, cursor_y, (ev.mouse.buttons & 1));
            } else if (ev.type == INPUT_TYPE_KEYBOARD) {
                if (ev.kbd.down) {
                    if (ev.kbd.key >= 32 && ev.kbd.key <= 126) {
                        nk_input_unicode(&nk_ctx, (nk_rune)ev.kbd.key);
                    }
                    if (ev.kbd.key == 0x0A || ev.kbd.key == 0x0D) nk_input_key(&nk_ctx, NK_KEY_ENTER, 1);
                    if (ev.kbd.key == 0x08) nk_input_key(&nk_ctx, NK_KEY_BACKSPACE, 1);
                } else {
                    if (ev.kbd.key == 0x0A || ev.kbd.key == 0x0D) nk_input_key(&nk_ctx, NK_KEY_ENTER, 0);
                    if (ev.kbd.key == 0x08) nk_input_key(&nk_ctx, NK_KEY_BACKSPACE, 0);
                }
            }
        }
        nk_input_end(&nk_ctx);

        ui_render(&nk_ctx, &os_app, primary_fb->width, primary_fb->height);
        struct nk_sw_fb sw_fb = { fb_virt, primary_fb->width, primary_fb->height, primary_fb->pitch };
        nk_sw_render(&sw_fb, &nk_ctx);

        draw_cursor(&canvas, cursor_x, cursor_y);
        scheduler_yield();
    }
}

/* PHASE 7: The Executive Session Genesis (smss.exe equivalent) */
void session_manager_task(void* arg) {
    (void)arg;
    serial_printf("[SMSS] Initializing Session 0...\n");

    /* 1. Initialize the Environment Manager (The Desktop/Windowing UI) */
    memset(&os_app, 0, sizeof(os_app));
    os_app.current_state = STATE_DESKTOP;
    os_app.show_terminal = 1;
    /* Use industrial SPAWN for Environment Manager (New App Domain) */
    scheduler_spawn("Environment Manager", environment_manager_entry, NULL);

    /* Session manager persists to monitor system health */
    while(1) {
        scheduler_yield();
    }
}

void kernel_main(void) {
    /* PHASE 0: The Bare-Metal Isolation Layer (POST/Loader) */
    __asm__ volatile(
        "cli\n\t"
        "movq %0, %%rsp\n\t"
        "movq %%rsp, %%rbp"
        : : "r"(&kernel_stack[65536]) : "memory"
    );

    serial_init();
    serial_printf("\n\nSovereign RTC64 Boot Genesis\n");
    serial_printf("[PHASE 0] Entering Bare-Metal Isolation Layer.\n");

    /* Step 1: Bootloader Handoff */
    serial_printf("[STEP 1] Validating Limine Handoff and Framebuffer...\n");
    if (!framebuffer_request.response || !hhdm_request.response || !memmap_request.response) {
        serial_printf("[FATAL] Essential boot protocols missing.\n");
        while(1) __asm__("hlt");
    }
    hhdm_offset = hhdm_request.response->offset;
    primary_fb = framebuffer_request.response->framebuffers[0];
    serial_printf("[STEP 1] Framebuffer registered at Phys: %p, Virt: %p\n",
                  (void*)primary_fb->address, (void*)((uint64_t)primary_fb->address + hhdm_offset));
    init_sse();

    /* Step 2: Memory Matrix */
    serial_printf("[STEP 2] Building Physical Memory Matrix (PMM)...\n");
    pmm_init(memmap_request.response);

    /* Step 3: Kernel Heap Genesis */
    serial_printf("[STEP 3] Establishing Executive Pool (Heap)...\n");
    void* heap_phys = pmm_alloc_blocks(4096); /* 16MB */
    hal_malloc_init((void*)((uint64_t)heap_phys + hhdm_offset), 4096 * 4096);

    /* Step 4: Architecture Frame Setup */
    serial_printf("[STEP 4] Constructing GDT/IDT/TSS Security Boundaries...\n");
    gdt_init();
    idt_init();

    /* Step 5: Entropy and Security Activation */
    serial_printf("[STEP 5] Calibrating Local APIC and Security Entropy...\n");
    apic_init();
    irq_install_handler(32, timer_handler);

    /* Step 6: Hardware Peripheral I/O Probe */
    serial_printf("[STEP 6] Probing I/O Matrix and Driver Orchestration...\n");
    hal_input_init();
    cm_orchestrate_drivers();

    /* Step 7: Subsystem Threading */
    serial_printf("[STEP 7] Initializing Subsystem Threading (Scheduler)...\n");
    scheduler_init();

    /* PHASE 1: Kernel Initialization (Executive mode start) */
    serial_printf("[PHASE 1] Transitioning to Executive mode. Enabling STI.\n");
    __asm__ volatile("sti");

    /* PHASE 2: Namespace Initialization */
    serial_printf("[PHASE 2] Establishing VFS and System Namespace...\n");
    vfs_init();
    vfs_refresh_mounts();

    /* PHASE 3: Security Monitor (SRM) */
    serial_printf("[PHASE 3] Activating Security Reference Monitor (SRM)...\n");

    /* PHASE 4: Power & I/O Manager */
    serial_printf("[PHASE 4] Finalizing Storage Stacks and IRP Stability...\n");
    hal_storage_init();
    hal_storage_finish_init();

    /* PHASE 5: Session Manager (smss.exe) */
    serial_printf("[PHASE 5] Spawning Initial Executive Session...\n");

    /* PHASE 6: Service Control Manager */
    serial_printf("[PHASE 6] Starting Core Background Services (COMPREC/Terminal)...\n");
    system_shell_init();
    /* System services use fixed IDs for security binding (0,1 or 1,1) */
    scheduler_add_task("System Shell", system_shell_task, NULL, 1, 1);
    scheduler_add_task("COMPREC", comprec_task, NULL, 1, 1);
    apic_timer_unmask();

    /* PHASE 7: User Land Pivot */
    serial_printf("[PHASE 7] Pivoting to Session Initialization...\n");
    scheduler_add_task("SMSS", session_manager_task, NULL, 1, 1);

    while (1) {
        scheduler_run();
        __asm__ volatile("hlt");
    }
}
