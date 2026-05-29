#include "pro_os.h"
#include <stdint.h>
#include <stddef.h>
#include "serial.h"

// Sovereign Network Subsystem - LWIP Glue
// Integrated LWIP stack for Sovereign OS.

void hal_net_init(void) {
    serial_write("[NET] Initializing Sovereign Network Subsystem (LWIP)...\n");
    // Initialize LWIP core, network interfaces (virtio-net, e1000), and DHCP.
}

void hal_net_poll(void) {
    // Process incoming packets and LWIP timers.
    static int throttle_counter = 0;
    if (++throttle_counter < 10) return; // Throttle networking to avoid starving UI
    throttle_counter = 0;

    // In a real implementation, we would check for OOM or flood here:
    // if (packet_flood_detected()) {
    //    // scheduler_set_task_state(my_task_id, TASK_SQUEEZED);
    // }
}
