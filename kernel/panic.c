 #include <stdint.h>
#include <stddef.h>
#include "pro_os.h"

/* ========================================================================= */
/* 1. ARCHITECTURAL ASSEMBLY STUBS (NATIVE HOOKS)                            */
/* ========================================================================= */

// Global definitions so your Interrupt Descriptor Table (IDT) can link them
void page_fault_stub(void);
void gpf_stub(void);
void double_fault_stub(void);
void core_panic_handler(void *rsp_pointer);

__asm__(
    ".global page_fault_stub\n"
    ".global gpf_stub\n"
    ".global double_fault_stub\n"

    "page_fault_stub:\n"
    "    cli\n"
    "    pushq $14\n" // Vector 14 (Page Fault)
    "    jmp exception_common\n"

    "gpf_stub:\n"
    "    cli\n"
    "    pushq $13\n" // Vector 13 (GPF)
    "    jmp exception_common\n"

    "double_fault_stub:\n"
    "    cli\n"
    "    pushq $8\n"  // Vector 8 (Double Fault)
    "    jmp exception_common\n"

    "exception_common:\n"
    "    /* Push General Purpose Registers */\n"
    "    pushq %rax\n"
    "    pushq %rbx\n"
    "    pushq %rcx\n"
    "    pushq %rdx\n"
    "    pushq %rsi\n"
    "    pushq %rdi\n"
    "    pushq %rbp\n"
    "    pushq %r8\n"
    "    pushq %r9\n"
    "    pushq %r10\n"
    "    pushq %r11\n"
    "    pushq %r12\n"
    "    pushq %r13\n"
    "    pushq %r14\n"
    "    pushq %r15\n"

    "    /* Capture Control Registers */\n"
    "    movq %cr2, %rax\n"
    "    pushq %rax\n"
    "    movq %cr3, %rax\n"
    "    pushq %rax\n"
    "    movq %cr4, %rax\n"
    "    pushq %rax\n"

    "    /* Capture Segment Registers */\n"
    "    xorq %rax, %rax\n"
    "    movw %ds, %ax\n"
    "    pushq %rax\n"
    "    movw %es, %ax\n"
    "    pushq %rax\n"
    "    movw %fs, %ax\n"
    "    pushq %rax\n"
    "    movw %gs, %ax\n"
    "    pushq %rax\n"

    "    /* First parameter for C function (RDI) is current stack pointer */\n"
    "    movq %rsp, %rdi\n"
    "    call core_panic_handler\n"

    "    cli\n"
    "    hlt\n"
);

/* ========================================================================= */
/* 2. C DATA STRUCTURES                                                      */
/* ========================================================================= */

struct panic_framebuffer {
    uint64_t address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
};

// Extern hook to pull your Limine or kernel display structures
extern struct panic_framebuffer* get_kernel_framebuffer(void);

/* ========================================================================= */
/* 3. INDEPENDENT BITMAP FONT SYSTEM (8x8 Scaled)                            */
/* ========================================================================= */

