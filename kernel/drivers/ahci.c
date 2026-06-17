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
typedef struct { uint8_t fis_type, pmport:4, rsv0:3, c:1, command, featurel, lba0, lba1, lba2, device, lba3, lba4, lba5, featureh, countl, counth, icc, control, rsv1[4]; } FIS_REG_H2D;
typedef struct { uint32_t dba, dbau, rsv0, dbc:22, rsv1:9, i:1; } HBA_PRDT_ENTRY;
typedef struct { uint8_t cfis[64], acmd[16], rsv[48]; HBA_PRDT_ENTRY prdt_entry[1]; } HBA_CMD_TBL;
typedef struct { uint8_t cfl:5, a:1, w:1, p:1, t:1, r:1, b:1, c:1, pmp:4; uint16_t prdtl; volatile uint32_t prdbc; uint32_t ctba, ctbau, rsv[4]; } HBA_CMD_HEADER;
typedef struct { uint32_t clb, clbu, fb, fbu, is, ie, cmd, rsv0, tfd, sig, ssts, sctl, serr, sact, ci, sntf, fbs, rsv1[11], vendor[4]; } HBA_PORT;
typedef struct { uint32_t cap, ghc, is, pi, vs, ccc_ctl, ccc_pts, em_loc, em_ctl, cap2, bohc; uint8_t rsv[0xA0-0x2C], vendor[0x100-0xA0]; HBA_PORT ports[1]; } HBA_MEM;

static HBA_PORT *active_port = NULL; static storage_device_t sata_dev;

static int find_cmd_slot(HBA_PORT *port) {
	uint32_t slots = (port->sact | port->ci);
	for (int i = 0; i < 32; i++) { if ((slots & 1) == 0) return i; slots >>= 1; }
	return -1;
}

int ahci_read(storage_device_t* d, uint64_t lba, void* buf, uint32_t count) {
	(void)d; if (!active_port) return -1; int slot = find_cmd_slot(active_port); if (slot == -1) return -1;
	HBA_CMD_HEADER *hdr = (HBA_CMD_HEADER*)(((uintptr_t)active_port->clb | ((uintptr_t)active_port->clbu << 32)) + hhdm_offset);
	hdr += slot; hdr->cfl = sizeof(FIS_REG_H2D)/4; hdr->w = 0; hdr->prdtl = (uint16_t)((count-1)>>4) + 1;
	HBA_CMD_TBL *tbl = (HBA_CMD_TBL*)(((uintptr_t)hdr->ctba | ((uintptr_t)hdr->ctbau << 32)) + hhdm_offset);
	memset(tbl, 0, sizeof(HBA_CMD_TBL) + (hdr->prdtl-1)*sizeof(HBA_PRDT_ENTRY));
	uintptr_t phys_buf = (uintptr_t)buf - hhdm_offset;
	for (int i = 0; i < hdr->prdtl - 1; i++) {
		tbl->prdt_entry[i].dba = (uint32_t)phys_buf; tbl->prdt_entry[i].dbau = (uint32_t)(phys_buf >> 32);
		tbl->prdt_entry[i].dbc = 8*1024-1; tbl->prdt_entry[i].i = 1; phys_buf += 8*1024; count -= 16;
	}
	tbl->prdt_entry[hdr->prdtl-1].dba = (uint32_t)phys_buf; tbl->prdt_entry[hdr->prdtl-1].dbau = (uint32_t)(phys_buf >> 32);
	tbl->prdt_entry[hdr->prdtl-1].dbc = (count << 9) - 1; tbl->prdt_entry[hdr->prdtl-1].i = 1;
	FIS_REG_H2D *fis = (FIS_REG_H2D*)(&tbl->cfis); fis->fis_type = FIS_TYPE_REG_H2D; fis->c = 1; fis->command = 0x25;
	fis->lba0 = (uint8_t)lba; fis->lba1 = (uint8_t)(lba>>8); fis->lba2 = (uint8_t)(lba>>16); fis->device = 1 << 6;
	fis->lba3 = (uint8_t)(lba>>24); fis->lba4 = (uint8_t)(lba>>32); fis->lba5 = (uint8_t)(lba>>40);
	fis->countl = (uint8_t)count; fis->counth = (uint8_t)(count >> 8);
	while ((active_port->tfd & (0x80 | 0x08))) __asm__("pause"); active_port->ci = (1 << slot);
	while (active_port->ci & (1 << slot)) { if (active_port->is & (1 << 30)) return -1; }
	return 0;
}

