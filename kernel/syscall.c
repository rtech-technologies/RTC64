/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <stddef.h>
#include <string.h>
#include "syscall_nums.h"
#include "serial.h"

static bool is_valid_pointer(const void* ptr, size_t sz) {
    if (!ptr) return true; /* NULL is generally handled by logic */
    uint64_t addr = (uint64_t)ptr;

    /* Kernel space check (High half) */
    if (addr >= 0xFFFF800000000000ULL) return false;

    /* Overflow check */
    if (addr + sz < addr) return false;

    /* Per-task user boundary check */
    int idx = scheduler_get_current_task_idx();
    if (idx < 0) return true; /* Kernel task */

    task_t* t = scheduler_get_task(idx);
    if (!t || t->uaid == 0) return true; /* Industrial Executive */

    if (t->user_size == 0) return false; /* No user memory allocated */

    bool in_user = (addr >= t->user_base && (addr + sz) <= (t->user_base + t->user_size));
    bool in_stack = (addr >= t->stack_base && (addr + sz) <= (t->stack_base + t->stack_size));

    if (!in_user && !in_stack) {
        serial_printf("[SECURITY] Illegal pointer access by task %d: %p (size %d)\n", idx, ptr, (int)sz);
        return false;
    }
    return true;
}

static bool is_administrator(void) {
    const char* user = registry_get("SESSION/CurrentUser");
    if (!user) return false;
    char role_key[64];
    snprintf(role_key, sizeof(role_key), "USERS/%s/Role", user);
    const char* role = registry_get(role_key);
    return (role && strcmp(role, "Administrator") == 0);
}

int syscall_dispatch(int num, const void* a1, void* a2, size_t a3) {
    /* Boundary Validation */
    /* a1 validation */
    if (num == SYS_VFS_LS || num == SYS_VFS_CAT || num == SYS_VFS_MKDIR ||
        num == SYS_VFS_WRITE || num == SYS_VFS_RM || num == SYS_NET_FETCH ||
        num == SYS_SERIAL_WRITE || num == SYS_SPAWN || num == SYS_FREE || num == SYS_VFS_READ ||
        num == SYS_VFS_MOUNTS || num == SYS_DEVMGR_LIST) {
        if (!is_valid_pointer(a1, (num == SYS_VFS_MOUNTS || num == SYS_DEVMGR_LIST) ? a3 : 1)) return -1;
    }
    /* a2 validation */
    if (num == SYS_VFS_LS || num == SYS_VFS_CAT || num == SYS_VFS_READ ||
        num == SYS_NET_FETCH || num == SYS_VFS_WRITE || num == SYS_GET_UPTIME || num == SYS_SPAWN) {
        if (!is_valid_pointer(a2, (num == SYS_GET_UPTIME) ? sizeof(uint64_t) : (a3 ? a3 : 1))) return -1;
    }

    /* Authorization */
    if (num == SYS_VFS_RM || num == SYS_VFS_MKDIR || num == SYS_VFS_WRITE) {
        if (!is_administrator()) {
            serial_printf("[SECURITY] Access denied for sensitive syscall %d\n", num);
            return -1;
        }
    }

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
        case SYS_VFS_RM:      return vfs_rm((const char*)a1);
        case SYS_NET_FETCH:    return rsl_web_fetch((const char*)a1, (char*)a2, a3);
        default:
            serial_printf("[SYSCALL] Unknown syscall: %d\n", num);
            return -1;
    }
}

void* syscall_dispatch_ptr(int num, const void* a1, void* a2, size_t a3) {
    (void)a2; (void)a3;
    if (num == SYS_I18N_TRANSLATE) {
        if (!is_valid_pointer(a1, 1)) return NULL;
    }
    switch (num) {
        case SYS_I18N_TRANSLATE: return (void*)i18n_translate((const char*)a1);
        case SYS_MALLOC:         return malloc((size_t)a1);
        default: return NULL;
    }
}
