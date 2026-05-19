#include "pro_os.h"
#include "hal.h"
#include "usbh_core.h"
#include "usbh_hid.h"
#include <string.h>

/*
 * Sovereign Input Implementation
 * Genuine integration with CherryUSB Host HID Stack
 */

#define INPUT_QUEUE_SIZE 64
static input_event_t input_queue[INPUT_QUEUE_SIZE];
static volatile int queue_head = 0;
static volatile int queue_tail = 0;

void hal_input_init(void) {
    queue_head = 0;
    queue_tail = 0;
}

void hal_input_push_event(input_event_t ev) {
    int next = (queue_head + 1) % INPUT_QUEUE_SIZE;
    if (next != queue_tail) {
        input_queue[queue_head] = ev;
        queue_head = next;
    }
}

bool hal_input_pop_event(input_event_t* ev) {
    if (queue_head == queue_tail) return false;
    *ev = input_queue[queue_tail];
    queue_tail = (queue_tail + 1) % INPUT_QUEUE_SIZE;
    return true;
}

/* CherryUSB HID Mouse Report Parsing */
struct mouse_report {
    uint8_t buttons;
    int8_t x;
    int8_t y;
    int8_t wheel;
} __attribute__((packed));

USB_NOCACHE_RAM_SECTION USB_MEM_ALIGNX static uint8_t mouse_buffer[32];

static void usbh_hid_mouse_callback(void *arg, int nbytes) {
    struct usbh_hid *hid_class = (struct usbh_hid *)arg;

    if (nbytes >= 3) {
        struct mouse_report *m = (struct mouse_report *)mouse_buffer;
        input_event_t ev;
        ev.type = INPUT_TYPE_MOUSE;
        ev.mouse.x = m->x;
        ev.mouse.y = m->y;
        ev.mouse.buttons = m->buttons;
        ev.mouse.scroll = (nbytes > 3) ? m->wheel : 0;
        hal_input_push_event(ev);
    }

    /* Resubmit URB for next report */
    usbh_submit_urb(&hid_class->intin_urb);
}

void usbh_hid_run(struct usbh_hid *hid_class) {
    /* Check if it's a mouse (Protocol 2 in Boot Interface) */
    if (hid_class->hport->config.intf[hid_class->intf].altsetting[0].intf_desc.bInterfaceProtocol == 2) {
        usbh_int_urb_fill(&hid_class->intin_urb, hid_class->hport, hid_class->intin, mouse_buffer,
                         hid_class->intin->wMaxPacketSize, 0, usbh_hid_mouse_callback, hid_class);
        usbh_submit_urb(&hid_class->intin_urb);
    }
}

void usbh_hid_stop(struct usbh_hid *hid_class) {
    (void)hid_class;
}

void usbh_hid_callback(struct usbh_hid *hid_class, uint8_t event) {
    (void)hid_class;
    (void)event;
}
