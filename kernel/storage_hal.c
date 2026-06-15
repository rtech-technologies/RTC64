#include "hal.h"
#include <string.h>
#include "pro_os.h"

static storage_device_t* g_storage_devices[16];
static int g_storage_device_count = 0;

void hal_storage_init(void) {
    g_storage_device_count = 0;
    memset(g_storage_devices, 0, sizeof(g_storage_devices));

    /* Bus Probing handled by Stage 2 pci_scan in kernel.c */
}

void hal_storage_finish_init(void) {
    if (g_storage_device_count == 0) {
        ramdisk_init();
    }
}

int hal_storage_register_device(storage_device_t *dev) {
    /* Hardening: Validate device structure and required callbacks (Point 96) */
    if (!dev || !dev->name || !dev->read) {
        return -1;
    }
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

int hal_nvme_init(uint64_t mmio) {
    return nvme_init(mmio);
}

int hal_sata_init(uint64_t mmio) {
    return ahci_init(mmio);
}
