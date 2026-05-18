#include <rsl.h>

extern void vga_putc(char c);
struct nk_context;
extern struct nk_context ctx;

extern void nk_input_begin_wrap(struct nk_context *ctx);
extern void nk_input_char_wrap(struct nk_context *ctx, char c);
extern void nk_input_end_wrap(struct nk_context *ctx);

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

void print(const char* msg) {
    while(*msg) vga_putc(*msg++);
}

// Unified Input Polling: Multiplexing PS/2 and Serial COM1
char* input(const char* prompt) {
    print(prompt);
    static char buf[128];
    int i = 0;

    for(;;) {
        // 1. Serial poll (COM1)
        if ((inb(0x3F8 + 5) & 1)) {
            char c = inb(0x3F8);

            // Route to Nuklear
            nk_input_begin_wrap(&ctx);
            nk_input_char_wrap(&ctx, c);
            nk_input_end_wrap(&ctx);

            if (c == '\r' || c == '\n') {
                vga_putc('\n');
                buf[i] = '\0';
                return buf;
            } else if (c == 0x7F || c == 0x08) {
                if (i > 0) {
                    i--;
                    vga_putc('\b'); vga_putc(' '); vga_putc('\b');
                }
            } else if (i < 127) {
                buf[i++] = c;
                vga_putc(c);
            }
        }

        // 2. PS/2 Keyboard poll
        if ((inb(0x64) & 1)) {
            (void)inb(0x60);
        }
    }
}
