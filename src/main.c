/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#define RSL_IMPLEMENTATION
#include "rsl.h"

void system_genesis(void* arg) {
    (void)arg;
    rsl_printf("[GENESIS] Industrial Userland Active.\n");
    rsl_mkdir("/mnt/disk0/system");
    rsl_write("/mnt/disk0/system/registry.bin", "SOVEREIGN_INDUSTRIAL_V5");
    rsl_printf("[GENESIS] System baseline stabilized.\n");
    while(1) { rsl_yield(); }
}

int main(void) {
    rsl_spawn("Userland Genesis", system_genesis, NULL);
    return 0;
}
