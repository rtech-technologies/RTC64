#include "pro_os.h"
#include "hal.h"
#include "ff.h"
#include "serial.h"
#include <string.h>

typedef struct { char mount_point[32]; storage_device_t *device; FATFS fs; bool mounted; } mount_t;
static mount_t mounts[16];

void vfs_init(void) { memset(mounts, 0, sizeof(mounts)); }
void vfs_refresh_mounts(void) {
    serial_write("[VFS] Syncing Physical -> Logical mounts...\n");
    int dev_count = hal_storage_get_device_count();
    for (int i = 0; i < dev_count; i++) {
        storage_device_t *dev = hal_storage_get_device(i);
        if (!dev) continue;
        bool already = false;
        for (int j=0; j<16; j++) if (mounts[j].device == dev) already = true;
        if (!already) {
            for (int j=0; j<16; j++) if (!mounts[j].device) {
                mounts[j].device = dev;
                const char* pr = (dev->type == STORAGE_TYPE_NVME) ? "nvme" : "disk";
                snprintf(mounts[j].mount_point, 32, "/mnt/%s%d", pr, j);
                char drv[4]; snprintf(drv, 4, "%d:", i);
                mounts[j].mounted = (f_mount(&mounts[j].fs, drv, 1) == FR_OK);
                if (mounts[j].mounted) serial_printf("[VFS] Mounted %s to %s\n", dev->name, mounts[j].mount_point);
                break;
            }
        }
    }
}
int vfs_ls(const char* path, char* out, size_t sz) {
    DIR dir; FILINFO fno; int off = 0;
    if (f_opendir(&dir, path) == FR_OK) {
        while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0]) {
            int len = snprintf(out + off, sz - off, "%s %s\n", (fno.fattrib & AM_DIR) ? "<DIR>" : "     ", fno.fname);
            off += len; if (off >= (int)sz - 1) break;
        }
        f_closedir(&dir); return 0;
    }
    return -1;
}
