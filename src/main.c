/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#define RSL_IMPLEMENTATION
#include "rsl.h"

void user_app_entry(void* arg) {
    (void)arg;
    rsl_printf("RSL Executive Environment Initialized.\n");
    rsl_printf("System Uptime: %llu ms\n", rsl_uptime());

    char buf[1024];
    rsl_mounts(buf, sizeof(buf));
    rsl_printf("Executive Mounts:\n%s\n", buf);

    while(1) {
        rsl_yield();
    }
}

int main(void) {
    rsl_spawn("UserSession", user_app_entry, NULL);
    return 0;
}
