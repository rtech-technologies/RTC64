#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "drivers/font_8x8.h"
#include "pro_os.h"
#include "limine.h"
#include "app_ui.h"
#include "nk_software_renderer.h"
#include "serial.h"
#include "pmm.h"
#include "idt.h"

// --- Sovereign 4-Stage Boot Architecture ---
typedef enum { STAGE_1_PRIMING=1, STAGE_2_MULTITASKING=2, STAGE_3_USB=3, STAGE_4_USER=4 } boot_stage_t;
static boot_stage_t current_stage = STAGE_1_PRIMING;

volatile struct limine_framebuffer_request framebuffer_request = { .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0 };

struct panic_framebuffer { uint64_t address; uint64_t width; uint64_t height; uint64_t pitch; };
static struct panic_framebuffer pfb;
struct panic_framebuffer* get_kernel_framebuffer(void) {
    if (framebuffer_request.response && framebuffer_request.response->framebuffer_count > 0) {
        struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
        pfb.address = (uintptr_t)fb->address;
        pfb.width = fb->width;
        pfb.height = fb->height;
        pfb.pitch = fb->pitch;
        return &pfb;
    }
    return NULL;
}

static volatile struct limine_hhdm_request hhdm_request = { .id = LIMINE_HHDM_REQUEST, .revision = 0 };
static volatile struct limine_memmap_request memmap_request = { .id = LIMINE_MEMMAP_REQUEST, .revision = 0 };
static volatile struct limine_rsdp_request rsdp_request = { .id = LIMINE_RSDP_REQUEST, .revision = 0 };
uint64_t hhdm_offset = 0;

static float font_get_width(nk_handle handle, float height, const char *text, int len) { (void)handle; (void)height; (void)text; return (float)len * 8.0f; }
static void* nk_malloc(nk_handle handle, void* old, nk_size size) { (void)handle; return old ? realloc(old, size) : malloc(size); }
static void nk_mfree(nk_handle handle, void* ptr) { (void)handle; free(ptr); }

void kernel_main(void) {
    // STAGE 1: SYSTEM PRIMING (Interrupts DISABLED)
    serial_init();
    serial_write("[STAGE 1] System Priming (Step 1-7)...\n");

    if (hhdm_request.response) hhdm_offset = hhdm_request.response->offset;
    if (memmap_request.response) pmm_init(memmap_request.response);
    if (rsdp_request.response) acpi_init(rsdp_request.response->address);

    extern void init_gdt(void); init_gdt();
    idt_init();

    if (!framebuffer_request.response || framebuffer_request.response->framebuffer_count < 1) {
        serial_write("[ERROR] No Graphical Framebuffer\n"); while(1) __asm__("hlt");
    }

    static uint8_t kernel_heap[16 * 1024 * 1024];
    hal_malloc_init(kernel_heap, sizeof(kernel_heap));
    hal_storage_init();
    extern void rsl_init(void); rsl_init();
    hal_input_init();

    // STAGE 2: MULTITASKING & BUS RUNTIMES (Interrupts ENABLED)
    current_stage = STAGE_2_MULTITASKING;
    serial_write("[STAGE 2] Runtime Start (Step 8-11)...\n");
    __asm__ volatile("sti");

    scheduler_init();
    vfs_init();
    extern void pci_scan(void); pci_scan();
    vfs_refresh_mounts();

    // STAGE 3: USB SUBSYSTEM (Step 12-15)
    current_stage = STAGE_3_USB;
    serial_write("[STAGE 3] USB Subsystem Activation...\n");
    scheduler_add_task("USB Poller", hal_usb_poll);
    hal_usb_init();

    // STAGE 4: USER SPACE & GUI (Step 16-18)
    current_stage = STAGE_4_USER;
    serial_write("[STAGE 4] Launching User Space UI...\n");

    struct nk_context ctx; struct nk_user_font font;
    font.userdata = nk_handle_ptr(0); font.height = 8.0f; font.width = font_get_width;
    struct nk_allocator alloc = { .alloc = nk_malloc, .free = nk_mfree };
    nk_init(&ctx, &alloc, &font);
    ui_init_style(&ctx);

    struct app_state app; memset(&app, 0, sizeof(app));
    app.current_state = STATE_LOGIN;
    chell_init(&app.chell); lab_init(&app.lab); installer_init(&app.installer);

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    tgx_canvas_t canvas = { (uint32_t*)fb->address, fb->width, fb->height, fb->pitch };
    int cx = fb->width / 2, cy = fb->height / 2;

    while (1) {
        tgx_clear(&canvas, 0x001010);
        nk_input_begin(&ctx);
        input_event_t ev;
        while (hal_input_pop_event(&ev)) {
            if (ev.type == INPUT_TYPE_MOUSE) {
                cx += ev.mouse.x; cy += ev.mouse.y;
                if (cx < 0) cx = 0;
                if (cy < 0) cy = 0;
                if (cx >= (int)fb->width) cx = fb->width-1;
                if (cy >= (int)fb->height) cy = fb->height-1;
                nk_input_motion(&ctx, cx, cy);
                nk_input_button(&ctx, NK_BUTTON_LEFT, cx, cy, (ev.mouse.buttons & 1));
            } else if (ev.type == INPUT_TYPE_KEYBOARD && ev.kbd.down) {
                char c = 0;
                if (ev.kbd.key >= 0x04 && ev.kbd.key <= 0x1D) c = 'a' + (ev.kbd.key - 0x04);
                else if (ev.kbd.key == 0x28) c = '\n';
                if (c) nk_input_char(&ctx, c);
            }
        }
        nk_input_end(&ctx);
        scheduler_run();
        ui_render(&ctx, &app, (int)fb->width, (int)fb->height);
        struct nk_sw_fb sw_fb = { fb->address, fb->width, fb->height, fb->pitch };
        nk_sw_render(&sw_fb, &ctx);

        // Upgrade cursor to Arrow (simulated with primitive)
        for(int r=0; r<8; r++) { uint8_t bits = cursor_arrow[r*2]; for(int b=0; b<8; b++) { if(bits & (0x80>>b)) tgx_blit_rect(&canvas, cx+b, cy+r, 1, 1, 0x00FFFF); } }

        __asm__("pause");
    }
}

// Minimal GDT Helper
struct gdt_ptr { uint16_t limit; uint64_t base; } __attribute__((packed));
static uint64_t gdt_raw[3] = { 0, 0x00AF9A000000FFFF, 0x00CF92000000FFFF };
void init_gdt(void) {
    static struct gdt_ptr gp; gp.limit = sizeof(gdt_raw)-1; gp.base = (uint64_t)&gdt_raw;
    __asm__ volatile("lgdt %0\n\tpush $0x08\n\tlea 1f(%%rip), %%rax\n\tpush %%rax\n\tlretq\n\t1:\n\tmov $0x10, %%ax\n\tmov %%ax, %%ds\n\tmov %%ax, %%es\n\tmov %%ax, %%fs\n\tmov %%ax, %%gs\n\tmov %%ax, %%ss" : : "m"(gp) : "rax", "memory");
}
