#include "pro_os.h"
#include <string.h>

/* Configuration Manager (cm) module - Unified Registry Manager */

void cm_init(void) {
    /* Ready to coordinate workstation setup configs */
}

int cm_read_config(const char *key, char *out_val, int max_len) {
    char path[128];
    snprintf(path, sizeof(path), "/System/Config/%s", key);
    return vfs_read(path, out_val, max_len);
}

void cm_write_config(const char *key, const char *val) {
    char path[128];
    snprintf(path, sizeof(path), "/System/Config/%s", key);
    vfs_write(path, val, strlen(val));
}
