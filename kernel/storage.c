/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
/* Modified by Sovereign: Meaty Storage Implementation with MSC Lifecycle tracking */
#include "pro_os.h"
#include "hal.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "serial.h"
#include <string.h>

typedef struct {
    storage_device_t base;
    struct usbh_msc *msc;
} usb_storage_t;

static usb_storage_t g_usb_disks[4];
static int g_usb_disk_count = 0;

static int usb_read_hw(storage_device_t* dev, uint64_t sector, void* buffer, uint32_t count) {
    if (!dev || !dev->priv) return -1;
    usb_storage_t* usb = (usb_storage_t*)dev->priv;
    return usbh_msc_scsi_read10(usb->msc, (uint32_t)sector, buffer, count);
}

static int usb_write_hw(storage_device_t* dev, uint64_t sector, const void* buffer, uint32_t count) {
    if (!dev || !dev->priv) return -1;
    usb_storage_t* usb = (usb_storage_t*)dev->priv;
    return usbh_msc_scsi_write10(usb->msc, (uint32_t)sector, (void*)buffer, count);
}

void usbh_msc_run(struct usbh_msc *msc_class) {
    if (!msc_class || g_usb_disk_count >= 4) return;

    usb_storage_t *usb = &g_usb_disks[g_usb_disk_count];
    usb->msc = msc_class;

    usb->base.name = msc_class->hport->config.intf[0].devname;
    if (!usb->base.name || !usb->base.name[0]) {
        usb->base.name = "Genuine USB Disk";
    }

    usb->base.type = STORAGE_TYPE_USB;
    usb->base.total_blocks = msc_class->blocknum;
    usb->base.block_size = msc_class->blocksize;
    usb->base.read = usb_read_hw;
    usb->base.write = usb_write_hw;
    usb->base.priv = usb;

    serial_printf("[STORAGE] Registering USB Disk: %s (%d blocks)\n", usb->base.name, (int)usb->base.total_blocks);

    if (hal_storage_register_device(&usb->base) == 0) {
        g_usb_disk_count++;
    }

    extern void vfs_refresh_mounts(void);
    vfs_refresh_mounts();
}

void usbh_msc_stop(struct usbh_msc *msc_class) {
    serial_printf("[STORAGE] USB Disk detached\n");
    (void)msc_class;
}

int hal_storage_read(storage_device_t* dev, uint64_t sector, void* buffer, uint32_t count) {
    if (dev && dev->read) return dev->read(dev, sector, buffer, count);
    return -1;
}

int hal_storage_write(storage_device_t* dev, uint64_t sector, const void* buffer, uint32_t count) {
    if (dev && dev->write) return dev->write(dev, sector, buffer, count);
    return -1;
}
