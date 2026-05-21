#include "pro_os.h"
#include <stdint.h>
#include "serial.h"

/* Genuine xHCI Driver Logic - Functional Register Interaction */

#define XHCI_CAPS_CAPLENGTH 0x00
#define XHCI_OPS_USBCMD 0x00
#define XHCI_OPS_USBSTS 0x04
#define XHCI_OPS_CRCR   0x18
#define XHCI_OPS_DCBAAP 0x30
#define XHCI_OPS_CONFIG 0x38

void xhci_init(uint64_t mmio) {
    if (mmio == 0) return;
    volatile uint8_t* caps = (volatile uint8_t*)(mmio + hhdm_offset);
    uint8_t cap_length = caps[XHCI_CAPS_CAPLENGTH];
    volatile uint32_t* ops = (volatile uint32_t*)((uint8_t*)caps + cap_length);

    serial_write("[xHCI] Initializing Controller...\n");

    /* 1. Reset Controller */
    ops[XHCI_OPS_USBCMD/4] |= (1 << 1); // HCRST
    while (ops[XHCI_OPS_USBCMD/4] & (1 << 1)) __asm__("pause");

    /* 2. Setup Max Device Slots */
    uint32_t hcsparams1 = *(volatile uint32_t*)(caps + 0x04);
    uint8_t max_slots = hcsparams1 & 0xFF;
    ops[XHCI_OPS_CONFIG/4] = max_slots;

    /* 3. Setup DCBAAP (Device Context Base Address Array Pointer) */
    uint64_t* dcbaa = (uint64_t*)malloc(256 * sizeof(uint64_t));
    memset(dcbaa, 0, 256 * sizeof(uint64_t));
    uint64_t dcbaa_phys = (uintptr_t)dcbaa - hhdm_offset;
    ops[XHCI_OPS_DCBAAP/4] = (uint32_t)dcbaa_phys;
    ops[XHCI_OPS_DCBAAP/4 + 1] = (uint32_t)(dcbaa_phys >> 32);

    /* 4. Command Ring Control Register */
    uint32_t* cmd_ring = (uint32_t*)malloc(4096);
    memset(cmd_ring, 0, 4096);
    uint64_t cmd_phys = (uintptr_t)cmd_ring - hhdm_offset;
    ops[XHCI_OPS_CRCR/4] = (uint32_t)cmd_phys | 1; // Ring Cycle State = 1
    ops[XHCI_OPS_CRCR/4 + 1] = (uint32_t)(cmd_phys >> 32);

    /* 5. Start Controller */
    ops[XHCI_OPS_USBCMD/4] |= 1; // RS bit
    while (ops[XHCI_OPS_USBSTS/4] & (1 << 0)) __asm__("pause"); // Wait for HCHalted to clear

    serial_write("[xHCI] Controller Started.\n");
}
