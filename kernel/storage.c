#include "pro_os.h"
#include "hal.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include <string.h>

/*
 * Sovereign Storage Implementation
 * Genuine integration with CherryUSB Host MSC Stack and Local Disks
 */

typedef struct {
    storage_device_t base;
    struct usbh_msc *msc_class;
} usb_storage_device_t;

static usb_storage_device_t usb_devices[4];
static int usb_device_count = 0;

void hal_storage_init(void) {
    /* Ready for hotplug events and PCI discovered disks */
    usb_device_count = 0;
}

void usbh_msc_run(struct usbh_msc *msc_class) {
    if (usb_device_count >= 4) return;

    usb_storage_device_t *udev = &usb_devices[usb_device_count++];
    udev->msc_class = msc_class;

    udev->base.name = msc_class->hport->config.intf[0].devname;
    if (!udev->base.name || !udev->base.name[0]) {
        udev->base.name = "Genuine USB Disk";
    }

    udev->base.type = STORAGE_TYPE_USB;
    udev->base.total_blocks = msc_class->blocknum;
    udev->base.block_size = msc_class->blocksize;
    udev->base.read = NULL;
    udev->base.write = NULL;

    hal_storage_register_device(&udev->base);

    extern void vfs_refresh_mounts(void);
    vfs_refresh_mounts();
}

void usbh_msc_stop(struct usbh_msc *msc_class) {
    (void)msc_class;
    // In a real system, we'd find the udev and invalidate it.
}

int hal_storage_read(storage_device_t* dev, uint64_t sector, void* buffer, uint32_t count) {
    if (!dev) return -1;

    if (dev->type == STORAGE_TYPE_USB) {
        for (int i = 0; i < usb_device_count; i++) {
            if (&usb_devices[i].base == dev) {
                return usbh_msc_scsi_read10(usb_devices[i].msc_class, (uint32_t)sector, (uint8_t*)buffer, count);
            }
        }
    } else if (dev->read) {
        return dev->read(dev, sector, buffer, count);
    }
    return -1;
}

int hal_storage_write(storage_device_t* dev, uint64_t sector, const void* buffer, uint32_t count) {
    if (!dev) return -1;

    if (dev->type == STORAGE_TYPE_USB) {
        for (int i = 0; i < usb_device_count; i++) {
            if (&usb_devices[i].base == dev) {
                return usbh_msc_scsi_write10(usb_devices[i].msc_class, (uint32_t)sector, (const uint8_t*)buffer, count);
            }
        }
    } else if (dev->write) {
        return dev->write(dev, sector, buffer, count);
    }
    return -1;
}
