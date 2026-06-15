/* Modified by Sovereign: Meaty VFS with deep path translation and binary file support */
#include "pro_os.h"
#include "hal.h"
#include "fatfs/ff.h"
#include "serial.h"
#include <string.h>

typedef struct {
    char mount_point[32];
    storage_device_t *device;
    FATFS fs;
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
    int dev_count = hal_storage_get_device_count();
    for (int i = 0; i < dev_count; i++) {
        storage_device_t *dev = hal_storage_get_device(i);
        if (!dev) continue;

        bool already = false;
        for (int j = 0; j < mount_count; j++) {
            if (mounts[j].device == dev) { already = true; break; }
        }
        if (already) continue;
        if (mount_count >= 16) break;

        mount_t *m = &mounts[mount_count];
        m->device = dev;
        m->id = i;
        snprintf(m->mount_point, 32, "/mnt/disk%d", i);

        char drv_path[4];
        snprintf(drv_path, 4, "%d:", i);

        FRESULT res = f_mount(&m->fs, drv_path, 1);
        m->mounted = (res == FR_OK);

        if (m->mounted) {
            serial_printf("[VFS] Mounted disk %d (%s) at %s\n", i, dev->name, m->mount_point);
        } else {
            serial_printf("[VFS] Failed to mount disk %d (%s), error %d\n", i, dev->name, (int)res);
        }
        mount_count++;
    }
}

static const char* vfs_translate(const char* path, char* out_drv) {
    if (out_drv) out_drv[0] = '\0';
    if (!path) return "/";

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
    if (!path || !out) return -1;
    if (strcmp(path, "/mnt") == 0 || strcmp(path, "/mnt/") == 0) {
        int off = 0;
        for (int i = 0; i < mount_count; i++) {
            int len = snprintf(out + off, sz - off, "<DIR> %s\n", mounts[i].mount_point + 5);
            off += len; if (off >= (int)sz - 1) break;
        }
        return 0;
    }
    if (strcmp(path, "/connect") == 0 || strcmp(path, "/connect/") == 0) {
        /* Modified by Sovereign: List external hot-pluggable devices */
        int off = 0;
        off += snprintf(out + off, sz - off, "External Connections:\n");
        int dev_count = hal_storage_get_device_count();
        for (int i = 0; i < dev_count; i++) {
            storage_device_t *dev = hal_storage_get_device(i);
            if (dev && dev->type == STORAGE_TYPE_USB) {
                off += snprintf(out + off, sz - off, "[USB] %s (Connected)\n", dev->name);
            }
        }
        /* Future: Add HDMI/Audio status here */
        off += snprintf(out + off, sz - off, "[HDMI] No External Monitor\n");
        off += snprintf(out + off, sz - off, "[JACK] No Audio Device\n");
        return 0;
    }

    DIR dir;
    FILINFO fno;
    FRESULT res;
    char drv[8] = {0}, fpath[256];
    const char* translated = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", drv, translated);

    res = f_opendir(&dir, fpath);
    if (res == FR_OK) {
        int off = 0;
        for (;;) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break;
            int len = snprintf(out + off, sz - off, "%s %s\n", (fno.fattrib & AM_DIR) ? "<DIR>" : "     ", fno.fname);
            off += len; if (off >= (int)sz - 1) break;
        }
        f_closedir(&dir);
        return 0;
    }
    return (int)res;
}

int vfs_cat(const char* path, char* out, size_t sz) {
    if (!path || !out) return -1;
    FIL fil;
    FRESULT res;
    UINT br;
    char drv[8] = {0}, fpath[256];
    const char* translated = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", drv, translated);

    res = f_open(&fil, fpath, FA_READ);
    if (res == FR_OK) {
        f_read(&fil, out, sz - 1, &br);
        out[br] = '\0';
        f_close(&fil);
        return 0;
    }
    return (int)res;
}

void* vfs_read_file(const char* path, size_t* out_sz) {
    if (!path) return NULL;
    FIL fil;
    FRESULT res;
    UINT br;
    char drv[8] = {0}, fpath[256];
    const char* translated = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", drv, translated);

    res = f_open(&fil, fpath, FA_READ);
    if (res == FR_OK) {
        FSIZE_t sz = f_size(&fil);
        void* buf = malloc((size_t)sz);
        if (buf) {
            f_read(&fil, buf, (UINT)sz, &br);
            if (out_sz) *out_sz = (size_t)br;
        }
        f_close(&fil);
        return buf;
    }
    return NULL;
}

int vfs_mkdir(const char* path) {
    if (!path) return -1;
    char drv[8] = {0}, fpath[256];
    const char* translated = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", drv, translated);
    return (int)f_mkdir(fpath);
}

int vfs_write(const char* path, const char* content) {
    if (!path || !content) return -1;
    FIL fil;
    FRESULT res;
    UINT bw;
    char drv[8] = {0}, fpath[256];
    const char* translated = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", drv, translated);

    res = f_open(&fil, fpath, FA_WRITE | FA_CREATE_ALWAYS);
    if (res == FR_OK) {
        f_write(&fil, content, (UINT)strlen(content), &bw);
        f_close(&fil);
        return 0;
    }
    return (int)res;
}

int vfs_get_mounts(char* out, size_t sz) {
    if (!out) return -1;
    int off = 0;
    for (int i = 0; i < mount_count; i++) {
        int len = snprintf(out + off, sz - off, "%s -> %s [%s]\n", mounts[i].mount_point, mounts[i].device->name, mounts[i].mounted ? "OK" : "ERR");
        off += len; if (off >= (int)sz - 1) break;
    }
    if (mount_count == 0) snprintf(out, sz, "No active mounts.");
    return 0;
}

int devmgr_list(char* out, size_t sz) {
    if (!out) return -1;
    int off = 0;

    /* Storage Devices */
    int count = hal_storage_get_device_count();
    off += snprintf(out + off, sz - off, "--- Storage Devices ---\n");
    for (int i = 0; i < count; i++) {
        storage_device_t *dev = hal_storage_get_device(i);
        int len = snprintf(out + off, sz - off, "[Disk %d] %s (%llu blocks)\n", i, dev->name, (unsigned long long)dev->total_blocks);
        off += len; if (off >= (int)sz - 1) break;
    }

    /* PCI Devices */
    if (off < (int)sz - 32) {
        int pci_count = pci_get_device_count();
        off += snprintf(out + off, sz - off, "\n--- PCI Hardware ---\n");
        for (int i = 0; i < pci_count; i++) {
            char pci_info[64];
            pci_get_device_info(i, pci_info, sizeof(pci_info));
            int len = snprintf(out + off, sz - off, "[PCI %d] %s\n", i, pci_info);
            off += len; if (off >= (int)sz - 1) break;
        }
    }

    if (off == 0) snprintf(out, sz, "No hardware detected.");
    return 0;
}

const char* vfs_resolve(const char *path) {
    if (!path) return "/";
    static char resolved[256];
    char drv[8] = {0};
    const char *translated = vfs_translate(path, drv);
    snprintf(resolved, sizeof(resolved), "%s%s", drv, translated);
    return resolved;
}
