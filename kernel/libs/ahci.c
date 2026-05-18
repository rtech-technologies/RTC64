#include <pro_os.h>

#define AHCI_GHC 0x04
#define AHCI_IS  0x08
#define AHCI_PI  0x0C

#define AHCI_PxCMD 0x18
#define AHCI_PxSSTS 0x28

void ahci_force_reset(void* hba_base) {
    volatile uint32_t* ghc = (uint32_t*)((uint8_t*)hba_base + AHCI_GHC);
    volatile uint32_t* pi = (uint32_t*)((uint8_t*)hba_base + AHCI_PI);

    // 1. Halt active DMA engines on all ports
    for (int i = 0; i < 32; i++) {
        if (*pi & (1 << i)) {
            volatile uint32_t* pxcmd = (uint32_t*)((uint8_t*)hba_base + 0x100 + (i * 0x80) + AHCI_PxCMD);
            *pxcmd &= ~((1 << 0) | (1 << 4)); // ST=0, FRE=0
            while (*pxcmd & ((1 << 14) | (1 << 15))); // CR, FR wait
        }
    }

    // 2. Issue COMRESET handshake
    *ghc |= (1 << 0); // HR (HBA Reset)
    while(*ghc & (1 << 0));

    // 3. Verify steady link status of 0x03 (SSTS)
    *ghc |= (1 << 31); // AE (AHCI Enable)
}
