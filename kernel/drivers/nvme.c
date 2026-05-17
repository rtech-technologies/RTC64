#include "pro_os.h"
#include <stdint.h>

/* Sovereign NVMe Driver - Native implementation */

#define NVME_REG_CC 0x14
#define NVME_REG_CSTS 0x1C

void nvme_init(uint64_t base_addr) {
    if (base_addr == 0) return;
    volatile uint32_t* regs = (volatile uint32_t*)(base_addr + hhdm_offset);
    (void)regs;
    /* 1. Disable controller to configure */
    // PRO_TASK: Implement safe NVMe configuration sequence

    /* 2. Setup Admin Queues */
    // PRO_TASK: Allocate and set ASQ, ACQ, and AQA

    /* 3. Enable controller */
    regs[NVME_REG_CC/4] |= 0x01;
}
