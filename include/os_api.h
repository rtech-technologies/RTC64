/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#ifndef OS_API_H
#define OS_API_H

#include "syscall_nums.h"
#include <stddef.h>

extern int snprintf(char* str, size_t size, const char* format, ...);

/* --- System Dispatch Prototypes --- */
int syscall_dispatch(int num, const void* a1, void* a2, size_t a3);
void* syscall_dispatch_ptr(int num, const void* a1, void* a2, size_t a3);

/* --- Application level macros that map to numeric calls --- */

static inline int os_vfs_ls(const char* path, char* out, size_t sz) {
    return syscall_dispatch(SYS_VFS_LS, path, out, sz);
}

static inline int os_vfs_cat(const char* path, char* out, size_t sz) {
    return syscall_dispatch(SYS_VFS_CAT, path, out, sz);
}

static inline int os_vfs_mkdir(const char* path) {
    return syscall_dispatch(SYS_VFS_MKDIR, path, (void*)0, 0);
}

static inline int os_vfs_write(const char* path, const char* content) {
    return syscall_dispatch(SYS_VFS_WRITE, path, (void*)content, 0);
}

static inline int os_vfs_get_mounts(char* out, size_t sz) {
    return syscall_dispatch(SYS_VFS_MOUNTS, out, (void*)0, sz);
}

static inline int os_devmgr_list(char* out, size_t sz) {
    return syscall_dispatch(SYS_DEVMGR_LIST, out, (void*)0, sz);
}

static inline const char* os_i18n_translate(const char* key) {
    return (const char*)syscall_dispatch_ptr(SYS_I18N_TRANSLATE, key, (void*)0, 0);
}

#endif
