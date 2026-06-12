#include "pro_os.h"
#include <string.h>

static app_permit_t policies[MAX_TASKS];

bool uac_check_permit(int app_id, const char *action) {
    if (app_id < 0 || app_id >= MAX_TASKS) return false;
    if (strcmp(action, "network") == 0) return policies[app_id].can_network;
    if (strcmp(action, "storage") == 0) return policies[app_id].can_storage;
    return false;
}

void uac_request_permit(int app_id, const char *action) {
    // In a real OS, this would trigger the UAC popup
    printf("UAC: Request permit for app %d, action %s\n", app_id, action);
}

void uac_set_permit(int app_id, bool net, bool storage) {
    if (app_id >= 0 && app_id < MAX_TASKS) {
        policies[app_id].can_network = net;
        policies[app_id].can_storage = storage;
    }
}
