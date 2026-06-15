/* Modified by Sovereign: License Compliance Update */
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
    int h, m, s;
    rtc_get_time(&h, &m, &s);
    return ((DWORD)(2024 - 1980) << 25) | ((DWORD)6 << 21) | ((DWORD)15 << 16) |
           ((DWORD)h << 11) | ((DWORD)m << 5) | ((DWORD)s >> 1);
}
