#include "ff.h"
#include "diskio.h"
#include "hal.h"
#include <string.h>

DSTATUS disk_initialize(BYTE pdrv) {
    storage_device_t *dev = hal_storage_get_device(pdrv);
    if (!dev) return STA_NOINIT;
    return 0;
}

DSTATUS disk_status(BYTE pdrv) {
    storage_device_t *dev = hal_storage_get_device(pdrv);
    if (!dev) return STA_NOINIT;
    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    storage_device_t *dev = hal_storage_get_device(pdrv);
    if (!dev) return RES_PARERR;
    if (hal_storage_read(dev, sector, buff, count) == 0) {
        return RES_OK;
    }
    return RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    storage_device_t *dev = hal_storage_get_device(pdrv);
    if (!dev) return RES_PARERR;
    if (hal_storage_write(dev, sector, buff, count) == 0) {
        return RES_OK;
    }
    return RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    storage_device_t *dev = hal_storage_get_device(pdrv);
    if (!dev) return RES_PARERR;

    switch (cmd) {
        case CTRL_SYNC:
            return RES_OK;
        case GET_SECTOR_COUNT:
            *(LBA_t*)buff = dev->total_blocks;
            return RES_OK;
        case GET_SECTOR_SIZE:
            *(WORD*)buff = dev->block_size;
            return RES_OK;
        case GET_BLOCK_SIZE:
            *(DWORD*)buff = 1; // Unknown
            return RES_OK;
        default:
            return RES_PARERR;
    }
}
