#include "pro_os.h"
#include "hal.h"
#include "ff.h"
#include "serial.h"
#include <string.h>

typedef struct { char mount_point[32]; storage_device_t *device; FATFS fs; bool mounted; int id; } mount_t;
static mount_t mounts[16];

void vfs_init(void) { memset(mounts, 0, sizeof(mounts)); }
void vfs_refresh_mounts(void) {
    serial_write("[VFS] Syncing Physical -> Logical mounts...\n");
    int dev_count = devmgr_get_count();
    for (int i = 0; i < dev_count; i++) {
        storage_device_t *dev = devmgr_get_device(i);
        if (!dev) continue;
        bool already = false;
        for (int j=0; j<16; j++) if (mounts[j].device == dev) already = true;
        if (!already) {
            for (int j=0; j<16; j++) if (!mounts[j].device) {
                mounts[j].device = dev;
                mounts[j].id = i;
                const char* label = devmgr_get_label(i);
                // Simplify label for mount point: "SATA(0) Disk 0" -> "/mnt/sata0"
                char mnt[32]; int mptr = 0;
                for(int k=0; label[k] && label[k]!=' '; k++) {
                    if(label[k]>='A' && label[k]<='Z') mnt[mptr++] = label[k]+32;
                    else if(label[k]>='0' && label[k]<='9') mnt[mptr++] = label[k];
                }
                mnt[mptr] = '\0';
                snprintf(mounts[j].mount_point, 32, "/mnt/%s", mnt);

                char drv[4]; snprintf(drv, 4, "%d:", i);
                mounts[j].mounted = (f_mount(&mounts[j].fs, drv, 1) == FR_OK);
                if (mounts[j].mounted) serial_printf("[VFS] Mounted %s to %s\n", dev->name, mounts[j].mount_point);
                break;
            }
        }
    }
}

static const char* vfs_translate(const char* path, char* out_drv) {
    for (int i = 0; i < 16; i++) {
        if (mounts[i].mounted && strncmp(path, mounts[i].mount_point, strlen(mounts[i].mount_point)) == 0) {
            snprintf(out_drv, 8, "%d:", mounts[i].id);
            const char* sub = path + strlen(mounts[i].mount_point);
            if (*sub == '\0') return "/";
            return sub;
        }
    }
    return path;
}

int vfs_ls(const char* path, char* out, size_t sz) {
    if (strcmp(path, "/mnt") == 0 || strcmp(path, "/mnt/") == 0) {
        int off = 0;
        for (int i = 0; i < 16; i++) {
            if (mounts[i].device) {
                int len = snprintf(out + off, sz - off, "<DIR> %s\n", mounts[i].mount_point + 5);
                off += len; if (off >= (int)sz - 1) break;
            }
        }
        return 0;
    }
    DIR dir; FILINFO fno; int off = 0; char drv[8], fpath[256];
    const char* translated = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", (path[0] == '/') ? drv : "", translated);
    if (f_opendir(&dir, fpath) == FR_OK) {
        while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0]) {
            int len = snprintf(out + off, sz - off, "%s %s\n", (fno.fattrib & AM_DIR) ? "<DIR>" : "     ", fno.fname);
            off += len; if (off >= (int)sz - 1) break;
        }
        f_closedir(&dir); return 0;
    }
    return -1;
}

int vfs_cat(const char* path, char* out, size_t sz) {
    FIL fil; UINT br; char drv[8], fpath[256];
    const char* translated = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", (path[0] == '/') ? drv : "", translated);
    if (f_open(&fil, fpath, FA_READ) == FR_OK) { f_read(&fil, out, sz - 1, &br); out[br] = '\0'; f_close(&fil); return 0; }
    return -1;
}

void* vfs_read_file(const char* path, size_t* out_sz) {
    FIL fil; char drv[8], fpath[256];
    const char* translated = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", (path[0] == '/') ? drv : "", translated);
    if (f_open(&fil, fpath, FA_READ) == FR_OK) {
        FSIZE_t sz = f_size(&fil);
        void* buf = malloc(sz);
        if (buf) {
            UINT br;
            f_read(&fil, buf, (UINT)sz, &br);
            if (out_sz) *out_sz = (size_t)br;
        }
        f_close(&fil);
        return buf;
    }
    return NULL;
}

int vfs_mkdir(const char* path) {
    char drv[8], fpath[256]; const char* translated = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", (path[0] == '/') ? drv : "", translated);
    return f_mkdir(fpath) == FR_OK ? 0 : -1;
}

int vfs_write(const char* path, const char* content) {
    FIL fil; UINT bw; char drv[8], fpath[256];
    const char* translated = vfs_translate(path, drv);
    snprintf(fpath, sizeof(fpath), "%s%s", (path[0] == '/') ? drv : "", translated);
    if (f_open(&fil, fpath, FA_WRITE | FA_CREATE_ALWAYS) == FR_OK) { f_write(&fil, content, strlen(content), &bw); f_close(&fil); return 0; }
    return -1;
}

int vfs_get_mounts(char* out, size_t sz) {
    int off = 0;
    for (int i = 0; i < 16; i++) {
        if (mounts[i].device) {
            int len = snprintf(out + off, sz - off, "%s -> %s [%s]\n", mounts[i].mount_point, devmgr_get_label(mounts[i].id), mounts[i].mounted ? "OK" : "ERR");
            off += len; if (off >= (int)sz - 1) break;
        }
    }
    return 0;
}
