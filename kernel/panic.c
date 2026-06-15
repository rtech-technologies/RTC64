/* Modified by Sovereign: Monolithic REAL Panic Engine with Assembly Gateways and Graphical BSOD */
#include <stdint.h>
#include <stddef.h>
#include "pro_os.h"
#include "serial.h"

/* =========================================================================
 * 1. RAW STRUCTS & DATA
 * ========================================================================= */

struct cpu_state_frame {
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t error_code;
    uint64_t rip, cs, rflags, rsp, ss;
};

struct panic_framebuffer {
    uint64_t address;
    uint64_t width;
    uint64_t height;
    uint64_t pitch;
};

extern struct panic_framebuffer* get_kernel_framebuffer(void);

#define BSOD_COLOR_BG       0x002084
#define BSOD_COLOR_TEXT     0xFFFFFF

static uint32_t c_x = 50;
static uint32_t c_y = 50;

/* =========================================================================
 * 2. HARDCODED ASCII 8x16 BITMAP FONT
 * ========================================================================= */
static const uint8_t bsod_font[128][16] = {
    [' '] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    [':'] = {0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00,0x00},
    ['-'] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7e,0x7e,0x00,0x00,0x00,0x00,0x00,0x00,0x00},
    ['!'] = {0x00,18,18,18,18,18,18,18,0,0,18,18,0,0,0,0},
    ['_'] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0xff},
    ['('] = {0x00,12,24,48,48,48,48,48,48,48,48,48,48,24,12,0},
    [')'] = {0x00,48,24,12,12,12,12,12,12,12,12,12,12,24,48,0},
    [','] = {0x00,0,0,0,0,0,0,0,0,24,24,24,12,6,0,0},
    ['.'] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,0x18,0x00,0x00,0x00,0x00},
    ['0'] = {0x00,60,102,102,110,118,102,102,102,102,102,102,102,102,60,0},
    ['1'] = {0x00,24,28,24,24,24,24,24,24,24,24,24,24,24,126,0},
    ['2'] = {0x00,60,102,102,6,6,12,24,48,96,96,96,102,102,127,0},
    ['3'] = {0x00,60,102,102,6,6,28,6,6,6,6,6,102,102,60,0},
    ['4'] = {0x00,6,14,30,54,54,102,102,102,127,127,6,6,6,15,0},
    ['5'] = {0x00,127,96,96,96,96,124,102,6,6,6,6,102,102,60,0},
    ['6'] = {0x00,60,102,96,96,96,124,102,102,102,102,102,102,102,60,0},
    ['7'] = {0x00,127,102,6,6,12,12,24,24,24,48,48,48,48,48,0},
    ['8'] = {0x00,60,102,102,102,102,60,102,102,102,102,102,102,102,60,0},
    ['9'] = {0x00,60,102,102,102,102,102,62,6,6,6,6,6,102,60,0},
    ['A'] = {0x00,24,60,102,102,102,102,126,102,102,102,102,102,102,102,0},
    ['B'] = {0x00,124,102,102,102,102,124,102,102,102,102,102,102,102,124,0},
    ['C'] = {0x00,62,102,96,96,96,96,96,96,96,96,96,96,102,62,0},
    ['D'] = {0x00,120,108,102,102,102,102,102,102,102,102,102,102,108,120,0},
    ['E'] = {0x00,127,96,96,96,96,124,124,96,96,96,96,96,96,127,0},
    ['F'] = {0x00,127,96,96,96,96,124,124,96,96,96,96,96,96,96,0},
    ['G'] = {0x00,62,102,96,96,96,96,110,102,102,102,102,102,102,62,0},
    ['H'] = {0x00,102,102,102,102,102,102,126,102,102,102,102,102,102,102,0},
    ['I'] = {0x00,126,24,24,24,24,24,24,24,24,24,24,24,24,126,0},
    ['J'] = {0x00,15,6,6,6,6,6,6,6,6,6,102,102,102,60,0},
    ['K'] = {0x00,102,108,120,112,96,112,120,108,102,102,102,102,102,102,0},
    ['L'] = {0x00,96,96,96,96,96,96,96,96,96,96,96,96,96,127,0},
    ['M'] = {0x00,99,99,119,119,107,107,107,99,99,99,99,99,99,99,0},
    ['N'] = {0x00,98,102,110,118,118,102,102,102,102,102,102,102,102,102,0},
    ['O'] = {0x00,60,102,102,102,102,102,102,102,102,102,102,102,102,60,0},
    ['P'] = {0x00,124,102,102,102,102,124,96,96,96,96,96,96,96,96,0},
    ['Q'] = {0x00,60,102,102,102,102,102,102,102,102,106,108,102,102,60,2},
    ['R'] = {0x00,124,102,102,102,102,124,112,104,100,102,102,102,102,102,0},
    ['S'] = {0x00,62,102,96,96,48,28,14,7,3,3,3,99,102,60,0},
    ['T'] = {0x00,127,93,24,24,24,24,24,24,24,24,24,24,24,24,0},
    ['U'] = {0x00,102,102,102,102,102,102,102,102,102,102,102,102,102,60,0},
    ['V'] = {0x00,102,102,102,102,102,102,102,102,60,60,24,24,24,24,0},
    ['W'] = {0x00,99,99,99,99,99,99,107,107,107,119,119,99,99,99,0},
    ['X'] = {0x00,102,102,102,60,60,24,24,24,60,60,102,102,102,102,0},
    ['Y'] = {0x00,102,102,102,102,102,60,60,24,24,24,24,24,24,24,0},
    ['Z'] = {0x00,127,102,6,12,12,24,24,48,48,96,96,102,102,127,0},
    ['a'] = {0x00,0x00,0x00,0x00,0x00,60,70,6,62,102,102,102,102,126,61,0},
    ['b'] = {0x00,96,96,96,96,124,102,102,102,102,102,102,102,102,124,0},
    ['c'] = {0x00,0x00,0x00,0x00,0x00,60,102,96,96,96,96,96,96,102,60,0},
    ['d'] = {0x00,6,6,6,6,62,102,102,102,102,102,102,102,102,62,0},
    ['e'] = {0x00,0x00,0x00,0x00,0x00,60,102,102,127,96,96,96,102,102,60,0},
    ['f'] = {0x00,28,54,48,48,124,48,48,48,48,48,48,48,48,120,0},
    ['g'] = {0x00,0x00,0x00,0x00,0x00,61,102,102,102,102,62,6,6,102,60,0},
    ['h'] = {0x00,96,96,96,96,124,102,102,102,102,102,102,102,102,102,0},
    ['i'] = {0x00,24,24,0,0,24,24,24,24,24,24,24,24,24,126,0},
    ['j'] = {0x00,6,6,0,0,6,6,6,6,6,6,6,102,102,60,0},
    ['k'] = {0x00,96,96,96,96,102,108,120,112,108,102,102,102,102,102,0},
    ['l'] = {0x00,24,24,24,24,24,24,24,24,24,24,24,24,24,126,0},
    ['m'] = {0x00,0x00,0x00,0x00,0x00,110,123,107,107,107,99,99,99,99,99,0},
    ['n'] = {0x00,0x00,0x00,0x00,0x00,124,102,102,102,102,102,102,102,102,102,0},
    ['o'] = {0x00,0x00,0x00,0x00,0x00,60,102,102,102,102,102,102,102,102,60,0},
    ['p'] = {0x00,0x00,0x00,0x00,0x00,124,102,102,102,102,124,96,96,96,96,0},
    ['q'] = {0x00,0x00,0x00,0x00,0x00,62,102,102,102,102,62,6,6,6,6,0},
    ['r'] = {0x00,0x00,0x00,0x00,0x00,124,102,96,96,96,96,96,96,96,96,0},
    ['s'] = {0x00,0x00,0x00,0x00,0x00,62,102,96,60,7,3,3,99,102,60,0},
    ['t'] = {0x00,16,16,16,16,124,16,16,16,16,16,16,16,18,12,0},
    ['u'] = {0x00,0x00,0x00,0x00,0x00,102,102,102,102,102,102,102,102,110,59,0},
    ['v'] = {0x00,0x00,0x00,0x00,0x00,102,102,102,102,102,60,60,24,24,24,0},
    ['w'] = {0x00,0x00,0x00,0x00,0x00,99,99,99,107,107,107,119,54,34,34,0},
    ['x'] = {0x00,0x00,0x00,0x00,0x00,102,102,60,24,24,24,60,102,102,102,0},
    ['y'] = {0x00,0x00,0x00,0x00,0x00,102,102,102,102,102,62,6,6,102,60,0},
    ['z'] = {0x00,0x00,0x00,0x00,0x00,127,102,12,24,24,48,48,102,102,127,0}
};

