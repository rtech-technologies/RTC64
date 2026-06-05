#ifndef PRO_OS_H
#define PRO_OS_H

#include "limine.h"
#include "nuklear_config.h"
#include "nuklear.h"

#include "external/stb_image.h"
#include "external/stb_truetype.h"
#include "external/tlsf.h"
#include "external/tgx.h"
#include "hal.h"

/* Standard C Library Prototypes for Freestanding Environment */
void* memset(void* s, int c, size_t n);
void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
void* memchr(const void* s, int c, size_t n);
int memcmp(const void* s1, const void* s2, size_t n);
size_t strlen(const char* s);
char* strcpy(char* dest, const char* src);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strchr(const char* s, int c);
int snprintf(char* str, size_t size, const char* format, ...);

/* Scheduler / Task Manager */
#define MAX_TASKS 5
typedef enum { TASK_IDLE, TASK_RUNNING, TASK_SQUEEZED } task_state_t;

typedef struct {
    int id;
    const char *name;
    task_state_t state;
    void (*entry)(void);
} task_t;

void scheduler_init(void);
void scheduler_add_task(const char *name, void (*entry)(void));
void scheduler_run(void);
int scheduler_get_task_count(void);
task_t* scheduler_get_task(int index);

/* VFS */
void vfs_init(void);
void vfs_refresh_mounts(void);
const char* vfs_resolve(const char *path);

/* Security / UAC */
typedef struct {
    bool can_network;
    bool can_storage;
    bool can_input;
} app_permit_t;

bool uac_check_permit(int app_id, const char *action);
void uac_request_permit(int app_id, const char *action);

/* I18n */
const char* i18n_translate(const char *key);

struct cpu_state {
    // Segment registers
    uint64_t gs, fs, es, ds;
    // Control registers
    uint64_t cr4, cr3, cr2;
    // General purpose registers
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    // Pushed automatically by CPU and stubs
    uint64_t interrupt_number;
    uint64_t error_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
};

void kpanic(const char* message);

/* Hardware & Memory */
void hal_malloc_init(void* mem, size_t bytes);

/* Hardware Driver Interfaces */
void xhci_init(uint64_t mmio);
void ehci_init(uint64_t mmio);
int nvme_init(uint64_t mmio);
int ahci_init(uint64_t mmio);
void pci_scan(void);

/* VFS Prototypes */
int vfs_ls(const char* path, char* out, size_t sz);
int vfs_cat(const char* path, char* out, size_t sz);
int vfs_mkdir(const char* path);
int vfs_write(const char* path, const char* content);
int vfs_get_mounts(char* out, size_t sz);
int devmgr_list(char* out, size_t sz);

extern uint64_t hhdm_offset;
extern uint64_t kernel_phys_offset;
extern uint64_t xhci_mmio_base;
extern uint64_t ehci_mmio_base;
extern uint64_t nvme_mmio_base;
extern uint64_t ahci_mmio_base;

#endif
