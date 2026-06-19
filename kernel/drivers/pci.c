/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "pro_os.h"
#include "hal.h"
#include "serial.h"

uint64_t xhci_mmio_base = 0;
uint64_t ehci_mmio_base = 0;
uint64_t nvme_mmio_base = 0;
uint64_t ahci_mmio_base = 0;

typedef struct {
    uint16_t vendor;
    uint16_t device;
    uint8_t  class_id;
    uint8_t  subclass;
    uint8_t  prog_if;
} pci_device_info_t;

#define MAX_PCI_DEVICES 64
static pci_device_info_t g_pci_devices[MAX_PCI_DEVICES];
static int g_pci_count = 0;

static uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                       ((uint32_t)func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000);
    outl(0xCF8, address);
    return inl(0xCFC);
}

uint64_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_index) {
    uint32_t bar = pci_read_config(bus, slot, func, 0x10 + (bar_index * 4));
    if ((bar & 0x6) == 0x04) {
        uint32_t bar_high = pci_read_config(bus, slot, func, 0x14 + (bar_index * 4));
        return ((uint64_t)bar_high << 32) | (bar & 0xFFFFFFF0);
    }
    return bar & 0xFFFFFFF0;
}

void pci_scan(void) {
    serial_printf("[PCI] Starting system hardware scan...\n");
    g_pci_count = 0;
    memset(g_pci_devices, 0, sizeof(g_pci_devices));

    for (int bus = 0; bus < 256; bus++) {
        for (int slot = 0; slot < 32; slot++) {
            for (int func = 0; func < 8; func++) {
                uint32_t vendor_device = pci_read_config(bus, slot, func, 0);
                if ((vendor_device & 0xFFFF) == 0xFFFF) continue;

                uint16_t vendor = vendor_device & 0xFFFF;
                uint16_t device = (vendor_device >> 16) & 0xFFFF;

                uint32_t class_rev = pci_read_config(bus, slot, func, 0x08);
                uint8_t base_class = (class_rev >> 24) & 0xFF;
                uint8_t sub_class = (class_rev >> 16) & 0xFF;
                uint8_t prog_if = (class_rev >> 8) & 0xFF;

                serial_printf("[PCI] Found: %02x:%02x:%d Vendor:%04x Device:%04x Class:%02x\n",
                             bus, slot, func, vendor, device, base_class);

                if (g_pci_count < MAX_PCI_DEVICES) {
                    g_pci_devices[g_pci_count].vendor = vendor;
                    g_pci_devices[g_pci_count].device = device;
                    g_pci_devices[g_pci_count].class_id = base_class;
                    g_pci_devices[g_pci_count].subclass = sub_class;
                    g_pci_devices[g_pci_count].prog_if = prog_if;
                    g_pci_count++;
                }

                if (base_class == 0x0C && sub_class == 0x03 && prog_if == 0x30) {
                    uint64_t mmio = pci_get_bar(bus, slot, func, 0);
                    xhci_mmio_base = mmio;
                    serial_printf("[PCI] xHCI Controller at BAR0: %p\n", mmio);
                    xhci_init(mmio);
                } else if (base_class == 0x0C && sub_class == 0x03 && prog_if == 0x20) {
                    uint64_t mmio = pci_get_bar(bus, slot, func, 0);
                    ehci_mmio_base = mmio;
                    serial_printf("[PCI] EHCI Controller at BAR0: %p\n", mmio);
                    ehci_init(mmio);
                } else if (base_class == 0x01 && sub_class == 0x08 && prog_if == 0x02) {
                    uint64_t mmio = pci_get_bar(bus, slot, func, 0);
                    nvme_mmio_base = mmio;
                    serial_printf("[PCI] NVMe Controller at BAR0: %p\n", mmio);
                    nvme_init(mmio);
                } else if (base_class == 0x01 && sub_class == 0x06 && prog_if == 0x01) {
                    uint64_t mmio = pci_get_bar(bus, slot, func, 5);
                    ahci_mmio_base = mmio;
                    serial_printf("[PCI] AHCI Controller at BAR5: %p\n", mmio);
                    ahci_init(mmio);
                }

                if (func == 0) {
                    uint32_t header_type = pci_read_config(bus, slot, 0, 0x0C);
                    if (!(header_type & 0x800000)) break;
                }
            }
        }
    }
    serial_printf("[PCI] Scan complete. Total devices: %d\n", g_pci_count);
}

int pci_get_device_count(void) {
    return g_pci_count;
}

int pci_get_device_info(int index, char* buf, size_t sz) {
    if (index < 0 || index >= g_pci_count) return -1;
    pci_device_info_t* d = &g_pci_devices[index];
    snprintf(buf, sz, "V:%04X D:%04X C:%02X S:%02X P:%02X",
             d->vendor, d->device, d->class_id, d->subclass, d->prog_if);
    return 0;
}
