#include "pro_os.h"
#include <string.h>

/* Compliance Recording (comprec) module - Audit Security Log Manager & COM1 Serial Logger */

#define COM1_PORT 0x3F8

static inline uint8_t inb(uint16_t port) {
    uint8_t data;
    __asm__ volatile("inb %1, %0" : "=a"(data) : "Nd"(port));
    return data;
}

static inline void outb(uint16_t port, uint8_t data) {
    __asm__ volatile("outb %0, %1" : : "a"(data), "Nd"(port));
}

void serial_init(void) {
    outb(COM1_PORT + 1, 0x00);    // Disable all interrupts
    outb(COM1_PORT + 3, 0x80);    // Enable DLAB (set divisor)
    outb(COM1_PORT + 0, 0x01);    // Set divisor to 1 (115200 baud)
    outb(COM1_PORT + 1, 0x00);    //
    outb(COM1_PORT + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);    // Enable FIFO, clear, 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);    // RTS/DSR set
}

static int is_transmit_empty(void) {
    return inb(COM1_PORT + 5) & 0x20;
}

void serial_write_char(char c) {
    while (is_transmit_empty() == 0);
    outb(COM1_PORT, c);
}

void serial_write_string(const char *str) {
    while (*str) {
        if (*str == '\n') {
            serial_write_char('\r');
        }
        serial_write_char(*str++);
    }
    serial_write_char('\r');
    serial_write_char('\n');
}

void comprec_init(void) {
    /* Initialize physical serial logging COM1 port */
    serial_init();
    serial_write_string("[COMPREC] COM1 Serial logging initialized successfully.");

    /* Write fresh audit log header */
    vfs_write("/System/Config/audit.log", "[COMPREC] Compliance Recording Audit Log Started\n", 49);
}

void comprec_log(const char *event) {
    /* Log to standard serial port output */
    serial_write_string(event);

    /* Log to on-disk persistent file system log */
    char buf[3072] = {0};
    int len = vfs_read("/System/Config/audit.log", buf, sizeof(buf) - 256);
    if (len < 0) len = 0;
    buf[len] = '\0';

    char line[256];
    snprintf(line, sizeof(line), "[AUDIT] %s\n", event);
    strcat(buf, line);
    vfs_write("/System/Config/audit.log", buf, strlen(buf));
}
