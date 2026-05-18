#ifndef PRO_OS_H
#define PRO_OS_H

#include <stdint.h>
#include <stddef.h>
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

// Memory
void* malloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
void* arc_alloc(size_t size);

#endif
