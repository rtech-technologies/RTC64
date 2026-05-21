#include "pro_os.h"
#include "hal.h"
#include "ff.h"
#include "serial.h"
#include <string.h>

/* Virtual File System - Sovereign Implementation with Comprehensive Storage Support */

typedef struct {
    char mount_point[32];
    storage_device_t *device;
    FATFS fs;
    bool mounted;
} mount_t;

static mount_t mounts[16];

void vfs_init(void) {
    memset(mounts, 0, sizeof(mounts));
}

void vfs_refresh_mounts(void) {
    serial_write("[VFS] Refreshing mount points...\n");
    int dev_count = hal_storage_get_device_count();

    // First, clear old mounts that are no longer present
    for (int j = 0; j < 16; j++) {
        if (mounts[j].mounted) {
            bool still_exists = false;
            for (int i = 0; i < dev_count; i++) {
                if (hal_storage_get_device(i) == mounts[j].device) {
                    still_exists = true;
                    break;
                }
            }
            if (!still_exists) {
                char drv_path[4];
                snprintf(drv_path, 4, "%d:", j);
                f_mount(NULL, drv_path, 0);
                memset(&mounts[j], 0, sizeof(mount_t));
            }
        }
    }

    for (int i = 0; i < dev_count; i++) {
        storage_device_t *dev = hal_storage_get_device(i);
        if (!dev) continue;

        bool already_mounted = false;
        for (int j = 0; j < 16; j++) {
            if (mounts[j].device == dev) {
                already_mounted = true;
                break;
            }
        }

        if (!already_mounted) {
            // Find first free slot
            int slot = -1;
            for (int j = 0; j < 16; j++) {
                if (!mounts[j].device) { slot = j; break; }
            }

            if (slot != -1) {
                mount_t *m = &mounts[slot];
                m->device = dev;

                const char* prefix = "usb";
                if (dev->type == STORAGE_TYPE_NVME) prefix = "nvme";
                else if (dev->type == STORAGE_TYPE_SATA) prefix = "sata";

                snprintf(m->mount_point, 32, "/mnt/%s%d", prefix, slot);

                char drv_path[4];
                snprintf(drv_path, 4, "%d:", i);
                FRESULT res = f_mount(&m->fs, drv_path, 1);
                m->mounted = (res == FR_OK);
                if (m->mounted) {
                    serial_printf("[VFS] Mounted %s to %s (Physical %d)\n", dev->name, m->mount_point, i);
                } else {
                    serial_printf("[VFS] Failed to mount physical drive %d\n", i);
                }
            }
        }
    }
}

void vfs_unmount(storage_device_t* dev) {
    for (int i = 0; i < 16; i++) {
        if (mounts[i].device == dev) {
            char drv_path[4];
            snprintf(drv_path, 4, "%d:", i);
            f_mount(NULL, drv_path, 0);
            memset(&mounts[i], 0, sizeof(mount_t));
        }
    }
}

const char* vfs_resolve(const char *path) {
    for (int i = 0; i < 16; i++) {
        if (mounts[i].mounted && strncmp(path, mounts[i].mount_point, strlen(mounts[i].mount_point)) == 0) {
            return path;
        }
    }
    return "/root";
}

int vfs_ls(const char* path, char* out_buf, size_t buf_size) {
    DIR dir;
    FILINFO fno;
    FRESULT res;
    int offset = 0;

    res = f_opendir(&dir, path);
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            int len = snprintf(out_buf + offset, buf_size - offset, "%s %s\n",
                               (fno.fattrib & AM_DIR) ? "<DIR>" : "     ", fno.fname);
            offset += len;
            if (offset >= (int)buf_size - 1) break;
        }
        f_closedir(&dir);
        return 0;
    }
    return -1;
}
