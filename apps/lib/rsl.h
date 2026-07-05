/* Rtech Standard Library (RSL) Header */
#ifndef RSL_H
#define RSL_H
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define SYS_VFS_LS 0x01
#define SYS_VFS_CAT 0x02
#define SYS_VFS_MKDIR 0x03
#define SYS_VFS_WRITE 0x04
#define SYS_MALLOC 0x08
#define SYS_EXIT 0x0E

static inline long syscall3(long num, void* a1, void* a2, size_t a3) {
    long ret;
    __asm__ volatile("mov %1, %%rax\n\t"
                     "mov %2, %%rdi\n\t"
                     "mov %3, %%rsi\n\t"
                     "mov %4, %%rdx\n\t"
                     "syscall" : "=a"(ret) : "g"(num), "g"(a1), "g"(a2), "g"(a3) : "rcx", "r11", "memory");
    return ret;
}

static inline int rsl_ls(const char* path, char* buf, size_t sz) { return (int)syscall3(SYS_VFS_LS, (void*)path, (void*)buf, sz); }
static inline int rsl_write(const char* path, const char* content) { return (int)syscall3(SYS_VFS_WRITE, (void*)path, (void*)content, 0); }
static inline void* rsl_malloc(size_t sz) { return (void*)syscall3(SYS_MALLOC, (void*)sz, 0, 0); }
static inline void rsl_exit(void) { syscall3(SYS_EXIT, 0, 0, 0); }

#endif
