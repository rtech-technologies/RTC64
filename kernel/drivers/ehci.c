#include "pro_os.h"
#include <stdint.h>
#include "serial.h"

/* Genuine EHCI Driver Logic - Functional Register Interaction */

#define EHCI_CAPS_CAPLENGTH 0x00
#define EHCI_OPS_USBCMD 0x00
#define EHCI_OPS_USBSTS 0x04
#define EHCI_OPS_CTRLDSSEGMENT 0x08
#define EHCI_OPS_PERIODICLISTBASE 0x14
#define EHCI_OPS_ASYNCLISTADDR 0x18
#define EHCI_OPS_CONFIGFLAG 0x40

void ehci_init(uint64_t mmio) {
    if (mmio == 0) return;
    volatile uint8_t* caps = (volatile uint8_t*)(mmio + hhdm_offset);
    uint8_t cap_length = caps[EHCI_CAPS_CAPLENGTH];
    volatile uint32_t* ops = (volatile uint32_t*)((uint8_t*)caps + cap_length);

    serial_write("[EHCI] Initializing Controller...\n");

    /* 1. Halt Controller */
    ops[EHCI_OPS_USBCMD/4] &= ~1; // Clear Run/Stop
    while (!(ops[EHCI_OPS_USBSTS/4] & (1 << 12))) __asm__("pause"); // Wait for HCHalted

    /* 2. Reset Controller */
    ops[EHCI_OPS_USBCMD/4] |= (1 << 1); // HCRESET
    while (ops[EHCI_OPS_USBCMD/4] & (1 << 1)) __asm__("pause");

    /* 3. Set Control Data Structure Segment (for 64-bit kernels) */
    ops[EHCI_OPS_CTRLDSSEGMENT/4] = 0; // Sovereign uses 32-bit physical addressing below 4GB for DMA

    /* 4. Take ownership (Config Flag) */
    ops[EHCI_OPS_CONFIGFLAG/4] = 1;

    /* 5. Start Controller */
    ops[EHCI_OPS_USBCMD/4] |= 1;
    serial_write("[EHCI] Controller Ready.\n");
}
