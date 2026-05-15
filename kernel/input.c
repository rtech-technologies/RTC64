#include "hal.h"
#include <stddef.h>

#define MAX_INPUT_EVENTS 64
static input_event_t g_input_queue[MAX_INPUT_EVENTS];
static int g_input_head = 0;
static int g_input_tail = 0;

void hal_input_init(void) {
    g_input_head = 0;
    g_input_tail = 0;
}

void hal_input_push_event(input_event_t ev) {
    int next = (g_input_tail + 1) % MAX_INPUT_EVENTS;
    if (next != g_input_head) {
        g_input_queue[g_input_tail] = ev;
        g_input_tail = next;
    }
}

bool hal_input_pop_event(input_event_t *ev) {
    if (g_input_head == g_input_tail) return false;
    *ev = g_input_queue[g_input_head];
    g_input_head = (g_input_head + 1) % MAX_INPUT_EVENTS;
    return true;
}

void hal_input_poll(void) {
    // Poll hardware directly or let USB callbacks push events
}
