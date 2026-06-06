#include "hal.h"
#include <string.h>

static storage_device_t* g_storage_devices[8];
static int g_storage_device_count = 0;

int hal_storage_register_device(storage_device_t *dev) {
    if (g_storage_device_count >= 8) return -1;
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

void hal_nvme_init(uint64_t mmio) {
    /* External call from PCI scan */
    extern void nvme_init(uint64_t);
    nvme_init(mmio);
}

void hal_sata_init(uint64_t mmio) {
    /* External call from PCI scan */
    extern void ahci_init(uint64_t);
    ahci_init(mmio);
}
