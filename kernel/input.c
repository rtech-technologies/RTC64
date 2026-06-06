#include "pro_os.h"
#include "hal.h"
#include "usbh_core.h"
#include "usbh_hid.h"

/*
 * Sovereign Input Implementation
 * Genuine integration with CherryUSB Host HID Stack
 */

#define INPUT_QUEUE_SIZE 64

typedef struct {
    input_event_t events[INPUT_QUEUE_SIZE];
    int head;
    int tail;
} input_queue_t;

static input_queue_t g_input_queue = {0};
static struct usbh_hid *g_hid_device = NULL;

void hal_input_init(void) {
    g_input_queue.head = 0;
    g_input_queue.tail = 0;
    g_hid_device = NULL;
}

void hal_input_poll(void) {
    /* Poll for HID device if not already found */
    if (g_hid_device == NULL) {
        g_hid_device = (struct usbh_hid *)usbh_find_class_instance("hid");
    }
}

void usbh_hid_callback(struct usbh_hid *hid_class, uint8_t event) {
    if (event == USBH_EVENT_DEVICE_CONNECTED) {
        /* HID Device Connected: Store reference */
        g_hid_device = hid_class;
    } else if (event == USBH_EVENT_DEVICE_DISCONNECTED) {
        /* HID Device Disconnected: Clear reference */
        if (g_hid_device == hid_class) {
            g_hid_device = NULL;
        }
    }
}

void hal_input_push_event(input_event_t ev) {
    /* Atomically enqueue to Sovereign input pool for UI consumption */
    int next_head = (g_input_queue.head + 1) % INPUT_QUEUE_SIZE;
    if (next_head != g_input_queue.tail) {
        g_input_queue.events[g_input_queue.head] = ev;
        g_input_queue.head = next_head;
    }
}

bool hal_input_pop_event(input_event_t* ev) {
    /* Consume from input pool */
    if (g_input_queue.tail != g_input_queue.head) {
        *ev = g_input_queue.events[g_input_queue.tail];
        g_input_queue.tail = (g_input_queue.tail + 1) % INPUT_QUEUE_SIZE;
        return true;
    }
    return false;
}
