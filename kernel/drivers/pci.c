#include <stdint.h>
#include <stddef.h>
#include "pro_os.h"
#include "hal.h"

uint64_t xhci_mmio_base = 0;
uint64_t ehci_mmio_base = 0;
uint64_t nvme_mmio_base = 0;
uint64_t ahci_mmio_base = 0;

/* Sovereign PCI Discovery System */

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                       ((uint32_t)func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000);
    __asm__ volatile("outl %0, %1" : : "a"(address), "Nd"(PCI_CONFIG_ADDRESS));
    uint32_t val;
    __asm__ volatile("inl %1, %0" : "=a"(val) : "Nd"(PCI_CONFIG_DATA));
    return val;
}

uint64_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_index) {
    uint32_t bar = pci_read_config(bus, slot, func, 0x10 + (bar_index * 4));
    if ((bar & 0x6) == 0x04) { /* 64-bit BAR */
        uint32_t bar_high = pci_read_config(bus, slot, func, 0x14 + (bar_index * 4));
        return ((uint64_t)bar_high << 32) | (bar & 0xFFFFFFF0);
    }
    return bar & 0xFFFFFFF0;
}

void pci_scan(void) {
    for (int bus = 0; bus < 256; bus++) {
        for (int slot = 0; slot < 32; slot++) {
            for (int func = 0; func < 8; func++) {
                uint32_t vendor_device = pci_read_config(bus, slot, func, 0);
                if ((vendor_device & 0xFFFF) == 0xFFFF) continue;

                uint32_t class_rev = pci_read_config(bus, slot, func, 0x08);
                uint8_t base_class = (class_rev >> 24) & 0xFF;
                uint8_t sub_class = (class_rev >> 16) & 0xFF;
                uint8_t prog_if = (class_rev >> 8) & 0xFF;

                /* Identify xHCI (USB 3.0), EHCI (USB 2.0), NVMe, AHCI */
                if (base_class == 0x0C && sub_class == 0x03 && prog_if == 0x30) {
                    uint64_t mmio = pci_get_bar(bus, slot, func, 0);
                    xhci_mmio_base = mmio;
                    xhci_init(mmio);
                } else if (base_class == 0x0C && sub_class == 0x03 && prog_if == 0x20) {
                    uint64_t mmio = pci_get_bar(bus, slot, func, 0);
                    ehci_mmio_base = mmio;
                    ehci_init(mmio);
                } else if (base_class == 0x01 && sub_class == 0x08 && prog_if == 0x02) {
                    uint64_t mmio = pci_get_bar(bus, slot, func, 0);
                    nvme_mmio_base = mmio;
                    hal_nvme_init(mmio);
                    nvme_init(mmio);
                } else if (base_class == 0x01 && sub_class == 0x06 && prog_if == 0x01) {
                    uint64_t mmio = pci_get_bar(bus, slot, func, 5); /* AHCI BAR is usually 5 */
                    ahci_mmio_base = mmio;
                    hal_sata_init(mmio);
                    ahci_init(mmio);
                }

                if (func == 0) {
                    uint32_t header_type = pci_read_config(bus, slot, 0, 0x0C);
                    if (!(header_type & 0x800000)) break;
                }
            }
        }
    }
}
