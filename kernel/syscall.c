/* Modified by Sovereign: License Compliance Update */
#include "pro_os.h"
#include <stddef.h>
#include <string.h>
#include "syscall_nums.h"
#include "serial.h"

/* Section 0: SCMT (System Call Mapping Table) ABI Verification */
typedef struct {
    int id;
    const char* name;
    int (*handler)(const void*, void*, size_t);
} scmt_entry_t;

static int sys_vfs_ls_h(const void* a1, void* a2, size_t a3) { return vfs_ls((const char*)a1, (char*)a2, a3); }
static int sys_vfs_cat_h(const void* a1, void* a2, size_t a3) { return vfs_cat((const char*)a1, (char*)a2, a3); }
static int sys_vfs_mkdir_h(const void* a1, void* a2, size_t a3) { (void)a2; (void)a3; return vfs_mkdir((const char*)a1); }
static int sys_vfs_write_h(const void* a1, void* a2, size_t a3) { (void)a3; return vfs_write((const char*)a1, (const char*)a2); }
static int sys_vfs_mounts_h(const void* a1, void* a2, size_t a3) { (void)a1; return vfs_get_mounts((char*)a2, a3); }
static int sys_devmgr_list_h(const void* a1, void* a2, size_t a3) { (void)a1; return devmgr_list((char*)a2, a3); }

static scmt_entry_t SCMT[] = {
    { SYS_VFS_LS,      "VFS_LS",      sys_vfs_ls_h },
    { SYS_VFS_CAT,     "VFS_CAT",     sys_vfs_cat_h },
    { SYS_VFS_MKDIR,   "VFS_MKDIR",   sys_vfs_mkdir_h },
    { SYS_VFS_WRITE,   "VFS_WRITE",   sys_vfs_write_h },
    { SYS_VFS_MOUNTS,  "VFS_MOUNTS",  sys_vfs_mounts_h },
    { SYS_DEVMGR_LIST, "DEVMGR_LIST", sys_devmgr_list_h },
};

int syscall_dispatch(int num, const void* a1, void* a2, size_t a3) {
    if (num >= SYS_VFS_LS && num <= SYS_VFS_WRITE) {
        if (!a1) return -1;
    }
    if (num == SYS_VFS_LS || num == SYS_VFS_CAT || num == SYS_VFS_MOUNTS || num == SYS_DEVMGR_LIST) {
        if (!a2) return -1;
    }

    /* Sovereign ABI Audit: Verify parameters based on SCMT mapping */
    for (size_t i = 0; i < sizeof(SCMT)/sizeof(SCMT[0]); i++) {
        if (SCMT[i].id == num) {
            /* Security: Ensure pointer arguments are present for relevant calls */
            if (num != SYS_VFS_MOUNTS && num != SYS_DEVMGR_LIST && !a1) return -1;

            return SCMT[i].handler(a1, a2, a3);
        }
    }
    serial_printf("[SYSCALL] INVALID_CALL_ID: %d\n", num);
    return -1;
}

void* syscall_dispatch_ptr(int num, const void* a1, void* a2, size_t a3) {
    (void)a2; (void)a3;
    switch (num) {
        case SYS_I18N_TRANSLATE: return (void*)i18n_translate((const char*)a1);
        default: return NULL;
    }
}
