#ifndef LINUX_COMPAT_H
#define LINUX_COMPAT_H

#include "linux/types.h"
#include "linux/kernel.h"
#include "linux/init.h"
#include "linux/module.h"
#include "linux/slab.h"
#include "linux/pci.h"
#include "linux/pci_regs.h"
#include "linux/interrupt.h"
#include "linux/dma-mapping.h"
#include "linux/errno.h"
#include "linux/netdevice.h"
#include "linux/skbuff.h"
#include "linux/etherdevice.h"

int pci_register_driver(struct pci_driver *driver);
int pci_unregister_driver(struct pci_driver *driver);
void linux_compat_probe_pci_device(uint8_t bus, uint8_t slot, uint8_t func,
                                  uint16_t vendor, uint16_t device,
                                  uint16_t subsystem_vendor, uint16_t subsystem_device,
                                  uint8_t class, uint8_t subclass, uint8_t prog_if);
int linux_compat_init(void);

#endif
