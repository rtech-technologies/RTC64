/* Modified by Sovereign: License Compliance Update */
#include "serial.h"
#include <stdarg.h>
#include "pro_os.h"
#include "hal.h"

#define COM1 0x3F8

static volatile int serial_lock = 0;

static void spin_lock(volatile int* lock) {
    while (__sync_lock_test_and_set(lock, 1)) {
        __asm__("pause");
    }
}

static void spin_unlock(volatile int* lock) {
    __sync_lock_release(lock);
}

void serial_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

static int is_transmit_empty(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_putc(char c) {
    while (is_transmit_empty() == 0);
    outb(COM1, c);
}

void serial_write(const char* str) {
    spin_lock(&serial_lock);
    while (*str) serial_putc(*str++);
    spin_unlock(&serial_lock);
}

void serial_printf(const char* fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);

    spin_lock(&serial_lock);
    const char* p = buf;
    while (*p) serial_putc(*p++);
    spin_unlock(&serial_lock);

    va_end(args);
}

int serial_received(void) {
    return inb(COM1 + 5) & 1;
}

char serial_read(void) {
    while (serial_received() == 0);
    return inb(COM1);
}
