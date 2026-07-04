#ifndef USER_RSL_H
#define USER_RSL_H

#include <stddef.h>
#include <stdint.h>

/* These should match include/syscall_nums.h */
#define SYS_VFS_LS          1
#define SYS_VFS_CAT         2
#define SYS_VFS_MKDIR       3
#define SYS_VFS_WRITE       4
#define SYS_VFS_MOUNTS      5
#define SYS_DEVMGR_LIST     6
#define SYS_MALLOC          7
#define SYS_FREE            8
#define SYS_GET_UPTIME      9
#define SYS_I18N_TRANSLATE  10
#define SYS_VFS_READ        11
#define SYS_EXIT            12
#define SYS_SERIAL_WRITE    13

static inline long syscall(long num, const void* a1, void* a2, size_t a3) {
    long ret;
    __asm__ volatile (
        "syscall"
        : "=a"(ret)
        : "a"(num), "D"(a1), "S"(a2), "d"(a3)
        : "rcx", "r11", "memory"
    );
    return ret;
}

void rsl_printf(const char* fmt, ...);
int rsl_ls(const char* path, char* out, size_t sz);

#endif
