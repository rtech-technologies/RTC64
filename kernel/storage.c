#include "hal.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include <string.h>

static storage_device_t* devices[16]; static int count = 0;
int hal_storage_register_device(storage_device_t *d) { if (count<16) { devices[count++] = d; return 0; } return -1; }
int hal_storage_get_device_count(void) { return count; }
storage_device_t* hal_storage_get_device(int i) { return (i>=0 && i<count) ? devices[i] : 0; }
int hal_storage_read(storage_device_t* d, uint64_t s, void* b, uint32_t c) { if (d && d->read) return d->read(d, s, b, c); return -1; }
int hal_storage_write(storage_device_t* d, uint64_t s, const void* b, uint32_t c) { if (d && d->write) return d->write(d, s, b, c); return -1; }
void hal_storage_init(void) { count = 0; }

typedef struct { storage_device_t base; struct usbh_msc *msc; } usb_dev_t;
static usb_dev_t usb_pool[4]; static int usb_ptr = 0;

static int usb_read_wrap(storage_device_t* d, uint64_t s, void* b, uint32_t c) {
    usb_dev_t* ud = (usb_dev_t*)d;
    return usbh_msc_scsi_read10(ud->msc, (uint32_t)s, b, c);
}
static int usb_write_wrap(storage_device_t* d, uint64_t s, const void* b, uint32_t c) {
    usb_dev_t* ud = (usb_dev_t*)d;
    return usbh_msc_scsi_write10(ud->msc, (uint32_t)s, b, c);
}

void usbh_msc_run(struct usbh_msc *msc_class) {
    if (usb_ptr >= 4) return;
    usb_dev_t *d = &usb_pool[usb_ptr++];
    d->msc = msc_class;
    d->base.name = "Genuine USB Disk"; d->base.type = STORAGE_TYPE_USB;
    d->base.total_blocks = msc_class->blocknum; d->base.block_size = msc_class->blocksize;
    d->base.read = usb_read_wrap; d->base.write = usb_write_wrap;
    hal_storage_register_device(&d->base);
    extern void vfs_refresh_mounts(void); vfs_refresh_mounts();
}
void usbh_msc_stop(struct usbh_msc *m) { (void)m; }
