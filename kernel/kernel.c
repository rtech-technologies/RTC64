#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "pro_os.h"
#include "app_ui.h"
#include "nk_software_renderer.h"
#include "drivers/pci.h"

__attribute__((used, section(".limine_requests_start")))
volatile uint64_t limine_requests_start_marker[4] = { 0xf6b8f4b39de7d1ae, 0xfab91a6940fcb9cf, 0x785c6ed015d3e316, 0x181e920a7852b9d9 };

__attribute__((used, section(".limine_requests")))
volatile uint64_t limine_base_revision[3] = { 0xf9562b2d5c95a6c8, 0x6a7b384944536bdc, 0 };

__attribute__((used, section(".limine_requests")))
static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_stack_size_request stack_size_request = {
    .id = LIMINE_STACK_SIZE_REQUEST,
    .revision = 0,
    .stack_size = 0x80000 // 512KB stack
};

__attribute__((used, section(".limine_requests")))
static volatile struct limine_kernel_address_request kernel_address_request = {
    .id = LIMINE_KERNEL_ADDRESS_REQUEST,
    .revision = 0
};

__attribute__((used, section(".limine_requests_end")))
volatile uint64_t limine_requests_end_marker[2] = { 0xadc0e0531bb10d03, 0x9572709f31764c62 };

uint64_t hhdm_offset = 0;
uint64_t kernel_phys_offset = 0;

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  flags;
    uint16_t base_mid;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__((packed));

struct idtr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct idt_entry idt[256];
static struct idtr idtr;

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_mid;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

static struct gdt_entry gdt[3];
static struct gdt_ptr gdtr;

static void init_gdt(void) {
    memset(&gdt[0], 0, sizeof(struct gdt_entry));
    gdt[1].limit_low = 0;
    gdt[1].base_low = 0;
    gdt[1].base_mid = 0;
    gdt[1].access = 0x9A;
    gdt[1].granularity = 0x20;
    gdt[1].base_high = 0;
    gdt[2].limit_low = 0;
    gdt[2].base_low = 0;
    gdt[2].base_mid = 0;
    gdt[2].access = 0x92;
    gdt[2].granularity = 0;
    gdt[2].base_high = 0;

    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (uintptr_t)&gdt;
    __asm__ volatile ("lgdt %0" : : "m"(gdtr));

    __asm__ volatile (
        "pushq $0x08\n"
        "pushq $1f\n"
        "lretq\n"
        "1:\n"
        "movw $0x10, %ax\n"
        "movw %ax, %ds\n"
        "movw %ax, %es\n"
        "movw %ax, %ss\n"
        "movw %ax, %fs\n"
        "movw %ax, %gs\n"
    );
}

extern void page_fault_stub(void);
extern void gpf_stub(void);
extern void double_fault_stub(void);

static void idt_set_gate(uint8_t vector, void* handler, uint8_t flags) {
    uintptr_t base = (uintptr_t)handler;
    idt[vector].base_low = base & 0xFFFF;
    idt[vector].selector = 0x08;
    idt[vector].ist = 0;
    idt[vector].flags = flags;
    idt[vector].base_mid = (base >> 16) & 0xFFFF;
    idt[vector].base_high = (base >> 32) & 0xFFFFFFFF;
    idt[vector].reserved = 0;
}

static void init_idt(void) {
    for (int i = 0; i < 256; i++) {
        memset(&idt[i], 0, sizeof(struct idt_entry));
    }
    idt_set_gate(8,  double_fault_stub, 0x8E);
    idt_set_gate(13, gpf_stub,          0x8E);
    idt_set_gate(14, page_fault_stub,   0x8E);
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uintptr_t)&idt;
    __asm__ volatile ("lidt %0" : : "m"(idtr));
}

static void init_sse(void) {
    uint64_t cr0, cr4;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~((uint64_t)1 << 2);
    cr0 |= (uint64_t)1 << 1;
    __asm__ volatile ("mov %0, %%cr0" : : "r"(cr0));
    __asm__ volatile ("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (uint64_t)3 << 9;
    __asm__ volatile ("mov %0, %%cr4" : : "r"(cr4));
}

static float font_get_width(nk_handle handle, float height, const char *text, int len) {
    (void)handle; (void)height; (void)text;
    return (float)len * 8.0f;
}

static void hcf(void) {
    __asm__ ("cli");
    for (;;) __asm__ ("hlt");
}

struct panic_framebuffer {
    uint64_t address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
};

struct panic_framebuffer* get_kernel_framebuffer(void) {
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        return NULL;
    }
    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    static struct panic_framebuffer pfb;
    pfb.address = (uint64_t)fb->address;
    pfb.width = fb->width;
    pfb.height = fb->height;
    pfb.pitch = fb->pitch;
    return &pfb;
}

static int cursor_x = 0;
static int cursor_y = 0;

void _start(void) {
    if (limine_base_revision[2] == (uint64_t)-1) {
        hcf();
    }

    init_sse();
    init_gdt();
    init_idt();

    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
        hcf();
    }

    if (hhdm_request.response != NULL) {
        hhdm_offset = hhdm_request.response->offset;
    }
    if (kernel_address_request.response != NULL) {
        kernel_phys_offset = kernel_address_request.response->physical_base;
    }

    struct limine_framebuffer *fb = framebuffer_request.response->framebuffers[0];
    tgx_canvas_t canvas = { (uint32_t*)fb->address, fb->width, fb->height, fb->pitch };

    /* Allocate 16MB for the kernel heap */
    static uint8_t kernel_heap[16 * 1024 * 1024];
    hal_malloc_init(kernel_heap, sizeof(kernel_heap));

    hal_storage_init();
    hal_input_init();
    scheduler_init();
    pci_scan();
    vfs_init();
    hal_usb_init();

    struct nk_context ctx;
    struct nk_user_font font;
    font.userdata = nk_handle_ptr(0);
    font.height = 8.0f;
    font.width = font_get_width;

    nk_init_default(&ctx, &font);
    ui_init_style(&ctx);

    struct app_state app;
    memset(&app, 0, sizeof(app));
    app.current_state = STATE_LOGIN;

    while (1) {
        tgx_clear(&canvas, 0x000000);
        hal_usb_poll();

        input_event_t ev;
        nk_input_begin(&ctx);
        while (hal_input_pop_event(&ev)) {
            if (ev.type == INPUT_TYPE_MOUSE) {
                cursor_x = ev.mouse.x;
                cursor_y = ev.mouse.y;
                nk_input_motion(&ctx, cursor_x, cursor_y);
                nk_input_button(&ctx, NK_BUTTON_LEFT, cursor_x, cursor_y, (ev.mouse.buttons & 1));
            }
        }
        nk_input_end(&ctx);

        if (cursor_x < 5 && cursor_y < 5 && cursor_x > 0) {
            kpanic("USER TRIGGERED PANIC TEST");
        }

        scheduler_run();
        ui_render(&ctx, &app, fb->width, fb->height);

        struct nk_sw_fb sw_fb = { fb->address, fb->width, fb->height, fb->pitch };
        nk_sw_render(&sw_fb, &ctx);
        tgx_blit_rect(&canvas, cursor_x, cursor_y, 8, 8, 0xFFFFFFFF);

        for (volatile int i = 0; i < 500000; i++);
    }
}
