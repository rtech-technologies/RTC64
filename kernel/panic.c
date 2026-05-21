#include <stdint.h>
#include <stddef.h>
#include "serial.h"

/* ========================================================================= */
/* 1. DATA STRUCTURES & EXTERNS                                              */
/* ========================================================================= */

struct panic_framebuffer {
    uint64_t address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
};

struct cpu_state {
    uint64_t gs, fs, es, ds;
    uint64_t cr4, cr3, cr2;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t interrupt_number;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

extern struct panic_framebuffer* get_kernel_framebuffer(void);

/* ========================================================================= */
/* 2. BITMAP FONT (8x8)                                                      */
/* ========================================================================= */

// Minimalist 8x8 bitmap font data for hex and basic labels
static const uint8_t panic_font[128][8] = {
    ['0']={0x3C,0x66,0x6E,0x7E,0x76,0x66,0x3C,0x00}, ['1']={0x18,0x38,0x18,0x18,0x18,0x18,0x7E,0x00},
    ['2']={0x3E,0x66,0x06,0x1E,0x30,0x62,0x7E,0x00}, ['3']={0x3E,0x66,0x06,0x1C,0x06,0x66,0x3E,0x00},
    ['4']={0x06,0x0E,0x1E,0x36,0x7E,0x06,0x06,0x00}, ['5']={0x7E,0x60,0x7C,0x06,0x06,0x66,0x3E,0x00},
    ['6']={0x1C,0x30,0x60,0x7C,0x66,0x66,0x3E,0x00}, ['7']={0x7E,0x46,0x0C,0x18,0x30,0x30,0x30,0x00},
    ['8']={0x3C,0x66,0x66,0x3C,0x66,0x66,0x3C,0x00}, ['9']={0x3E,0x66,0x66,0x3E,0x06,0x0C,0x38,0x00},
    ['A']={0x18,0x3C,0x66,0x66,0x7E,0x66,0x66,0x00}, ['B']={0x7C,0x66,0x66,0x7C,0x66,0x66,0x7C,0x00},
    ['C']={0x3C,0x66,0x60,0x60,0x60,0x66,0x3C,0x00}, ['D']={0x78,0x6C,0x66,0x66,0x66,0x6C,0x78,0x00},
    ['E']={0x7E,0x60,0x60,0x7C,0x60,0x60,0x7E,0x00}, ['F']={0x7E,0x60,0x60,0x7C,0x60,0x60,0x60,0x00},
    ['x']={0x00,0x00,0x66,0x3C,0x18,0x3C,0x66,0x00}, [':']={0x00,0x12,0x12,0x00,0x00,0x12,0x12,0x00},
    [' ']={0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, ['-']={0x00,0x00,0x00,0x7E,0x00,0x00,0x00,0x00},
    ['!']={0x18,0x18,0x18,0x18,0x18,0x00,0x18,0x00}, ['?']={0x3E,0x46,0x06,0x1C,0x18,0x00,0x18,0x00},
    ['R']={0x7C,0x66,0x66,0x7C,0x6C,0x66,0x66,0x00}, ['I']={0x3E,0x0C,0x0C,0x0C,0x0C,0x0C,0x3E,0x00},
    ['P']={0x7C,0x66,0x66,0x7C,0x60,0x60,0x60,0x00}, ['V']={0x66,0x66,0x66,0x66,0x66,0x3C,0x18,0x00},
    ['T']={0x7E,0x18,0x18,0x18,0x18,0x18,0x18,0x00}, ['O']={0x3C,0x66,0x66,0x66,0x66,0x66,0x3C,0x00},
    ['a']={0x00,0x00,0x3C,0x06,0x3E,0x66,0x3B,0x00}, ['c']={0x00,0x00,0x3C,0x66,0x60,0x66,0x3C,0x00},
    ['u']={0x00,0x00,0x66,0x66,0x66,0x66,0x3E,0x00}, ['l']={0x18,0x18,0x18,0x18,0x18,0x18,0x1C,0x00},
    ['t']={0x10,0x10,0x7C,0x10,0x10,0x10,0x0E,0x00}, ['n']={0x00,0x00,0x7C,0x66,0x66,0x66,0x66,0x00},
    ['e']={0x00,0x00,0x3C,0x66,0x7E,0x60,0x3C,0x00}, ['r']={0x00,0x00,0x5C,0x62,0x60,0x60,0x60,0x00}
};

/* ========================================================================= */
/* 3. STACK-ONLY RENDERER                                                     */
/* ========================================================================= */

static void panic_put_pixel(struct panic_framebuffer* fb, int x, int y, uint32_t color) {
    if (x < 0 || x >= (int)fb->width || y < 0 || y >= (int)fb->height) return;
    uint32_t* dest = (uint32_t*)(fb->address + (y * fb->pitch) + (x * 4));
    *dest = color;
}

static void panic_draw_char(struct panic_framebuffer* fb, char c, int x, int y, uint32_t color) {
    uint8_t idx = (uint8_t)c;
    if (idx > 127) idx = '?';
    for (int r = 0; r < 8; r++) {
        uint8_t bits = panic_font[idx][r];
        for (int b = 0; b < 8; b++) {
            if (bits & (0x80 >> b)) {
                // 2x2 scaling
                panic_put_pixel(fb, x + (b*2),   y + (r*2),   color);
                panic_put_pixel(fb, x + (b*2)+1, y + (r*2),   color);
                panic_put_pixel(fb, x + (b*2),   y + (r*2)+1, color);
                panic_put_pixel(fb, x + (b*2)+1, y + (r*2)+1, color);
            }
        }
    }
}

static int cursor_x = 40;
static int cursor_y = 40;

static void panic_printf(struct panic_framebuffer* fb, const char* fmt, uint64_t a1, uint64_t a2, uint64_t a3, uint64_t a4) {
    uint64_t reg_args[4] = {a1, a2, a3, a4};
    int arg_idx = 0;

    const char* p = fmt;
    while (*p) {
        if (*p == '%' && *(p+1)) {
            p++;
            uint64_t val = 0;
            if (arg_idx < 4) val = reg_args[arg_idx++];

            if (*p == 's') {
                const char* s = (const char*)val;
                while (*s) {
                    char c = *s++;
                    serial_putc(c);
                    if (cursor_x + 16 > (int)fb->width) { cursor_x = 40; cursor_y += 24; }
                    panic_draw_char(fb, c, cursor_x, cursor_y, 0xFFFFFF);
                    cursor_x += 16;
                }
            } else if (*p == 'x') {
                // Hex printing
                const char* hex = "0123456789ABCDEF";
                for (int i = 15; i >= 0; i--) {
                    char c = hex[(val >> (i * 4)) & 0xF];
                    serial_putc(c);
                    if (cursor_x + 16 > (int)fb->width) { cursor_x = 40; cursor_y += 24; }
                    panic_draw_char(fb, c, cursor_x, cursor_y, 0xFFFFFF);
                    cursor_x += 16;
                }
            }
        } else if (*p == '\n') {
            serial_putc('\n');
            cursor_x = 40;
            cursor_y += 24;
        } else {
            serial_putc(*p);
            if (cursor_x + 16 > (int)fb->width) { cursor_x = 40; cursor_y += 24; }
            panic_draw_char(fb, *p, cursor_x, cursor_y, 0xFFFFFF);
            cursor_x += 16;
        }
        p++;
    }
}

/* ========================================================================= */
/* 4. ASSEMBLY STUBS & HANDLER                                               */
/* ========================================================================= */

void isr0_stub(void); void isr8_stub(void); void isr13_stub(void); void isr14_stub(void);
// ... others defined in same way if needed ...

__asm__(
    ".macro isr_err_stub nr\n"
    ".global isr\\nr\\()_stub\n"
    "isr\\nr\\()_stub:\n"
    "    cli\n"
    "    pushq $\\nr\n"
    "    jmp exception_common\n"
    ".endm\n"

    ".macro isr_no_err_stub nr\n"
    ".global isr\\nr\\()_stub\n"
    "isr\\nr\\()_stub:\n"
    "    cli\n"
    "    pushq $0\n"
    "    pushq $\\nr\n"
    "    jmp exception_common\n"
    ".endm\n"

    "isr_no_err_stub 0\n"
    "isr_no_err_stub 1\n"
    "isr_no_err_stub 2\n"
    "isr_no_err_stub 3\n"
    "isr_no_err_stub 4\n"
    "isr_no_err_stub 5\n"
    "isr_no_err_stub 6\n"
    "isr_no_err_stub 7\n"
    "isr_err_stub    8\n"
    "isr_no_err_stub 9\n"
    "isr_err_stub    10\n"
    "isr_err_stub    11\n"
    "isr_err_stub    12\n"
    "isr_err_stub    13\n"
    "isr_err_stub    14\n"
    "isr_no_err_stub 15\n"
    "isr_no_err_stub 16\n"
    "isr_err_stub    17\n"
    "isr_no_err_stub 18\n"
    "isr_no_err_stub 19\n"
    "isr_no_err_stub 20\n"
    "isr_err_stub    21\n"
    "isr_no_err_stub 22\n"
    "isr_no_err_stub 23\n"
    "isr_no_err_stub 24\n"
    "isr_no_err_stub 25\n"
    "isr_no_err_stub 26\n"
    "isr_no_err_stub 27\n"
    "isr_no_err_stub 28\n"
    "isr_no_err_stub 29\n"
    "isr_err_stub    30\n"
    "isr_no_err_stub 31\n"

    "exception_common:\n"
    "    pushq %rax; pushq %rbx; pushq %rcx; pushq %rdx\n"
    "    pushq %rsi; pushq %rdi; pushq %rbp; pushq %r8\n"
    "    pushq %r9;  pushq %r10; pushq %r11; pushq %r12\n"
    "    pushq %r13; pushq %r14; pushq %r15\n"
    "    movq %cr2, %rax; pushq %rax\n"
    "    movq %cr3, %rax; pushq %rax\n"
    "    movq %cr4, %rax; pushq %rax\n"
    "    xorq %rax, %rax\n"
    "    movw %ds, %ax; pushq %rax\n"
    "    movw %es, %ax; pushq %rax\n"
    "    movw %fs, %ax; pushq %rax\n"
    "    movw %gs, %ax; pushq %rax\n"
    "    movq %rsp, %rdi\n"
    "    subq $8, %rsp\n"
    "    call core_panic_handler\n"
    "    hlt\n"
);

void core_panic_handler(void* rsp_pointer) {
    struct cpu_state* state = (struct cpu_state*)rsp_pointer;
    struct panic_framebuffer* fb = get_kernel_framebuffer();

    if (!fb || !fb->address) {
        while(1) __asm__("hlt");
    }

    // Fill screen with dark orange
    for (uint64_t i = 0; i < fb->height * fb->width; i++) {
        ((uint32_t*)fb->address)[i] = 0xAA4400;
    }

    cursor_x = 40; cursor_y = 40;
    panic_printf(fb, "SOVEREIGN OS KERNEL PANIC\n", 0, 0, 0, 0);
    panic_printf(fb, "--------------------------\n", 0, 0, 0, 0);
    panic_printf(fb, "VECTOR: %x\n", state->interrupt_number, 0, 0, 0);
    panic_printf(fb, "RIP:    %x\n", state->rip, 0, 0, 0);
    panic_printf(fb, "CS:     %x\n", state->cs, 0, 0, 0);
    panic_printf(fb, "ERR:    %x\n", state->error_code, 0, 0, 0);
    panic_printf(fb, "CR2:    %x\n", state->cr2, 0, 0, 0);
    panic_printf(fb, "CR3:    %x\n", state->cr3, 0, 0, 0);
    panic_printf(fb, "CR4:    %x\n", state->cr4, 0, 0, 0);
    panic_printf(fb, "\nREGISTERS:\n", 0, 0, 0, 0);
    panic_printf(fb, "RAX: %x RBX: %x\n", state->rax, state->rbx, 0, 0);
    panic_printf(fb, "RCX: %x RDX: %x\n", state->rcx, state->rdx, 0, 0);
    panic_printf(fb, "RSI: %x RDI: %x\n", state->rsi, state->rdi, 0, 0);
    panic_printf(fb, "RBP: %x RSP: %x\n", state->rbp, state->rsp, 0, 0);
    panic_printf(fb, "R8 : %x R9 : %x\n", state->r8,  state->r9,  0, 0);
    panic_printf(fb, "R10: %x R11: %x\n", state->r10, state->r11, 0, 0);
    panic_printf(fb, "R12: %x R13: %x\n", state->r12, state->r13, 0, 0);
    panic_printf(fb, "R14: %x R15: %x\n", state->r14, state->r15, 0, 0);

    while(1) __asm__("cli; hlt");
}

void kpanic(const char* msg) {
    struct panic_framebuffer* fb = get_kernel_framebuffer();
    if (!fb) while(1) __asm__("hlt");

    for (uint64_t i = 0; i < fb->height * fb->width; i++) {
        ((uint32_t*)fb->address)[i] = 0xAA4400;
    }

    cursor_x = 40; cursor_y = 40;
    panic_printf(fb, "SOFTWARE KERNEL PANIC: %s\n", (uintptr_t)msg, 0, 0, 0);
    while(1) __asm__("cli; hlt");
}
