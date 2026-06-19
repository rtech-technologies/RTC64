/* Modified by Sovereign: Meaty NVMe implementation with Read/Write and Excessive Logging
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "serial.h"

#define NVME_REG_CAP     0x00
#define NVME_REG_CC      0x14
#define NVME_REG_CSTS    0x1C
#define NVME_REG_AQA     0x24
#define NVME_REG_ASQ     0x28
#define NVME_REG_ACQ     0x30
#define NVME_REG_SQ0TDBL 0x1000

extern uint64_t hhdm_offset;
extern void* pmm_alloc_low(void);

typedef struct {
    uint32_t cdw0, nsid, rsvd2, rsvd3, mptr_l, mptr_h, dptr[2], cdw10, cdw11, cdw12, cdw13, cdw14, cdw15;
} nvme_cmd_t;

static uint64_t nvme_base = 0;
static void* sq0_virt = NULL;
static void* cq0_virt = NULL;
static uint16_t sq0_tail = 0;
static uint16_t cq0_head = 0;

int nvme_init(uint64_t mmio) {
    if (mmio == 0) return -1;
    nvme_base = mmio + hhdm_offset;
    serial_printf("[NVME] Initializing Controller BAR: %p -> Virtual: %p\n", mmio, nvme_base);

    volatile uint32_t* regs = (volatile uint32_t*)nvme_base;

    /* 1. Disable Controller for reset */
    serial_printf("[NVME] Resetting controller...\n");
    regs[NVME_REG_CC/4] &= ~1;
    int timeout = 0;
    while ((regs[NVME_REG_CSTS/4] & 1) && timeout++ < 1000000) __asm__("pause");

    /* 2. Setup Admin Queues */
    void* asq_phys = pmm_alloc_low();
    void* acq_phys = pmm_alloc_low();
    sq0_virt = (void*)((uint64_t)asq_phys + hhdm_offset);
    cq0_virt = (void*)((uint64_t)acq_phys + hhdm_offset);
    memset(sq0_virt, 0, 4096);
    memset(cq0_virt, 0, 4096);

    regs[NVME_REG_AQA/4] = (63 << 16) | 63;
    *(volatile uint64_t*)(nvme_base + NVME_REG_ASQ) = (uint64_t)asq_phys;
    *(volatile uint64_t*)(nvme_base + NVME_REG_ACQ) = (uint64_t)acq_phys;

    /* 3. Enable Controller */
    regs[NVME_REG_CC/4] = (0 << 16) | (0 << 14) | (4 << 11) | (0 << 7) | 1;
    timeout = 0;
    while (!(regs[NVME_REG_CSTS/4] & 1) && timeout++ < 1000000) __asm__("pause");

    serial_printf("[NVME] Executive initialization complete.\n");
    return 0;
}

static int nvme_submit_io(uint8_t opcode, uint64_t lba, uint16_t blocks, void* buffer) {
    if (!nvme_base || !sq0_virt) return -1;
    serial_printf("[NVME] I/O Request: Op=%02x, LBA=%llu, Count=%u, Buffer=%p\n", opcode, lba, blocks, buffer);

    nvme_cmd_t* cmd = &((nvme_cmd_t*)sq0_virt)[sq0_tail];
    memset(cmd, 0, sizeof(nvme_cmd_t));
    cmd->cdw0 = opcode;
    cmd->nsid = 1;
    cmd->dptr[0] = (uint32_t)(uintptr_t)buffer;
    cmd->dptr[1] = (uint32_t)((uintptr_t)buffer >> 32);
    cmd->cdw10 = (uint32_t)lba;
    cmd->cdw11 = (uint32_t)(lba >> 32);
    cmd->cdw12 = (blocks - 1);

    sq0_tail = (sq0_tail + 1) % 64;
    *(volatile uint32_t*)(nvme_base + NVME_REG_SQ0TDBL) = sq0_tail;

    /* Wait for completion (simplified polling) */
    volatile uint32_t* cq = (volatile uint32_t*)cq0_virt;
    int timeout = 0;
    while (!(cq[cq0_head * 4 + 3] & 0x1) && timeout++ < 1000000) __asm__("pause");
    cq0_head = (cq0_head + 1) % 64;

    return 0;
}

int nvme_read_hw(uint64_t lba, uint16_t count, void* buffer) {
    return nvme_submit_io(0x02, lba, count, buffer);
}

int nvme_write_hw(uint64_t lba, uint16_t count, void* buffer) {
    return nvme_submit_io(0x01, lba, count, buffer);
}
