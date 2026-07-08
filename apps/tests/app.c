#include "rsl.h"
#include <string.h>

void test_syscall_edge_cases() {
    rsl_printf("[TEST] Starting Syscall Edge Case Tests...\n");

    /* 1. NULL Pointer Validation */
    rsl_printf("[TEST] Testing NULL pointer in rsl_ls...\n");
    int res = rsl_ls(NULL, NULL, 0);
    rsl_printf("[TEST] Result: %d (Expected -1)\n", res);

    /* 2. Kernel Space Pointer (Security Violation) */
    rsl_printf("[TEST] Testing kernel space pointer in rsl_read...\n");
    void* kernel_ptr = (void*)0xFFFF800000000000ULL;
    res = rsl_read("/", kernel_ptr, 10);
    rsl_printf("[TEST] Result: %d (Expected -1 due to Security policy)\n", res);

    /* 3. Deep Path Test */
    rsl_printf("[TEST] Testing deep path in rsl_mkdir...\n");
    char deep_path[512];
    for (int i = 0; i < 511; i++) deep_path[i] = 'A';
    deep_path[511] = '\0';
    res = rsl_mkdir(deep_path);
    rsl_printf("[TEST] Result: %d (Expected -1 due to length check)\n", res);

    rsl_printf("[TEST] Edge case testing complete.\n");
}

int main(void) {
    test_syscall_edge_cases();
    rsl_exit(0);
    return 0;
}
