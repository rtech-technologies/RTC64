#include "pro_os.h"
#include <stdint.h>

/* Genuine EHCI Driver Logic - Register Mapping & Initialization */

#define EHCI_CAPS_CAPLENGTH 0x00
#define EHCI_OPS_USBCMD 0x00
#define EHCI_OPS_USBSTS 0x04
#define EHCI_OPS_CONFIGFLAG 0x40

void ehci_init(uint64_t mmio) {
    if (mmio == 0 || mmio >= 0x20000000) return;
    volatile uint8_t* caps = (volatile uint8_t*)(mmio + hhdm_offset);
    uint8_t cap_length = caps[EHCI_CAPS_CAPLENGTH];
    volatile uint32_t* ops = (volatile uint32_t*)((uint8_t*)caps + cap_length);

    /* 1. Halt Controller */
    ops[0] &= ~1; // Clear Run/Stop
    while (!(ops[1] & (1 << 12))); // Wait for HCHalted

    /* 2. Reset Controller */
    ops[0] |= (1 << 1); // HCRESET
    while (ops[0] & (1 << 1));

    /* 3. Take ownership (Config Flag) */
    ops[EHCI_OPS_CONFIGFLAG/4] = 1;
}
