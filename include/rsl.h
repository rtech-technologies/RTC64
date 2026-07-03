/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#ifndef RSL_H
#define RSL_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>

/*
 * Rtech Standard Library (RSL) - Definitive Executive API
 * Designed for High-Power Sovereign RTC64 Applications.
 * Format: Single-header library (GML-style)
 */

#ifdef __cplusplus
extern "C" {
#endif

/* --- Core Types --- */
typedef int rsl_handle_t;

/* --- Memory Management --- */
void* rsl_malloc(size_t size);
void  rsl_free(void* ptr);

/* --- File System (Executive VFS) --- */
int rsl_ls(const char* path, char* out, size_t sz);
int rsl_cat(const char* path, char* out, size_t sz);
int rsl_read(const char* path, void* buffer, size_t sz);
int rsl_mkdir(const char* path);
int rsl_write(const char* path, const char* content);
int rsl_mounts(char* out, size_t sz);

/* --- System Telemetry --- */
int      rsl_hw_list(char* out, size_t sz);
uint64_t rsl_uptime(void);
int      rsl_cpu_load(void);

/* --- Process & Tasking --- */
int  rsl_spawn(const char* name, void (*entry)(void*), void* arg);
void rsl_yield(void);
void rsl_exit(int code);

/* --- Output & Logging --- */
void rsl_printf(const char* fmt, ...);

/* --- Internationalization --- */
const char* rsl_i18n(const char* key);

#ifdef RSL_IMPLEMENTATION
#include "os_api.h"
#include <stdio.h>

void* rsl_malloc(size_t size) {
    return syscall_dispatch_ptr(SYS_MALLOC, (void*)size, NULL, 0);
}

void rsl_free(void* ptr) {
    syscall_dispatch(SYS_FREE, ptr, NULL, 0);
}

int rsl_ls(const char* path, char* out, size_t sz) {
    return syscall_dispatch(SYS_VFS_LS, path, out, sz);
}

int rsl_cat(const char* path, char* out, size_t sz) {
    return syscall_dispatch(SYS_VFS_CAT, path, out, sz);
}

int rsl_read(const char* path, void* buffer, size_t sz) {
    return syscall_dispatch(SYS_VFS_READ, path, buffer, sz);
}

int rsl_mkdir(const char* path) {
    return syscall_dispatch(SYS_VFS_MKDIR, path, NULL, 0);
}

int rsl_write(const char* path, const char* content) {
    return syscall_dispatch(SYS_VFS_WRITE, path, (void*)content, 0);
}

int rsl_mounts(char* out, size_t sz) {
    return syscall_dispatch(SYS_VFS_MOUNTS, out, NULL, sz);
}

int rsl_hw_list(char* out, size_t sz) {
    return syscall_dispatch(SYS_DEVMGR_LIST, out, NULL, sz);
}

uint64_t rsl_uptime(void) {
    uint64_t u;
    syscall_dispatch(SYS_GET_UPTIME, NULL, &u, 0);
    return u;
}

int rsl_cpu_load(void) {
    return syscall_dispatch(SYS_GET_CPU_LOAD, NULL, NULL, 0);
}

int rsl_spawn(const char* name, void (*entry)(void*), void* arg) {
    return syscall_dispatch(SYS_SPAWN, name, (void*)entry, (size_t)arg);
}

void rsl_yield(void) {
    syscall_dispatch(SYS_YIELD, NULL, NULL, 0);
}

void rsl_exit(int code) {
    (void)code;
    syscall_dispatch(SYS_EXIT, NULL, NULL, 0);
}

const char* rsl_i18n(const char* key) {
    return (const char*)syscall_dispatch_ptr(SYS_I18N_TRANSLATE, key, NULL, 0);
}

void rsl_printf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[1024];
    vsnprintf(buf, sizeof(buf), fmt, args);
    syscall_dispatch(SYS_SERIAL_WRITE, buf, NULL, 0);
    va_end(args);
}

#endif /* RSL_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif

#endif /* RSL_H */
