#include "pro_os.h"
#include <string.h>

/* Virtual File System - Sovereign Implementation */

typedef struct {
    char mount_point[32];
    storage_device_t *device;
} mount_t;

static mount_t mounts[16];
static int mount_count = 0;

static vfs_node_t files[MAX_VFS_FILES];
static int file_count = 0;

void vfs_init(void) {
    mount_count = 0;
    file_count = 0;

    // Add default directories
    vfs_create_file("/", true);
    vfs_create_file("/root", true);
    vfs_create_file("/home", true);
    vfs_create_file("/system", true);
    vfs_create_file("/dev", true);

    // Add default welcome text file
    vfs_create_file("/home/welcome.txt", false);
    const char *welcome = "Welcome to Sovereign RTC64 OS Pro!\nYour daily-driver workstation is fully ready.\n";
    vfs_write_file("/home/welcome.txt", welcome, strlen(welcome));
}

void vfs_refresh_mounts(void) {
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

bool vfs_create_file(const char *path, bool is_dir) {
    if (file_count >= MAX_VFS_FILES) return false;

    // Check if it already exists
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].path, path) == 0) return false;
    }

    strncpy(files[file_count].path, path, MAX_PATH_LEN);
    files[file_count].content[0] = '\0';
    files[file_count].size = 0;
    files[file_count].is_dir = is_dir;
    file_count++;
    return true;
}

bool vfs_write_file(const char *path, const char *content, size_t size) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].path, path) == 0 && !files[i].is_dir) {
            if (size >= MAX_FILE_SIZE) size = MAX_FILE_SIZE - 1;
            memcpy(files[i].content, content, size);
            files[i].content[size] = '\0';
            files[i].size = size;
            return true;
        }
    }
    // If doesn't exist, create and write
    if (vfs_create_file(path, false)) {
        return vfs_write_file(path, content, size);
    }
    return false;
}

bool vfs_read_file(const char *path, char *buffer, size_t max_size) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].path, path) == 0 && !files[i].is_dir) {
            size_t to_copy = (files[i].size < max_size) ? files[i].size : max_size - 1;
            memcpy(buffer, files[i].content, to_copy);
            buffer[to_copy] = '\0';
            return true;
        }
    }
    return false;
}

bool vfs_delete_file(const char *path) {
    for (int i = 0; i < file_count; i++) {
        if (strcmp(files[i].path, path) == 0) {
            // Delete by shifting
            for (int j = i; j < file_count - 1; j++) {
                files[j] = files[j + 1];
            }
            file_count--;
            return true;
        }
    }
    return false;
}

int vfs_list_dir(const char *dir_path, char filenames[][64], bool is_dirs[], int max_items) {
    int count = 0;
    size_t dir_len = strlen(dir_path);

    // Add real hardware devices if listing /dev
    if (strcmp(dir_path, "/dev") == 0) {
        int dev_count = hal_storage_get_device_count();
        for (int i = 0; i < dev_count && count < max_items; i++) {
            storage_device_t *dev = hal_storage_get_device(i);
            strncpy(filenames[count], dev->name, 64);
            is_dirs[count] = false;
            count++;
        }
    }

    for (int i = 0; i < file_count && count < max_items; i++) {
        const char *p = files[i].path;
        if (strcmp(p, dir_path) == 0) continue; // Don't list the directory itself

        if (strncmp(p, dir_path, dir_len) == 0) {
            // Check if it is a direct child
            const char *sub = p + dir_len;
            if (dir_len > 1 && *sub == '/') sub++;
            if (*sub == '\0') continue;

            // Check for further slashes (sub-directories)
            const char *next_slash = strchr(sub, '/');
            if (next_slash == NULL) {
                strncpy(filenames[count], sub, 64);
                is_dirs[count] = files[i].is_dir;
                count++;
            }
        }
    }
    return count;
}
