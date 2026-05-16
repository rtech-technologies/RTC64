#include "pro_os.h"
#include <stdint.h>

/* Sovereign xHCI Driver - Native implementation */

#define XHCI_REG_USBSTS 0x04
#define XHCI_REG_USBSTS_HCH 0x01

void xhci_init(uint64_t base_addr) {
    volatile uint32_t* regs = (volatile uint32_t*)(base_addr + hhdm_offset);

    /* 1. Basic Hardware Reset */
    regs[0] |= (1 << 1); // Reset bit
    while(regs[0] & (1 << 1));

    /* 2. Configure Operational Registers */
    // PRO_TASK: Initialize DCBAAP, CONFIG, and Event Rings
}

void xhci_poll(uint64_t base_addr) {
    volatile uint32_t* regs = (volatile uint32_t*)(base_addr + hhdm_offset);
    if (regs[XHCI_REG_USBSTS/4] & XHCI_REG_USBSTS_HCH) {
        // PRO_REFINE: Handle Host Controller Halt
    }
}
