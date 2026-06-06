#include "hal.h"
#include "pro_os.h"
#include <string.h>

/* Ramdisk storage: 2048 blocks × 512 bytes = 1 MB */
#define RAMDISK_BLOCKS 2048
#define RAMDISK_BLOCK_SIZE 512
#define RAMDISK_SIZE (RAMDISK_BLOCKS * RAMDISK_BLOCK_SIZE)

static uint8_t ramdisk_buffer[RAMDISK_SIZE];
static storage_device_t ramdisk_device;

static int ramdisk_read(storage_device_t *dev, uint64_t lba, void *buffer, uint32_t count) {
    (void)dev;
    
    if (lba + count > RAMDISK_BLOCKS) {
        return -1; /* Out of bounds */
    }
    
    uint64_t offset = lba * RAMDISK_BLOCK_SIZE;
    memcpy(buffer, ramdisk_buffer + offset, count * RAMDISK_BLOCK_SIZE);
    return 0;
}

static int ramdisk_write(storage_device_t *dev, uint64_t lba, const void *buffer, uint32_t count) {
    (void)dev;
    
    if (lba + count > RAMDISK_BLOCKS) {
        return -1; /* Out of bounds */
    }
    
    uint64_t offset = lba * RAMDISK_BLOCK_SIZE;
    memcpy(ramdisk_buffer + offset, buffer, count * RAMDISK_BLOCK_SIZE);
    return 0;
}

int ramdisk_init(void) {
    /* Initialize ramdisk buffer with zeros */
    memset(ramdisk_buffer, 0, sizeof(ramdisk_buffer));
    
    /* Configure device structure */
    ramdisk_device.name = "Ramdisk";
    ramdisk_device.type = STORAGE_TYPE_RAMDISK;
    ramdisk_device.total_blocks = RAMDISK_BLOCKS;
    ramdisk_device.block_size = RAMDISK_BLOCK_SIZE;
    ramdisk_device.read = ramdisk_read;
    ramdisk_device.write = ramdisk_write;
    ramdisk_device.priv = NULL;
    
    return hal_storage_register_device(&ramdisk_device);
}
