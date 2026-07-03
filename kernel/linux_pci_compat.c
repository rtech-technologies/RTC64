/* Linux-compatible PCI helper wrappers using the existing RTC64 PCI subsystem. */
#include <stdint.h>
#include <stddef.h>
#include "linux/pci.h"
#include "linux/pci_regs.h"
#include "pro_os.h"
#include "serial.h"

extern uint64_t hhdm_offset;

int pci_enable_device(struct pci_dev *dev) {
    if (!dev) return -1;
    uint32_t command = pci_read_config(dev->bus, dev->slot, dev->func, PCI_COMMAND);
    command |= PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | PCI_COMMAND_IO;
    /* Note: There is no pci_write_config helper yet in this kernel, so use existing config I/O directly. */
    uint32_t address = (uint32_t)((uint32_t)dev->bus << 16) | ((uint32_t)dev->slot << 11) |
                       ((uint32_t)dev->func << 8) | (PCI_COMMAND & 0xFC) | 0x80000000;
    outl(0xCF8, address);
    outl(0xCFC, command);
    return 0;
}

void pci_disable_device(struct pci_dev *dev) {
    if (!dev) return;
    uint32_t command = pci_read_config(dev->bus, dev->slot, dev->func, PCI_COMMAND);
    command &= ~(PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER | PCI_COMMAND_IO);
    uint32_t address = (uint32_t)((uint32_t)dev->bus << 16) | ((uint32_t)dev->slot << 11) |
                       ((uint32_t)dev->func << 8) | (PCI_COMMAND & 0xFC) | 0x80000000;
    outl(0xCF8, address);
    outl(0xCFC, command);
}

bool pci_is_enabled(struct pci_dev *dev) {
    if (!dev) return false;
    uint32_t command = pci_read_config(dev->bus, dev->slot, dev->func, PCI_COMMAND);
    return (command & (PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER)) != 0;
}

unsigned long pci_resource_start(struct pci_dev *dev, int bar) {
    if (!dev || bar < 0 || bar > 5) return 0;
    return (unsigned long)pci_get_bar(dev->bus, dev->slot, dev->func, (uint8_t)bar);
}

void __iomem *pci_ioremap_bar(struct pci_dev *dev, int bar) {
    unsigned long start = pci_resource_start(dev, bar);
    if (!start) return NULL;
    return (void *)(uintptr_t)(start + hhdm_offset);
}

int pci_set_master(struct pci_dev *dev) {
    if (!dev) return -1;
    uint32_t command = pci_read_config(dev->bus, dev->slot, dev->func, PCI_COMMAND);
    command |= PCI_COMMAND_MASTER;
    uint32_t address = (uint32_t)((uint32_t)dev->bus << 16) | ((uint32_t)dev->slot << 11) |
                       ((uint32_t)dev->func << 8) | (PCI_COMMAND & 0xFC) | 0x80000000;
    outl(0xCF8, address);
    outl(0xCFC, command);
    return 0;
}

void pci_clear_master(struct pci_dev *dev) {
    if (!dev) return;
    uint32_t command = pci_read_config(dev->bus, dev->slot, dev->func, PCI_COMMAND);
    command &= ~PCI_COMMAND_MASTER;
    uint32_t address = (uint32_t)((uint32_t)dev->bus << 16) | ((uint32_t)dev->slot << 11) |
                       ((uint32_t)dev->func << 8) | (PCI_COMMAND & 0xFC) | 0x80000000;
    outl(0xCF8, address);
    outl(0xCFC, command);
}
