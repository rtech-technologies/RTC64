/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "external/tlsf.h"
#include "serial.h"

#define XHCI_CAPS_CAPLENGTH 0x00
#define XHCI_CAPS_HCSPARAMS1 0x04
#define XHCI_OPS_USBCMD 0x00
#define XHCI_OPS_USBSTS 0x04
#define XHCI_OPS_DCBAAP 0x30
#define XHCI_OPS_CONFIG 0x38

#define XHCI_MAX_SLOTS 255
#define XHCI_MAX_EVENTS 256

extern void* tlsf_get_global(void);
extern uint64_t hhdm_offset;
extern void* pmm_alloc_low(void);

typedef struct {
    uint64_t dcbaa[XHCI_MAX_SLOTS + 1];
    uint64_t event_ring[XHCI_MAX_EVENTS];
} xhci_context_t;

void xhci_init(uint64_t mmio) {
    if (mmio == 0) return;
    uint64_t base = mmio + hhdm_offset;
    serial_printf("[XHCI] Initializing Controller BAR: %p -> Virtual: %p\n", (void*)mmio, (void*)base);
    
    volatile uint8_t* caps = (volatile uint8_t*)base;
    uint8_t cap_length = caps[XHCI_CAPS_CAPLENGTH];
    volatile uint32_t* ops = (volatile uint32_t*)((uint8_t*)caps + cap_length);
    volatile uint64_t* ops64 = (volatile uint64_t*)((uint8_t*)caps + cap_length);

    /* 1. Reset Controller */
    ops[XHCI_OPS_USBCMD/4] |= (1 << 1); /* HCRST */
    int timeout = 0;
    while ((ops[XHCI_OPS_USBCMD/4] & (1 << 1)) && timeout++ < 1000000) __asm__("pause");
    if (timeout >= 1000000) { serial_printf("[XHCI] Timeout waiting for reset\n"); return; }

    /* 2. Setup Device Context Base Address Array (Force <4GB for DMA compatibility) */
    void* phys_ctx = pmm_alloc_low();
    if (phys_ctx) {
        xhci_context_t *ctx = (xhci_context_t *)((uint64_t)phys_ctx + hhdm_offset);
        memset(ctx, 0, 4096);
        uint64_t phys_dcbaa = (uint64_t)phys_ctx;
        ops64[XHCI_OPS_DCBAAP/8] = phys_dcbaa;
        
        /* 3. Configure Max Slots from HCSPARAMS1 */
        uint32_t hcsparams1 = *(volatile uint32_t*)(caps + XHCI_CAPS_HCSPARAMS1);
        uint32_t max_slots = hcsparams1 & 0xFF;
        if (max_slots == 0) max_slots = 32;

        uint32_t config = ops[XHCI_OPS_CONFIG/4];
        config &= ~0xFF;
        config |= max_slots;
        ops[XHCI_OPS_CONFIG/4] = config;

        serial_printf("[XHCI] Configured %d slots. DCBAAP set to Phys: %p\n", (int)max_slots, (void*)phys_dcbaa);
        
        /* 4. Run Controller */
        ops[XHCI_OPS_USBCMD/4] |= 1; /* RS=1 */
        timeout = 0;
        while ((ops[XHCI_OPS_USBSTS/4] & (1 << 0)) && timeout++ < 1000000) __asm__("pause");
        serial_printf("[XHCI] Controller running.\n");
    } else {
        serial_printf("[XHCI] FATAL: Failed to allocate low-memory context.\n");
    }
}