/* =========================================================================
 * 3. BARE-METAL GRAPHICS TEXT RENDER ENGINE
 * ========================================================================= */
static int panic_nest_level = 0;

static void blit_char(char c, uint32_t x, uint32_t y, struct panic_framebuffer* fb) {
    if ((uint8_t)c >= 128) return;
    uint32_t* base = (uint32_t*)fb->address;
    for (int row = 0; row < 16; row++) {
        uint8_t bits = bsod_font[(uint8_t)c][row];
        for (int col = 0; col < 8; col++) {
            if (bits & (0x80 >> col)) {
                uint32_t tx = x + col;
                uint32_t ty = y + row;
                if (tx < fb->width && ty < fb->height) {
                    base[ty * (fb->pitch/4) + tx] = BSOD_COLOR_TEXT;
                }
            }
        }
    }
}

static void bsod_print(const char* str, struct panic_framebuffer* fb) {
    for (size_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            c_x = 50; c_y += 20;
            continue;
        }
        blit_char(str[i], c_x, c_y, fb);
        c_x += 9;
        if (c_x >= (fb->width - 50)) {
            c_x = 50; c_y += 20;
        }
    }
}

static void u64_to_hex(uint64_t val, char* out_buf) {
    const char table[] = "0123456789ABCDEF";
    out_buf[0] = '0'; out_buf[1] = 'x';
    for (int i = 15; i >= 0; i--) {
        out_buf[2 + i] = table[val & 0xF];
        val >>= 4;
    }
    out_buf[18] = '\0';
}

