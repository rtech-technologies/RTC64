#include "hal.h"
#include "pro_os.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "serial.h"
#include <string.h>

static storage_device_t* devices[16];
static int count = 0;

int hal_storage_register_device(storage_device_t *d) {
    if (count < 16) {
        d->id = count;
        devices[count++] = d;
        serial_printf("[STORAGE] Registered device %d: %s (%llu blocks)\n", d->id, d->name, d->total_blocks);
        return 0;
    }
    return -1;
}

int hal_storage_get_device_count(void) {
    return count;
}

storage_device_t* hal_storage_get_device(int i) {
    if (i >= 0 && i < count) return devices[i];
    return NULL;
}

int hal_storage_read(int disk_id, uint64_t lba, void* buffer, uint32_t count_blocks) {
    if (disk_id >= 0 && disk_id < count) {
        storage_device_t* d = devices[disk_id];
        if (d && d->read) return d->read(d, lba, buffer, count_blocks);
    }
    return -1;
}

int hal_storage_write(int disk_id, uint64_t lba, const void* buffer, uint32_t count_blocks) {
    if (disk_id >= 0 && disk_id < count) {
        storage_device_t* d = devices[disk_id];
        if (d && d->write) return d->write(d, lba, buffer, count_blocks);
    }
    return -1;
}

void pci_scan(void);
void hal_storage_init(void) {
    count = 0;
    serial_write("[STORAGE] Initializing storage subsystem...\n");
    pci_scan();

    if (count == 0) {
        ramdisk_init();
    }
}

typedef struct {
    storage_device_t base;
    struct usbh_msc *msc;
} usb_dev_t;

static usb_dev_t usb_pool[4];
static int usb_ptr = 0;

static int usb_read_wrap(storage_device_t* d, uint64_t s, void* b, uint32_t c) {
    usb_dev_t* ud = (usb_dev_t*)d;
    return usbh_msc_scsi_read10(ud->msc, (uint32_t)s, b, c);
}

static int usb_write_wrap(storage_device_t* d, uint64_t s, const void* b, uint32_t c) {
    usb_dev_t* ud = (usb_dev_t*)d;
    return usbh_msc_scsi_write10(ud->msc, (uint32_t)s, (void*)b, c);
}

void usbh_msc_run(struct usbh_msc *msc_class) {
    if (usb_ptr >= 4) return;
    usb_dev_t *d = &usb_pool[usb_ptr++];
    d->msc = msc_class;
    d->base.name = "Genuine USB Disk";
    d->base.type = STORAGE_TYPE_USB;
    d->base.total_blocks = msc_class->blocknum;
    d->base.block_size = msc_class->blocksize;
    d->base.read = usb_read_wrap;
    d->base.write = (int (*)(storage_device_t *, uint64_t, const void *, uint32_t))usb_write_wrap;

    hal_storage_register_device(&d->base);
    extern void vfs_refresh_mounts(void);
    vfs_refresh_mounts();
}

void usbh_msc_stop(struct usbh_msc *m) {
    (void)m;
}

int hal_nvme_init(uint64_t mmio) {
    extern void nvme_init(uint64_t);
    nvme_init(mmio);
    // In a real implementation, nvme_init would return success/fail
    // and would register the device.
    return 0;
}

int hal_sata_init(uint64_t mmio) {
    extern void ahci_init(uint64_t);
    ahci_init(mmio);
    return 0;
}
