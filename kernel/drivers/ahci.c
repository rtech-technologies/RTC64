#include "pro_os.h"
#include <stdint.h>
#include "serial.h"
#include "pmm.h"
#include <string.h>

#define SATA_SIG_ATA   0x00000101
#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_SEMB  0xC33C0101
#define SATA_SIG_PM    0x96690101

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

typedef enum { FIS_TYPE_REG_H2D = 0x27, FIS_TYPE_REG_D2H = 0x34, FIS_TYPE_DMA_ACT = 0x39, FIS_TYPE_DMA_SETUP = 0x41, FIS_TYPE_DATA = 0x46, FIS_TYPE_BIST = 0x58, FIS_TYPE_PIO_SETUP = 0x5F, FIS_TYPE_DEV_BITS = 0xA1 } FIS_TYPE;

typedef struct {
	uint8_t  fis_type;
	uint8_t  pmport:4;
	uint8_t  rsv0:3;
	uint8_t  c:1;
	uint8_t  command;
	uint8_t  featurel;
	uint8_t  lba0;
	uint8_t  lba1;
	uint8_t  lba2;
	uint8_t  device;
	uint8_t  lba3;
	uint8_t  lba4;
	uint8_t  lba5;
	uint8_t  featureh;
	uint8_t  countl;
	uint8_t  counth;
	uint8_t  icc;
	uint8_t  control;
	uint8_t  rsv1[4];
} FIS_REG_H2D;

typedef struct {
	uint32_t dba;
	uint32_t dbau;
	uint32_t rsv0;
	uint32_t dbc:22;
	uint32_t rsv1:9;
	uint32_t i:1;
} HBA_PRDT_ENTRY;

typedef struct {
	uint8_t  cfis[64];
	uint8_t  acmd[16];
	uint8_t  rsv[48];
	HBA_PRDT_ENTRY prdt_entry[1];
} HBA_CMD_TBL;

typedef struct {
	uint8_t  cfl:5;
	uint8_t  a:1;
	uint8_t  w:1;
	uint8_t  p:1;
	uint8_t  t:1;
	uint8_t  r:1;
	uint8_t  b:1;
	uint8_t  c:1;
	uint8_t  pmp:4;
	uint16_t prdtl;
	volatile uint32_t prdbc;
	uint32_t ctba;
	uint32_t ctbau;
	uint32_t rsv[4];
} HBA_CMD_HEADER;

typedef struct {
	uint32_t clb;
	uint32_t clbu;
	uint32_t fb;
	uint32_t fbu;
	uint32_t is;
	uint32_t ie;
	uint32_t cmd;
	uint32_t rsv0;
	uint32_t tfd;
	uint32_t sig;
	uint32_t ssts;
	uint32_t sctl;
	uint32_t serr;
	uint32_t sact;
	uint32_t ci;
	uint32_t sntf;
	uint32_t fbs;
	uint32_t rsv1[11];
	uint32_t vendor[4];
} HBA_PORT;

typedef struct {
	uint32_t cap;
	uint32_t ghc;
	uint32_t is;
	uint32_t pi;
	uint32_t vs;
	uint32_t ccc_ctl;
	uint32_t ccc_pts;
	uint32_t em_loc;
	uint32_t em_ctl;
	uint32_t cap2;
	uint32_t bohc;
	uint8_t  rsv[0xA0-0x2C];
	uint8_t  vendor[0x100-0xA0];
	HBA_PORT ports[1];
} HBA_MEM;

static HBA_PORT *active_port = NULL;
static storage_device_t sata_dev;

static int find_cmd_slot(HBA_PORT *port) {
	uint32_t slots = (port->sact | port->ci);
	for (int i = 0; i < 32; i++) {
		if ((slots & 1) == 0) return i;
		slots >>= 1;
	}
	return -1;
}

