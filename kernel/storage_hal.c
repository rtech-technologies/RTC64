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
void hal_nvme_init(void) {}
void hal_sata_init(void) {}

void usbh_msc_run(struct usbh_msc *msc_class) {
    static storage_device_t usb_devs[4]; static int usb_count = 0;
    if (usb_count >= 4) return;
    storage_device_t *d = &usb_devs[usb_count++];
    d->name = "Genuine USB Disk"; d->type = STORAGE_TYPE_USB;
    d->total_blocks = msc_class->blocknum; d->block_size = msc_class->blocksize;
    // For simplicity in this refactor, I'll use a wrapper if needed, but we'll see
    hal_storage_register_device(d);
}
void usbh_msc_stop(struct usbh_msc *m) { (void)m; }
