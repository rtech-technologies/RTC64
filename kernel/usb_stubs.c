#include <stdint.h>
#include <stddef.h>

void usbd_ep_open(void) {}
void usbd_ep_close(void) {}
void usbd_get_port_speed(void) {}
void usbd_set_address(void) {}
void usbd_ep_is_stalled(void) {}
void usbd_ep_clear_stall(void) {}
void usbd_ep_set_stall(void) {}
void usbd_ep_start_read(void) {}
void usbd_ep_start_write(void) {}
void usbd_set_remote_wakeup(void) {}
void usb_dc_init(void) {}
void usb_dc_deinit(void) {}
void usbh_kill_urb(void) {}
void usbh_hub_initialize(void) {}
void usbh_hub_deinitialize(void) {}
void usbh_submit_urb(void) {}

/* Linker symbols for hub */
uintptr_t __usbh_class_info_start__ = 0;
uintptr_t __usbh_class_info_end__ = 0;
