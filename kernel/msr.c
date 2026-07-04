/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <stdint.h>

#define EFER_MSR    0xC0000080
#define STAR_MSR    0xC0000081
#define LSTAR_MSR   0xC0000082
#define SFMASK_MSR  0xC0000084

extern void syscall_entry(void);

void msr_init(void) {
    /* 1. Enable syscall/sysret in EFER (bit 0: SCE) */
    uint32_t low, high;
    __asm__ volatile("rdmsr" : "=a"(low), "=d"(high) : "c"(EFER_MSR));
    low |= 1;
    __asm__ volatile("wrmsr" : : "a"(low), "d"(high), "c"(EFER_MSR));

    /* 2. Set STAR (Segment selectors for syscall/sysret)
     * Bits 31-0: Reserved
     * Bits 47-32: Syscall CS/SS (Kernel). Base selector, CS = val, SS = val + 8
     * Bits 63-48: Sysret CS/SS (User). Base selector, CS = val + 16, SS = val + 8
     * Layout: Null (0), KCode (8), KData (16), UData (24), UCode (32)
     */
    uint64_t star = (0x08ULL << 32) | (0x13ULL << 48);
    __asm__ volatile("wrmsr" : : "a"((uint32_t)star), "d"((uint32_t)(star >> 32)), "c"(STAR_MSR));

    /* 3. Set LSTAR (The actual syscall entry point) */
    uint64_t lstar = (uint64_t)syscall_entry;
    __asm__ volatile("wrmsr" : : "a"((uint32_t)lstar), "d"((uint32_t)(lstar >> 32)), "c"(LSTAR_MSR));

    /* 4. Set SFMASK (RFLAGS mask)
     * We want to mask IF (bit 9), TF (bit 8), DF (bit 10), etc.
     * 0x200 = IF bit.
     */
    uint64_t sfmask = 0x200;
    __asm__ volatile("wrmsr" : : "a"((uint32_t)sfmask), "d"((uint32_t)(sfmask >> 32)), "c"(SFMASK_MSR));
}
