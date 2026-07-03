#ifndef LINUX_PCI_REGS_H
#define LINUX_PCI_REGS_H

#include <stdint.h>

#define PCI_BASE_ADDRESS_0 0x10
#define PCI_VENDOR_ID       0x00
#define PCI_DEVICE_ID       0x02
#define PCI_COMMAND        0x04
#define PCI_STATUS         0x06
#define PCI_REVISION_ID    0x08
#define PCI_CLASS_REVISION 0x08
#define PCI_HEADER_TYPE    0x0C
#define PCI_SUBSYSTEM_VENDOR_ID 0x2C
#define PCI_SUBSYSTEM_ID   0x2E

#define PCI_COMMAND_IO      0x1
#define PCI_COMMAND_MEMORY  0x2
#define PCI_COMMAND_MASTER  0x4

#endif
