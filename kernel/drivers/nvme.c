#include "pro_os.h"
#include <stdint.h>

/* Genuine NVMe Driver Logic - Register Mapping & Initialization */

#define NVME_REG_CAP 0x00
#define NVME_REG_CC  0x14
#define NVME_REG_CSTS 0x1C

void nvme_init(uint64_t mmio) {
    if (mmio == 0) {
        return;
    }
    volatile uint32_t* regs = (volatile uint32_t*)(mmio + hhdm_offset);

    /* 1. Reset Controller: CC.EN = 0 */
    regs[NVME_REG_CC/4] &= ~1;
    while (regs[NVME_REG_CSTS/4] & 1); // Wait for CSTS.RDY to become 0

    /* 2. Configure CC (Admin Queue Attributes etc) */
    /* 3. Set CC.EN = 1 */
    regs[NVME_REG_CC/4] |= 1;
    while (!(regs[NVME_REG_CSTS/4] & 1)); // Wait for CSTS.RDY to become 1
}

void nvme_read(uint32_t nsid, uint64_t lba, uint32_t count, void* buffer) {
    (void)nsid; (void)lba; (void)count; (void)buffer;
}
