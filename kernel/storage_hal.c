/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "hal.h"
#include <string.h>
#include <stdio.h>
#include "pro_os.h"

static storage_device_t* g_storage_devices[16];
static int g_storage_device_count = 0;

void hal_storage_init(void) {
    g_storage_device_count = 0;
    memset(g_storage_devices, 0, sizeof(g_storage_devices));

    /* Bus Probing handled by Stage 2 pci_scan in kernel.c */
}

void hal_storage_finish_init(void) {
    if (g_storage_device_count == 0) {
        ramdisk_init();
    }
}

int hal_storage_register_device(storage_device_t *dev) {
    if (g_storage_device_count >= 16) return -1;
    g_storage_devices[g_storage_device_count++] = dev;
    return 0;
}

int hal_storage_get_device_count(void) {
    return g_storage_device_count;
}

storage_device_t* hal_storage_get_device(int index) {
    if (index < 0 || index >= g_storage_device_count) return NULL;
    return g_storage_devices[index];
}

int hal_nvme_init(uint64_t mmio) {
    return nvme_init(mmio);
}

int hal_sata_init(uint64_t mmio) {
    return ahci_init(mmio);
}

extern int pci_get_device_count(void);
extern int pci_get_device_info(int index, char* buf, size_t sz);

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
