#include <stdint.h>
#include <stddef.h>

void usbd_ep_open(void) {}
void usbd_ep_close(void) {}
void usbh_get_port_speed(void) {}
uint8_t usbd_get_port_speed(uint8_t busid) { (void)busid; return 0; }
void usbd_set_address(void) {}
void usbd_ep_is_stalled(void) {}
void usbd_ep_clear_stall(void) {}
void usbd_ep_set_stall(void) {}
void usbd_ep_start_read(void) {}
void usbd_ep_start_read_without_zlp(void) {}
void usbd_ep_start_write(void) {}
void usbd_set_remote_wakeup(void) {}
void usb_dc_init(void) {}
void usb_dc_deinit(void) {}
void usbh_hub_initialize(void) {}
void usbh_hub_deinitialize(void) {}
void usbh_hub_thread_wakeup(void) {}

/* Linker symbols for hub */
uintptr_t __usbh_class_info_start__ = 0;
uintptr_t __usbh_class_info_end__ = 0;

/* Minimal printf/snprintf for kernel */
int printf(const char *format, ...) {
    (void)format;
    return 0;
}

int snprintf(char *str, size_t size, const char *format, ...) {
    (void)str; (void)size; (void)format;
    return 0;
}
