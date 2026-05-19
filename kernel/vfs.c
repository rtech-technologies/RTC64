#include "pro_os.h"
#include "hal.h"
#include "ff.h"
#include <string.h>

/* Virtual File System - Sovereign Implementation with FatFs Integration */

typedef struct {
    char mount_point[32];
    storage_device_t *device;
    FATFS fs;
    bool mounted;
} mount_t;

static mount_t mounts[16];
static int mount_count = 0;

void vfs_init(void) {
    mount_count = 0;
}

void vfs_refresh_mounts(void) {
    int dev_count = hal_storage_get_device_count();

    // Check for new devices
    for (int i = 0; i < dev_count; i++) {
        storage_device_t *dev = hal_storage_get_device(i);
        bool already_mounted = false;

        for (int j = 0; j < mount_count; j++) {
            if (mounts[j].device == dev) {
                already_mounted = true;
                break;
            }
        }

        if (!already_mounted && mount_count < 16) {
            mount_t *m = &mounts[mount_count];
            m->device = dev;
            snprintf(m->mount_point, 32, "/mnt/usb%d", mount_count);

            // Try to mount with FatFs
            char drv_path[4];
            snprintf(drv_path, 4, "%d:", i);
            FRESULT res = f_mount(&m->fs, drv_path, 1);
            if (res == FR_OK) {
                m->mounted = true;
            } else {
                m->mounted = false;
            }
            mount_count++;
        }
    }
}

const char* vfs_resolve(const char *path) {
    /* Resolves logical paths to hardware-backed endpoints */
    for (int i = 0; i < mount_count; i++) {
        if (strncmp(path, mounts[i].mount_point, strlen(mounts[i].mount_point)) == 0) {
            return path;
        }
    }
    return "/root";
}
