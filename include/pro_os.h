#ifndef PRO_OS_H
#define PRO_OS_H

#include "limine.h"
#include "nuklear_config.h"
#include "nuklear.h"

#include "external/stb_image.h"
#include "external/stb_truetype.h"
#include "external/tlsf.h"
#include "external/tgx.h"
#include "wolfssl/wolfip.h"
#include "hal.h"

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

/* VFS */
void vfs_init(void);
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

/* Hardware Driver Interfaces */
void xhci_init(uint64_t mmio);
void ehci_init(uint64_t mmio);
void nvme_init(uint64_t mmio);
void ahci_init(uint64_t mmio);

extern uint64_t hhdm_offset;
extern uint64_t kernel_phys_offset;
extern uint64_t xhci_mmio_base;
extern uint64_t ehci_mmio_base;
extern uint64_t nvme_mmio_base;
extern uint64_t ahci_mmio_base;

#endif
