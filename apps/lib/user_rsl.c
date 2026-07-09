#include "user_rsl.h"
#include <stdarg.h>
#include <stdio.h>

/* Very minimal vsnprintf for userland apps if we don't link with kernel implementation */
/* For now, just a dummy or we can try to reuse the kernel headers if we're careful. */
/* Userland shouldn't ideally include kernel headers. */

static void rsl_itoa(unsigned long long n, char* s, int base, int sign) {
    (void)s;
    char buf[64];
    int i = 0;
    if (sign && (long long)n < 0) {
        syscall(SYS_SERIAL_WRITE, "-", NULL, 0);
        n = -(long long)n;
    }
    if (n == 0) {
        buf[i++] = '0';
    } else {
        while (n > 0) {
            int rem = n % base;
            buf[i++] = (rem < 10) ? (rem + '0') : (rem - 10 + 'a');
            n /= base;
        }
    }
    while (i > 0) {
        char c[2] = {buf[--i], '\0'};
        syscall(SYS_SERIAL_WRITE, c, NULL, 0);
    }
}

void rsl_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    const char *p = fmt;
    while (*p) {
        if (*p == '%') {
            p++;
            if (*p == '\0') break;
            if (*p == 'd') {
                int val = va_arg(args, int);
                rsl_itoa((unsigned long long)val, NULL, 10, 1);
            } else if (*p == 'u') {
                unsigned int val = va_arg(args, unsigned int);
                rsl_itoa((unsigned long long)val, NULL, 10, 0);
            } else if (*p == 'x') {
                unsigned int val = va_arg(args, unsigned int);
                rsl_itoa((unsigned long long)val, NULL, 16, 0);
            } else if (*p == 'p') {
                void* val = va_arg(args, void*);
                syscall(SYS_SERIAL_WRITE, "0x", NULL, 0);
                rsl_itoa((unsigned long long)val, NULL, 16, 0);
            } else if (*p == 's') {
                const char* s = va_arg(args, const char*);
                if (s) syscall(SYS_SERIAL_WRITE, s, NULL, 0);
                else syscall(SYS_SERIAL_WRITE, "(null)", NULL, 0);
            } else if (*p == '%') {
                syscall(SYS_SERIAL_WRITE, "%", NULL, 0);
            } else {
                char c[2] = {*p, '\0'};
                syscall(SYS_SERIAL_WRITE, c, NULL, 0);
            }
        } else {
            char c[2] = {*p, '\0'};
            syscall(SYS_SERIAL_WRITE, c, NULL, 0);
        }
        p++;
    }
    va_end(args);
}

int rsl_ls(const char* path, char* out, size_t sz) {
    return (int)syscall(SYS_VFS_LS, path, out, sz);
}

int rsl_read(const char* path, void* buffer, size_t sz) {
    return (int)syscall(SYS_VFS_READ, path, buffer, sz);
}

int rsl_mkdir(const char* path) {
    return (int)syscall(SYS_VFS_MKDIR, path, NULL, 0);
}

void rsl_exit(int code) {
    (void)code;
    syscall(SYS_EXIT, NULL, NULL, 0);
}

void* rsl_malloc(size_t size) {
    return (void*)syscall(SYS_MALLOC, (const void*)size, NULL, 0);
}

void rsl_free(void* ptr) {
    syscall(SYS_FREE, ptr, NULL, 0);
}

int rsl_cat(const char* path, char* out, size_t sz) {
    return (int)syscall(SYS_VFS_CAT, path, out, sz);
}

int rsl_write(const char* path, const char* content) {
    return (int)syscall(SYS_VFS_WRITE, path, (void*)(uintptr_t)content, 0);
}

int rsl_mounts(char* out, size_t sz) {
    return (int)syscall(SYS_VFS_MOUNTS, out, NULL, sz);
}

int rsl_hw_list(char* out, size_t sz) {
    return (int)syscall(SYS_DEVMGR_LIST, out, NULL, sz);
}

uint64_t rsl_uptime(void) {
    uint64_t u = 0;
    syscall(SYS_GET_UPTIME, NULL, &u, 0);
    return u;
}

int rsl_cpu_load(void) {
    return (int)syscall(SYS_GET_CPU_LOAD, NULL, NULL, 0);
}

int rsl_spawn(const char* name, void (*entry)(void*), void* arg) {
    return (int)syscall(SYS_SPAWN, name, (void*)(uintptr_t)entry, (size_t)arg);
}

void rsl_yield(void) {
    syscall(SYS_YIELD, NULL, NULL, 0);
}

const char* rsl_i18n(const char* key) {
    return (const char*)syscall(SYS_I18N_TRANSLATE, key, NULL, 0);
}

void* memset(void* s, int c, size_t n) {
    unsigned char* p = (unsigned char*)s;
    for (size_t i = 0; i < n; i++) {
        p[i] = (unsigned char)c;
    }
    return s;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dest;
    const unsigned char* s = (const unsigned char*)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dest;
}
