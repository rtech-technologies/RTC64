#include "hal.h"
#include "usbh_core.h"
#include "usbh_hid.h"
#include "usbh_msc.h"

void hal_usb_init(void) {
    usbh_initialize(0, 0x3F8, NULL); // Sovereign IO base for EHCI/XHCI in QEMU
}

void hal_usb_poll(void) {
    // CherryUSB background tasks if not using threads
}

/* Callbacks from CherryUSB for HID devices */
void usbh_hid_callback(struct usbh_hid *hid_class, uint8_t event) {
    // Handle Keyboard/Mouse events and push to hal_input_push_event
}

/* Callbacks from CherryUSB for MSC (Storage) devices */
void usbh_msc_run(struct usbh_msc *msc_class) {
    static storage_device_t msc_dev;
    msc_dev.name = "USB Drive";
    msc_dev.type = STORAGE_TYPE_USB;
    // Register to HAL storage
    hal_storage_register_device(&msc_dev);
}

void usbh_msc_stop(struct usbh_msc *msc_class) {
    // Handle removal
}
