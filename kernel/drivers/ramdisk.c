#include "hal.h"
#include "pro_os.h"
#include "pmm.h"
#include "serial.h"
#include <string.h>

#define RAMDISK_SIZE (4 * 1024 * 1024) // 4MB
static void* ramdisk_mem = NULL;
static storage_device_t ramdisk_dev;

static int ramdisk_read(storage_device_t* d, uint64_t lba, void* buffer, uint32_t count) {
    (void)d;
    if (!ramdisk_mem) return -1;
    memcpy(buffer, (uint8_t*)ramdisk_mem + (lba * 512), count * 512);
    return 0;
}

static int ramdisk_write(storage_device_t* d, uint64_t lba, const void* buffer, uint32_t count) {
    (void)d;
    if (!ramdisk_mem) return -1;
    memcpy((uint8_t*)ramdisk_mem + (lba * 512), buffer, count * 512);
    return 0;
}

void ramdisk_init(void) {
    serial_write("[RAMDISK] Initializing fallback ramdisk...\n");
    ramdisk_mem = pmm_alloc(RAMDISK_SIZE / 4096);
    if (!ramdisk_mem) {
        serial_write("[RAMDISK] Failed to allocate memory for ramdisk!\n");
        return;
    }
    memset(ramdisk_mem, 0, RAMDISK_SIZE);

    ramdisk_dev.name = "Sovereign Volatile Ramdisk";
    ramdisk_dev.type = STORAGE_TYPE_RAMDISK;
    ramdisk_dev.total_blocks = RAMDISK_SIZE / 512;
    ramdisk_dev.block_size = 512;
    ramdisk_dev.read = ramdisk_read;
    ramdisk_dev.write = ramdisk_write;

    hal_storage_register_device(&ramdisk_dev);
    serial_write("[RAMDISK] Fallback ramdisk registered as a system drive.\n");
}
