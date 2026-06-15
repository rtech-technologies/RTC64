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
int vsnprintf(char* str, size_t size, const char* format, va_list ap);

/* Scheduler / Task Manager */
#define MAX_TASKS 16
typedef enum { TASK_DEAD, TASK_RUNNING, TASK_SQUEEZED } task_state_t;

typedef struct {
    int id;
    const char *name;
    task_state_t state;
    void (*entry)(void);
} task_t;

void scheduler_init(void);
void scheduler_add_task(const char *name, void (*entry)(void));
void scheduler_remove_task(int task_id);
uint64_t scheduler_switch(uint64_t current_rsp);
void scheduler_run(void);
int scheduler_get_task_count(void);
task_t* scheduler_get_task(int index);

/* System Shell */
void system_shell_init(void);
void system_shell_task(void);
void usb_osal_tick_handler(void);

/* VFS */
void vfs_init(void);
void vfs_refresh_mounts(void);
const char* vfs_resolve(const char *path);
void* vfs_read_file(const char* path, size_t* out_sz);

/* Security / UAC */
typedef struct {
    bool can_network;
    bool can_storage;
    bool can_input;
} app_permit_t;

bool uac_check_permit(int app_id, const char *action);
void uac_request_permit(int app_id, const char *action);
void uac_set_permit(int app_id, bool net, bool storage);

/* I18n */
const char* i18n_translate(const char *key);

struct cpu_state {
    uint8_t fxsave_region[512]; /* 512-byte area for FPU/SSE state */
    uint64_t padding; /* 8-byte padding for 16-byte alignment */
    uint64_t ds, es, fs, gs;
    uint64_t cr4, cr3, cr2;
    uint64_t r15, r14, r13, r12, r11, r10, r9, r8;
    uint64_t rbp, rdi, rsi, rdx, rcx, rbx, rax;
    uint64_t interrupt_number, error_code;
    uint64_t rip, cs, rflags, rsp, ss;
} __attribute__((aligned(16)));

void kpanic(const char* message);
void render_bsod_screen(const char* error_title, void* rsp_pointer);

/* Hardware & Memory */
void hal_malloc_init(void* mem, size_t bytes);
size_t hal_malloc_get_used(void);
size_t hal_malloc_get_total(void);
void pmm_init(struct limine_memmap_response* map);
void* pmm_alloc_blocks(size_t count);
void gdt_init(void);
void idt_init(void);
void apic_init(void);
void apic_write(uint32_t reg, uint32_t val);
void apic_eoi(void);
void irq_install_handler(int irq, void (*handler)(struct cpu_state*));
void timer_handler(struct cpu_state* state);

/* Exception Handlers */
void handler_divide_by_zero(void);
void handler_general_protection_fault(void);
void handler_page_fault(void);
void handler_double_fault(void);
void isr_stub_39(void);

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
extern uint64_t xhci_mmio_base;
extern uint64_t ehci_mmio_base;
extern uint64_t nvme_mmio_base;
extern uint64_t ahci_mmio_base;

#endif
