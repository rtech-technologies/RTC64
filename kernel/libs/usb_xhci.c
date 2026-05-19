#include <pro_os.h>

#define XHCI_HCCPARAMS1 0x10
#define XHCI_ECP_MASK   0xFFFF0000

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

static inline void outl(uint16_t port, uint32_t val) {
    __asm__ volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile ( "inl %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}

uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(PCI_CONFIG_ADDR, address);
    return inl(PCI_CONFIG_DATA);
}

void pci_write_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) | (func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(PCI_CONFIG_ADDR, address);
    outl(PCI_CONFIG_DATA, val);
}

void xhci_bios_handover(void* base) {
    uint32_t hccp1 = *(volatile uint32_t*)((uint8_t*)base + XHCI_HCCPARAMS1);
    uint32_t ext_cap_ptr = (hccp1 & XHCI_ECP_MASK) >> 14;

    while (ext_cap_ptr) {
        volatile uint32_t* cap = (volatile uint32_t*)((uint8_t*)base + ext_cap_ptr);
        uint32_t cap_id = *cap & 0xFF;
        if (cap_id == 1) {
            volatile uint8_t* bios_sem = (volatile uint8_t*)cap + 2;
            volatile uint8_t* os_sem   = (volatile uint8_t*)cap + 3;
            *os_sem = 1;
            while ((*bios_sem & 1) && (*os_sem & 1));
            *(volatile uint32_t*)((uint8_t*)cap + 4) = 0;
            break;
        }
        uint32_t next = (*cap >> 8) & 0xFF;
        if (!next) break;
        ext_cap_ptr += (next << 2);
    }
}

void xhci_init(void) {
    xhci_pci_scan();
}

void xhci_pci_scan(void) {
    for (uint32_t bus = 0; bus < 256; bus++) {
        for (uint32_t slot = 0; slot < 32; slot++) {
            uint32_t id = pci_read_config(bus, slot, 0, 0);
            if (id != 0xFFFFFFFF) {
                // Enable MMIO for the device
                uint32_t command = pci_read_config(bus, slot, 0, 0x04);
                pci_write_config(bus, slot, 0, 0x04, command | (1 << 1) | (1 << 2)); // Memory Space + Bus Master

                uint32_t class_reg = pci_read_config(bus, slot, 0, 0x08);
                uint8_t class_code = (class_reg >> 24) & 0xFF;
                uint8_t sub_class  = (class_reg >> 16) & 0xFF;
                uint8_t prog_if    = (class_reg >> 8)  & 0xFF;

                if (class_code == 0x0C && sub_class == 0x03 && prog_if == 0x30) {
                    uint32_t bar0 = pci_read_config(bus, slot, 0, 0x10);
                    if (bar0 && !(bar0 & 1)) {
                         void* mmio = (void*)(uint64_t)(bar0 & 0xFFFFFFF0);
                         xhci_bios_handover(mmio);
                    }
                }
            }
        }
    }
}
