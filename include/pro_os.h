/* Copyright (C) 2025 Sovereign RTC64 Project. All rights reserved.
 * Licensed under the 'respect people's property' OS license. */
#ifndef PRO_OS_H
#define PRO_OS_H
#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>
#include "limine.h"
#include "nuklear_config.h"
#include "external/stb_image.h"
#include "external/stb_truetype.h"
#include "external/tlsf.h"
#include "external/tgx.h"
#include "hal.h"

void* memset(void* s, int c, size_t n);
void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
void* memchr(const void* s, int c, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t n);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strchr(const char* s, int c);
char* strrchr(const char* s, int c);
int snprintf(char* str, size_t size, const char* format, ...);
int vsnprintf(char* str, size_t size, const char* format, va_list ap);

#define MAX_TASKS 16
typedef enum { TASK_DEAD, TASK_RUNNING, TASK_SQUEEZED } task_state_t;
typedef struct { int id; uint32_t uaid; uint32_t upid; char name[32]; task_state_t state; void (*entry)(void*); void *arg; } task_t;

void scheduler_init(void);
int scheduler_add_task(const char *name, void (*entry)(void*), void *arg, uint32_t uaid, uint32_t upid);
int scheduler_spawn(const char* name, void (*entry)(void*), void* arg);
int scheduler_fork(const char* name, void (*entry)(void*), void* arg);
void scheduler_remove_task(int task_id);
void scheduler_stop_all(void);
uint64_t scheduler_switch(uint64_t current_rsp);
void scheduler_yield(void);
void scheduler_run(void);
int scheduler_get_task_count(void);
task_t* scheduler_get_task(int index);
int scheduler_get_current_task_idx(void);
uint32_t scheduler_get_current_uaid(void);
uint32_t scheduler_get_current_upid(void);
int scheduler_get_cpu_load(void);
void scheduler_audit_stacks(void);
uint64_t scheduler_get_ctx_switches(void);
void task_crash_cleanup(void);

typedef struct { bool can_network; bool can_storage; bool can_input; } app_permit_t;
bool uac_check_permit(int app_id, const char *action);
void uac_request_permit(int app_id, const char *action);
void uac_set_permit(int app_id, bool net, bool storage);

void debug_shell_init(void);
void debug_shell_task(void* arg);
void comprec_task(void* arg);
void kbd_task(void* arg);
void mouse_task(void* arg);
void comprec_log(const char* tag, const char* event);
void session_manager_task(void* arg);

struct cpu_state {
    uint8_t fxsave_region[512];

    uint64_t padding;
    uint64_t ds, es, fs, gs;
    uint64_t cr4, cr3, cr2;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t interrupt_number, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((aligned(16)));

void timer_handler(struct cpu_state* state);
void irq_install_handler(int i, void (*handler)(struct cpu_state*));

void hal_malloc_init(void* mem, size_t bytes);
size_t hal_malloc_get_used(void);
size_t hal_malloc_get_total(void);
void* malloc(size_t size);
size_t hal_malloc_get_used(void);
void free(void* ptr);
void* tlsf_get_global(void);

void pmm_init(struct limine_memmap_response* map);
void* pmm_alloc_blocks(size_t count);
void* pmm_alloc_blocks_low(size_t count);
void* pmm_alloc_low(void);

void gdt_init(void);
void idt_init(void);
void apic_init(void);
void apic_timer_unmask(void);
void cm_orchestrate_drivers(void);
void init_sse(void);
void kpanic(const char* message);
void pci_scan(void);
uint32_t pci_read_config(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
uint64_t pci_get_bar(uint8_t bus, uint8_t slot, uint8_t func, uint8_t bar_index);
int linux_compat_init(void);
int virtio_net_linux_init(void);

int nvme_init(uint64_t mmio);
int ahci_init(uint64_t mmio);
void xhci_init(uint64_t mmio);
void ehci_init(uint64_t mmio);

void hal_usb_init(void);
void hal_usb_poll(void);

int vfs_ls(const char* path, char* out, size_t sz);
int vfs_cat(const char* path, char* out, size_t sz);
int vfs_read(const char* path, void* buffer, size_t sz);
int vfs_mkdir(const char* path);
int vfs_rm(const char* path);
int vfs_write(const char* path, const char* content);
int vfs_get_mounts(char* out, size_t sz);
int devmgr_list(char* out, size_t sz);
void vfs_init(void);
void vfs_refresh_mounts(void);
const char* vfs_resolve(const char *path);

void serial_init(void);
void serial_printf(const char* fmt, ...);
void serial_write(const char* str);
int serial_received(void);
char serial_read(void);

void vga_log(const char* str);
void vga_disable_log(void);

const char* i18n_translate(const char* key);

int ramdisk_init(void);

extern uint64_t hhdm_offset;
extern volatile struct limine_framebuffer_request framebuffer_request;

#endif
