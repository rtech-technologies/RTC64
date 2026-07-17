#include "pro_os.h"
#include <string.h>

/* Block-backed Custom FAT Filesystem and VFS Implementation */

typedef struct {
    char name[48];
    uint8_t is_dir;
    uint8_t active;
    uint16_t first_cluster;
    uint32_t size;
    char pad[10]; // pad to 64 bytes
} __attribute__((packed)) fat_dirent_t;

#define FAT_ENTRY_FREE 0x0000
#define FAT_ENTRY_EOF  0xFFFF

static storage_device_t *sata_disk = NULL;
static uint16_t fat_table[4096];

typedef struct {
    char mount_point[32];
    storage_device_t *device;
} mount_t;

static mount_t mounts[16];
static int mount_count = 0;

static int disk_read_sector(uint32_t sector, void *buffer) {
    if (!sata_disk) return -1;
    return sata_disk->read(sector, buffer, 1);
}

static int disk_write_sector(uint32_t sector, const void *buffer) {
    if (!sata_disk) return -1;
    return sata_disk->write(sector, buffer, 1);
}

static void load_fat(void) {
    for (int i = 0; i < 16; i++) {
        disk_read_sector(1 + i, ((uint8_t*)fat_table) + i * 512);
    }
}

static void save_fat(void) {
    for (int i = 0; i < 16; i++) {
        disk_write_sector(1 + i, ((uint8_t*)fat_table) + i * 512);
    }
}

void fat_format(void) {
    uint8_t block[512];
    // Superblock (signature 0xAA55)
    memset(block, 0, 512);
    block[0] = 0xAA;
    block[1] = 0x55;
    disk_write_sector(0, block);

    // Clear FAT table
    memset(block, 0, 512);
    for (int i = 1; i <= 16; i++) {
        disk_write_sector(i, block);
    }

    // Clear directories
    for (int i = 17; i <= 32; i++) {
        disk_write_sector(i, block);
    }
}

static int find_free_dirent(uint32_t *out_sec, uint32_t *out_idx) {
    fat_dirent_t ents[8];
    for (uint32_t sec = 17; sec <= 32; sec++) {
        disk_read_sector(sec, ents);
        for (int i = 0; i < 8; i++) {
            if (!ents[i].active) {
                *out_sec = sec;
                *out_idx = i;
                return 0;
            }
        }
    }
    return -1;
}

static int find_dirent(const char *path, fat_dirent_t *out_ent, uint32_t *out_sec, uint32_t *out_idx) {
    fat_dirent_t ents[8];
    for (uint32_t sec = 17; sec <= 32; sec++) {
        disk_read_sector(sec, ents);
        for (int i = 0; i < 8; i++) {
            if (ents[i].active && strcmp(ents[i].name, path) == 0) {
                if (out_ent) *out_ent = ents[i];
                if (out_sec) *out_sec = sec;
                if (out_idx) *out_idx = i;
                return 0;
            }
        }
    }
    return -1;
}

static void free_cluster_chain(uint16_t first_cluster) {
    if (first_cluster == 0) return;
    load_fat();
    uint16_t curr = first_cluster;
    while (curr != FAT_ENTRY_EOF && curr != FAT_ENTRY_FREE) {
        uint16_t next = fat_table[curr];
        fat_table[curr] = FAT_ENTRY_FREE;
        curr = next;
    }
    save_fat();
}

void vfs_init(void) {
    mount_count = 0;
    sata_disk = NULL;

    // Refresh devices
    int dev_count = hal_storage_get_device_count();
    for (int i = 0; i < dev_count; i++) {
        storage_device_t *dev = hal_storage_get_device(i);
        if (strcmp(dev->name, "SATA_Disk_0") == 0) {
            sata_disk = dev;
        }
        snprintf(mounts[mount_count].mount_point, 32, "/dev/%s", dev->name);
        mounts[mount_count].device = dev;
        mount_count++;
    }

    if (sata_disk) {
        uint8_t sb[512];
        disk_read_sector(0, sb);
        if (sb[0] != 0xAA || sb[1] != 0x55) {
            fat_format();
            // Pre-populate system folders
            vfs_mkdir("/System");
            vfs_mkdir("/System/Config");
            vfs_mkdir("/Users");

            // Create default welcome files
            const char *welcome = "Welcome to your Sovereign Workstation OS!\nFeel free to explore the system apps.\n";
            vfs_write("/System/welcome.txt", welcome, strlen(welcome));
        }
    }
}

void vfs_refresh_mounts(void) {
    vfs_init();
}

const char* vfs_resolve(const char *path) {
    for (int i = 0; i < mount_count; i++) {
        if (strncmp(path, mounts[i].mount_point, strlen(mounts[i].mount_point)) == 0) {
            return path;
        }
    }
    return "/root";
}

void vfs_mkdir(const char *path) {
    if (find_dirent(path, NULL, NULL, NULL) == 0) return; // Already exists

    uint32_t sec, idx;
    if (find_free_dirent(&sec, &idx) == 0) {
        fat_dirent_t ents[8];
        disk_read_sector(sec, ents);
        memset(&ents[idx], 0, sizeof(fat_dirent_t));
        strcpy(ents[idx].name, path);
        ents[idx].is_dir = 1;
        ents[idx].active = 1;
        ents[idx].first_cluster = 0;
        ents[idx].size = 0;
        disk_write_sector(sec, ents);
    }
}

