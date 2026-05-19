#include <pro_os.h>

#define AHCI_GHC 0x04
#define AHCI_IS  0x08
#define AHCI_PI  0x0C

#define AHCI_PxCMD  0x18
#define AHCI_PxSSTS 0x28
#define AHCI_PxSERR 0x30

extern uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
extern void pci_write_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val);

void ahci_force_reset(void* hba_base) {
    volatile uint32_t* ghc = (uint32_t*)((uint8_t*)hba_base + AHCI_GHC);
    volatile uint32_t* pi = (uint32_t*)((uint8_t*)hba_base + AHCI_PI);
    for (int i = 0; i < 32; i++) {
        if (*pi & (1 << i)) {
            volatile uint32_t* pxcmd = (uint32_t*)((uint8_t*)hba_base + 0x100 + (i * 0x80) + AHCI_PxCMD);
            volatile uint32_t* pxserr = (uint32_t*)((uint8_t*)hba_base + 0x100 + (i * 0x80) + AHCI_PxSERR);
            *pxcmd &= ~((1 << 0) | (1 << 4));
            while (*pxcmd & ((1 << 14) | (1 << 15)));
            *pxserr = 0xFFFFFFFF;
        }
    }
    *ghc |= (1 << 0);
    while(*ghc & (1 << 0));
    *ghc |= (1 << 31);
    for (int i = 0; i < 32; i++) {
        if (*pi & (1 << i)) {
            volatile uint32_t* pxssts = (uint32_t*)((uint8_t*)hba_base + 0x100 + (i * 0x80) + AHCI_PxSSTS);
            while ((*pxssts & 0x0F) != 0x03);
        }
    }
}

void ahci_init(void) {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t id = pci_read_config(bus, slot, 0, 0);
            if (id != 0xFFFFFFFF) {
                uint32_t class_reg = pci_read_config(bus, slot, 0, 0x08);
                uint8_t class_code = (class_reg >> 24) & 0xFF;
                uint8_t sub_class  = (class_reg >> 16) & 0xFF;
                if (class_code == 0x01 && sub_class == 0x06) {
                    uint32_t command = pci_read_config(bus, slot, 0, 0x04);
                    pci_write_config(bus, slot, 0, 0x04, command | (1 << 1) | (1 << 2));
                    uint32_t bar5 = pci_read_config(bus, slot, 0, 0x24);
                    if (bar5) {
                        void* mmio = (void*)(uint64_t)(bar5 & 0xFFFFFFF0);
                        ahci_force_reset(mmio);
                    }
                }
            }
        }
    }
}
