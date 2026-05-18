#include <pro_os.h>
#include <ff.h>
#include <diskio.h>

typedef struct {
    uint32_t sector_size;
    uint64_t total_lba;
    int (*read_lba)(uint64_t lba, uint32_t count, void* buffer);
    int (*write_lba)(uint64_t lba, uint32_t count, const void* buffer);
    char name[16];
} vdisk_node_t;

static vdisk_node_t vdisk_matrix[8];
static int vdisk_count = 0;

void register_vdisk(vdisk_node_t node) {
    if (vdisk_count < 8) {
        vdisk_matrix[vdisk_count++] = node;
    }
}

// FatFs Media Access Hooks
DSTATUS disk_status (BYTE pdrv) {
    if (pdrv >= vdisk_count) return STA_NOINIT;
    return 0;
}

DSTATUS disk_initialize (BYTE pdrv) {
    if (pdrv >= vdisk_count) return STA_NOINIT;
    return 0;
}

DRESULT disk_read (BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv >= vdisk_count) return RES_PARERR;
    if (vdisk_matrix[pdrv].read_lba(sector, count, buff) == 0) {
        return RES_OK;
    }
    return RES_ERROR;
}

DRESULT disk_write (BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    if (pdrv >= vdisk_count) return RES_PARERR;
    if (vdisk_matrix[pdrv].write_lba(sector, count, buff) == 0) {
        return RES_OK;
    }
    return RES_ERROR;
}

DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff) {
    if (pdrv >= vdisk_count) return RES_PARERR;
    switch (cmd) {
        case CTRL_SYNC: return RES_OK;
        case GET_SECTOR_COUNT: *(LBA_t*)buff = vdisk_matrix[pdrv].total_lba; return RES_OK;
        case GET_SECTOR_SIZE:  *(WORD*)buff = (WORD)vdisk_matrix[pdrv].sector_size; return RES_OK;
        case GET_BLOCK_SIZE:   *(DWORD*)buff = 1; return RES_OK;
    }
    return RES_PARERR;
}

uint32_t get_fattime (void) {
    return 0;
}

void scan_sovereign_status() {
    for (int i = 0; i < vdisk_count; i++) {
        uint8_t buffer[512];
        if (vdisk_matrix[i].read_lba(1, 1, buffer) == 0) {
            if (buffer[0] == 'E' && buffer[1] == 'F' && buffer[2] == 'I' && buffer[3] == ' ') {
                 // GPT Header found
            }
        }
    }
}
