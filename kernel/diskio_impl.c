/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "fatfs/ff.h"
#include "fatfs/diskio.h"
#include "hal.h"
#include "pro_os.h"

DSTATUS disk_status(BYTE pdrv) {
    if (hal_storage_get_device(pdrv) == NULL) return STA_NODISK;
    return 0;
}

DSTATUS disk_initialize(BYTE pdrv) {
    if (hal_storage_get_device(pdrv) == NULL) return STA_NODISK;
    return 0;
}

DRESULT disk_read(BYTE pdrv, BYTE* buff, LBA_t sector, UINT count) {
    storage_device_t* dev = hal_storage_get_device(pdrv);
    if (dev && hal_storage_read(dev, sector, buff, count) == 0) return RES_OK;
    return RES_ERROR;
}

DRESULT disk_write(BYTE pdrv, const BYTE* buff, LBA_t sector, UINT count) {
    storage_device_t* dev = hal_storage_get_device(pdrv);
    if (dev && hal_storage_write(dev, sector, (void*)buff, count) == 0) return RES_OK;
    return RES_ERROR;
}

DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void* buff) {
    storage_device_t* dev = hal_storage_get_device(pdrv);
    if (!dev) return RES_PARERR;

    switch (cmd) {
        case CTRL_SYNC: return RES_OK;
        case GET_SECTOR_COUNT: *(LBA_t*)buff = dev->total_blocks; return RES_OK;
        case GET_SECTOR_SIZE: *(WORD*)buff = dev->block_size; return RES_OK;
        case GET_BLOCK_SIZE: *(DWORD*)buff = 1; return RES_OK;
        default: return RES_PARERR;
    }
}

DWORD get_fattime(void) {
    return 0; // No RTC for now
}
