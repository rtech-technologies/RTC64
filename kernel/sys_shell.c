#include "pro_os.h"
#include "serial.h"
#include <string.h>
#define SHELL_BUF_SIZE 128
static char shell_buffer[SHELL_BUF_SIZE];
static int shell_ptr = 0;
void system_shell_init(void) {
    serial_write("\n\nSovereign HIGH-POWER System Shell\n");
    serial_write("Hardware-accelerated console active.\n");
    serial_write("Type 'help' for commands.\n");
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
                    if (strcmp(shell_buffer, "help") == 0) { serial_write("Commands: tasks, mem, pci, cpu, panic, help, uptime\n"); }
                    else if (strcmp(shell_buffer, "tasks") == 0) {
                        int count = scheduler_get_task_count();
                        for (int i = 0; i < count; i++) { task_t* t = scheduler_get_task(i); serial_printf(" [%d] %s\n", i, t->name); }
                    } else if (strcmp(shell_buffer, "mem") == 0) { serial_printf("Heap Used: %d KB\n", (int)(hal_malloc_get_used()/1024)); }
                    else if (strcmp(shell_buffer, "pci") == 0) { pci_scan(); }
                    else if (strcmp(shell_buffer, "panic") == 0) { kpanic("USER_REQUESTED_PANIC"); }
                    else if (strcmp(shell_buffer, "uptime") == 0) { serial_printf("Uptime: %d ms\n", (int)hal_get_uptime_ms()); }
                    else { serial_write("Unknown command.\n"); }
                }
                serial_write("> ");
                shell_ptr = 0;
            } else if (c == '\b' || c == 127) {
                if (shell_ptr > 0) { shell_ptr--; serial_write("\b \b"); }
            } else if (shell_ptr < SHELL_BUF_SIZE - 1) {
                shell_buffer[shell_ptr++] = c;
                char echo[2] = {c, 0}; serial_write(echo);
            }
        }
        scheduler_yield();
    }
}
