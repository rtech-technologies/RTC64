/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include "serial.h"
#include <string.h>
#include <stdio.h>

#define SHELL_BUF_SIZE 256
static char shell_buffer[SHELL_BUF_SIZE];
static int shell_ptr = 0;

extern int g_mouse_x;
extern int g_mouse_y;

void debug_shell_init(void) {
    serial_write("\n\nSovereign Debug Console (Serial)\n");
    serial_write("> ");
    shell_ptr = 0;
}

static void shell_execute(char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        serial_write("Debug Commands: tasks, uptime, cpu, panic, usb\n");
    } else if (strcmp(cmd, "tasks") == 0) {
        serial_printf("Active Tasks: %d\n", scheduler_get_task_count());
    } else if (strcmp(cmd, "uptime") == 0) {
        serial_printf("Uptime: %llu ms\n", hal_get_uptime_ms());
    } else if (strcmp(cmd, "cpu") == 0) {
        serial_printf("CPU Load: %d%%\n", scheduler_get_cpu_load());
    } else if (strcmp(cmd, "panic") == 0) {
        kpanic("USER_REQUESTED_PANIC");
    } else if (strcmp(cmd, "usb") == 0) {
        serial_printf("Mouse Position: X=%d, Y=%d\n", g_mouse_x, g_mouse_y);
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
