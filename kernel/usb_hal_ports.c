#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "usb_util.h"
#include "usb_def.h"
#include "usb_dc.h"
#include "usbh_core.h"

/* Sovereign USB Porting Layer */

int usbd_ep_open(uint8_t busid, const struct usb_endpoint_descriptor *ep) { (void)busid; (void)ep; return 0; }
int usbd_ep_close(uint8_t busid, uint8_t ep) { (void)busid; (void)ep; return 0; }
uint8_t usbh_get_port_speed(struct usbh_bus *bus, const uint8_t port) { (void)bus; (void)port; return 3; /* USB_SPEED_HIGH */ }
uint8_t usbd_get_port_speed(uint8_t busid) { (void)busid; return 3; /* USB_SPEED_HIGH */ }
int usbd_set_address(uint8_t busid, uint8_t addr) { (void)busid; (void)addr; return 0; }
int usbd_ep_is_stalled(uint8_t busid, uint8_t ep, uint8_t *stalled) { (void)busid; (void)ep; (void)stalled; return 0; }
int usbd_ep_clear_stall(uint8_t busid, uint8_t ep) { (void)busid; (void)ep; return 0; }
int usbd_ep_set_stall(uint8_t busid, uint8_t ep) { (void)busid; (void)ep; return 0; }
int usbd_ep_start_read(uint8_t busid, uint8_t ep, uint8_t *buffer, uint32_t len) { (void)busid; (void)ep; (void)buffer; (void)len; return 0; }
void usbd_ep_start_read_without_zlp(uint8_t busid, uint8_t ep, uint8_t *buffer, uint32_t len) { (void)busid; (void)ep; (void)buffer; (void)len; }
int usbd_ep_start_write(uint8_t busid, uint8_t ep, const uint8_t *buffer, uint32_t len) { (void)busid; (void)ep; (void)buffer; (void)len; return 0; }
int usbd_set_remote_wakeup(uint8_t busid) { (void)busid; return 0; }
int usb_dc_init(uint8_t busid) { (void)busid; return 0; }
int usb_dc_deinit(uint8_t busid) { (void)busid; return 0; }

/* Linker symbols for hub and class discovery */
uintptr_t __usbh_class_info_start__ = 0;
uintptr_t __usbh_class_info_end__ = 0;

/* Core system logging */
int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    // In a real sovereign kernel, this would write to COM1 or Framebuffer
    va_end(args);
    return 0;
}

int snprintf(char *str, size_t size, const char *format, ...) {
    (void)str; (void)size; (void)format;
    return 0;
}
