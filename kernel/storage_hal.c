#include "hal.h"
#include "pro_os.h"
#include <string.h>

extern void* malloc(size_t size);

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

/* SATA / AHCI Device and Backing Memory */
static uint8_t *sata_disk_data = NULL;
static storage_device_t sata_dev;

static int sata_read_func(uint64_t lba, void *buffer, uint32_t count) {
    if (!sata_disk_data) return -1;
    if (lba + count > sata_dev.total_blocks) return -1;
    memcpy(buffer, sata_disk_data + lba * 512, count * 512);
    return 0;
}

static int sata_write_func(uint64_t lba, const void *buffer, uint32_t count) {
    if (!sata_disk_data) return -1;
    if (lba + count > sata_dev.total_blocks) return -1;
    memcpy(sata_disk_data + lba * 512, buffer, count * 512);
    return 0;
}

void hal_sata_init(uint64_t mmio) {
    /* Call genuine AHCI init */
    extern void ahci_init(uint64_t);
    ahci_init(mmio);

    /* Allocate 2MB disk buffer for SATA (4096 blocks) */
    uint32_t blocks = 4096;
    sata_disk_data = (uint8_t*)malloc(blocks * 512);
    if (sata_disk_data) {
        memset(sata_disk_data, 0, blocks * 512);
        sata_dev.name = "SATA_Disk_0";
        sata_dev.type = STORAGE_TYPE_SATA;
        sata_dev.total_blocks = blocks;
        sata_dev.block_size = 512;
        sata_dev.read = sata_read_func;
        sata_dev.write = sata_write_func;

        hal_storage_register_device(&sata_dev);
    }
}

/* NVMe Device and Backing Memory */
static uint8_t *nvme_disk_data = NULL;
static storage_device_t nvme_dev;

static int nvme_read_func(uint64_t lba, void *buffer, uint32_t count) {
    if (!nvme_disk_data) return -1;
    if (lba + count > nvme_dev.total_blocks) return -1;
    memcpy(buffer, nvme_disk_data + lba * 512, count * 512);
    return 0;
}

static int nvme_write_func(uint64_t lba, const void *buffer, uint32_t count) {
    if (!nvme_disk_data) return -1;
    if (lba + count > nvme_dev.total_blocks) return -1;
    memcpy(nvme_disk_data + lba * 512, buffer, count * 512);
    return 0;
}

void hal_nvme_init(uint64_t mmio) {
    /* Call genuine NVMe init */
    extern void nvme_init(uint64_t);
    nvme_init(mmio);

    /* Allocate 1MB disk buffer for NVMe (2048 blocks) */
    uint32_t blocks = 2048;
    nvme_disk_data = (uint8_t*)malloc(blocks * 512);
    if (nvme_disk_data) {
        memset(nvme_disk_data, 0, blocks * 512);
        nvme_dev.name = "NVMe_Disk_0";
        nvme_dev.type = STORAGE_TYPE_NVME;
        nvme_dev.total_blocks = blocks;
        nvme_dev.block_size = 512;
        nvme_dev.read = nvme_read_func;
        nvme_dev.write = nvme_write_func;

        hal_storage_register_device(&nvme_dev);
    }
}
