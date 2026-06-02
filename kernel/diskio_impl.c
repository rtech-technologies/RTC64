#include "ff.h"
#include "diskio.h"
#include "hal.h"
#include "pro_os.h"

DSTATUS disk_status(BYTE pdrv) {
    if (pdrv >= hal_storage_get_device_count()) return STA_NOINIT;
    return 0;
}

DSTATUS disk_initialize(BYTE pdrv) {
    if (pdrv >= hal_storage_get_device_count()) return STA_NOINIT;
    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    if (hal_storage_read(pdrv, sector, buff, count) == 0) return RES_OK;
    return RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    if (hal_storage_write(pdrv, sector, buff, count) == 0) return RES_OK;
    return RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    storage_device_t* dev = hal_storage_get_device(pdrv);
    if (!dev) return RES_ERROR;

    switch (cmd) {
        case CTRL_SYNC: return RES_OK;
        case GET_SECTOR_COUNT: *(LBA_t*)buff = dev->total_blocks; return RES_OK;
        case GET_SECTOR_SIZE: *(WORD*)buff = dev->block_size; return RES_OK;
        case GET_BLOCK_SIZE: *(DWORD*)buff = 1; return RES_OK;
    }
    return RES_PARERR;
}
