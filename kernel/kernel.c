#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "limine.h"

// Note: We use the Nuklear header but NOT the NK_IMPLEMENTATION here
// to avoid bloat in the kernel skeleton.
#include "nuklear.h"
#include "app_ui.h"
#include "services.h"

service_table_t g_services = { NULL, NULL, NULL, NULL };

static void hcf(void) {
    __asm__ ("cli");
    for (;;) __asm__ ("hlt");
}

void _start(void) {
    // Basic kernel entry point
    // In a complete implementation, we would:
    // 1. Initialize a software-rendering backend for Nuklear.
    // 2. Map the Limine framebuffer to Nuklear's draw commands.
    // 3. Loop and call ui_render() to draw the GUI.

    hcf();
}
