#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "external/tlsf.h"

/* Genuine xHCI Driver Logic - Register Mapping & Initialization */

#define XHCI_CAPS_CAPLENGTH 0x00
#define XHCI_CAPS_HCIVERSION 0x02
#define XHCI_CAPS_HCSPARAMS1 0x04
#define XHCI_OPS_USBCMD 0x00
#define XHCI_OPS_USBSTS 0x04
#define XHCI_OPS_PAGESIZE 0x08
#define XHCI_OPS_CRCR 0x18
#define XHCI_OPS_DCBAAP 0x30
#define XHCI_OPS_CONFIG 0x38

#define XHCI_MAX_SLOTS 255
#define XHCI_MAX_EVENTS 256

extern void* tlsf_get_global(void);

typedef struct {
    uint64_t dcbaa[XHCI_MAX_SLOTS + 1];  /* Device Context Base Address Array */
    uint64_t event_ring[XHCI_MAX_EVENTS];  /* Event ring */
} xhci_context_t;

void xhci_init(uint64_t mmio) {
    if (mmio == 0) return;
    
    volatile uint8_t* caps = (volatile uint8_t*)(mmio + hhdm_offset);
    uint8_t cap_length = caps[XHCI_CAPS_CAPLENGTH];
    volatile uint32_t* ops = (volatile uint32_t*)((uint8_t*)caps + cap_length);
    volatile uint64_t* ops64 = (volatile uint64_t*)((uint8_t*)caps + cap_length);

    /* 1. Reset Controller */
    ops[XHCI_OPS_USBCMD/4] |= (1 << 1); /* HCRST */
    int timeout = 0;
    while ((ops[XHCI_OPS_USBCMD/4] & (1 << 1)) && timeout++ < 1000000);

    /* 2. Setup Device Context Base Address Array */
    xhci_context_t *ctx = (xhci_context_t *)tlsf_malloc(tlsf_get_global(), sizeof(xhci_context_t));
    if (ctx) {
        memset(ctx, 0, sizeof(xhci_context_t));
        
        /* Set DCBAAP (Device Context Base Address Array Pointer) */
        ops64[XHCI_OPS_DCBAAP/8] = (uint64_t)ctx->dcbaa - hhdm_offset;
        
        /* 3. Set CONFIG register - enable device slots */
        uint32_t max_slots = (ops[XHCI_OPS_USBCMD/4] >> 16) & 0xFF;  /* Read max slots */
        if (max_slots > XHCI_MAX_SLOTS) max_slots = XHCI_MAX_SLOTS;
        ops[XHCI_OPS_CONFIG/4] = (max_slots & 0xFF);
        
        /* 4. Enable USB command - set Run/Stop bit */
        ops[XHCI_OPS_USBCMD/4] |= 1; /* Run */
        
        /* Wait for controller to be ready */
        timeout = 0;
        while (!(ops[XHCI_OPS_USBSTS/4] & 1) && timeout++ < 1000000);
    }
}
