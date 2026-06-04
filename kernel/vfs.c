#include "pro_os.h"
#include "hal.h"
#include "serial.h"
#include <string.h>

typedef struct {
    char mount_point[32];
    storage_device_t *device;
    bool mounted;
    int id;
} mount_t;

static mount_t mounts[16];
static int mount_count = 0;

void vfs_init(void) {
    memset(mounts, 0, sizeof(mounts));
    mount_count = 0;
}

void vfs_refresh_mounts(void) {
    mount_count = 0;
    int dev_count = hal_storage_get_device_count();
    for (int i = 0; i < dev_count; i++) {
        storage_device_t *dev = hal_storage_get_device(i);
        if (!dev || mount_count >= 16) continue;

        snprintf(mounts[mount_count].mount_point, 32, "/mnt/disk%d", i);
        mounts[mount_count].device = dev;
        mounts[mount_count].id = i;
        mounts[mount_count].mounted = true;
        mount_count++;
    }
}

static const char* vfs_translate(const char* path, char* out_drv) {
    if (out_drv) out_drv[0] = '\0'; // Initialize to empty string

    for (int i = 0; i < mount_count; i++) {
        size_t mnt_len = strlen(mounts[i].mount_point);
        if (strncmp(path, mounts[i].mount_point, mnt_len) == 0) {
            char next = path[mnt_len];
            if (next == '/' || next == '\0') {
                if (out_drv) snprintf(out_drv, 8, "%d:", mounts[i].id);
                const char* sub = path + mnt_len;
                if (*sub == '\0') return "/";
                return sub;
            }
        }
    }
    return path;
}

int vfs_ls(const char* path, char* out, size_t sz) {
    if (strcmp(path, "/mnt") == 0 || strcmp(path, "/mnt/") == 0) {
        int off = 0;
        for (int i = 0; i < mount_count; i++) {
            int len = snprintf(out + off, sz - off, "<DIR> %s\n", mounts[i].mount_point + 5);
            off += len; if (off >= (int)sz - 1) break;
        }
        return 0;
    }

    char drv[8] = {0}, fpath[256];
    const char* translated = vfs_translate(path, drv);
    // If drv is empty, we don't prefix. If it has "0:", we prefix.
    snprintf(fpath, sizeof(fpath), "%s%s", drv, translated);

    // Functional skeleton for file operations
    snprintf(out, sz, "Listing for %s (translated: %s)\n[Empty Directory]", path, fpath);
    return 0;
}

int vfs_cat(const char* path, char* out, size_t sz) {
    char drv[8] = {0}, fpath[256];
    const char* translated = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", drv, translated);

    snprintf(out, sz, "Content of %s:\n(No hardware backend yet)", fpath);
    return 0;
}

int vfs_mkdir(const char* path) {
    (void)path;
    return 0;
}

int vfs_write(const char* path, const char* content) {
    (void)path; (void)content;
    return 0;
}

int vfs_get_mounts(char* out, size_t sz) {
    int off = 0;
    for (int i = 0; i < mount_count; i++) {
        int len = snprintf(out + off, sz - off, "%s -> %s\n", mounts[i].mount_point, mounts[i].device->name);
        off += len; if (off >= (int)sz - 1) break;
    }
    if (mount_count == 0) snprintf(out, sz, "No active mounts.");
    return 0;
}

int devmgr_list(char* out, size_t sz) {
    int count = hal_storage_get_device_count();
    int off = 0;
    for (int i = 0; i < count; i++) {
        storage_device_t *dev = hal_storage_get_device(i);
        int len = snprintf(out + off, sz - off, "[Disk %d] %s (%lu blocks)\n", i, dev->name, (unsigned long)dev->total_blocks);
        off += len; if (off >= (int)sz - 1) break;
    }
    if (count == 0) snprintf(out, sz, "No hardware detected.");
    return 0;
}
