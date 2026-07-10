/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <string.h>

static app_permit_t policies[MAX_TASKS];

bool uac_check_permit(int app_id, const char *action) {
    if (app_id < 0 || app_id >= MAX_TASKS) return false;
    if (strcmp(action, "network") == 0) return policies[app_id].can_network;
    if (strcmp(action, "storage") == 0) return policies[app_id].can_storage;
    return false;
}

/* Modified by Sovereign: Fixed unused parameter warnings */
void uac_request_permit(int app_id, const char *action) {
    /* Sovereign UAC: Future implementation will trigger secure interrupt for elevation */
    if (app_id < 0 || !action) return;
    (void)app_id;
}

void uac_set_permit(int app_id, bool net, bool storage) {
    if (app_id >= 0 && app_id < MAX_TASKS) {
        policies[app_id].can_network = net;
        policies[app_id].can_storage = storage;
    }
}
