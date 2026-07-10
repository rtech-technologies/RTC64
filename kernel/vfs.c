/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
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
        m->id = mount_count;
        snprintf(m->mount_point, 32, "/mnt/disk%d", i);

        char drv_path[4];
        snprintf(drv_path, 4, "%d:", i);

        FRESULT res = f_mount(&m->fs, drv_path, 1);

        if (res == FR_NO_FILESYSTEM) {
            serial_printf("[VFS] Disk %d uninitialized. Formatting FAT32...\n", i);
            BYTE work[FF_MAX_SS];
            MKFS_PARM opt = {FM_FAT32, 0, 0, 0, 0};
            res = f_mkfs(drv_path, &opt, work, sizeof(work));
            if (res == FR_OK) {
                res = f_mount(&m->fs, drv_path, 1);
            }
        }

        m->mounted = (res == FR_OK);
        if (m->mounted) {
            serial_printf("[VFS] Mounted %s at %s\n", dev->name, m->mount_point);
        }
        mount_count++;
    }
}

static const char* vfs_translate(const char* path, char* out_drv) {
    if (!path) return "/";
    for (int i = 0; i < mount_count; i++) {
        size_t mlen = strlen(mounts[i].mount_point);
        if (strncmp(path, mounts[i].mount_point, mlen) == 0) {
            if (path[mlen] == '/' || path[mlen] == '\0') {
                if (out_drv) snprintf(out_drv, 8, "%d:", mounts[i].id);
                return (path[mlen] == '\0') ? "/" : path + mlen;
            }
        }
    }
    if (out_drv) strcpy(out_drv, "0:");
    return path;
}

int vfs_ls(const char* path, char* out, size_t sz) {
    if (!path || !out || sz == 0) return -1;
    DIR dir; FILINFO fno;
    char drv[8], fpath[256];
    const char* sub = vfs_translate(path, drv);
    if (strlen(sub) > 250) return -1; /* Path too deep */
    snprintf(fpath, sizeof(fpath), "%s%s", drv, sub);
    if (f_opendir(&dir, fpath) == FR_OK) {
        int off = 0;
        for (;;) {
            if (f_readdir(&dir, &fno) != FR_OK || fno.fname[0] == 0) break;
            off += snprintf(out + off, sz - off, "%s %s\n", (fno.fattrib & AM_DIR) ? "<DIR>" : "     ", fno.fname);
            if (off >= (int)sz - 1) break;
        }
        f_closedir(&dir); return 0;
    }
    return -1;
}

int vfs_cat(const char* path, char* out, size_t sz) {
    if (!path || !out || sz == 0) return -1;
    FIL fil; UINT br;
    char drv[8], fpath[256];
    const char* sub = vfs_translate(path, drv);
    if (strlen(sub) > 250) return -1;
    snprintf(fpath, sizeof(fpath), "%s%s", drv, sub);
    if (f_open(&fil, fpath, FA_READ) == FR_OK) {
        f_read(&fil, out, sz - 1, &br); out[br] = '\0';
        f_close(&fil); return 0;
    }
    return -1;
}

int vfs_read(const char* path, void* buffer, size_t sz) {
    FIL fil; UINT br;
    char drv[8], fpath[256];
    const char* sub = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", drv, sub);
    if (f_open(&fil, fpath, FA_READ) == FR_OK) {
        f_read(&fil, buffer, sz, &br);
        f_close(&fil);
        return (int)br;
    }
    return -1;
}

int vfs_mkdir(const char* path) {
    char drv[8], fpath[256];
    const char* sub = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", drv, sub);
    return (int)f_mkdir(fpath);
}

int vfs_rm(const char* path) {
    char drv[8], fpath[256];
    const char* sub = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", drv, sub);
    return (int)f_unlink(fpath);
}

int vfs_rename(const char* old_path, const char* new_path) {
    char drv1[8], fpath1[256];
    const char* sub1 = vfs_translate(old_path, drv1);
    snprintf(fpath1, sizeof(fpath1), "%s%s", drv1, sub1);

    char drv2[8], fpath2[256];
    const char* sub2 = vfs_translate(new_path, drv2);
    snprintf(fpath2, sizeof(fpath2), "%s%s", drv2, sub2);

    return (int)f_rename(fpath1, fpath2);
}

int vfs_write(const char* path, const char* content) {
    if (!path || !content) return -1;
    FIL fil; UINT bw;
    char drv[8], fpath[256];
    const char* sub = vfs_translate(path, drv);
    if (strlen(sub) > 250) return -1;
    snprintf(fpath, sizeof(fpath), "%s%s", drv, sub);
    if (f_open(&fil, fpath, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) {
        f_write(&fil, content, strlen(content), &bw);
        f_close(&fil); return 0;
    }
    return -1;
}

int vfs_copy_file(const char* src, const char* dst) {
    if (!src || !dst) return -1;
    FIL fsrc, fdst;
    UINT br, bw;
    char drv_src[8], fpath_src[256];
    char drv_dst[8], fpath_dst[256];

    const char* sub_src = vfs_translate(src, drv_src);
    snprintf(fpath_src, sizeof(fpath_src), "%s%s", drv_src, sub_src);

    const char* sub_dst = vfs_translate(dst, drv_dst);
    snprintf(fpath_dst, sizeof(fpath_dst), "%s%s", drv_dst, sub_dst);

    if (f_open(&fsrc, fpath_src, FA_READ) != FR_OK) return -1;
    if (f_open(&fdst, fpath_dst, FA_WRITE | FA_CREATE_ALWAYS) != FR_OK) {
        f_close(&fsrc);
        return -1;
    }

    char buf[1024];
    for (;;) {
        if (f_read(&fsrc, buf, sizeof(buf), &br) != FR_OK || br == 0) break;
        if (f_write(&fdst, buf, br, &bw) != FR_OK || bw < br) break;
    }

    f_close(&fsrc);
    f_close(&fdst);
    return 0;
}

int vfs_get_mounts(char* out, size_t sz) {
    int off = 0;
    for (int i = 0; i < mount_count; i++) {
        off += snprintf(out + off, sz - off, "%s -> %s [%s]\n", mounts[i].mount_point, mounts[i].device->name, mounts[i].mounted ? "OK" : "ERR");
    }
    return 0;
}

const char* vfs_resolve(const char* path) {
    static char res[256]; char drv[8];
    const char* sub = vfs_translate(path, drv);
    snprintf(res, sizeof(res), "%s%s", drv, sub);
    return res;
}
