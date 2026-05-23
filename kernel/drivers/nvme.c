#include "pro_os.h"
#include <stdint.h>
#include <string.h>
#include "serial.h"
#include "pmm.h"

#define NVME_REG_CAP  0x00
#define NVME_REG_CC   0x14
#define NVME_REG_CSTS 0x1C
#define NVME_REG_AQA  0x24
#define NVME_REG_ASQ  0x28
#define NVME_REG_ACQ  0x30

typedef struct { uint32_t cdw0, nsid; uint64_t rsv0, metadata, prp1, prp2; uint32_t cdw10, cdw11, cdw12, cdw13, cdw14, cdw15; } nvme_sqe_t;
typedef struct { uint32_t result, rsv0; uint16_t sq_head, sq_id, command_id, status; } nvme_cqe_t;

typedef struct {
    volatile uint32_t* regs;
    nvme_sqe_t *admin_sq, *io_sq; nvme_cqe_t *admin_cq, *io_cq;
    uint16_t asq_tail, acq_head, acq_phase, io_sq_tail, io_cq_head, io_cq_phase;
    uint32_t db_stride;
} nvme_ctrl_t;

static nvme_ctrl_t g_nvme; static storage_device_t nvme_dev;

static void nvme_db(nvme_ctrl_t* c, int qid, int is_cq, uint16_t val) { c->regs[(0x1000 + (2*qid + (is_cq?1:0))*c->db_stride)/4] = val; }

static int nvme_cmd(nvme_ctrl_t* c, int qid, nvme_sqe_t* sqe_in) {
    nvme_sqe_t* sq; nvme_cqe_t* cq; uint16_t *tail, *head, *phase;
    if (qid==0) { sq=c->admin_sq; cq=c->admin_cq; tail=&c->asq_tail; head=&c->acq_head; phase=&c->acq_phase; }
    else { sq=c->io_sq; cq=c->io_cq; tail=&c->io_sq_tail; head=&c->io_cq_head; phase=&c->io_cq_phase; }

    memcpy(&sq[*tail], sqe_in, sizeof(nvme_sqe_t));
    *tail = (*tail + 1) % 64;
    nvme_db(c, qid, 0, *tail);

    while(1) {
        volatile nvme_cqe_t* e = &cq[*head];
        if ((e->status & 1) == *phase) {
            *head = (*head + 1) % 64; if (*head == 0) *phase ^= 1;
            nvme_db(c, qid, 1, *head);
            return (e->status >> 1) == 0 ? 0 : -1;
        }
        __asm__("pause");
    }
}

int nvme_read(storage_device_t* d, uint64_t lba, void* b, uint32_t c) {
    (void)d; nvme_sqe_t sqe = { .cdw0=0x02, .nsid=1, .prp1=(uintptr_t)b-hhdm_offset, .cdw10=(uint32_t)lba, .cdw11=(uint32_t)(lba>>32), .cdw12=(c-1)&0xFFFF };
    return nvme_cmd(&g_nvme, 1, &sqe);
}
int nvme_write(storage_device_t* d, uint64_t lba, const void* b, uint32_t c) {
    (void)d; nvme_sqe_t sqe = { .cdw0=0x01, .nsid=1, .prp1=(uintptr_t)b-hhdm_offset, .cdw10=(uint32_t)lba, .cdw11=(uint32_t)(lba>>32), .cdw12=(c-1)&0xFFFF };
    return nvme_cmd(&g_nvme, 1, &sqe);
}

void nvme_init(uint64_t mmio) {
    if(!mmio) return;
    nvme_ctrl_t* c = &g_nvme; c->regs = (volatile uint32_t*)(mmio + hhdm_offset);
    serial_printf("[NVMe] Initializing Sovereign Controller at %lx\n", mmio);
    c->regs[NVME_REG_CC/4] &= ~1; while(c->regs[NVME_REG_CSTS/4] & 1) __asm__("pause");
    c->db_stride = 4 << ((*(uint64_t*)&c->regs[0] >> 32) & 0xF);
    c->admin_sq = (nvme_sqe_t*)pmm_alloc(1); c->admin_cq = (nvme_cqe_t*)pmm_alloc(1);
    memset(c->admin_sq, 0, 4096); memset(c->admin_cq, 0, 4096);
    c->asq_tail=0; c->acq_head=0; c->acq_phase=1;
    c->regs[NVME_REG_ASQ/4] = (uint32_t)((uintptr_t)c->admin_sq - hhdm_offset);
    c->regs[NVME_REG_ASQ/4+1] = (uint32_t)(((uintptr_t)c->admin_sq - hhdm_offset)>>32);
    c->regs[NVME_REG_ACQ/4] = (uint32_t)((uintptr_t)c->admin_cq - hhdm_offset);
    c->regs[NVME_REG_ACQ/4+1] = (uint32_t)(((uintptr_t)c->admin_cq - hhdm_offset)>>32);
    c->regs[NVME_REG_AQA/4] = (63 << 16) | 63;
    c->regs[NVME_REG_CC/4] = (0<<16)|(6<<12)|(4<<7)|1; while(!(c->regs[NVME_REG_CSTS/4] & 1)) __asm__("pause");

    c->io_sq = (nvme_sqe_t*)pmm_alloc(1); c->io_cq = (nvme_cqe_t*)pmm_alloc(1);
    memset(c->io_sq, 0, 4096); memset(c->io_cq, 0, 4096);
    c->io_sq_tail=0; c->io_cq_head=0; c->io_cq_phase=1;
    nvme_sqe_t cq_sqe = { .cdw0=0x05, .prp1=(uintptr_t)c->io_cq-hhdm_offset, .cdw10=(63<<16)|1, .cdw11=1 }; nvme_cmd(c, 0, &cq_sqe);
    nvme_sqe_t sq_sqe = { .cdw0=0x01, .prp1=(uintptr_t)c->io_sq-hhdm_offset, .cdw10=(63<<16)|1, .cdw11=1 }; nvme_cmd(c, 0, &sq_sqe);

    uint8_t ns_id[4096]; nvme_sqe_t id_sqe = { .cdw0=0x06, .nsid=1, .prp1=(uintptr_t)ns_id-hhdm_offset, .cdw10=0 }; nvme_cmd(c, 0, &id_sqe);
    nvme_dev.name = "Sovereign NVMe"; nvme_dev.type = STORAGE_TYPE_NVME;
    nvme_dev.total_blocks = *(uint64_t*)&ns_id[0]; nvme_dev.block_size = 1 << ns_id[128 + ns_id[26]*4 + 2];
    nvme_dev.read = nvme_read; nvme_dev.write = nvme_write;
    hal_storage_register_device(&nvme_dev);
}
