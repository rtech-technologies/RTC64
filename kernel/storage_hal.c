#include "hal.h"
#include <string.h>

static storage_device_t* g_storage_devices[16];
static int g_storage_device_count = 0;

int hal_storage_register_device(storage_device_t *dev) {
    if (g_storage_device_count >= 16) return -1;
    g_storage_devices[g_storage_device_count++] = dev;
    return 0;
}

int hal_storage_get_device_count(void) {
    return g_storage_device_count;
}

storage_device_t* hal_storage_get_device(int index) {
    if (index < 0 || index >= g_storage_device_count) return NULL;
    return g_storage_devices[index];
}

void hal_nvme_init(void) {
    /* Ready for namespaces */
}

void hal_sata_init(void) {
    /* Ready for ports */
}
