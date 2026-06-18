/* Modified by Sovereign: Meaty Input system with Circular Buffer and HID Packet Parsing */
#include "hal.h"
#include <string.h>
#include "usbh_core.h"
#include "usbh_hid.h"

#define INPUT_QUEUE_SIZE 128

static input_event_t g_input_queue[INPUT_QUEUE_SIZE];
static volatile int g_queue_head = 0;
static volatile int g_queue_tail = 0;

static int g_mouse_x = 400;
static int g_mouse_y = 300;

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

        int8_t rel_x = (int8_t)data[1];
        int8_t rel_y = (int8_t)data[2];

        g_mouse_x += rel_x;
        g_mouse_y += rel_y;

        if (g_mouse_x < 0) g_mouse_x = 0;
        if (g_mouse_y < 0) g_mouse_y = 0;
        if (g_mouse_x > 1919) g_mouse_x = 1919;
        if (g_mouse_y > 1079) g_mouse_y = 1079;

        input_event_t ev;
        ev.type = INPUT_TYPE_MOUSE;
        ev.mouse.x = g_mouse_x;
        ev.mouse.y = g_mouse_y;
        ev.mouse.buttons = data[0];
        hal_input_push_event(ev);
    }
}
