#include <pro_os.h>

#define XHCI_HCCPARAMS1 0x10
#define XHCI_ECP_MASK   0xFFFF0000

void xhci_bios_handover(void* base) {
    uint32_t hccp1 = *(volatile uint32_t*)((uint8_t*)base + XHCI_HCCPARAMS1);
    uint32_t ext_cap_ptr = (hccp1 & XHCI_ECP_MASK) >> 14; // Dword offset

    while (ext_cap_ptr) {
        volatile uint32_t* cap = (volatile uint32_t*)((uint8_t*)base + ext_cap_ptr);
        uint32_t cap_id = *cap & 0xFF;

        if (cap_id == 1) { // USB Legacy Support
            // BIOS Semaphores at offset 2 and 3
            volatile uint8_t* bios_sem = (volatile uint8_t*)cap + 2;
            volatile uint8_t* os_sem   = (volatile uint8_t*)cap + 3;

            *os_sem = 1; // OS Request
            while ((*bios_sem & 1) && (*os_sem & 1)); // Wait for BIOS to release

            // Disable Legacy SMI
            *(volatile uint32_t*)((uint8_t*)cap + 4) = 0;
            break;
        }

        uint32_t next = (*cap >> 8) & 0xFF;
        if (!next) break;
        ext_cap_ptr += (next << 2);
    }
}
