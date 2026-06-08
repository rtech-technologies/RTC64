/* Modified by Sovereign: Meaty xHCI implementation with DCBAAP and Slot configuration and Logging */
#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "external/tlsf.h"
#include "serial.h"

#define XHCI_CAPS_CAPLENGTH 0x00
#define XHCI_OPS_USBCMD 0x00
#define XHCI_OPS_USBSTS 0x04
#define XHCI_OPS_DCBAAP 0x30
#define XHCI_OPS_CONFIG 0x38

#define XHCI_MAX_SLOTS 255
#define XHCI_MAX_EVENTS 256

extern void* tlsf_get_global(void);
extern uint64_t hhdm_offset;

typedef struct {
    uint64_t dcbaa[XHCI_MAX_SLOTS + 1];
    uint64_t event_ring[XHCI_MAX_EVENTS];
} xhci_context_t;

void xhci_init(uint64_t mmio) {
    if (mmio == 0) return;
    uint64_t base = mmio + hhdm_offset;
    serial_printf("[XHCI] Initializing controller at %p\n", base);
    
    volatile uint8_t* caps = (volatile uint8_t*)base;
    uint8_t cap_length = caps[XHCI_CAPS_CAPLENGTH];
    volatile uint32_t* ops = (volatile uint32_t*)((uint8_t*)caps + cap_length);
    volatile uint64_t* ops64 = (volatile uint64_t*)((uint8_t*)caps + cap_length);

    /* 1. Reset Controller */
    ops[XHCI_OPS_USBCMD/4] |= (1 << 1); /* HCRST */
    int timeout = 0;
    while ((ops[XHCI_OPS_USBCMD/4] & (1 << 1)) && timeout++ < 1000000) __asm__("pause");
    if (timeout >= 1000000) { serial_printf("[XHCI] Timeout waiting for reset\n"); return; }

    /* 2. Setup Device Context Base Address Array */
    xhci_context_t *ctx = (xhci_context_t *)tlsf_malloc(tlsf_get_global(), sizeof(xhci_context_t));
    if (ctx) {
        memset(ctx, 0, sizeof(xhci_context_t));
        ops64[XHCI_OPS_DCBAAP/8] = (uint64_t)ctx->dcbaa - hhdm_offset;
        
        /* 3. Configure Max Slots */
        uint32_t max_slots = (ops[XHCI_OPS_CONFIG/4] >> 0) & 0xFF;
        ops[XHCI_OPS_CONFIG/4] = (max_slots & 0xFF);
        serial_printf("[XHCI] Configured %d slots. DCBAAP set to %p\n", max_slots, ops64[XHCI_OPS_DCBAAP/8]);
        
        /* 4. Run Controller */
        ops[XHCI_OPS_USBCMD/4] |= 1; /* RS=1 */
        
        timeout = 0;
        while ((ops[XHCI_OPS_USBSTS/4] & 1) && timeout++ < 1000000) __asm__("pause");
        serial_printf("[XHCI] Controller running.\n");
    }
}
