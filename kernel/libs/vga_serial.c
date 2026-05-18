#include <pro_os.h>
#include <limine.h>

#define COM1 0x3F8

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

static void serial_init() {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

static void serial_putc(char c) {
    while ((inb(COM1 + 5) & 0x20) == 0);
    outb(COM1, c);
}

struct limine_framebuffer *fb = NULL;

void vga_serial_service(kernel_event_t event) {
    if (event == EVENT_INIT) {
        serial_init();
        extern struct limine_framebuffer_response *get_framebuffer(void);
        struct limine_framebuffer_response *fb_res = get_framebuffer();
        if (fb_res && fb_res->framebuffer_count > 0) {
            fb = fb_res->framebuffers[0];
        }
        const char *msg = "RTECH OSx2 Forensic Service Initialized.\n";
        while (*msg) serial_putc(*msg++);
    }
}

void vga_putc(char c) {
    serial_putc(c);
}