int ahci_write(storage_device_t* d, uint64_t lba, const void* buf, uint32_t count) {
	(void)d; if (!active_port) return -1; int slot = find_cmd_slot(active_port); if (slot == -1) return -1;
	HBA_CMD_HEADER *hdr = (HBA_CMD_HEADER*)(((uintptr_t)active_port->clb | ((uintptr_t)active_port->clbu << 32)) + hhdm_offset);
	hdr += slot; hdr->cfl = sizeof(FIS_REG_H2D)/4; hdr->w = 1; hdr->prdtl = (uint16_t)((count-1)>>4) + 1;
	HBA_CMD_TBL *tbl = (HBA_CMD_TBL*)(((uintptr_t)hdr->ctba | ((uintptr_t)hdr->ctbau << 32)) + hhdm_offset);
	memset(tbl, 0, sizeof(HBA_CMD_TBL) + (hdr->prdtl-1)*sizeof(HBA_PRDT_ENTRY));
	uintptr_t phys_buf = (uintptr_t)buf - hhdm_offset;
	for (int i = 0; i < hdr->prdtl - 1; i++) {
		tbl->prdt_entry[i].dba = (uint32_t)phys_buf; tbl->prdt_entry[i].dbau = (uint32_t)(phys_buf >> 32);
		tbl->prdt_entry[i].dbc = 8*1024-1; tbl->prdt_entry[i].i = 1; phys_buf += 8*1024; count -= 16;
	}
	tbl->prdt_entry[hdr->prdtl-1].dba = (uint32_t)phys_buf; tbl->prdt_entry[hdr->prdtl-1].dbau = (uint32_t)(phys_buf >> 32);
	tbl->prdt_entry[hdr->prdtl-1].dbc = (count << 9) - 1; tbl->prdt_entry[hdr->prdtl-1].i = 1;
	FIS_REG_H2D *fis = (FIS_REG_H2D*)(&tbl->cfis); fis->fis_type = FIS_TYPE_REG_H2D; fis->c = 1; fis->command = 0x35;
	fis->lba0 = (uint8_t)lba; fis->lba1 = (uint8_t)(lba>>8); fis->lba2 = (uint8_t)(lba>>16); fis->device = 1 << 6;
	fis->lba3 = (uint8_t)(lba>>24); fis->lba4 = (uint8_t)(lba>>32); fis->lba5 = (uint8_t)(lba>>40);
	fis->countl = (uint8_t)count; fis->counth = (uint8_t)(count >> 8);
	while ((active_port->tfd & (0x80 | 0x08))) __asm__("pause"); active_port->ci = (1 << slot);
	while (active_port->ci & (1 << slot)) { if (active_port->is & (1 << 30)) return -1; }
	return 0;
}

void ahci_init(uint64_t mmio) {
	if(!mmio) return; HBA_MEM *hba = (HBA_MEM*)(mmio + hhdm_offset);
	serial_printf("[AHCI] Sovereign HBA probing at %lx\n", mmio);
	hba->ghc |= (1U << 31);
	int g_timeout = 1000000; while (!(hba->ghc & (1U << 31)) && --g_timeout > 0) __asm__("pause");
	if (g_timeout <= 0) { serial_printf("[AHCI] ERROR: Controller Enable Timeout.\n"); return; }
	uint32_t pi = hba->pi;
	for (int i = 0; i < 32; i++) {
		if (pi & (1 << i)) {
			HBA_PORT *p = &hba->ports[i]; uint32_t ssts = p->ssts;
			if ((ssts & 0xF) == HBA_PORT_DET_PRESENT && ((ssts >> 8) & 0xF) == HBA_PORT_IPM_ACTIVE && p->sig == SATA_SIG_ATA) {
				serial_printf("[AHCI] Found Drive on port %d\n", i);
				active_port = p;
				void* cl_virt = pmm_alloc(1); uintptr_t cl_phys = (uintptr_t)cl_virt - hhdm_offset;
				p->clb = (uint32_t)cl_phys; p->clbu = (uint32_t)(cl_phys >> 32);
				void* fis_virt = pmm_alloc(1); uintptr_t fis_phys = (uintptr_t)fis_virt - hhdm_offset;
				p->fb = (uint32_t)fis_phys; p->fbu = (uint32_t)(fis_phys >> 32);
				HBA_CMD_HEADER *hdr = (HBA_CMD_HEADER*)cl_virt;
				for(int j=0; j<32; j++) { void* tbl_virt = pmm_alloc(1); uintptr_t tbl_phys = (uintptr_t)tbl_virt - hhdm_offset; hdr[j].ctba = (uint32_t)tbl_phys; hdr[j].ctbau = (uint32_t)(tbl_phys >> 32); }
				p->cmd |= (1 << 4) | (1 << 0);
				uint16_t identify[256]; int slot = find_cmd_slot(p);
				HBA_CMD_HEADER *h = &((HBA_CMD_HEADER*)cl_virt)[slot]; h->cfl = sizeof(FIS_REG_H2D)/4; h->w = 0; h->prdtl = 1;
				HBA_CMD_TBL *t = (HBA_CMD_TBL*)(((uintptr_t)h->ctba | ((uintptr_t)h->ctbau << 32)) + hhdm_offset); memset(t, 0, sizeof(HBA_CMD_TBL));
				t->prdt_entry[0].dba = (uintptr_t)identify - hhdm_offset; t->prdt_entry[0].dbc = 511; t->prdt_entry[0].i = 1;
				FIS_REG_H2D *fis = (FIS_REG_H2D*)(&t->cfis); fis->fis_type = FIS_TYPE_REG_H2D; fis->c = 1; fis->command = 0xEC;
				p->ci = (1 << slot); int timeout = 1000000; while ((p->ci & (1 << slot)) && --timeout > 0) __asm__("pause");
				if (timeout <= 0) { serial_printf("[AHCI] ERROR: Port %d Identify Timeout.\n", i); continue; }
				uint64_t sectors = identify[60] | ((uint64_t)identify[61] << 16);
				if (identify[83] & (1 << 10)) sectors = identify[100] | ((uint64_t)identify[101] << 16) | ((uint64_t)identify[102] << 32) | ((uint64_t)identify[103] << 48);
				sata_dev.name = "Sovereign SATA"; sata_dev.type = STORAGE_TYPE_SATA; sata_dev.total_blocks = sectors; sata_dev.block_size = 512;
				sata_dev.read = ahci_read; sata_dev.write = ahci_write;
				devmgr_register_storage(&sata_dev); break;
			}
		}
	}
}
