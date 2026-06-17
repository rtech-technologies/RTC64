#include "services.h"
#include "pro_os.h"
#include <string.h>

static int rsl_storage_init(void) { return 0; }
static int rsl_storage_status(void) { return hal_storage_get_device_count() > 0 ? 1 : 0; }
static void rsl_storage_work(void) { }

static service_t storage_service = {
    .name = "Royal Storage Service",
    .init = rsl_storage_init,
    .get_status = rsl_storage_status,
    .do_work = rsl_storage_work
};

service_table_t g_services;

void rsl_init(void) {
    g_services.storage = &storage_service;
}
