#include "pro_os.h"
#include <stdint.h>
#include "hal.h"
#include "serial.h"

typedef struct { uint32_t clb, clbu, fb, fbu, is, ie, cmd, rsv0, tfd, sig, ssts, sctl, serr, sact, ci, sntf, fbs, rsv1[11], vendor[4]; } ahci_port_t;
typedef struct { ahci_port_t ports[32]; } HBA_MEM;

static storage_device_t ahci_dev;

static int ahci_read_wrap(storage_device_t* d, uint64_t lba, void* buffer, uint32_t count) {
    (void)d; (void)lba; (void)buffer; (void)count;
    return 0;
}

static int ahci_write_wrap(storage_device_t* d, uint64_t lba, const void* buffer, uint32_t count) {
    (void)d; (void)lba; (void)buffer; (void)count;
    return 0;
}

void ahci_init(uint64_t mmio) {
    if (mmio == 0) return;

    serial_printf("[AHCI] Initializing at %p...\n", (void*)(mmio + hhdm_offset));

    volatile uint32_t* ghc = (volatile uint32_t*)(mmio + hhdm_offset + 0x04);
    *ghc |= (1U << 31);
    *ghc |= (1 << 0);
    int timeout = 0;
    while ((*ghc & (1 << 0)) && timeout < 1000000) { timeout++; __asm__("pause"); }
    if (timeout >= 1000000) { serial_write("[AHCI] Reset timeout.\n"); return; }

    ahci_dev.name = "SATA Storage Device";
    ahci_dev.type = STORAGE_TYPE_SATA;
    ahci_dev.total_blocks = 0;
    ahci_dev.block_size = 512;
    ahci_dev.read = ahci_read_wrap;
    ahci_dev.write = ahci_write_wrap;

    hal_storage_register_device(&ahci_dev);
    serial_write("[AHCI] Driver initialized successfully.\n");
}

void ahci_read(ahci_port_t* active_port, uint32_t slot) { (void)active_port; (void)slot; }
void ahci_write(ahci_port_t* active_port, uint32_t slot) { (void)active_port; (void)slot; }
