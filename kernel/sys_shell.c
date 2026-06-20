/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "nuklear.h"
#include "pro_os.h"
#include "serial.h"
#include <string.h>
#include <stdio.h>

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
                    if (strcmp(shell_buffer, "help") == 0) {
                        serial_write("Commands: tasks, mem, uptime, pci, cpu, panic\n");
                    } else if (strcmp(shell_buffer, "tasks") == 0) {
                        serial_printf("Active Tasks: %d\n", scheduler_get_task_count());
                    } else if (strcmp(shell_buffer, "mem") == 0) {
                        serial_printf("Memory Metrics: TBD\n");
                    } else if (strcmp(shell_buffer, "uptime") == 0) {
                        serial_printf("Uptime: %llu ms\n", hal_get_uptime_ms());
                    } else if (strcmp(shell_buffer, "pci") == 0) {
                        serial_printf("PCI Hardware list: Use UI Device Manager\n");
                    } else if (strcmp(shell_buffer, "cpu") == 0) {
                        serial_printf("CPU Load: %d%%\n", scheduler_get_cpu_load());
                    } else if (strcmp(shell_buffer, "panic") == 0) {
                        kpanic("USER_REQUESTED_PANIC");
                    } else {
                        serial_printf("Unknown command: %s\n", shell_buffer);
                    }
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
