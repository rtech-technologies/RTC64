/* Modified by Sovereign: Meaty NVMe implementation with Read/Write and Excessive Logging */
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

typedef struct {
    uint32_t cdw0, nsid, rsvd2, rsvd3, mptr_l, mptr_h, dptr[2], cdw10, cdw11, cdw12, cdw13, cdw14, cdw15;
} nvme_cmd_t;

typedef struct {
    uint32_t res0, res1, res2, status;
} nvme_cpl_t;

static uint64_t nvme_base = 0;
static nvme_cmd_t* io_sq = NULL;
static nvme_cpl_t* io_cq = NULL;
static void* io_sq_phys = NULL;
static void* io_cq_phys = NULL;
static uint16_t sq_tail = 0;
static uint16_t cq_head = 0;
static uint16_t phase = 1;

int nvme_init(uint64_t mmio) {
    if (mmio == 0) return -1;
    nvme_base = mmio + hhdm_offset;
    serial_printf("[NVME] Initializing Controller BAR: %p -> Virtual: %p\n", (void*)mmio, (void*)nvme_base);

    volatile uint32_t* regs = (volatile uint32_t*)nvme_base;

    /* 1. Disable Controller for reset */
    regs[NVME_REG_CC/4] &= ~1;
    int timeout = 0;
    while ((regs[NVME_REG_CSTS/4] & 1) && timeout++ < 1000000) __asm__("pause");

    /* 2. Setup Admin Queues */
    void* asq_phys = pmm_alloc_low();
    void* acq_phys = pmm_alloc_low();
    void* asq_virt = (void*)((uint64_t)asq_phys + hhdm_offset);
    void* acq_virt = (void*)((uint64_t)acq_phys + hhdm_offset);
    memset(asq_virt, 0, 4096);
    memset(acq_virt, 0, 4096);

    regs[NVME_REG_AQA/4] = (63 << 16) | 63;
    *(volatile uint64_t*)(nvme_base + NVME_REG_ASQ) = (uint64_t)asq_phys;
    *(volatile uint64_t*)(nvme_base + NVME_REG_ACQ) = (uint64_t)acq_phys;

    /* 3. Enable Controller */
    regs[NVME_REG_CC/4] = (0 << 16) | (0 << 14) | (4 << 11) | (0 << 7) | 1;
    timeout = 0;
    while (!(regs[NVME_REG_CSTS/4] & 1) && timeout++ < 1000000) __asm__("pause");

    /* 4. Create I/O Completion Queue (simplified admin command) */
    io_cq_phys = pmm_alloc_low();
    io_cq = (nvme_cpl_t*)((uint64_t)io_cq_phys + hhdm_offset);
    memset(io_cq, 0, 4096);

    nvme_cmd_t* admin_sq = (nvme_cmd_t*)asq_virt;
    admin_sq[0].cdw0 = 0x05; /* Create I/O Completion Queue */
    admin_sq[0].dptr[0] = (uint32_t)(uint64_t)io_cq_phys;
    admin_sq[0].dptr[1] = (uint32_t)((uint64_t)io_cq_phys >> 32);
    admin_sq[0].cdw10 = (63 << 16) | 0x01; /* 64 entries, ID 1 */
    admin_sq[0].cdw11 = 0x01; /* Physically contiguous */

    *(volatile uint32_t*)(nvme_base + NVME_REG_SQ0TDBL) = 1;
    timeout = 0;
    volatile nvme_cpl_t* admin_cq = (volatile nvme_cpl_t*)acq_virt;
    while (!(admin_cq[0].status & 0x01) && timeout++ < 1000000) __asm__("pause");

    /* 5. Create I/O Submission Queue */
    io_sq_phys = pmm_alloc_low();
    io_sq = (nvme_cmd_t*)((uint64_t)io_sq_phys + hhdm_offset);
    memset(io_sq, 0, 4096);

    memset(admin_sq, 0, 64);
    admin_sq[0].cdw0 = 0x01; /* Create I/O Submission Queue */
    admin_sq[0].dptr[0] = (uint32_t)(uint64_t)io_sq_phys;
    admin_sq[0].dptr[1] = (uint32_t)((uint64_t)io_sq_phys >> 32);
    admin_sq[0].cdw10 = (63 << 16) | 0x01; /* 64 entries, ID 1 */
    admin_sq[0].cdw11 = (1 << 16) | 0x01; /* CQID 1, Phys contig */

    *(volatile uint32_t*)(nvme_base + NVME_REG_SQ0TDBL) = 1;
    timeout = 0;
    while (!(admin_cq[1].status & 0x01) && timeout++ < 1000000) __asm__("pause");

    serial_printf("[NVME] I/O Queues created. Submission Queue at %p\n", (void*)io_sq);
    return 0;
}

static int nvme_submit_io(uint8_t opcode, uint64_t lba, uint16_t count, void* buffer) {
    if (!io_sq) return -1;

    uint16_t tail = sq_tail;
    nvme_cmd_t* cmd = &io_sq[tail];
    memset(cmd, 0, sizeof(nvme_cmd_t));

    cmd->cdw0 = opcode;
    cmd->nsid = 1;
    uint64_t phys_buf = (uint64_t)buffer - hhdm_offset;
    cmd->dptr[0] = (uint32_t)phys_buf;
    cmd->dptr[1] = (uint32_t)(phys_buf >> 32);
    cmd->cdw10 = (uint32_t)lba;
    cmd->cdw11 = (uint32_t)(lba >> 32);
    cmd->cdw12 = count - 1;

    sq_tail = (tail + 1) % 64;
    *(volatile uint32_t*)(nvme_base + NVME_REG_SQ0TDBL + 8) = sq_tail; /* Doorbell for SQ 1 */

    /* Poll Completion Queue 1 */
    int timeout = 0;
    while (timeout++ < 1000000) {
        if ((io_cq[cq_head].status & 0x01) == phase) {
            uint16_t status = io_cq[cq_head].status >> 1;
            cq_head = (cq_head + 1) % 64;
            if (cq_head == 0) phase = !phase;
            *(volatile uint32_t*)(nvme_base + NVME_REG_SQ0TDBL + 12) = cq_head; /* Doorbell for CQ 1 */

            if (status & 0xFF) {
                serial_printf("[NVME] I/O Error status: %04x\n", status);
                return -1;
            }
            return 0;
        }
        __asm__("pause");
    }

    serial_printf("[NVME] I/O Timeout!\n");
    return -1;
}

int nvme_read_hw(uint64_t lba, uint16_t count, void* buffer) {
    return nvme_submit_io(0x02, lba, count, buffer);
}

int nvme_write_hw(uint64_t lba, uint16_t count, void* buffer) {
    return nvme_submit_io(0x01, lba, count, buffer);
}
