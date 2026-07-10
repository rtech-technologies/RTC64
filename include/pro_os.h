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
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strchr(const char* s, int c);
int snprintf(char* str, size_t size, const char* format, ...);

/* Scheduler / Task Manager */
#define MAX_TASKS 10
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
int scheduler_get_tasks(task_t *out_tasks, int max_tasks);
void scheduler_end_task(int id);

/* VFS */
#define MAX_VFS_FILES 128
#define MAX_FILE_SIZE 4096
#define MAX_PATH_LEN 128

typedef struct {
    char path[MAX_PATH_LEN];
    char content[MAX_FILE_SIZE];
    size_t size;
    bool is_dir;
} vfs_node_t;

void vfs_init(void);
const char* vfs_resolve(const char *path);
bool vfs_create_file(const char *path, bool is_dir);
bool vfs_write_file(const char *path, const char *content, size_t size);
bool vfs_read_file(const char *path, char *buffer, size_t max_size);
bool vfs_delete_file(const char *path);
int vfs_list_dir(const char *dir_path, char filenames[][64], bool is_dirs[], int max_items);

/* Security / UAC */
typedef struct {
    bool can_network;
    bool can_storage;
    bool can_input;
} app_permit_t;

typedef enum {
    ROLE_ADMIN,
    ROLE_STANDARD,
    ROLE_GUEST
} user_role_t;

typedef struct {
    char username[32];
    char password[32];
    user_role_t role;
} user_account_t;

bool uac_check_permit(int app_id, const char *action);
void uac_request_permit(int app_id, const char *action);
void uac_set_permit(int app_id, bool net, bool storage);

/* User Account Management */
void user_init(void);
bool user_add(const char *username, const char *password, user_role_t role);
bool user_authenticate(const char *username, const char *password);
user_role_t user_get_role(const char *username);
void user_get_active(char *username, user_role_t *role);
void user_set_active(const char *username);
bool user_delete(const char *username);
int user_get_all(char usernames[][32], user_role_t roles[], int max_users);
bool user_change_password(const char *username, const char *new_password);

/* I18n */
const char* i18n_translate(const char *key);
void i18n_set_language(const char *lang);
const char* i18n_get_language(void);

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
size_t hal_malloc_get_used(void);
size_t hal_malloc_get_free(void);

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
