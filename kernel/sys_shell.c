#include "pro_os.h"
#include "serial.h"
#include <string.h>

#define SHELL_BUF_SIZE 128

static char shell_buffer[SHELL_BUF_SIZE];
static int shell_ptr = 0;

void system_shell_init(void) {
    serial_write("\n\nSovereign System Shell (Serial Console)\n");
    serial_write("Type 'help' for commands.\n");
    serial_write("> ");
    shell_ptr = 0;
}

void system_shell_task(void) {
    while (serial_received()) {
        char c = serial_read();

        if (c == '\r' || c == '\n') {
            serial_write("\n");
            shell_buffer[shell_ptr] = '\0';

            if (shell_ptr > 0) {
                if (strcmp(shell_buffer, "help") == 0) {
                    serial_write("Available commands:\n");
                    serial_write("  tasks  - List scheduler tasks\n");
                    serial_write("  mem    - Show heap stats\n");
                    serial_write("  pci    - Trigger a PCI bus scan\n");
                    serial_write("  panic  - Trigger a kernel panic\n");
                    serial_write("  help   - Show this help\n");
                } else if (strcmp(shell_buffer, "tasks") == 0) {
                    serial_write("Active Tasks:\n");
                    serial_write("  [0] USB Poller (Background)\n");
                    serial_write("  [1] System Shell (Console)\n");
                } else if (strcmp(shell_buffer, "mem") == 0) {
                    serial_write("Memory Map: Sovereign 64-bit Higher Half\n");
                    serial_write("Kernel Heap: 16MB initialized.\n");
                } else if (strcmp(shell_buffer, "pci") == 0) {
                    serial_write("Starting PCI discovery...\n");
                    pci_scan();
                } else if (strcmp(shell_buffer, "panic") == 0) {
                    kpanic("User requested kernel panic from system shell.");
                } else {
                    serial_write("Unknown command: ");
                    serial_write(shell_buffer);
                    serial_write("\n");
                }
            }

            serial_write("> ");
            shell_ptr = 0;
        } else if (c == '\b' || c == 127) {
            if (shell_ptr > 0) {
                shell_ptr--;
                serial_write("\b \b");
            }
        } else if (shell_ptr < SHELL_BUF_SIZE - 1) {
            shell_buffer[shell_ptr++] = c;
            char echo[2] = {c, 0};
            serial_write(echo);
        }
    }
}
