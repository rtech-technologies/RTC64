/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "hal.h"
#include "pro_os.h"
#include "serial.h"

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

static void ps2_wait_write() {
    while (inb(PS2_STATUS) & 2);
}

static void ps2_wait_read() {
    while (!(inb(PS2_STATUS) & 1));
}

static void ps2_write_cmd(uint8_t cmd) {
    ps2_wait_write();
    outb(PS2_COMMAND, cmd);
}

static void ps2_write_data(uint8_t data) {
    ps2_wait_write();
    outb(PS2_DATA, data);
}

static uint8_t ps2_read_data() {
    ps2_wait_read();
    return inb(PS2_DATA);
}

void ps2_poll_kbd(void) {
    if (inb(PS2_STATUS) & 1) {
        uint8_t status = inb(PS2_STATUS);
        if (!(status & 0x20)) { /* Not mouse data */
            uint8_t scancode = inb(PS2_DATA);
            input_event_t ev;
            ev.type = INPUT_TYPE_KEYBOARD;
            ev.kbd.key = scancode & 0x7F;
            ev.kbd.down = !(scancode & 0x80);
            hal_input_push_event(ev);
        }
    }
}

static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[3];

void ps2_poll_mouse(void) {
    if (inb(PS2_STATUS) & 1) {
        uint8_t status = inb(PS2_STATUS);
        if (status & 0x20) { /* Mouse data */
            mouse_byte[mouse_cycle++] = inb(PS2_DATA);
            if (mouse_cycle == 3) {
                mouse_cycle = 0;
                input_event_t ev;
                ev.type = INPUT_TYPE_MOUSE;
                int x = mouse_byte[1];
                int y = mouse_byte[2];
                if (mouse_byte[0] & 0x10) x -= 256;
                if (mouse_byte[0] & 0x20) y -= 256;
                ev.mouse.x = x;
                ev.mouse.y = -y;
                ev.mouse.buttons = mouse_byte[0] & 0x07;
                hal_input_push_event(ev);
            }
        }
    }
}

void hal_ps2_init(void) {
    serial_printf("[PS2] Initializing i8042 controller (Polling Mode)...\n");

    hal_input_register_device("PS/2 Keyboard", INPUT_TYPE_KEYBOARD, INPUT_BUS_PS2);
    hal_input_register_device("PS/2 Mouse", INPUT_TYPE_MOUSE, INPUT_BUS_PS2);

    ps2_write_cmd(0xAD);
    ps2_write_cmd(0xA7);

    while (inb(PS2_STATUS) & 1) inb(PS2_DATA);

    ps2_write_cmd(0x20);
    uint8_t config = ps2_read_data();
    config &= ~0x03; /* Disable interrupts, use polling */
    config &= ~0x30;
    ps2_write_cmd(0x60);
    ps2_write_data(config);

    ps2_write_cmd(0xAE);
    ps2_write_cmd(0xA8);

    ps2_write_cmd(0xD4);
    ps2_write_data(0xF4);
    ps2_read_data();
}
