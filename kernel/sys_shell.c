/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "nuklear.h"
#include "pro_os.h"
#include "serial.h"
#include <string.h>
#define SHELL_BUF_SIZE 128
static char shell_buffer[SHELL_BUF_SIZE];
static int shell_ptr = 0;
void system_shell_init(void) {
    serial_write("\n\nSovereign System Shell\n");
    serial_write("> ");
    shell_ptr = 0;
}
void system_shell_task(void* arg) {
    (void)arg;
    while (1) {
        if (serial_received()) {
            char c = serial_read();
            if (c == '\r' || c == '\n') {
                serial_write("\n");
                shell_buffer[shell_ptr] = '\0';
                if (shell_ptr > 0) {
                    if (strcmp(shell_buffer, "help") == 0) serial_write("Commands: tasks, mem, uptime\n");
                    else if (strcmp(shell_buffer, "uptime") == 0) serial_printf("Uptime: %d ms\n", (int)hal_get_uptime_ms());
                }
                serial_write("> ");
                shell_ptr = 0;
            } else if (shell_ptr < SHELL_BUF_SIZE - 1) {
                shell_buffer[shell_ptr++] = c;
                char echo[2] = {c, 0}; serial_write(echo);
            }
        }
        scheduler_yield();
    }
}
