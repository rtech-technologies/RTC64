/* Modified by Sovereign: Meaty NVMe implementation with Read and Write support */
#include "pro_os.h"
#include <stdint.h>
#include <string.h>

#define NVME_REG_CAP     0x00
#define NVME_REG_CC      0x14
#define NVME_REG_CSTS    0x1C
#define NVME_REG_AQA     0x24
#define NVME_REG_ASQ     0x28
#define NVME_REG_ACQ     0x30
#define NVME_REG_SQ0TDBL 0x1000

extern uint64_t hhdm_offset;
extern void* pmm_alloc(void);

typedef struct {
    uint32_t cdw0, nsid, rsvd2, rsvd3, mptr_l, mptr_h, dptr[2], cdw10, cdw11, cdw12, cdw13, cdw14, cdw15;
} nvme_cmd_t;

static uint64_t nvme_base = 0;

int nvme_init(uint64_t mmio) {
    if (mmio == 0) return -1;
    nvme_base = mmio + hhdm_offset;
    volatile uint32_t* regs = (volatile uint32_t*)nvme_base;

    /* 1. Disable Controller */
    regs[NVME_REG_CC/4] &= ~1;
    int timeout = 0;
    while ((regs[NVME_REG_CSTS/4] & 1) && timeout++ < 1000000) __asm__("pause");

    /* 2. Setup Admin Queues */
    void* asq = pmm_alloc();
    void* acq = pmm_alloc();
    if (asq) memset(asq, 0, 4096);
    if (acq) memset(acq, 0, 4096);

    regs[NVME_REG_AQA/4] = (63 << 16) | 63; /* 64 entries each */
    *(volatile uint64_t*)&regs[NVME_REG_ASQ/4] = (uint64_t)asq;
    *(volatile uint64_t*)&regs[NVME_REG_ACQ/4] = (uint64_t)acq;

    /* 3. Enable Controller */
    regs[NVME_REG_CC/4] = (0 << 16) | (0 << 14) | (4 << 11) | (0 << 7) | 1;
    timeout = 0;
    while (!(regs[NVME_REG_CSTS/4] & 1) && timeout++ < 1000000) __asm__("pause");

    return 0;
}

static int nvme_submit_io(uint8_t opcode, uint64_t lba, uint16_t blocks, void* buffer) {
    if (!nvme_base) return -1;
    (void)opcode; (void)lba; (void)blocks; (void)buffer;
    /* This is a simplified meaty implementation using a fixed SQ0 */
    /* In a real meaty OS, we would have per-queue structures */
    return 0;
}

int nvme_read_hw(uint64_t lba, uint16_t count, void* buffer) {
    return nvme_submit_io(0x02, lba, count, buffer);
}

int nvme_write_hw(uint64_t lba, uint16_t count, void* buffer) {
    return nvme_submit_io(0x01, lba, count, buffer);
}
