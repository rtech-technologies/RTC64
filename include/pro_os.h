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

/* VFS */
void vfs_init(void);
const char* vfs_resolve(const char *path);
void vfs_mkdir(const char *path);
void fat_format(void);
int vfs_write(const char *path, const void *data, uint32_t size);
int vfs_read(const char *path, void *buffer, uint32_t max_size);
void vfs_readdir(const char *path, void (*callback)(const char *name, bool is_dir, uint32_t size));
int vfs_rm(const char *path);
bool vfs_exists(const char *path);

/* Configuration Manager & Compliance Recording */
void cm_init(void);
int cm_read_config(const char *key, char *out_val, int max_len);
void cm_write_config(const char *key, const char *val);
void comprec_init(void);
void comprec_log(const char *event);

/* Linux Driver Netdev Compatibility Shim */
struct net_device;
struct sk_buff {
    uint8_t *data;
    uint32_t len;
};

struct net_device_ops {
    int (*ndo_open)(struct net_device *dev);
    int (*ndo_start_xmit)(struct sk_buff *skb, struct net_device *dev);
    int (*ndo_stop)(struct net_device *dev);
};

struct net_device {
    char name[16];
    const struct net_device_ops *netdev_ops;
    uint8_t dev_addr[6];
    void *priv;
};

struct sk_buff* dev_alloc_skb(unsigned int length);
void dev_kfree_skb(struct sk_buff *skb);
struct net_device* alloc_etherdev(int sizeof_priv);
void free_netdev(struct net_device *dev);
int register_netdev(struct net_device *dev);
void unregister_netdev(struct net_device *dev);
void hal_virtio_net_probe(uint64_t mmio);
int sys_net_fetch(const char *url, char *buffer, uint32_t max_size);

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
void nvme_init(uint64_t mmio);
void ahci_init(uint64_t mmio);

extern uint64_t hhdm_offset;
extern uint64_t kernel_phys_offset;
extern uint64_t xhci_mmio_base;
extern uint64_t ehci_mmio_base;
extern uint64_t nvme_mmio_base;
extern uint64_t ahci_mmio_base;

#endif
