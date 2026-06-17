#include "pro_os.h"
#include <string.h>
#include "serial.h"

typedef struct {
    char label[32];
    int global_id;
    int type_id;
    storage_type_t type;
    storage_device_t* dev;
} dev_info_t;

static dev_info_t devices[32];
static int total_count = 0;
static int type_counts[4] = {0, 0, 0, 0}; // USB, NVME, SATA, SATAPI

void devmgr_register_storage(storage_device_t* d) {
    if (total_count >= 32) return;

    int tid = type_counts[d->type]++;
    const char* type_str = "UNK";
    if (d->type == STORAGE_TYPE_USB) type_str = "USB";
    else if (d->type == STORAGE_TYPE_NVME) type_str = "NVME";
    else if (d->type == STORAGE_TYPE_SATA) type_str = "SATA";

    dev_info_t* info = &devices[total_count];
    info->global_id = total_count;
    info->type_id = tid;
    info->type = d->type;
    info->dev = d;

    snprintf(info->label, 32, "%s(%d) Disk %d", type_str, tid, total_count);

    serial_printf("[DEVMGR] Registered: %s -> %s\n", info->label, d->name);
    total_count++;
}

int devmgr_get_count(void) { return total_count; }
const char* devmgr_get_label(int index) { return (index >= 0 && index < total_count) ? devices[index].label : "NONE"; }
storage_device_t* devmgr_get_device(int index) { return (index >= 0 && index < total_count) ? devices[index].dev : NULL; }

int devmgr_list(char* out, size_t sz) {
    int off = 0;
    for (int i = 0; i < total_count; i++) {
        int len = snprintf(out + off, sz - off, "[ID %d] %s [%s]\n", devices[i].global_id, devices[i].label, devices[i].dev->name);
        off += len; if (off >= (int)sz - 1) break;
    }
    return 0;
}