int vfs_write(const char *path, const void *data, uint32_t size) {
    fat_dirent_t ent;
    uint32_t ent_sec, ent_idx;
    bool exists = (find_dirent(path, &ent, &ent_sec, &ent_idx) == 0);

    if (exists) {
        if (ent.is_dir) return -1; // Cannot write data to directory
        free_cluster_chain(ent.first_cluster);
    } else {
        if (find_free_dirent(&ent_sec, &ent_idx) != 0) return -1; // Out of directory entries
        memset(&ent, 0, sizeof(fat_dirent_t));
        strcpy(ent.name, path);
        ent.is_dir = 0;
        ent.active = 1;
    }

    uint32_t num_clusters = (size + 511) / 512;
    if (num_clusters == 0) num_clusters = 1;

    load_fat();
    int first = -1;
    int prev = -1;
    for (uint32_t c = 0; c < num_clusters; c++) {
        int new_c = -1;
        for (int i = 1; i < 4096; i++) {
            if (fat_table[i] == FAT_ENTRY_FREE) {
                new_c = i;
                fat_table[i] = FAT_ENTRY_EOF;
                break;
            }
        }
        if (new_c == -1) {
            // Out of disk space, rollback
            free_cluster_chain(first);
            return -1;
        }
        if (first == -1) first = new_c;
        if (prev != -1) fat_table[prev] = new_c;
        prev = new_c;
    }
    save_fat();

    // Write data to clusters
    uint16_t curr = first;
    uint32_t bytes_written = 0;
    uint8_t sector_buf[512];
    while (bytes_written < size) {
        uint32_t chunk = size - bytes_written;
        if (chunk > 512) chunk = 512;
        memset(sector_buf, 0, 512);
        memcpy(sector_buf, ((const uint8_t*)data) + bytes_written, chunk);
        disk_write_sector(33 + curr, sector_buf);
        bytes_written += chunk;
        curr = fat_table[curr];
    }

    // If size is 0, write one blank sector
    if (size == 0) {
        memset(sector_buf, 0, 512);
        disk_write_sector(33 + first, sector_buf);
    }

    // Save directory entry
    fat_dirent_t ents[8];
    disk_read_sector(ent_sec, ents);
    ents[ent_idx] = ent;
    ents[ent_idx].first_cluster = first;
    ents[ent_idx].size = size;
    disk_write_sector(ent_sec, ents);

    return 0;
}

int vfs_read(const char *path, void *buffer, uint32_t max_size) {
    fat_dirent_t ent;
    if (find_dirent(path, &ent, NULL, NULL) != 0) return -1;
    if (ent.is_dir) return -1;

    load_fat();
    uint16_t curr = ent.first_cluster;
    uint32_t bytes_read = 0;
    uint8_t sector_buf[512];
    while (curr != FAT_ENTRY_EOF && bytes_read < ent.size) {
        disk_read_sector(33 + curr, sector_buf);
        uint32_t chunk = ent.size - bytes_read;
        if (chunk > 512) chunk = 512;
        if (bytes_read + chunk > max_size) {
            chunk = max_size - bytes_read;
        }
        memcpy(((uint8_t*)buffer) + bytes_read, sector_buf, chunk);
        bytes_read += chunk;
        if (bytes_read >= max_size) break;
        curr = fat_table[curr];
    }
    return bytes_read;
}

static bool is_direct_child(const char *parent, const char *child) {
    size_t parent_len = strlen(parent);
    if (strncmp(child, parent, parent_len) != 0) return false;

    // Handle root directory "/"
    if (strcmp(parent, "/") == 0) {
        if (child[0] != '/') return false;
        const char *next_slash = strchr(child + 1, '/');
        return (next_slash == NULL);
    }

    if (child[parent_len] != '/') return false;

    const char *next_slash = strchr(child + parent_len + 1, '/');
    return (next_slash == NULL);
}

void vfs_readdir(const char *path, void (*callback)(const char *name, bool is_dir, uint32_t size)) {
    if (!callback) return;

    fat_dirent_t ents[8];
    for (uint32_t sec = 17; sec <= 32; sec++) {
        disk_read_sector(sec, ents);
        for (int i = 0; i < 8; i++) {
            if (ents[i].active && is_direct_child(path, ents[i].name)) {
                // Return only the filename portion
                const char *filename = ents[i].name;
                if (strcmp(path, "/") == 0) {
                    filename = ents[i].name + 1; // skip slash
                } else {
                    filename = ents[i].name + strlen(path) + 1; // skip slash
                }
                callback(filename, ents[i].is_dir != 0, ents[i].size);
            }
        }
    }
}

int vfs_rm(const char *path) {
    fat_dirent_t ent;
    uint32_t sec, idx;
    if (find_dirent(path, &ent, &sec, &idx) != 0) return -1;

    free_cluster_chain(ent.first_cluster);

    fat_dirent_t ents[8];
    disk_read_sector(sec, ents);
    ents[idx].active = 0;
    disk_write_sector(sec, ents);
    return 0;
}

bool vfs_exists(const char *path) {
    return (find_dirent(path, NULL, NULL, NULL) == 0);
}
