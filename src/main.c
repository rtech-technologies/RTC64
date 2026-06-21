/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#define RSL_IMPLEMENTATION
#include "rsl.h"

void system_init_task(void* arg) {
    (void)arg;
    rsl_printf("[OS] Initializing high-power userland environment...\n");

    /* VFS Verification and Genesis */
    rsl_printf("[OS] Checking system partition integrity...\n");
    rsl_mkdir("/mnt/disk0/system");
    rsl_write("/mnt/disk0/system/registry.bin", "SOVEREIGN_V1");

    char buf[256];
    if (rsl_cat("/mnt/disk0/system/registry.bin", buf, sizeof(buf)) == 0) {
        rsl_printf("[OS] System Registry Loaded: %s\n", buf);
    }

    rsl_printf("[OS] Application Session Genesis complete.\n");

    while(1) {
        rsl_yield();
    }
}

int main(void) {
    /* entry point called by kernel SMSS */
    rsl_spawn("UserGenesis", system_init_task, NULL);
    return 0;
}
