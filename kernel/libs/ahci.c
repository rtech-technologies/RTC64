#include <pro_os.h>

#define AHCI_GHC 0x04
#define AHCI_IS  0x08
#define AHCI_PI  0x0C

#define AHCI_PxCMD  0x18
#define AHCI_PxSSTS 0x28
#define AHCI_PxSERR 0x30

void ahci_force_reset(void* hba_base) {
    volatile uint32_t* ghc = (uint32_t*)((uint8_t*)hba_base + AHCI_GHC);
    volatile uint32_t* pi = (uint32_t*)((uint8_t*)hba_base + AHCI_PI);

    // 1. Halt active DMA engines and clear error blocks
    for (int i = 0; i < 32; i++) {
        if (*pi & (1 << i)) {
            volatile uint32_t* pxcmd = (uint32_t*)((uint8_t*)hba_base + 0x100 + (i * 0x80) + AHCI_PxCMD);
            volatile uint32_t* pxserr = (uint32_t*)((uint8_t*)hba_base + 0x100 + (i * 0x80) + AHCI_PxSERR);

            *pxcmd &= ~((1 << 0) | (1 << 4)); // ST=0, FRE=0
            while (*pxcmd & ((1 << 14) | (1 << 15))); // CR, FR wait

            *pxserr = 0xFFFFFFFF; // Clear error register
        }
    }

    // 2. Issue COMRESET handshake
    *ghc |= (1 << 0); // HR (HBA Reset)
    while(*ghc & (1 << 0));

    // 3. Confirm steady link status of 0x03 (SSTS)
    *ghc |= (1 << 31); // AE (AHCI Enable)

    for (int i = 0; i < 32; i++) {
        if (*pi & (1 << i)) {
            volatile uint32_t* pxssts = (uint32_t*)((uint8_t*)hba_base + 0x100 + (i * 0x80) + AHCI_PxSSTS);
            while ((*pxssts & 0x0F) != 0x03); // Wait for link 0x03
        }
    }
}
