#ifndef PRO_OS_H
#define PRO_OS_H

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <limine.h>

typedef enum {
    EVENT_INIT,
    EVENT_MAIN,
    EVENT_CLEANUP
} kernel_event_t;

typedef void (*service_func_t)(kernel_event_t event);

void register_service(service_func_t init_func);
void dispatch_event(kernel_event_t event);

void shell_main(void);

// Serial/VGA
void vga_serial_service(kernel_event_t event);
void vga_putc(char c);

// Memory
void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
void* arc_alloc(size_t size);

// Standard C
int printf(const char* fmt, ...);
int snprintf(char* buf, size_t n, const char* fmt, ...);
void* memset(void* s, int c, size_t n);
void* memcpy(void* dest, const void* src, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
size_t strlen(const char* s);

#endif
