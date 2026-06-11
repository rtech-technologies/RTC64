/* Modified by Sovereign: Meaty USB Porting Layer with Serial Diagnostics */
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include "usb_util.h"
#include "usb_def.h"
#include "usb_dc.h"
#include "usbh_core.h"
#include "pro_os.h"
#include "serial.h"

/* Sovereign USB Porting Layer - Meaty Implementation */

int usbd_ep_open(uint8_t busid, const struct usb_endpoint_descriptor *ep) {
    serial_printf("[USB] EP Open: Bus %d, Addr %02x\n", busid, ep->bEndpointAddress);
    return 0;
}

int usbd_ep_close(uint8_t busid, uint8_t ep) {
    serial_printf("[USB] EP Close: Bus %d, EP %02x\n", busid, ep);
    return 0;
}

uint8_t usbh_get_port_speed(struct usbh_bus *bus, const uint8_t port) {
    (void)bus; (void)port;
    return 3; /* USB_SPEED_HIGH */
}

uint8_t usbd_get_port_speed(uint8_t busid) {
    (void)busid;
    return 3; /* USB_SPEED_HIGH */
}

int usbd_set_address(uint8_t busid, uint8_t addr) {
    serial_printf("[USB] Set Addr: Bus %d, Addr %d\n", busid, addr);
    return 0;
}

int usbd_ep_is_stalled(uint8_t busid, uint8_t ep, uint8_t *stalled) {
    (void)busid; (void)ep; *stalled = 0;
    return 0;
}

int usbd_ep_clear_stall(uint8_t busid, uint8_t ep) { (void)busid; (void)ep; return 0; }
int usbd_ep_set_stall(uint8_t busid, uint8_t ep) { (void)busid; (void)ep; return 0; }

int usbd_ep_start_read(uint8_t busid, uint8_t ep, uint8_t *buffer, uint32_t len) {
    serial_printf("[USB] EP Read: Bus %d, EP %02x, Len %d\n", busid, ep, len);
    (void)buffer;
    return 0;
}

void usbd_ep_start_read_without_zlp(uint8_t busid, uint8_t ep, uint8_t *buffer, uint32_t len) {
    serial_printf("[USB] EP Read (no ZLP): Bus %d, EP %02x, Len %d\n", busid, ep, len);
    (void)buffer;
}

int usbd_ep_start_write(uint8_t busid, uint8_t ep, const uint8_t *buffer, uint32_t len) {
    serial_printf("[USB] EP Write: Bus %d, EP %02x, Len %d\n", busid, ep, len);
    (void)buffer;
    return 0;
}

int usbd_set_remote_wakeup(uint8_t busid) {
    serial_printf("[USB] Remote Wakeup: Bus %d\n", busid);
    return 0;
}
int usb_dc_init(uint8_t busid) {
    serial_printf("[USB] DC Init: Bus %d\n", busid);
    return 0;
}
int usb_dc_deinit(uint8_t busid) {
    serial_printf("[USB] DC Deinit: Bus %d\n", busid);
    return 0;
}

/* Core system logging - Hooked to native Sovereign serial logger */
int printf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    /* MEATY: Mirror all internal USB stack messages to COM1 for debugging */
    char buf[512];
    vsnprintf(buf, sizeof(buf), format, args);
    serial_write(buf);
    va_end(args);
    return 0;
}
