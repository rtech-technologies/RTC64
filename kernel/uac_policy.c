/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <string.h>

static app_permit_t policies[MAX_TASKS];

bool uac_check_permit(int app_id, const char *action) {
    if (app_id < 0 || app_id >= MAX_TASKS) return false;

    bool allowed = false;
    if (strcmp(action, "network") == 0) allowed = policies[app_id].can_network;
    else if (strcmp(action, "storage") == 0) allowed = policies[app_id].can_storage;

    if (!allowed) {
        char event[64];
        snprintf(event, sizeof(event), "DENIED: %s", action);
        comprec_log("SECURITY", event);
    }

    return allowed;
}

void uac_request_permit(int app_id, const char *action) {
    if (app_id < 0 || !action) return;
    char event[64];
    snprintf(event, sizeof(event), "REQUEST: %s", action);
    comprec_log("UAC", event);
}

void uac_set_permit(int app_id, bool net, bool storage) {
    if (app_id >= 0 && app_id < MAX_TASKS) {
        policies[app_id].can_network = net;
        policies[app_id].can_storage = storage;
        comprec_log("POLICY", "Permissions Updated");
    }
}
