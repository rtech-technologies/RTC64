#include "ff.h"
#include "diskio.h"
#include "hal.h"
#include <string.h>
DSTATUS disk_initialize(BYTE p) { return hal_storage_get_device(p) ? 0 : STA_NOINIT; }
DSTATUS disk_status(BYTE p) { return hal_storage_get_device(p) ? 0 : STA_NOINIT; }
DRESULT disk_read(BYTE p, BYTE* b, LBA_t s, UINT c) {
    storage_device_t* d = hal_storage_get_device(p);
    if(!d) return RES_PARERR;
    return hal_storage_read(d,s,b,c)==0 ? RES_OK : RES_ERROR;
}
DRESULT disk_write(BYTE p, const BYTE* b, LBA_t s, UINT c) {
    storage_device_t* d = hal_storage_get_device(p);
    if(!d) return RES_PARERR;
    return hal_storage_write(d,s,b,c)==0 ? RES_OK : RES_ERROR;
}
DRESULT disk_ioctl(BYTE p, BYTE cmd, void* b) {
    storage_device_t* d = hal_storage_get_device(p); if(!d) return RES_PARERR;
    switch(cmd) {
        case CTRL_SYNC: return RES_OK;
        case GET_SECTOR_COUNT: *(LBA_t*)b = d->total_blocks; return RES_OK;
        case GET_SECTOR_SIZE: *(WORD*)b = (WORD)d->block_size; return RES_OK;
        case GET_BLOCK_SIZE: *(DWORD*)b = 1; return RES_OK;
        default: return RES_PARERR;
    }
}
