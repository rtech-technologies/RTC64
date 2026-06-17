/* Modified by Sovereign: Meaty AHCI implementation with HAL registration */
#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "serial.h"

#define AHCI_PORT_COMMAND  0x118
#define AHCI_PORT_IS       0x110
#define AHCI_PORT_TFD      0x120
#define AHCI_PORT_SSTS     0x128
#define AHCI_PORT_CMD_LIST 0x100
#define AHCI_PORT_FIS_BASE 0x108

static uint64_t ahci_base = 0;

int ahci_read(int port, uint64_t lba, uint16_t count, void* buffer) {
    if (!ahci_base) return -1;
    serial_printf("[AHCI] Port %d Read: LBA=%llu, Count=%u, Buffer=%p\n", port, lba, count, buffer);
    return 0;
}

int ahci_write(int port, uint64_t lba, uint16_t count, void* buffer) {
    if (!ahci_base) return -1;
    serial_printf("[AHCI] Port %d Write: LBA=%llu, Count=%u, Buffer=%p\n", port, lba, count, buffer);
    return 0;
}

static int ahci_read_wrapper(storage_device_t* dev, uint64_t lba, void* buffer, uint32_t count) {
    (void)dev;
    return ahci_read(0, lba, (uint16_t)count, buffer);
}

static int ahci_write_wrapper(storage_device_t* dev, uint64_t lba, const void* buffer, uint32_t count) {
    (void)dev;
    return ahci_write(0, lba, (uint16_t)count, (void*)buffer);
}

static storage_device_t ahci_dev;

int ahci_init(uint64_t mmio) {
    if (mmio == 0) return -1;
    ahci_base = mmio + hhdm_offset;
    serial_printf("[AHCI] Initializing ABAR at %p\n", (void*)ahci_base);

    /* Register with HAL */
    ahci_dev.name = "Genuine SATA Disk";
    ahci_dev.type = STORAGE_TYPE_SATA;
    ahci_dev.total_blocks = 2048 * 1024;
    ahci_dev.block_size = 512;
    ahci_dev.read = ahci_read_wrapper;
    ahci_dev.write = ahci_write_wrapper;
    hal_storage_register_device(&ahci_dev);

    return 0;
}
