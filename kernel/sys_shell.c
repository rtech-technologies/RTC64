/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "serial.h"
#include "hal.h"
#include <string.h>
#include <stdio.h>

#define SHELL_BUF_SIZE 256
static char shell_buffer[SHELL_BUF_SIZE];
static int shell_ptr = 0;

void debug_shell_init(void) {
    serial_write("\n\nSovereign Debug Console (Serial)\n");
    serial_write("> ");
    shell_ptr = 0;
}

static void shell_execute(char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        serial_write("Debug Commands: tasks, uptime, cpu, usb, input, panic\n");
    } else if (strcmp(cmd, "tasks") == 0) {
        serial_printf("Active Tasks: %d\n", scheduler_get_task_count());
    } else if (strcmp(cmd, "uptime") == 0) {
        serial_printf("Uptime: %llu ms\n", hal_get_uptime_ms());
    } else if (strcmp(cmd, "cpu") == 0) {
        serial_printf("CPU Load: %d%%\n", scheduler_get_cpu_load());
    } else if (strcmp(cmd, "usb") == 0) {
        int mx, my;
        hal_input_get_mouse_abs(&mx, &my);
        serial_printf("USB/PS2 Mouse Position: X=%d, Y=%d\n", mx, my);
    } else if (strcmp(cmd, "input") == 0) {
        int count = hal_input_get_device_count();
        serial_printf("Input Devices: %d\n", count);
        for (int i = 0; i < count; i++) {
            input_device_info_t info;
            if (hal_input_get_device_info(i, &info)) {
                serial_printf(" [%d] %-16s | %s | %s\n", i, info.name,
                    (info.bus == INPUT_BUS_USB ? "USB" : "PS2"),
                    (info.connected ? "CONNECTED" : "DISCONNECTED"));
            }
        }
    } else if (strcmp(cmd, "panic") == 0) {
        kpanic("USER_REQUESTED_PANIC");
    } else if (strlen(cmd) > 0) {
        serial_printf("Unknown debug command: %s\n", cmd);
    }
}

void debug_shell_task(void* arg) {
    (void)arg;
    while (1) {
        if (serial_received()) {
            char c = serial_read();
            if (c == '\r' || c == '\n') {
                serial_write("\n");
                shell_buffer[shell_ptr] = '\0';
                shell_execute(shell_buffer);
                serial_write("> ");
                shell_ptr = 0;
            } else if (c == 8 || c == 127) {
                if (shell_ptr > 0) {
                    shell_ptr--;
                    serial_write("\b \b");
                }
            } else if (shell_ptr < SHELL_BUF_SIZE - 1) {
                shell_buffer[shell_ptr++] = c;
                char echo[2] = {c, 0}; serial_write(echo);
            }
        }
        scheduler_yield();
    }
}
