#include "user_rsl.h"
#include <stdarg.h>
#include <stdio.h>

/* Very minimal vsnprintf for userland apps if we don't link with kernel implementation */
/* For now, just a dummy or we can try to reuse the kernel headers if we're careful. */
/* Userland shouldn't ideally include kernel headers. */

void rsl_printf(const char* fmt, ...) {
    /* For simplicity in this demo, just send the raw string to serial */
    /* Real implementation would use vsnprintf */
    syscall(SYS_SERIAL_WRITE, fmt, NULL, 0);
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
