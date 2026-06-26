/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "hal.h"
#include "pro_os.h"
#include <string.h>
#include "usbh_core.h"
#include "usbh_hid.h"
#include "usb_osal.h"
#include "serial.h"

#define INPUT_QUEUE_SIZE 128
static input_event_t g_input_queue[INPUT_QUEUE_SIZE];
static volatile int g_queue_head = 0;
static volatile int g_queue_tail = 0;

void hal_input_push_event(input_event_t ev) {
    int next = (g_queue_head + 1) % INPUT_QUEUE_SIZE;
    if (next != g_queue_tail) {
        g_input_queue[g_queue_head] = ev;
        g_queue_head = next;
    }
}

bool hal_input_pop_event(input_event_t *ev) {
    if (g_queue_head == g_queue_tail) return false;
    *ev = g_input_queue[g_queue_tail];
    g_queue_tail = (g_queue_tail + 1) % INPUT_QUEUE_SIZE;
    return true;
}

void hal_input_init(void) {
    g_queue_head = 0; g_queue_tail = 0;
    memset(g_input_queue, 0, sizeof(g_input_queue));
}

void usbh_hid_callback(void *arg, int nbytes) {
    struct usbh_hid *hid_class = (struct usbh_hid *)arg;
    if (nbytes >= 3) {
        uint8_t *data = (uint8_t*)hid_class->intin_urb.transfer_buffer;
        input_event_t ev;
        ev.type = INPUT_TYPE_MOUSE;
        ev.mouse.x = (int8_t)data[1];
        ev.mouse.y = (int8_t)data[2];
        ev.mouse.buttons = data[0];
        hal_input_push_event(ev);
    }
    usbh_submit_urb(&hid_class->intin_urb);
}

void usbh_hid_run(struct usbh_hid *hid_class) {
    serial_printf("[HID] Device connected. Setting Boot Protocol.\n");
    usbh_hid_set_protocol(hid_class, 0); /* Boot Protocol */
    usbh_hid_set_idle(hid_class, 0, 0);   /* Indefinite reporting */

    uint32_t mps = USB_GET_MAXPACKETSIZE(hid_class->intin->wMaxPacketSize);
    void* phys_buf = pmm_alloc_blocks_low(1);
    uint8_t *buffer = (uint8_t*)((uint64_t)phys_buf + hhdm_offset);
    memset(buffer, 0, mps);

    usbh_int_urb_fill(&hid_class->intin_urb, hid_class->hport, hid_class->intin, buffer, mps, 0, usbh_hid_callback, hid_class);
    usbh_submit_urb(&hid_class->intin_urb);
    serial_printf("[HID] Interrupt poll started.\n");
}

void usbh_hid_stop(struct usbh_hid *hid_class) {
    (void)hid_class;
}
