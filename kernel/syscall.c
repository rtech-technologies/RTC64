#include "pro_os.h"
#include <stddef.h>
#include <string.h>
#include "syscall_nums.h"

int syscall_dispatch(int num, const void* a1, void* a2, size_t a3) {
    switch (num) {
        case SYS_VFS_LS:      return vfs_ls((const char*)a1, (char*)a2, a3);
        case SYS_VFS_CAT:     return vfs_cat((const char*)a1, (char*)a2, a3);
        case SYS_VFS_MKDIR:   return vfs_mkdir((const char*)a1);
        case SYS_VFS_WRITE:   return vfs_write((const char*)a1, (const char*)a2);
        case SYS_VFS_MOUNTS:  return vfs_get_mounts((char*)a1, a3);
        case SYS_DEVMGR_LIST: return devmgr_list((char*)a1, a3);
        default: return -1;
    }
}

void* syscall_dispatch_ptr(int num, const void* a1, void* a2, size_t a3) {
    (void)a2; (void)a3;
    switch (num) {
        case SYS_I18N_TRANSLATE: return (void*)i18n_translate((const char*)a1);
        default: return NULL;
    }
}
