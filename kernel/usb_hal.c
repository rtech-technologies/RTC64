/* Modified by Sovereign: functional USB operation in polled mode */
#include "hal.h"
#include "pro_os.h"
#include "usbh_core.h"
#include "usbh_hid.h"
#include "usbh_msc.h"

extern uint64_t xhci_mmio_base;
extern void USBH_IRQHandler(uint8_t busid);

void hal_usb_init(void) {
    if (xhci_mmio_base != 0) {
        usbh_initialize(0, xhci_mmio_base + hhdm_offset, NULL);
    } else if (ehci_mmio_base != 0) {
        usbh_initialize(0, ehci_mmio_base + hhdm_offset, NULL);
    }
}

void hal_usb_poll(void) {
    /* MEATY: Drive CherryUSB stack in polled mode by manually invoking IRQ handler */
    /* This allows USB to function before the IDT is fully configured for hardware IRQs */
    USBH_IRQHandler(0);
}
