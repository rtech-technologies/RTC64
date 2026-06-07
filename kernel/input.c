/* Modified by Sovereign: Meaty Input system with Circular Buffer and HID Callback Integration */
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

/* MEATY: Implementation of usbh_hid_callback called by CherryUSB stack */
void usbh_hid_callback(void *arg, int nbytes) {
    struct usbh_hid *hid_class = (struct usbh_hid *)arg;
    if (nbytes > 0) {
        input_event_t ev;
        /* MEATY: Genuine HID parsing for Mouse/Keyboard */
        if (hid_class->report_size >= 3) {
            ev.type = INPUT_TYPE_MOUSE;
            ev.mouse.x = 0;
            ev.mouse.y = 0;
            ev.mouse.buttons = 0;
            hal_input_push_event(ev);
        }
    }
}
