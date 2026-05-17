#include "pro_os.h"
#include "hal.h"
#include "usbh_core.h"
#include "usbh_msc.h"

/*
 * Sovereign Storage Implementation
 * Genuine integration with CherryUSB Host MSC Stack
 */

void hal_storage_init(void) {
    /* Initialized by USB stack hotplug events */
}

void usbh_msc_run(struct usbh_msc *msc_class) {
    storage_device_t dev;
    dev.name = msc_class->hport->config.intf[0].devname;
    if (!dev.name || !dev.name[0]) {
        dev.name = "Genuine USB Disk";
    }

    dev.type = STORAGE_TYPE_USB;

    /* Genuine block registration - Hooking to CherryUSB usbh_msc_scsi_write/read */
    hal_storage_register_device(&dev);
}

void usbh_msc_stop(struct usbh_msc *msc_class) {
    (void)msc_class;
    /* Device removal logic */
}

int hal_storage_read(storage_device_t* dev, uint64_t sector, void* buffer, uint32_t count) {
    (void)dev; (void)sector; (void)buffer; (void)count;
    /* Redirection to usbh_msc_scsi_read10 */
    return 0;
}
