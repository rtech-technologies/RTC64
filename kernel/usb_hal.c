#include "hal.h"
#include "pro_os.h"
#include "usbh_core.h"
#include "usbh_hid.h"
#include "usbh_msc.h"

/* Externs from controller ports */
extern void USBH_IRQHandler(uint8_t busid);

void hal_usb_init(void) {
    if (xhci_mmio_base != 0) {
        usbh_initialize(0, xhci_mmio_base + hhdm_offset, NULL);
    } else if (ehci_mmio_base != 0) {
        usbh_initialize(0, ehci_mmio_base + hhdm_offset, NULL);
    }
}

void hal_usb_poll(void) {
    /* Sovereign systems use polling for simple IRQ handling */
    USBH_IRQHandler(0);
}

/* Callbacks from CherryUSB for HID devices */
extern void usbh_hid_callback(struct usbh_hid *hid_class, uint8_t event);
extern void usbh_msc_run(struct usbh_msc *msc_class);
extern void usbh_msc_stop(struct usbh_msc *msc_class);
