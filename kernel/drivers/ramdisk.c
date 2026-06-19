/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
/* Modified by Sovereign: Meaty Ramdisk Implementation with validation */
#include "hal.h"
#include "pro_os.h"
#include <string.h>

#define RAMDISK_BLOCKS 2048
#define RAMDISK_BLOCK_SIZE 512
#define RAMDISK_SIZE (RAMDISK_BLOCKS * RAMDISK_BLOCK_SIZE)

static uint8_t ramdisk_buffer[RAMDISK_SIZE];
static storage_device_t ramdisk_device;

static int ramdisk_read(storage_device_t *dev, uint64_t lba, void *buffer, uint32_t count) {
    if (!dev || !buffer || dev->type != STORAGE_TYPE_RAMDISK) return -1;
    if (lba + count > RAMDISK_BLOCKS) return -1;
    
    uint64_t offset = lba * RAMDISK_BLOCK_SIZE;
    memcpy(buffer, ramdisk_buffer + offset, count * RAMDISK_BLOCK_SIZE);
    return 0;
}

static int ramdisk_write(storage_device_t *dev, uint64_t lba, const void *buffer, uint32_t count) {
    if (!dev || !buffer || dev->type != STORAGE_TYPE_RAMDISK) return -1;
    if (lba + count > RAMDISK_BLOCKS) return -1;
    
    uint64_t offset = lba * RAMDISK_BLOCK_SIZE;
    memcpy(ramdisk_buffer + offset, buffer, count * RAMDISK_BLOCK_SIZE);
    return 0;
}

int ramdisk_init(void) {
    memset(ramdisk_buffer, 0, sizeof(ramdisk_buffer));
    
    ramdisk_device.name = "Sovereign Ramdisk (Meaty)";
    ramdisk_device.type = STORAGE_TYPE_RAMDISK;
    ramdisk_device.total_blocks = RAMDISK_BLOCKS;
    ramdisk_device.block_size = RAMDISK_BLOCK_SIZE;
    ramdisk_device.read = ramdisk_read;
    ramdisk_device.write = ramdisk_write;
    ramdisk_device.priv = NULL;
    
    return hal_storage_register_device(&ramdisk_device);
}
