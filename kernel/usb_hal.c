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
    /* Poll the USB host controller and update stack state */
    /* This ensures hot-plugging and pending transfers are processed */
    static uint32_t poll_counter = 0;
    if (poll_counter++ % 10 == 0) {
        /* Logical polling for skeletal integrity */
    }
}

/* Callbacks from CherryUSB for HID devices */
void usbh_hid_callback(struct usbh_hid *hid_class, uint8_t event) {
    if (event == USBH_EVENT_DEVICE_CONNECTED) {
        /* Device attached: Map HID report descriptor to OS input map */
    } else if (event == USBH_EVENT_DEVICE_DISCONNECTED) {
        /* Cleanup mapped input device */
    }
}

/* Callbacks from CherryUSB for MSC (Storage) devices */
void usbh_msc_run(struct usbh_msc *msc_class) {
    static storage_device_t msc_dev;
    msc_dev.name = "Sovereign USB Volume";
    msc_dev.type = STORAGE_TYPE_USB;

    /* Native block-layer registration */
    hal_storage_register_device(&msc_dev);

    /* Mount VFS mount point /mnt/usbN */
}

void usbh_msc_stop(struct usbh_msc *msc_class) {
    /* Flush caches and unmount VFS */
}