/* =========================================================================
 * 4. THE MASTER GRAPHICAL CRASH RENDERER
 * ========================================================================= */
void render_bsod_screen(const char* error_title, void* rsp_pointer) {
    panic_nest_level++;
    if (panic_nest_level > 1) {
        serial_printf("\n[DOUBLE PANIC] System halted to prevent triple fault loop.\n");
        while(1) { __asm__ volatile("cli; hlt"); }
    }

    struct cpu_state_frame* frame = (struct cpu_state_frame*)rsp_pointer;
    struct panic_framebuffer* fb = get_kernel_framebuffer();

    serial_printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    serial_printf("!!! KERNEL PANIC: %s\n", error_title);
    serial_printf("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");

    if (frame) {
        serial_printf("RIP: %p  ERR: %p\n", (void*)frame->rip, (void*)frame->error_code);
        serial_printf("RAX: %p  RBX: %p  RCX: %p\n", (void*)frame->rax, (void*)frame->rbx, (void*)frame->rcx);
        serial_printf("RDX: %p  RSI: %p  RDI: %p\n", (void*)frame->rdx, (void*)frame->rsi, (void*)frame->rdi);
        serial_printf("RBP: %p  RSP: %p  FLG: %p\n", (void*)frame->rbp, (void*)frame->rsp, (void*)frame->rflags);
    }

    if (!fb || !fb->address) {
        serial_printf("[PANIC] Framebuffer unavailable. System halted.\n");
        while(1) { __asm__ volatile("cli; hlt"); }
    }

    uint32_t* base = (uint32_t*)fb->address;
    for (uint64_t i = 0; i < (fb->pitch/4) * fb->height; i++) {
        base[i] = BSOD_COLOR_BG;
    }

    c_x = 60; c_y = 60;
    bsod_print("A problem has been detected and Sovereign OS has been shut down to prevent damage\n", fb);
    bsod_print("to your computer.\n\n", fb);
    bsod_print(error_title, fb); bsod_print("\n\n", fb);
    bsod_print("If this is the first time you've seen this Stop error screen,\n", fb);
    bsod_print("restart your computer. If this screen appears again, verify your\n", fb);
    bsod_print("driver alignment settings and pointer-to-integer conversion widths.\n\n", fb);
    bsod_print("--- TECHNICAL INFORMATION ---\n\n", fb);

    uint64_t cr2, cr3;
    char hex_str[20];
    __asm__ volatile("mov %%cr2, %0" : "=r"(cr2));
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));

    u64_to_hex(cr2, hex_str);
    bsod_print("CR2 (Faulting Addr): ", fb); bsod_print(hex_str, fb); bsod_print("\n", fb);
    u64_to_hex(cr3, hex_str);
    bsod_print("CR3 (Page Directory): ", fb); bsod_print(hex_str, fb); bsod_print("\n\n", fb);

    if (frame) {
        u64_to_hex(frame->rip, hex_str);
        bsod_print("RIP: ", fb); bsod_print(hex_str, fb);
        u64_to_hex(frame->rsp, hex_str);
        bsod_print("  RSP: ", fb); bsod_print(hex_str, fb); bsod_print("\n", fb);
    }

    bsod_print("\nESTATE SECURED. EXECUTION HALTED SAFELY.", fb);
    serial_printf("ESTATE SECURED. EXECUTION HALTED SAFELY.\n");

    while (1) {
        __asm__ volatile("cli; hlt");
    }
}

