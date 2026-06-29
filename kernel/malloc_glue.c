/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#include "pro_os.h"
#include <stdint.h>

static void* global_tlsf_control = NULL;
static spinlock_t g_malloc_lock = 0;

void hal_malloc_init(void* mem, size_t bytes) {
    if (mem && bytes > 0) {
        global_tlsf_control = tlsf_create_with_pool(mem, bytes);
    }
}

void* tlsf_get_global(void) {
    return global_tlsf_control;
}

void* malloc(size_t size) {
    spin_lock(&g_malloc_lock);
    void* ptr = tlsf_malloc(global_tlsf_control, size);
    spin_unlock(&g_malloc_lock);
    return ptr;
}

void free(void* ptr) {
    spin_lock(&g_malloc_lock);
    tlsf_free(global_tlsf_control, ptr);
    spin_unlock(&g_malloc_lock);
}

void* realloc(void* ptr, size_t size) {
    spin_lock(&g_malloc_lock);
    void* res = tlsf_realloc(global_tlsf_control, ptr, size);
    spin_unlock(&g_malloc_lock);
    return res;
}

void* calloc(size_t nmemb, size_t size) {
    void* ptr = malloc(nmemb * size);
    if (ptr) memset(ptr, 0, nmemb * size);
    return ptr;
}