int ahci_read(storage_device_t* d, uint64_t lba, void* buf, uint32_t count) {
	if (!active_port) return -1;
	int slot = find_cmd_slot(active_port);
	if (slot == -1) return -1;

	HBA_CMD_HEADER *hdr = (HBA_CMD_HEADER*)(((uintptr_t)active_port->clb | ((uintptr_t)active_port->clbu << 32)) + hhdm_offset);
	hdr += slot;
	hdr->cfl = sizeof(FIS_REG_H2D)/4;
	hdr->w = 0;
	hdr->prdtl = (uint16_t)((count-1)>>4) + 1;

	HBA_CMD_TBL *tbl = (HBA_CMD_TBL*)(((uintptr_t)hdr->ctba | ((uintptr_t)hdr->ctbau << 32)) + hhdm_offset);
	memset(tbl, 0, sizeof(HBA_CMD_TBL) + (hdr->prdtl-1)*sizeof(HBA_PRDT_ENTRY));

	uintptr_t phys_buf = (uintptr_t)buf - hhdm_offset;
	for (int i = 0; i < hdr->prdtl - 1; i++) {
		tbl->prdt_entry[i].dba = (uint32_t)phys_buf;
		tbl->prdt_entry[i].dbau = (uint32_t)(phys_buf >> 32);
		tbl->prdt_entry[i].dbc = 8*1024-1; // 8K per entry
		tbl->prdt_entry[i].i = 1;
		phys_buf += 8*1024;
		count -= 16;
	}
	tbl->prdt_entry[hdr->prdtl-1].dba = (uint32_t)phys_buf;
	tbl->prdt_entry[hdr->prdtl-1].dbau = (uint32_t)(phys_buf >> 32);
	tbl->prdt_entry[hdr->prdtl-1].dbc = (count << 9) - 1;
	tbl->prdt_entry[hdr->prdtl-1].i = 1;

	FIS_REG_H2D *fis = (FIS_REG_H2D*)(&tbl->cfis);
	fis->fis_type = FIS_TYPE_REG_H2D;
	fis->c = 1;
	fis->command = 0x25; // READ DMA EXT
	fis->lba0 = (uint8_t)lba; fis->lba1 = (uint8_t)(lba>>8); fis->lba2 = (uint8_t)(lba>>16);
	fis->device = 1 << 6; // LBA mode
	fis->lba3 = (uint8_t)(lba>>24); fis->lba4 = (uint8_t)(lba>>32); fis->lba5 = (uint8_t)(lba>>40);
	fis->countl = (uint8_t)(d->block_size == 512 ? count : count); // Simplification
	fis->counth = (uint8_t)(count >> 8);

	while ((active_port->tfd & (0x80 | 0x08)) && count > 0) __asm__("pause");
	active_port->ci = (1 << slot);
	while (1) {
		if (!(active_port->ci & (1 << slot))) break;
		if (active_port->is & (1 << 30)) return -1;
	}
	return 0;
}

int ahci_write(storage_device_t* d, uint64_t lba, const void* buf, uint32_t count) {
	(void)d;
	if (!active_port) return -1;
	int slot = find_cmd_slot(active_port);
	if (slot == -1) return -1;

	HBA_CMD_HEADER *hdr = (HBA_CMD_HEADER*)(((uintptr_t)active_port->clb | ((uintptr_t)active_port->clbu << 32)) + hhdm_offset);
	hdr += slot;
	hdr->cfl = sizeof(FIS_REG_H2D)/4;
	hdr->w = 1; // Write
	hdr->prdtl = (uint16_t)((count-1)>>4) + 1;

	HBA_CMD_TBL *tbl = (HBA_CMD_TBL*)(((uintptr_t)hdr->ctba | ((uintptr_t)hdr->ctbau << 32)) + hhdm_offset);
	memset(tbl, 0, sizeof(HBA_CMD_TBL) + (hdr->prdtl-1)*sizeof(HBA_PRDT_ENTRY));

	uintptr_t phys_buf = (uintptr_t)buf - hhdm_offset;
	for (int i = 0; i < hdr->prdtl - 1; i++) {
		tbl->prdt_entry[i].dba = (uint32_t)phys_buf;
		tbl->prdt_entry[i].dbau = (uint32_t)(phys_buf >> 32);
		tbl->prdt_entry[i].dbc = 8*1024-1;
		tbl->prdt_entry[i].i = 1;
		phys_buf += 8*1024;
		count -= 16;
	}
	tbl->prdt_entry[hdr->prdtl-1].dba = (uint32_t)phys_buf;
	tbl->prdt_entry[hdr->prdtl-1].dbau = (uint32_t)(phys_buf >> 32);
	tbl->prdt_entry[hdr->prdtl-1].dbc = (count << 9) - 1;
	tbl->prdt_entry[hdr->prdtl-1].i = 1;

	FIS_REG_H2D *fis = (FIS_REG_H2D*)(&tbl->cfis);
	fis->fis_type = FIS_TYPE_REG_H2D;
	fis->c = 1;
	fis->command = 0x35; // WRITE DMA EXT
	fis->lba0 = (uint8_t)lba; fis->lba1 = (uint8_t)(lba>>8); fis->lba2 = (uint8_t)(lba>>16);
	fis->device = 1 << 6;
	fis->lba3 = (uint8_t)(lba>>24); fis->lba4 = (uint8_t)(lba>>32); fis->lba5 = (uint8_t)(lba>>40);
	fis->countl = (uint8_t)count;
	fis->counth = (uint8_t)(count >> 8);

	while ((active_port->tfd & (0x80 | 0x08))) __asm__("pause");
	active_port->ci = (1 << slot);
	while (1) {
		if (!(active_port->ci & (1 << slot))) break;
		if (active_port->is & (1 << 30)) return -1;
	}
	return 0;
}

