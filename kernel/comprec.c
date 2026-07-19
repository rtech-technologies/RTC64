#include "pro_os.h"
#include <string.h>

/* Compliance Recording (comprec) module - Audit Security Log Manager */

void comprec_init(void) {
    /* Write fresh audit log header */
    vfs_write("/System/Config/audit.log", "[COMPREC] Compliance Recording Audit Log Started\n", 49);
}

void comprec_log(const char *event) {
    char buf[3072] = {0};
    int len = vfs_read("/System/Config/audit.log", buf, sizeof(buf) - 256);
    if (len < 0) len = 0;
    buf[len] = '\0';

    char line[256];
    snprintf(line, sizeof(line), "[AUDIT] %s\n", event);
    strcat(buf, line);
    vfs_write("/System/Config/audit.log", buf, strlen(buf));
}
