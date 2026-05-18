#include <pro_os.h>

// PCI / XHCI Register offsets
#define XHCI_CAPLENGTH 0x00
#define XHCI_HCCPARAMS1 0x10
#define XHCI_DBOFF 0x14
#define XHCI_RTSOFF 0x18

void xhci_pci_init() {
    // 1. Scan PCI bus for class 0x0C subclass 0x03 prog 0x30
    // 2. Map BAR0
    // 3. BIOS Handover

    // Placeholder for actual MMIO logic
    // volatile uint32_t* base = ...;
    // uint8_t cap_len = *(uint8_t*)((uint64_t)base + XHCI_CAPLENGTH);
}
