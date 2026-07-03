/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <stddef.h>
#include <string.h>
#include "syscall_nums.h"
#include "serial.h"

int syscall_dispatch(int num, const void* a1, void* a2, size_t a3) {
    switch (num) {
        case SYS_VFS_LS:      return vfs_ls((const char*)a1, (char*)a2, a3);
        case SYS_VFS_CAT:     return vfs_cat((const char*)a1, (char*)a2, a3);
        case SYS_VFS_MKDIR:   return vfs_mkdir((const char*)a1);
        case SYS_VFS_WRITE:   return vfs_write((const char*)a1, (const char*)a2);
        case SYS_VFS_MOUNTS:  return vfs_get_mounts((char*)a1, a3);
        case SYS_DEVMGR_LIST: return devmgr_list((char*)a1, a3);
        case SYS_FREE:        free((void*)a1); return 0;
        case SYS_GET_UPTIME:  *(uint64_t*)a2 = hal_get_uptime_ms(); return 0;
        case SYS_VFS_READ:    return vfs_read((const char*)a1, (void*)a2, a3);
        case SYS_GET_CPU_LOAD: return scheduler_get_cpu_load();
        case SYS_SPAWN:       return scheduler_spawn((const char*)a1, (void (*)(void*))a2, (void*)a3);
        case SYS_YIELD:       scheduler_yield(); return 0;
        case SYS_EXIT:        scheduler_remove_task(scheduler_get_current_task_idx()); scheduler_yield(); return 0;
        case SYS_SERIAL_WRITE: serial_write((const char*)a1); return 0;
        default:
            serial_printf("[SYSCALL] Unknown syscall: %d\n", num);
            return -1;
    }
}

void* syscall_dispatch_ptr(int num, const void* a1, void* a2, size_t a3) {
    (void)a2; (void)a3;
    switch (num) {
        case SYS_I18N_TRANSLATE: return (void*)i18n_translate((const char*)a1);
        case SYS_MALLOC:         return malloc((size_t)a1);
        default: return NULL;
    }
}
