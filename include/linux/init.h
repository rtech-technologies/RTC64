#ifndef LINUX_INIT_H
#define LINUX_INIT_H

#include "linux/types.h"

typedef int (*initcall_t)(void);

#define __init
#define __exit

#define module_init(fn) \
    static int __init fn(void); \
    static initcall_t __initcall_##fn __attribute__((used,section(".initcall"))) = fn;

#define module_exit(fn) \
    static void __exit fn(void); \
    static initcall_t __exitcall_##fn __attribute__((used,section(".exitcall"))) = fn;

#define __initdata

#endif