static const uint8_t panic_font[128][8] = {
    [' '] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['!'] = {0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x00},
    [':'] = {0x00, 0x12, 0x12, 0x00, 0x00, 0x12, 0x12, 0x00},
    ['.'] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18},
    [','] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x18, 0x18, 0x30},
    ['('] = {0x0C, 0x18, 0x30, 0x30, 0x30, 0x18, 0x0C, 0x00},
    [')'] = {0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x18, 0x30, 0x00},
    ['?'] = {0x3E, 0x46, 0x06, 0x1C, 0x18, 0x00, 0x18, 0x00},
    ['+'] = {0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x00, 0x00},
    ['/'] = {0x02, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x40, 0x00},
    ['<'] = {0x0E, 0x1C, 0x38, 0x70, 0x38, 0x1C, 0x0E, 0x00},
    ['>'] = {0x70, 0x38, 0x1C, 0x0E, 0x1C, 0x38, 0x70, 0x00},
    ['*'] = {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00},
    ['='] = {0x00, 0x00, 0x7E, 0x00, 0x7E, 0x00, 0x00, 0x00},
    ['#'] = {0x24, 0x24, 0x7E, 0x24, 0x7E, 0x24, 0x24, 0x00},
    ['$'] = {0x18, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x18, 0x00},
    ['%'] = {0x46, 0x66, 0x30, 0x18, 0x0C, 0x66, 0x62, 0x00},
    ['&'] = {0x3C, 0x66, 0x3C, 0x38, 0x67, 0x66, 0x3F, 0x00},
    ['@'] = {0x3C, 0x66, 0x6E, 0x6E, 0x60, 0x66, 0x3C, 0x00},
    ['['] = {0x3C, 0x30, 0x30, 0x30, 0x30, 0x30, 0x3C, 0x00},
    [']'] = {0x3C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3C, 0x00},
    ['^'] = {0x18, 0x3C, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['_'] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF},
    ['`'] = {0x30, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['{'] = {0x0E, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0E, 0x00},
    ['}'] = {0x70, 0x18, 0x18, 0x0C, 0x18, 0x18, 0x70, 0x00},
    ['~'] = {0x00, 0x00, 0x3B, 0x6E, 0x00, 0x00, 0x00, 0x00},
    ['\\'] = {0x40, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x02, 0x00},
    ['\''] = {0x18, 0x18, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00},
    ['\"'] = {0x24, 0x24, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00},
    [';'] = {0x00, 0x12, 0x12, 0x00, 0x12, 0x12, 0x24, 0x00},
    ['-'] = {0x00, 0x00, 0x00, 0x7E, 0x00, 0x00, 0x00, 0x00},
    ['|'] = {0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12},
    ['0'] = {0x3C, 0x66, 0x6E, 0x7E, 0x76, 0x66, 0x3C, 0x00},
    ['1'] = {0x18, 0x38, 0x18, 0x18, 0x18, 0x18, 0x7E, 0x00},
    ['2'] = {0x3E, 0x66, 0x06, 0x1E, 0x30, 0x62, 0x7E, 0x00},
    ['3'] = {0x3E, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3E, 0x00},
    ['4'] = {0x06, 0x0E, 0x1E, 0x36, 0x7E, 0x06, 0x06, 0x00},
    ['5'] = {0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3E, 0x00},
    ['6'] = {0x1C, 0x30, 0x60, 0x7C, 0x66, 0x66, 0x3E, 0x00},
    ['7'] = {0x7E, 0x46, 0x0C, 0x18, 0x30, 0x30, 0x30, 0x00},
    ['8'] = {0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C, 0x00},
    ['9'] = {0x3E, 0x66, 0x66, 0x3E, 0x06, 0x0C, 0x38, 0x00},
    ['A'] = {0x18, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x00},
    ['B'] = {0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C, 0x00},
    ['C'] = {0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C, 0x00},
    ['D'] = {0x78, 0x6C, 0x66, 0x66, 0x66, 0x6C, 0x78, 0x00},
    ['E'] = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E, 0x00},
    ['F'] = {0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60, 0x00},
    ['G'] = {0x3E, 0x66, 0x60, 0x6E, 0x66, 0x66, 0x3A, 0x00},
    ['H'] = {0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66, 0x00},
    ['I'] = {0x3E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x3E, 0x00},
    ['J'] = {0x1F, 0x06, 0x06, 0x06, 0x06, 0x66, 0x3C, 0x00},
    ['K'] = {0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66, 0x00},
    ['L'] = {0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E, 0x00},
    ['M'] = {0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63, 0x00},
    ['N'] = {0x66, 0x76, 0x7E, 0x7E, 0x6E, 0x66, 0x66, 0x00},
    ['O'] = {0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['P'] = {0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60, 0x60, 0x00},
    ['Q'] = {0x3C, 0x66, 0x66, 0x66, 0x6E, 0x3C, 0x0E, 0x00},
    ['R'] = {0x7C, 0x66, 0x66, 0x7C, 0x6C, 0x66, 0x66, 0x00},
    ['S'] = {0x3E, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3E, 0x00},
    ['T'] = {0x7E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    ['U'] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['V'] = {0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
    ['W'] = {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00},
    ['X'] = {0x66, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x66, 0x00},
    ['Y'] = {0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18, 0x00},
    ['Z'] = {0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E, 0x00},
    ['a'] = {0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3B, 0x00},
    ['b'] = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x00},
    ['c'] = {0x00, 0x00, 0x3C, 0x66, 0x60, 0x66, 0x3C, 0x00},
    ['d'] = {0x06, 0x06, 0x3E, 0x66, 0x66, 0x66, 0x3E, 0x00},
    ['e'] = {0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C, 0x00},
    ['f'] = {0x1C, 0x30, 0x7C, 0x30, 0x30, 0x30, 0x30, 0x00},
    ['g'] = {0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x3C},
    ['h'] = {0x60, 0x60, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
    ['i'] = {0x18, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x00},
    ['j'] = {0x0C, 0x00, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x38},
    ['k'] = {0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66, 0x00},
    ['l'] = {0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1C, 0x00},
    ['m'] = {0x00, 0x00, 0x6C, 0x92, 0x92, 0x92, 0x92, 0x00},
    ['n'] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x00},
    ['o'] = {0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C, 0x00},
    ['p'] = {0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60},
    ['q'] = {0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x06},
    ['r'] = {0x00, 0x00, 0x5C, 0x62, 0x60, 0x60, 0x60, 0x00},
    ['s'] = {0x00, 0x00, 0x3E, 0x60, 0x3C, 0x06, 0x7C, 0x00},
    ['t'] = {0x10, 0x10, 0x7C, 0x10, 0x10, 0x10, 0x0E, 0x00},
    ['u'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E, 0x00},
    ['v'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x00},
    ['w'] = {0x00, 0x00, 0x63, 0x6B, 0x6B, 0x7F, 0x36, 0x00},
    ['x'] = {0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66, 0x00},
    ['y'] = {0x00, 0x00, 0x66, 0x66, 0x66, 0x3E, 0x06, 0x3C},
    ['z'] = {0x00, 0x00, 0x7E, 0x0C, 0x18, 0x30, 0x7E, 0x00}
};

static void raw_pixel(struct panic_framebuffer* fb, uint32_t x, uint32_t y, uint32_t color) {
    if (x >= fb->width || y >= fb->height) return;
    uint32_t* dest = (uint32_t*)(fb->address + (y * fb->pitch) + (x * 4));
    *dest = color;
}

static void raw_print(struct panic_framebuffer* fb, int x, int y, const char* str, uint32_t color) {
    while (*str) {
        uint8_t idx = (uint8_t)*str;
        if (idx > 127) idx = '?';
        for (int r = 0; r < 8; r++) {
            uint8_t bits = panic_font[idx][r];
            for (int c = 0; c < 8; c++) {
                if (bits & (0x80 >> c)) {
                    // Double density 2x2 raster text scaling for high-DPI screens
                    raw_pixel(fb, x + (c * 2),     y + (r * 2),     color);
                    raw_pixel(fb, x + (c * 2) + 1, y + (r * 2),     color);
                    raw_pixel(fb, x + (c * 2),     y + (r * 2) + 1, color);
                    raw_pixel(fb, x + (c * 2) + 1, y + (r * 2) + 1, color);
                }
            }
        }
        x += 16;
        str++;
    }
}

/* ========================================================================= */
/* 4. FREESTANDING VALUE STRIPPERS (NO PRINTF DEPENDENCY)                    */
/* ========================================================================= */

static void raw_hex(uint64_t val, char* out) {
    const char* map = "0123456789ABCDEF";
    out[0] = '0'; out[1] = 'x';
    for (int i = 15; i >= 0; i--) {
        out[2 + i] = map[val & 0xF];
        val >>= 4;
    }
    out[18] = '\0';
}

static void raw_dec(uint64_t val, char* out) {
    char scratch[24];
    int idx = 0;
    if (val == 0) scratch[idx++] = '0';
    while (val > 0) {
        scratch[idx++] = '0' + (val % 10);
        val /= 10;
    }
    int pos = 0;
    while (idx > 0) out[pos++] = scratch[--idx];
    if (pos == 0) out[pos++] = '0';
    out[pos] = '\0';
}

/* ========================================================================= */
/* 5. MASTER CANVAS & RENDERING EXECUTION                                    */
/* ========================================================================= */

void display_panic_screen(const char* message, struct cpu_state* state) {
    struct panic_framebuffer* fb = get_kernel_framebuffer();
    if (!fb || !fb->address) {
        while(1) { __asm__ volatile("cli; hlt"); }
    }

    // Flood display with Orange OS Primary Orange (#FF4500)
    for (uint64_t y = 0; y < fb->height; y++) {
        uint32_t* line = (uint32_t*)(fb->address + y * fb->pitch);
        for (uint64_t x = 0; x < fb->width; x++) {
            line[x] = 0xFF4500;
        }
    }

    int y = 40;
    uint32_t white = 0xFFFFFF;
    char buffer[64];

    // Your Explicit Format Requirements
    raw_print(fb, 40, y, "OOPS!!, something went wrong!", white); y += 32;
    raw_print(fb, 40, y, "here is what went wrong:", white); y += 48;

    // Fault Identification Block
    raw_print(fb, 60, y, "FAULT INFRASTRUCTURE ERROR TYPE:", white); y += 24;
    raw_print(fb, 80, y, message, white); y += 40;

    if (state == NULL) {
        raw_print(fb, 60, y, "No CPU architecture registers dumped (Software kpanic).", white);
        while(1) { __asm__ volatile("cli; hlt"); }
    }

    // Comprehensive Architectural State Logging
    raw_print(fb, 60, y, "HARDWARE RUNTIME EXCEPTION TRACE METADATA:", white); y += 24;

    raw_print(fb, 80, y, "VECTOR IDENTIFIER: ", white); raw_dec(state->interrupt_number, buffer); raw_print(fb, 280, y, buffer, white); y += 20;
    raw_print(fb, 80, y, "HARDWARE ERR CODE: ", white); raw_hex(state->error_code, buffer);       raw_print(fb, 280, y, buffer, white); y += 20;
    raw_print(fb, 80, y, "EXCEPTION PC (RIP):", white); raw_hex(state->rip, buffer);              raw_print(fb, 280, y, buffer, white); y += 20;
    raw_print(fb, 80, y, "FAULT TARGET (CR2):", white); raw_hex(state->cr2, buffer);              raw_print(fb, 280, y, buffer, white); y += 40;

    // Full Matrix General Register Block
    raw_print(fb, 60, y, "GENERAL PURPOSE REGISTER COMPLETE DUMP MATRIX:", white); y += 24;

    raw_print(fb, 80, y, "RAX: ", white); raw_hex(state->rax, buffer); raw_print(fb, 140, y, buffer, white);
    raw_print(fb, 360, y, "RBX: ", white); raw_hex(state->rbx, buffer); raw_print(fb, 420, y, buffer, white);
    raw_print(fb, 640, y, "RCX: ", white); raw_hex(state->rcx, buffer); raw_print(fb, 700, y, buffer, white); y += 20;

    raw_print(fb, 80, y, "RDX: ", white); raw_hex(state->rdx, buffer); raw_print(fb, 140, y, buffer, white);
    raw_print(fb, 360, y, "RSI: ", white); raw_hex(state->rsi, buffer); raw_print(fb, 420, y, buffer, white);
    raw_print(fb, 640, y, "RDI: ", white); raw_hex(state->rdi, buffer); raw_print(fb, 700, y, buffer, white); y += 20;

    raw_print(fb, 80, y, "RSP: ", white); raw_hex(state->rsp, buffer); raw_print(fb, 140, y, buffer, white);
    raw_print(fb, 360, y, "RBP: ", white); raw_hex(state->rbp, buffer); raw_print(fb, 420, y, buffer, white);
    raw_print(fb, 640, y, "FLG: ", white); raw_hex(state->rflags, buffer); raw_print(fb, 700, y, buffer, white); y += 40;

    // Extended Control & System Segmentation Bounds
    raw_print(fb, 60, y, "PAGE DIRECTORY RECORD & KERNEL SEGMENT SELECTORS:", white); y += 24;

    raw_print(fb, 80, y, "CR3 (Paging Base Root System Table): ", white); raw_hex(state->cr3, buffer); raw_print(fb, 460, y, buffer, white); y += 20;
    raw_print(fb, 80, y, "CR4 (Processor Architectural Bounds): ", white); raw_hex(state->cr4, buffer); raw_print(fb, 460, y, buffer, white); y += 20;

    raw_print(fb, 80, y, "SEGMENT SELECTOR METRIC STATE BOUNDS: ", white);
    char s_buf[32];
    raw_hex(state->cs, buffer); s_buf[0] = buffer[14]; s_buf[1] = buffer[15]; s_buf[2] = ' '; s_buf[3] = '|'; s_buf[4] = ' ';
    raw_hex(state->ds, buffer); s_buf[5] = buffer[14]; s_buf[6] = buffer[15]; s_buf[7] = ' '; s_buf[8] = '|'; s_buf[9] = ' ';
    raw_hex(state->ss, buffer); s_buf[10] = buffer[14]; s_buf[11] = buffer[15]; s_buf[12] = '\0';
    raw_print(fb, 460, y, s_buf, white); y += 60;

    raw_print(fb, 40, y, "ESTATE SECURED. EXECUTION HALTED SAFELY.", white);
}

// Master authoritative entry-point routed from your assembly stubs
void core_panic_handler(void *rsp_pointer) {
    struct cpu_state* state = (struct cpu_state*)rsp_pointer;
    const char* msg = "GENERIC KERNEL UNHANDLED EXECUTION VIOLATION";

    if (state->interrupt_number == 8)  msg = "DOUBLE FAULT - THE KERNEL CRASHED WHILE TRYING TO HANDLE A CRASH";
    if (state->interrupt_number == 13) msg = "GENERAL PROTECTION FAULT - PRIVILEGE MEMORY LAYER VIOLATION";
    if (state->interrupt_number == 14) msg = "PAGE FAULT - ATTEMPTED TO ACCES UNMAPPED DATA OR NULL POINTER";

    display_panic_screen(msg, state);
    while (1) { __asm__ volatile ("cli; hlt"); }
}

// Software fallback handler for your drivers or filesystem bounds
void kpanic(const char* message) {
    display_panic_screen(message, NULL);
    while (1) { __asm__ volatile ("cli; hlt"); }
}
