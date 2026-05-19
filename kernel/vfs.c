#include "pro_os.h"
#include "hal.h"
#include "ff.h"
#include <string.h>

/* Virtual File System - Sovereign Implementation with Comprehensive Storage Support */

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
    memset(mounts, 0, sizeof(mounts));
}

void vfs_refresh_mounts(void) {
    int dev_count = hal_storage_get_device_count();

    for (int i = 0; i < dev_count; i++) {
        storage_device_t *dev = hal_storage_get_device(i);
        if (!dev) continue;

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

            const char* prefix = "usb";
            if (dev->type == STORAGE_TYPE_NVME) prefix = "nvme";
            else if (dev->type == STORAGE_TYPE_SATA) prefix = "sata";

            snprintf(m->mount_point, 32, "/mnt/%s%d", prefix, mount_count);

            char drv_path[4];
            snprintf(drv_path, 4, "%d:", i);
            FRESULT res = f_mount(&m->fs, drv_path, 1);
            m->mounted = (res == FR_OK);
            mount_count++;
        }
    }
}

void vfs_unmount(storage_device_t* dev) {
    for (int i = 0; i < mount_count; i++) {
        if (mounts[i].device == dev) {
            char drv_path[4];
            snprintf(drv_path, 4, "%d:", i); // This logic needs careful index tracking in real systems
            f_mount(NULL, drv_path, 0);
            mounts[i].device = NULL;
            mounts[i].mounted = false;
            // Shift remaining mounts to keep it clean if needed, or just leave NULL
        }
    }
}

const char* vfs_resolve(const char *path) {
    for (int i = 0; i < mount_count; i++) {
        if (mounts[i].device && strncmp(path, mounts[i].mount_point, strlen(mounts[i].mount_point)) == 0) {
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
