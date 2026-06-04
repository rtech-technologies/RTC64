#ifndef SERIAL_H
#define SERIAL_H

#include <stdint.h>

void serial_init(void);
void serial_putc(char c);
void serial_write(const char* str);
void serial_printf(const char* fmt, ...);
int serial_received(void);
char serial_read(void);

#endif
