/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "nuklear.h"
#include "pro_os.h"
#include "serial.h"
#include <string.h>
#include <stdio.h>

#define SHELL_BUF_SIZE 256
static char shell_buffer[SHELL_BUF_SIZE];
static int shell_ptr = 0;

void system_shell_init(void) {
    serial_write("\n\nSovereign System Shell\n");
    serial_write("> ");
    shell_ptr = 0;
}

static void shell_execute(char* cmd) {
    if (strcmp(cmd, "help") == 0) {
        serial_write("Commands: tasks, mem, uptime, pci, cpu, ls, cat, mkdir, mounts, panic\n");
    } else if (strcmp(cmd, "tasks") == 0) {
        serial_printf("Active Tasks: %d\n", scheduler_get_task_count());
    } else if (strcmp(cmd, "mem") == 0) {
        serial_printf("Memory metrics active via TLSF Audit.\n");
    } else if (strcmp(cmd, "uptime") == 0) {
        serial_printf("Uptime: %llu ms\n", hal_get_uptime_ms());
    } else if (strcmp(cmd, "pci") == 0 || strcmp(cmd, "hw") == 0) {
        char buf[2048];
        vfs_get_hardware_info(buf, sizeof(buf));
        serial_write(buf);
        serial_write("\n");
    } else if (strcmp(cmd, "cpu") == 0) {
        serial_printf("CPU Load: %d%%\n", scheduler_get_cpu_load());
    } else if (strncmp(cmd, "ls ", 3) == 0) {
        char buf[2048];
        if (vfs_ls(cmd + 3, buf, sizeof(buf)) == 0) serial_write(buf);
        else serial_printf("ls failed for %s\n", cmd + 3);
    } else if (strncmp(cmd, "cat ", 4) == 0) {
        char buf[4096];
        if (vfs_cat(cmd + 4, buf, sizeof(buf)) == 0) serial_write(buf);
        else serial_printf("cat failed for %s\n", cmd + 4);
    } else if (strncmp(cmd, "mkdir ", 6) == 0) {
        if (vfs_mkdir(cmd + 6) == 0) serial_printf("Directory created: %s\n", cmd + 6);
        else serial_printf("mkdir failed for %s\n", cmd + 6);
    } else if (strcmp(cmd, "mounts") == 0) {
        char buf[1024];
        vfs_get_mounts(buf, sizeof(buf));
        serial_write(buf);
        serial_write("\n");
    } else if (strcmp(cmd, "panic") == 0) {
        kpanic("USER_REQUESTED_PANIC");
    } else if (strlen(cmd) > 0) {
        serial_printf("Unknown command: %s\n", cmd);
    }
}

void system_shell_task(void* arg) {
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
            } else if (c == 8 || c == 127) { /* Backspace */
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
