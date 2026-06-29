/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "serial.h"
#include <stdarg.h>
#include "pro_os.h"
#include "hal.h"

#define COM1 0x3F8
static spinlock_t g_serial_lock = 0;

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
    while (*str) serial_putc(*str++);
}

void serial_printf(const char* fmt, ...) {
    spin_lock(&g_serial_lock);
    char buf[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    serial_write(buf);
    va_end(args);
    spin_unlock(&g_serial_lock);
}

int serial_received(void) {
    return inb(COM1 + 5) & 1;
}

char serial_read(void) {
    while (serial_received() == 0);
    return inb(COM1);
}
