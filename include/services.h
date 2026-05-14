#ifndef SERVICES_H
#define SERVICES_H

#include <stdint.h>

typedef struct {
    const char* name;
    int (*init)(void);
    int (*get_status)(void);
    void (*do_work)(void);
} service_t;

typedef struct {
    service_t* storage;
    service_t* clock;
    service_t* network;
    service_t* usb;
} service_table_t;

extern service_table_t g_services;

#endif
