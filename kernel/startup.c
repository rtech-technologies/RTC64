#include "pro_os.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>

void kernel_main(void);

// This file is now the entry point and controls the boot sequence
void startup(void) {
    kernel_main();
}
