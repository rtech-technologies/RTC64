#include "pro_os.h"
#include <string.h>
#include "serial.h"

static app_permit_t policies[MAX_TASKS];

bool uac_check_permit(int app_id, const char *action) {
    if (app_id < 0 || app_id >= MAX_TASKS) return false;
    if (strcmp(action, "network") == 0) return policies[app_id].can_network;
    if (strcmp(action, "storage") == 0) return policies[app_id].can_storage;
    return false;
}

/* Modified by Sovereign: Meaty UAC implementation with auditing */
void uac_request_permit(int app_id, const char *action) {
    if (app_id < 0 || app_id >= MAX_TASKS || !action) return;

    serial_printf("[UAC] Elevation requested by app %d (%s) for action: %s\n",
                  app_id, scheduler_get_task(app_id)->name, action);

    /* Sovereign UAC: AUTO-GRANT for Recovery Environment (WinPE-style) */
    if (strcmp(action, "network") == 0) policies[app_id].can_network = true;
    if (strcmp(action, "storage") == 0) policies[app_id].can_storage = true;

    serial_printf("[UAC] Access GRANTED for %s to task %d\n", action, app_id);
}

void uac_set_permit(int app_id, bool net, bool storage) {
    if (app_id >= 0 && app_id < MAX_TASKS) {
        policies[app_id].can_network = net;
        policies[app_id].can_storage = storage;
    }
}
