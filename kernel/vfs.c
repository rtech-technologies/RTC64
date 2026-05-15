#include "pro_os.h"
#include <string.h>

/* Virtual File System - Sovereign Implementation */

typedef struct {
    char mount_point[32];
    storage_device_t *device;
} mount_t;

static mount_t mounts[16];
static int mount_count = 0;

void vfs_init(void) {
    mount_count = 0;
    int dev_count = hal_storage_get_device_count();
    for (int i = 0; i < dev_count; i++) {
        storage_device_t *dev = hal_storage_get_device(i);
        snprintf(mounts[mount_count].mount_point, 32, "/dev/%s", dev->name);
        mounts[mount_count].device = dev;
        mount_count++;
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
