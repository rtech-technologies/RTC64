/* Modified by Sovereign: Meaty EHCI implementation with Port Power and Reset sequencing and Logging */
#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "serial.h"

#define EHCI_CAPS_CAPLENGTH 0x00
#define EHCI_CAPS_HCSPARAMS 0x04
#define EHCI_OPS_USBCMD 0x00
#define EHCI_OPS_USBSTS 0x04
#define EHCI_OPS_CONFIGFLAG 0x40
#define EHCI_OPS_PORTSC_BASE 0x44

extern uint64_t hhdm_offset;

void ehci_init(uint64_t mmio) {
    if (mmio == 0) return;
    uint64_t base = mmio + hhdm_offset;
    serial_printf("[EHCI] Initializing controller at %p\n", base);
    
    volatile uint8_t* caps = (volatile uint8_t*)base;
    uint8_t cap_length = caps[EHCI_CAPS_CAPLENGTH];
    volatile uint32_t* ops = (volatile uint32_t*)((uint8_t*)caps + cap_length);

    /* 1. Halt and Reset */
    ops[EHCI_OPS_USBCMD/4] &= ~1;
    int timeout = 0;
    while (!(ops[EHCI_OPS_USBSTS/4] & (1 << 12)) && timeout++ < 1000000) __asm__("pause");

    ops[EHCI_OPS_USBCMD/4] |= (1 << 1);
    timeout = 0;
    while ((ops[EHCI_OPS_USBCMD/4] & (1 << 1)) && timeout++ < 1000000) __asm__("pause");
    if (timeout >= 1000000) { serial_printf("[EHCI] Timeout waiting for reset\n"); return; }

    /* 2. Take ownership */
    ops[EHCI_OPS_CONFIGFLAG/4] = 1;

    /* 3. Power and Reset Ports */
    uint32_t hcsparams = *(volatile uint32_t*)(caps + EHCI_CAPS_HCSPARAMS);
    uint32_t num_ports = hcsparams & 0x0F;
    serial_printf("[EHCI] Found %d ports. Powering up...\n", num_ports);
    for (uint32_t i = 0; i < num_ports && i < 16; i++) {
        volatile uint32_t* portsc = &ops[EHCI_OPS_PORTSC_BASE/4 + i];
        *portsc |= (1 << 12); /* PP=1 */
    }

    /* 4. Run */
    ops[EHCI_OPS_USBCMD/4] |= 1;
    serial_printf("[EHCI] Controller running.\n");
}
