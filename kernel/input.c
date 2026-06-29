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
static spinlock_t g_input_lock = 0;

static int g_mouse_abs_x = 0;
static int g_mouse_abs_y = 0;

#define MAX_INPUT_DEVICES 8
static input_device_info_t g_input_devices[MAX_INPUT_DEVICES];
static int g_input_dev_count = 0;

static void* g_usb_hid_map[MAX_INPUT_DEVICES];
static int g_usb_hid_id[MAX_INPUT_DEVICES];

void hal_input_get_mouse_abs(int *x, int *y) {
    if (x) *x = g_mouse_abs_x;
    if (y) *y = g_mouse_abs_y;
}

int hal_input_register_device(const char* name, input_type_t type, input_bus_t bus) {
    if (g_input_dev_count >= MAX_INPUT_DEVICES) return -1;
    int id = g_input_dev_count++;
    strncpy(g_input_devices[id].name, name, 31);
    g_input_devices[id].type = type;
    g_input_devices[id].bus = bus;
    g_input_devices[id].connected = true;
    serial_printf("[EVENT] CONNECT: %s (%s)\n", name, (bus == INPUT_BUS_USB ? "USB" : "PS2"));
    return id;
}

void hal_input_set_device_status(int id, bool connected) {
    if (id >= 0 && id < g_input_dev_count) {
        g_input_devices[id].connected = connected;
        serial_printf("[EVENT] %s: %s\n", connected ? "CONNECT" : "DISCONNECT", g_input_devices[id].name);
    }
}

int hal_input_get_device_count(void) { return g_input_dev_count; }

bool hal_input_get_device_info(int index, input_device_info_t *info) {
    if (index >= 0 && index < g_input_dev_count) {
        *info = g_input_devices[index];
        return true;
    }
    return false;
}

void hal_input_push_event(input_event_t ev) {
    spin_lock(&g_input_lock);
    int next = (g_queue_head + 1) % INPUT_QUEUE_SIZE;
    if (next != g_queue_tail) {
        g_input_queue[g_queue_head] = ev;
        g_queue_head = next;
    }
    if (ev.type == INPUT_TYPE_MOUSE) {
        g_mouse_abs_x += ev.mouse.x;
        g_mouse_abs_y += ev.mouse.y;
    }
    spin_unlock(&g_input_lock);
}

bool hal_input_pop_event(input_event_t *ev) {
    spin_lock(&g_input_lock);
    if (g_queue_head == g_queue_tail) {
        spin_unlock(&g_input_lock);
        return false;
    }
    *ev = g_input_queue[g_queue_tail];
    g_queue_tail = (g_queue_tail + 1) % INPUT_QUEUE_SIZE;
    spin_unlock(&g_input_lock);
    return true;
}

void hal_input_init(void) {
    g_queue_head = 0; g_queue_tail = 0;
    g_mouse_abs_x = 0; g_mouse_abs_y = 0;
    g_input_dev_count = 0;
    memset(g_input_queue, 0, sizeof(g_input_queue));
    memset(g_input_devices, 0, sizeof(g_input_devices));
    memset(g_usb_hid_map, 0, sizeof(g_usb_hid_map));
}

/* Polling Tasks: Loop until something is returned, then yield */
void kbd_task(void* arg) {
    (void)arg;
    serial_printf("[INPUT] KBD Task Started.\n");
    while(1) {
        int old_head = g_queue_head;
        /* Poll until we get a keyboard event */
        while (g_queue_head == old_head) {
            hal_usb_poll();
            ps2_poll_kbd();
            if (g_queue_head != old_head) {
                /* Verify it was actually a keyboard event if needed, but for now we just return */
                break;
            }
            scheduler_yield();
        }
    }
}

void mouse_task(void* arg) {
    (void)arg;
    serial_printf("[INPUT] MOUSE Task Started.\n");
    while(1) {
        int old_head = g_queue_head;
        /* Poll until we get a mouse event */
        while (g_queue_head == old_head) {
            hal_usb_poll();
            ps2_poll_mouse();
            if (g_queue_head != old_head) break;
            scheduler_yield();
        }
    }
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
    int id = hal_input_register_device("USB Mouse", INPUT_TYPE_MOUSE, INPUT_BUS_USB);
    for (int i = 0; i < MAX_INPUT_DEVICES; i++) {
        if (g_usb_hid_map[i] == NULL) {
            g_usb_hid_map[i] = hid_class;
            g_usb_hid_id[i] = id;
            break;
        }
    }
    usbh_hid_set_protocol(hid_class, 0);
    uint32_t mps = USB_GET_MAXPACKETSIZE(hid_class->intin->wMaxPacketSize);
    void* phys_buf = pmm_alloc_blocks_low(1);
    uint8_t *buffer = (uint8_t*)((uint64_t)phys_buf + hhdm_offset);
    usbh_int_urb_fill(&hid_class->intin_urb, hid_class->hport, hid_class->intin, buffer, mps, 0, usbh_hid_callback, hid_class);
    usbh_submit_urb(&hid_class->intin_urb);
}

void usbh_hid_stop(struct usbh_hid *hid_class) {
    for (int i = 0; i < MAX_INPUT_DEVICES; i++) {
        if (g_usb_hid_map[i] == hid_class) {
            hal_input_set_device_status(g_usb_hid_id[i], false);
            g_usb_hid_map[i] = NULL;
            break;
        }
    }
}
