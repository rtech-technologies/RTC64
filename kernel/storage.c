#include "hal.h"
#include <stdio.h>

#define MAX_STORAGE_DEVICES 16
static storage_device_t *g_storage_devices[MAX_STORAGE_DEVICES];
static int g_storage_device_count = 0;

void hal_storage_init(void) {
    g_storage_device_count = 0;
}

int hal_storage_register_device(storage_device_t *dev) {
    if (g_storage_device_count >= MAX_STORAGE_DEVICES) return -1;
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

/* Expansion points for other storage types */
void hal_nvme_init(void) {
    static storage_device_t nvme_dev;
    nvme_dev.name = "NVMe SSD (PCIe)";
    nvme_dev.type = STORAGE_TYPE_NVME;
    hal_storage_register_device(&nvme_dev);
}

void hal_sata_init(void) {
    static storage_device_t sata_dev;
    sata_dev.name = "SATA HDD";
    sata_dev.type = STORAGE_TYPE_SATA;
    hal_storage_register_device(&sata_dev);
}

void hal_satapi_init(void) {
    static storage_device_t satapi_dev;
    satapi_dev.name = "SATAPI CD-ROM";
    satapi_dev.type = STORAGE_TYPE_SATAPI;
    hal_storage_register_device(&satapi_dev);
}
