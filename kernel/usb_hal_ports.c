/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include "serial.h"
#include "usbh_core.h"
#include "usbd_core.h"

void usb_assert(const char* file, int line) {
    serial_printf("USB ASSERT: %s:%d\n", file, line);
    kpanic("USB_ASSERT");
}

/* Porting layer for CherryUSB Host */
void usbh_otg_init(uint8_t busid) { (void)busid; }
void usb_hc_low_level_init(struct usbh_bus *bus) { (void)bus; serial_printf("[USB] Low level HC init\n"); }
void usb_hc_low_level_deinit(struct usbh_bus *bus) { (void)bus; }
uint8_t usbh_get_port_speed(struct usbh_bus *bus, const uint8_t port) { (void)bus; (void)port; return USB_SPEED_HIGH; }
int usbh_reset_port(struct usbh_bus *bus, const uint8_t port) { (void)bus; (void)port; return 0; }

/* Porting layer for CherryUSB Device (stubs to satisfy usbd_core link) */
int usbd_ep_open(uint8_t busid, const struct usb_endpoint_descriptor *ep) { (void)busid; (void)ep; return 0; }
int usbd_ep_close(uint8_t busid, uint8_t ep) { (void)busid; (void)ep; return 0; }
int usbd_ep_set_stall(uint8_t busid, uint8_t ep) { (void)busid; (void)ep; return 0; }
int usbd_ep_clear_stall(uint8_t busid, uint8_t ep) { (void)busid; (void)ep; return 0; }
int usbd_ep_is_stalled(uint8_t busid, uint8_t ep, uint8_t *stalled) { (void)busid; (void)ep; (void)stalled; return 0; }
int usbd_ep_start_write(uint8_t busid, uint8_t ep, const uint8_t *data, uint32_t data_len) { (void)busid; (void)ep; (void)data; (void)data_len; return 0; }
int usbd_ep_start_read(uint8_t busid, uint8_t ep, uint8_t *data, uint32_t data_len) { (void)busid; (void)ep; (void)data; (void)data_len; return 0; }
int usbd_set_address(uint8_t busid, uint8_t addr) { (void)busid; (void)addr; return 0; }
int usbd_set_remote_wakeup(uint8_t busid) { (void)busid; return 0; }
uint8_t usbd_get_port_speed(uint8_t busid) { (void)busid; return USB_SPEED_HIGH; }
int usb_dc_init(uint8_t busid) { (void)busid; return 0; }
int usb_dc_deinit(uint8_t busid) { (void)busid; return 0; }