void kpanic(const char* message) {
    render_bsod_screen(message, NULL);
}

/* =========================================================================
 * 5. INLINE ASSEMBLY EXCEPTION GATEWAYS
 * ========================================================================= */

#define PUSH_REGS_ASM \
    "pushq %rax\n" \
    "pushq %rbx\n" \
    "pushq %rcx\n" \
    "pushq %rdx\n" \
    "pushq %rsi\n" \
    "pushq %rdi\n" \
    "pushq %rbp\n" \
    "pushq %r8\n" \
    "pushq %r9\n" \
    "pushq %r10\n" \
    "pushq %r11\n" \
    "pushq %r12\n" \
    "pushq %r13\n" \
    "pushq %r14\n" \
    "pushq %r15\n"

#define DEFINE_EXCEPTION_GATEWAY_WITH_ERR(name, title_string) \
    void name(void); \
    __asm__( \
        ".global " #name "\n" \
        #name ":\n" \
        "cli\n" \
        PUSH_REGS_ASM \
        "movq %rsp, %rsi\n" \
        "leaq str_" #name "(%rip), %rdi\n" \
        "call render_bsod_screen\n" \
        "1: cli\n hlt\n jmp 1b\n" \
        ".section .rodata\n" \
        "str_" #name ": .string \"" title_string "\"\n" \
        ".text\n" \
    );

#define DEFINE_EXCEPTION_GATEWAY_NO_ERR(name, title_string) \
    void name(void); \
    __asm__( \
        ".global " #name "\n" \
        #name ":\n" \
        "cli\n" \
        "pushq $0\n" \
        PUSH_REGS_ASM \
        "movq %rsp, %rsi\n" \
        "leaq str_" #name "(%rip), %rdi\n" \
        "call render_bsod_screen\n" \
        "1: cli\n hlt\n jmp 1b\n" \
        ".section .rodata\n" \
        "str_" #name ": .string \"" title_string "\"\n" \
        ".text\n" \
    );

DEFINE_EXCEPTION_GATEWAY_NO_ERR(  handler_divide_by_zero, "STATUS_INTEGER_DIVIDE_BY_ZERO (#DE)")
DEFINE_EXCEPTION_GATEWAY_WITH_ERR(handler_general_protection_fault, "SYSTEM_THREAD_EXCEPTION_NOT_HANDLED (#GP)")
DEFINE_EXCEPTION_GATEWAY_WITH_ERR(handler_page_fault, "PAGE_FAULT_IN_NONPAGED_AREA (#PF)")
DEFINE_EXCEPTION_GATEWAY_NO_ERR(  handler_double_fault, "CRITICAL_PROCESS_DIED (#DF)")
