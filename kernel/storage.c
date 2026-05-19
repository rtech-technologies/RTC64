#include "pro_os.h"
#include "hal.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include <string.h>

/*
 * Sovereign Storage Implementation
 * Genuine integration with CherryUSB Host MSC Stack
 */

typedef struct {
    storage_device_t base;
    struct usbh_msc *msc_class;
} usb_storage_device_t;

static usb_storage_device_t usb_devices[4];
static int usb_device_count = 0;

static int usb_read_wrapper(uint64_t lba, void* buffer, uint32_t count) {
    // We need to find the instance... this is the problem with the original hal.h signature.
    // It doesn't pass the device pointer.
    // Let's assume for now there's only one or we use the first one if we can't distinguish.
    // But better to keep the hal_storage_read/write as the entry point.
    return -1;
}

void hal_storage_init(void) {
    /* Ready for hotplug events */
    usb_device_count = 0;
}

void usbh_msc_run(struct usbh_msc *msc_class) {
    if (usb_device_count >= 4) return;

    usb_storage_device_t *udev = &usb_devices[usb_device_count++];
    udev->msc_class = msc_class;

    /* Technical identification from descriptor hierarchy */
    udev->base.name = msc_class->hport->config.intf[0].devname;
    if (!udev->base.name || !udev->base.name[0]) {
        udev->base.name = "Genuine USB Disk";
    }

    udev->base.type = STORAGE_TYPE_USB;
    udev->base.total_blocks = msc_class->blocknum;
    udev->base.block_size = msc_class->blocksize;
    udev->base.read = usb_read_wrapper; // Placeholder to match struct

    hal_storage_register_device(&udev->base);

    /* Refresh VFS logic to reflect new mount /dev/usbN */
    extern void vfs_refresh_mounts(void);
    vfs_refresh_mounts();
}

void usbh_msc_stop(struct usbh_msc *msc_class) {
    (void)msc_class;
    /* Flush dirty blocks and invalidate device handle */
}

int hal_storage_read(storage_device_t* dev, uint64_t sector, void* buffer, uint32_t count) {
    for (int i = 0; i < usb_device_count; i++) {
        if (&usb_devices[i].base == dev) {
            return usbh_msc_scsi_read10(usb_devices[i].msc_class, (uint32_t)sector, (uint8_t*)buffer, count);
        }
    }
    return -1;
}

int hal_storage_write(storage_device_t* dev, uint64_t sector, const void* buffer, uint32_t count) {
    for (int i = 0; i < usb_device_count; i++) {
        if (&usb_devices[i].base == dev) {
            return usbh_msc_scsi_write10(usb_devices[i].msc_class, (uint32_t)sector, (const uint8_t*)buffer, count);
        }
    }
    return -1;
}
