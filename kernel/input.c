/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
/* Modified by Sovereign: Meaty Input system with Circular Buffer and HID Packet Parsing */
#include "hal.h"
#include <string.h>
#include "usbh_core.h"
#include "usbh_hid.h"

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
    g_queue_head = 0;
    g_queue_tail = 0;
    memset(g_input_queue, 0, sizeof(g_input_queue));
}

void usbh_hid_callback(void *arg, int nbytes) {
    struct usbh_hid *hid_class = (struct usbh_hid *)arg;

    if (nbytes >= 3) {
        uint8_t *data = (uint8_t*)hid_class->intin_urb.transfer_buffer;

        input_event_t ev;
        ev.type = INPUT_TYPE_MOUSE;
        ev.mouse.x = (int8_t)data[1]; /* Delta X */
        ev.mouse.y = (int8_t)data[2]; /* Delta Y */
        ev.mouse.buttons = data[0];
        hal_input_push_event(ev);
    }
}
