#include <stdint.h>
#include <stddef.h>
#include "pro_os.h"

/* Sovereign xHCI USB Host Controller Driver */

typedef struct {
    uint64_t mmio_base;
    uint32_t page_size;
} xhci_controller_t;

void xhci_init(uint64_t mmio) {
    xhci_controller_t ctrl;
    ctrl.mmio_base = mmio;

    /* Map MMIO BAR and initialize Operational Registers */
    uint32_t *cap_regs = (uint32_t*)mmio;
    uint8_t cap_length = cap_regs[0] & 0xFF;
    uint32_t *op_regs = (uint32_t*)(mmio + cap_length);

    /* Reset Controller */
    op_regs[0] |= 0x2; /* USBCMD Reset */
    while (op_regs[0] & 0x2);

    /* Configure Max Device Slots */
    uint32_t config = op_regs[14]; /* CONFIG */
    op_regs[14] = (config & ~0xFF) | 32;
}
