#ifndef LINUX_PCI_H
#define LINUX_PCI_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef __iomem
#define __iomem
#endif

#define PCI_ANY_ID 0xffff

struct pci_dev {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint16_t vendor;
    uint16_t device;
    uint16_t subsystem_vendor;
    uint16_t subsystem_device;
    uint8_t class;
    uint8_t subclass;
    uint8_t prog_if;
    unsigned long resource[6];
    void *sysdata;
};

typedef struct pci_dev pci_dev_t;

struct pci_device_id {
    uint16_t vendor;
    uint16_t device;
    uint16_t subvendor;
    uint16_t subdevice;
    uint32_t class;
    unsigned long driver_data;
};

uint64_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_index);

struct pci_driver {
    const char *name;
    const struct pci_device_id *id_table;
    int (*probe)(struct pci_dev *dev, const struct pci_device_id *id);
    void (*remove)(struct pci_dev *dev);
};

int pci_enable_device(struct pci_dev *dev);
void pci_disable_device(struct pci_dev *dev);
bool pci_is_enabled(struct pci_dev *dev);
unsigned long pci_resource_start(struct pci_dev *dev, int bar);
void *__iomem pci_ioremap_bar(struct pci_dev *dev, int bar);
int pci_set_master(struct pci_dev *dev);
void pci_clear_master(struct pci_dev *dev);

static inline uint32_t pci_domain_nr(struct pci_dev *pdev) { (void)pdev; return 0; }
static inline uint8_t pci_bus_number(struct pci_dev *pdev) { return pdev->bus; }
static inline uint8_t pci_slot_number(struct pci_dev *pdev) { return pdev->slot; }
static inline uint8_t pci_devfn(struct pci_dev *pdev) { return (pdev->slot << 3) | pdev->func; }

#endif
