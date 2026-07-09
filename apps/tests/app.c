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

void test_malloc_operations() {
    rsl_printf("[TEST] Starting User-space Heap Tests...\n");

    rsl_printf("[TEST] Allocating block of 1024 bytes...\n");
    char* ptr = (char*)rsl_malloc(1024);
    rsl_printf("[TEST] Allocated pointer: %p\n", ptr);
    if (!ptr) {
        rsl_printf("[TEST] FAIL: rsl_malloc returned NULL!\n");
        return;
    }

    rsl_printf("[TEST] Writing test sequence and checking memory block integrity...\n");
    for (int i = 0; i < 1023; i++) {
        ptr[i] = (char)('A' + (i % 26));
    }
    ptr[1023] = '\0';

    bool match = true;
    for (int i = 0; i < 1023; i++) {
        if (ptr[i] != (char)('A' + (i % 26))) {
            match = false;
            break;
        }
    }

    if (match) {
        rsl_printf("[TEST] Memory block content verification: SUCCESS\n");
    } else {
        rsl_printf("[TEST] FAIL: Memory data corrupted!\n");
    }

    rsl_printf("[TEST] Freeing memory block...\n");
    rsl_free(ptr);
    rsl_printf("[TEST] Heap tests complete.\n");
}

void test_system_telemetry() {
    rsl_printf("[TEST] Starting System Telemetry Tests...\n");

    uint64_t t1 = rsl_uptime();
    rsl_printf("[TEST] Current System Uptime: %d ms\n", (int)t1);

    int cpu = rsl_cpu_load();
    rsl_printf("[TEST] Current CPU Load: %d%%\n", cpu);
    if (cpu >= 0 && cpu <= 100) {
        rsl_printf("[TEST] CPU Load validation: SUCCESS\n");
    } else {
        rsl_printf("[TEST] FAIL: CPU Load invalid (%d)\n", cpu);
    }

    char hw_buf[256];
    int hw_res = rsl_hw_list(hw_buf, sizeof(hw_buf));
    rsl_printf("[TEST] Hardware List (Result=%d):\n%s\n", hw_res, hw_buf);

    char mount_buf[256];
    int mount_res = rsl_mounts(mount_buf, sizeof(mount_buf));
    rsl_printf("[TEST] Mount points list (Result=%d):\n%s\n", mount_res, mount_buf);

    rsl_printf("[TEST] System Telemetry tests complete.\n");
}

void test_vfs_io() {
    rsl_printf("[TEST] Starting Executive VFS I/O Tests...\n");

    const char* filepath = "/test_file.txt";
    const char* content = "Sovereign Industrial System Test Run. Success!";
    rsl_printf("[TEST] Writing content to file '%s'...\n", filepath);
    int res = rsl_write(filepath, content);
    rsl_printf("[TEST] Write Result: %d\n", res);

    rsl_printf("[TEST] Reading back content using rsl_cat...\n");
    char read_buf[128];
    memset(read_buf, 0, sizeof(read_buf));
    res = rsl_cat(filepath, read_buf, sizeof(read_buf));
    rsl_printf("[TEST] Read Result (cat): %d\n", res);
    rsl_printf("[TEST] Read Content: '%s'\n", read_buf);

    int cmp = 0;
    const char *p1 = content, *p2 = read_buf;
    while (*p1 && *p2 && *p1 == *p2) { p1++; p2++; }
    if (*p1 == '\0' && *p2 == '\0') cmp = 0;
    else cmp = (*p1 > *p2) ? 1 : -1;

    if (cmp == 0) {
        rsl_printf("[TEST] File content verification: SUCCESS\n");
    } else {
        rsl_printf("[TEST] FAIL: File content mismatch!\n");
    }

    rsl_printf("[TEST] Reading back content using rsl_read...\n");
    char read_buf2[128];
    memset(read_buf2, 0, sizeof(read_buf2));
    res = rsl_read(filepath, read_buf2, sizeof(read_buf2));
    rsl_printf("[TEST] Read Result (read): %d\n", res);
    rsl_printf("[TEST] Read Content: '%s'\n", read_buf2);

    rsl_printf("[TEST] VFS I/O tests complete.\n");
}

void test_i18n() {
    rsl_printf("[TEST] Starting Internationalization Translation Tests...\n");
    const char* key = "SYS_READY";
    const char* translation = rsl_i18n(key);
    rsl_printf("[TEST] Key: '%s' -> Translation: '%s'\n", key, translation);
    rsl_printf("[TEST] I18n translation complete.\n");
}

int main(void) {
    rsl_printf("===========================================\n");
    rsl_printf("SOVEREIGN RTC64 INDUSTRIAL TEST SUITE\n");
    rsl_printf("===========================================\n");

    test_syscall_edge_cases();
    test_malloc_operations();
    test_system_telemetry();
    test_vfs_io();
    test_i18n();

    rsl_printf("===========================================\n");
    rsl_printf("ALL INDUSTRIAL TESTS COMPLETE!\n");
    rsl_printf("===========================================\n");

    rsl_exit(0);
    return 0;
}
