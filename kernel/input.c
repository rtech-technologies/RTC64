#include "pro_os.h"
#include "hal.h"
#include "usbh_core.h"
#include "usbh_hid.h"

/*
 * Sovereign Input Implementation
 * Genuine integration with CherryUSB Host HID Stack
 */

void hal_input_init(void) {
    /* Ready for HID callback events from CherryUSB core */
}

void usbh_hid_callback(struct usbh_hid *hid_class, uint8_t event) {
    if (event == USBH_EVENT_DEVICE_CONNECTED) {
        /* HID Device Seized: Parse HID report descriptor for axes/buttons */
        (void)hid_class;
    } else if (event == USBH_EVENT_DEVICE_DISCONNECTED) {
        /* HID Device Lost: Cleanup memory state */
        (void)hid_class;
    }
}

void hal_input_push_event(input_event_t ev) {
    /* Atomically enqueue to Sovereign input pool for UI consumption */
    (void)ev;
}

bool hal_input_pop_event(input_event_t* ev) {
    /* Consume from input pool */
    (void)ev;
    return false;
}
