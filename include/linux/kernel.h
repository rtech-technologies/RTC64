#ifndef LINUX_KERNEL_H
#define LINUX_KERNEL_H

#include <stdarg.h>
#include "linux/types.h"

#define KERN_INFO "<6>"
#define KERN_ERR  "<3>"

#define pr_info(fmt, ...) printk(KERN_INFO fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...) printk(KERN_ERR fmt, ##__VA_ARGS__)

static inline void printk(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    extern int vsnprintf(char *str, size_t size, const char *format, va_list ap);
    extern void serial_printf(const char* fmt, ...);
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    serial_printf("%s", buf);
    va_end(args);
}

#endif
