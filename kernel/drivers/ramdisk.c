/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "hal.h"
#include "pro_os.h"
#include <string.h>
#include "limine.h"
#include "serial.h"

extern volatile struct limine_module_request module_request;

static uint8_t *ramdisk_ptr = NULL;
static uint64_t ramdisk_size = 0;
static storage_device_t ramdisk_device;

static int ramdisk_read(storage_device_t *dev, uint64_t lba, void *buffer, uint32_t count) {
    if (!ramdisk_ptr) return -1;
    uint64_t offset = lba * dev->block_size;
    if (offset + (uint64_t)count * dev->block_size > ramdisk_size) return -1;
    memcpy(buffer, ramdisk_ptr + offset, count * dev->block_size);
    return 0;
}

static int ramdisk_write(storage_device_t *dev, uint64_t lba, const void *buffer, uint32_t count) {
    if (!ramdisk_ptr) return -1;
    uint64_t offset = lba * dev->block_size;
    if (offset + (uint64_t)count * dev->block_size > ramdisk_size) return -1;
    memcpy(ramdisk_ptr + offset, buffer, count * dev->block_size);
    return 0;
}

int ramdisk_init(void) {
    if (module_request.response && module_request.response->module_count > 0) {
        struct limine_file *module = module_request.response->modules[0];
        ramdisk_ptr = (uint8_t *)module->address;
        ramdisk_size = module->size;
        serial_printf("[RAMDISK] Loaded Limine module: %llu bytes\n", ramdisk_size);
    } else {
        vga_log("\n\n!!! CRITICAL ERROR: SOVEREIGN RAMDISK NOT FOUND !!!\n");
        vga_log("Check your bootloader configuration (limine.cfg).\n\n");

        static uint8_t fallback[1024*1024];
        ramdisk_ptr = fallback;
        ramdisk_size = sizeof(fallback);
    }

    ramdisk_device.name = "Sovereign Ramdisk (Meaty)";
    ramdisk_device.type = STORAGE_TYPE_RAMDISK;
    ramdisk_device.total_blocks = ramdisk_size / 512;
    ramdisk_device.block_size = 512;
    ramdisk_device.read = ramdisk_read;
    ramdisk_device.write = ramdisk_write;
    ramdisk_device.priv = NULL;
    
    return hal_storage_register_device(&ramdisk_device);
}
