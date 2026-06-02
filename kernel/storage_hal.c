#include "hal.h"
#include "pro_os.h"

void hal_nvme_init(uint64_t mmio) {
    extern void nvme_init(uint64_t);
    nvme_init(mmio);
}

void hal_sata_init(uint64_t mmio) {
    extern void ahci_init(uint64_t);
    ahci_init(mmio);
}
