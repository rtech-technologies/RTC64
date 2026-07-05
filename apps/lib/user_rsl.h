#ifndef USER_RSL_H
#define USER_RSL_H

#include <stddef.h>
#include <stdint.h>

/* These should match include/syscall_nums.h */
#define SYS_VFS_LS          0x01
#define SYS_VFS_CAT         0x02
#define SYS_VFS_MKDIR       0x03
#define SYS_VFS_WRITE       0x04
#define SYS_VFS_MOUNTS      0x05
#define SYS_DEVMGR_LIST     0x06
#define SYS_I18N_TRANSLATE  0x07
#define SYS_MALLOC          0x08
#define SYS_FREE            0x09
#define SYS_GET_UPTIME      0x0A
#define SYS_GET_CPU_LOAD    0x0B
#define SYS_SPAWN           0x0C
#define SYS_YIELD           0x0D
#define SYS_EXIT            0x0E
#define SYS_SERIAL_WRITE    0x0F
#define SYS_VFS_READ        0x10
#define SYS_VFS_RM          0x11

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