void ahci_init(uint64_t mmio) {
	if(!mmio) return;
	HBA_MEM *hba = (HBA_MEM*)(mmio + hhdm_offset);
	serial_printf("[AHCI] Init at %lx\n", mmio);
	hba->ghc |= (1U << 31); // AE
	uint32_t pi = hba->pi;
	for (int i = 0; i < 32; i++) {
		if (pi & (1 << i)) {
			HBA_PORT *p = &hba->ports[i];
			uint32_t ssts = p->ssts;
			uint8_t det = ssts & 0xF;
			uint8_t ipm = (ssts >> 8) & 0xF;
			if (det == HBA_PORT_DET_PRESENT && ipm == HBA_PORT_IPM_ACTIVE) {
				if (p->sig == SATA_SIG_ATA) {
					serial_printf("[AHCI] Found SATA Drive on port %d\n", i);
					active_port = p;

					void* cl_virt = pmm_alloc(1);
					uintptr_t cl_phys = (uintptr_t)cl_virt - hhdm_offset;
					p->clb = (uint32_t)cl_phys; p->clbu = (uint32_t)(cl_phys >> 32);

					void* fis_virt = pmm_alloc(1);
					uintptr_t fis_phys = (uintptr_t)fis_virt - hhdm_offset;
					p->fb = (uint32_t)fis_phys; p->fbu = (uint32_t)(fis_phys >> 32);

					HBA_CMD_HEADER *hdr = (HBA_CMD_HEADER*)cl_virt;
					for(int j=0; j<32; j++) {
						void* tbl_virt = pmm_alloc(1);
						uintptr_t tbl_phys = (uintptr_t)tbl_virt - hhdm_offset;
						hdr[j].ctba = (uint32_t)tbl_phys;
						hdr[j].ctbau = (uint32_t)(tbl_phys >> 32);
					}

					p->cmd |= (1 << 4) | (1 << 0);

					uint16_t identify[256];
					int slot = find_cmd_slot(p);
					HBA_CMD_HEADER *h = &((HBA_CMD_HEADER*)cl_virt)[slot];
					h->cfl = sizeof(FIS_REG_H2D)/4; h->w = 0; h->prdtl = 1;
					HBA_CMD_TBL *t = (HBA_CMD_TBL*)(((uintptr_t)h->ctba | ((uintptr_t)h->ctbau << 32)) + hhdm_offset);
					memset(t, 0, sizeof(HBA_CMD_TBL));
					t->prdt_entry[0].dba = (uintptr_t)identify - hhdm_offset;
					t->prdt_entry[0].dbc = 511; t->prdt_entry[0].i = 1;
					FIS_REG_H2D *fis = (FIS_REG_H2D*)(&t->cfis);
					fis->fis_type = FIS_TYPE_REG_H2D; fis->c = 1; fis->command = 0xEC;
					p->ci = (1 << slot);
					while (p->ci & (1 << slot)) __asm__("pause");

					uint64_t sectors = identify[60] | ((uint64_t)identify[61] << 16);
					if (identify[83] & (1 << 10)) sectors = identify[100] | ((uint64_t)identify[101] << 16) | ((uint64_t)identify[102] << 32) | ((uint64_t)identify[103] << 48);

					sata_dev.name = "SATA Disk"; sata_dev.type = STORAGE_TYPE_SATA;
					sata_dev.total_blocks = sectors; sata_dev.block_size = 512;
					sata_dev.read = ahci_read; sata_dev.write = ahci_write;
					hal_storage_register_device(&sata_dev);
					break;
				}
			}
		}
	}
}
