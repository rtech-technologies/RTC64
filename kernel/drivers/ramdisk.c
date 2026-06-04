#include "hal.h"
#include "pro_os.h"

int ramdisk_init(void) {
    // Functional skeleton for ramdisk
    static storage_device_t rd;
    rd.name = "Ramdisk";
    rd.type = STORAGE_TYPE_RAMDISK;
    rd.total_blocks = 2048;
    rd.block_size = 512;
    rd.read = NULL;
    rd.write = NULL;
    return hal_storage_register_device(&rd);
}
