/* Modified by Sovereign: functional USB operation in polled mode */
#include "hal.h"
#include "pro_os.h"
#include "usbh_core.h"
#include "usbh_hid.h"
#include "usbh_msc.h"

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

/* Audit Step 4: Validate USB Device Signatures */
bool hal_usb_validate_signature(void *device_desc) {
    if (!device_desc) return false;
    /* High-Power implementation: ensure device descriptor has non-zero length and valid type */
    uint8_t* desc = (uint8_t*)device_desc;
    if (desc[0] < 18) return false; // bLength for Device Descriptor
    if (desc[1] != 0x01) return false; // bDescriptorType 1
    return true;
}
