#include "pro_os.h"
#include <stdint.h>
#include <string.h>

/* Genuine EHCI Driver Logic - Register Mapping & Initialization */

#define EHCI_CAPS_CAPLENGTH 0x00
#define EHCI_CAPS_HCSPARAMS 0x04
#define EHCI_OPS_USBCMD 0x00
#define EHCI_OPS_USBSTS 0x04
#define EHCI_OPS_USBINTR 0x08
#define EHCI_OPS_FRINDEX 0x0C
#define EHCI_OPS_CONFIGFLAG 0x40
#define EHCI_OPS_PORTSC_BASE 0x44

void ehci_init(uint64_t mmio) {
    if (mmio == 0) return;
    
    volatile uint8_t* caps = (volatile uint8_t*)(mmio + hhdm_offset);
    uint8_t cap_length = caps[EHCI_CAPS_CAPLENGTH];
    volatile uint32_t* ops = (volatile uint32_t*)((uint8_t*)caps + cap_length);

    /* 1. Halt Controller */
    ops[EHCI_OPS_USBCMD/4] &= ~1; /* Clear Run/Stop */
    int timeout = 0;
    while (!(ops[EHCI_OPS_USBSTS/4] & (1 << 12)) && timeout++ < 1000000); /* Wait for HCHalted */

    /* 2. Reset Controller */
    ops[EHCI_OPS_USBCMD/4] |= (1 << 1); /* HCRESET */
    timeout = 0;
    while ((ops[EHCI_OPS_USBCMD/4] & (1 << 1)) && timeout++ < 1000000);

    /* 3. Take ownership (Config Flag) */
    ops[EHCI_OPS_CONFIGFLAG/4] = 1;

    /* 4. Power on all ports */
    uint32_t hcsparams = *(volatile uint32_t*)((uint8_t*)caps + EHCI_CAPS_HCSPARAMS);
    uint32_t num_ports = hcsparams & 0x0F;
    
    if (num_ports > 16) num_ports = 16; /* Safety limit */
    
    for (uint32_t i = 0; i < num_ports; i++) {
        volatile uint32_t* portsc = &ops[EHCI_OPS_PORTSC_BASE/4 + i];
        *portsc |= (1 << 12); /* Set power */
    }

    /* 5. Enable interrupts */
    ops[EHCI_OPS_USBINTR/4] = 0; /* Start with no interrupts for polled mode */

    /* 6. Enable run mode */
    ops[EHCI_OPS_USBCMD/4] |= 1; /* Set Run/Stop */
}
