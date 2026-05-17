#include "pro_os.h"
#include <stdint.h>

/* Sovereign AHCI Driver - Native implementation */

#define AHCI_GHC_HR (1 << 0)
#define AHCI_GHC_IE (1 << 1)
#define AHCI_GHC_AE (1 << 31)

void ahci_init(uint64_t base_addr) {
    if (base_addr == 0) return;
    volatile uint32_t* regs = (volatile uint32_t*)(base_addr + hhdm_offset);
    (void)regs;
    /* 1. Enable AHCI mode and Reset */
    // PRO_TASK: Implement safe AHCI initialization

    /* 2. Probe Ports */
    uint32_t pi = regs[0x0C/4]; // Port Implemented
    for (int i = 0; i < 32; i++) {
        if (pi & (1 << i)) {
            // PRO_TASK: Initialize port i
        }
    }
}
