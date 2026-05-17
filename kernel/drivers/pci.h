#ifndef PCI_H
#define PCI_H

#include <stdint.h>

void pci_scan(void);
uint64_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_index);

#endif
