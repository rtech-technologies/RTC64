# Detected Errors and Warnings Audit Report

## 1. Compilation Failures (High Severity)

### USB Hub Stack Failure
- **File:** `external/CherryUSB/class/hub/usbh_hub.c`
- **Error:** `CONFIG_USB_OSAL_THREAD_GET_ARGV` and `USB_OSAL_WAITING_FOREVER` are undeclared.
- **Context:** The `usbh_hub_thread` function fails to compile because these macros are not defined in `include/usb_osal.h` or `kernel/usb_config.h`.

### Missing Header
- **File:** `kernel/usb_hal_ports.c`
- **Error:** `usb_util.h: No such file or directory`
- **Context:** The file attempts to include `usb_util.h` directly, but the compiler cannot find it in the provided include paths, even though it exists in `external/CherryUSB/common/usb_util.h`.

---

## 2. Architectural Warnings (Medium Severity)

### Pointer Truncation (64-bit Compatibility)
- **File:** `external/CherryUSB/port/ehci/usb_hc_ehci.c`
- **Warnings:** `cast from pointer to integer of different size [-Wpointer-to-int-cast]`
- **Context:** Multiple macros (e.g., `QH_HLP_QH`) cast 64-bit pointers to `uint32_t`. In a 64-bit higher-half kernel, this causes pointer truncation and will lead to memory corruption or crashes when the USB controller attempts to access these addresses.

---

## 3. Logical and Implementation Gaps (Functional Errors)

### Incomplete Memory Allocator (TLSF)
- **File:** `kernel/tlsf_impl.c`
- **Issue:** The `tlsf_malloc` implementation is a skeleton. It lacks proper block splitting, coalescing, and comprehensive boundary tag management. It effectively "leaks" the remainder of blocks and does not support full memory reclamation.

### Stubbed Storage Drivers
- **Files:** `kernel/drivers/nvme.c`, `kernel/drivers/ahci.c`, `kernel/drivers/ramdisk.c`
- **Issue:**
    - `nvme_read` is an empty stub.
    - `ahci_init` only performs a global reset and enables AHCI mode but does not initialize ports or implement read/write.
    - `ramdisk_init` registers a device with `NULL` read/write callbacks, causing a kernel panic if accessed.

### Missing Real-Time Clock (RTC)
- **File:** `kernel/diskio_impl.c`
- **Issue:** `get_fattime` returns a constant `0`. Files created on FatFs volumes will have invalid timestamps (January 1st, 1980).

### Improper VFS Resource Management
- **File:** `kernel/vfs.c`
- **Issue:** `vfs_cat` and `vfs_ls` use fixed-size stack buffers (`char fpath[256]`). Long paths or many directory entries could cause stack overflows or silent truncation of data.

---

## 4. UI and UX Inconsistencies

### Hardcoded Mouse Cursor
- **File:** `kernel/kernel.c`
- **Issue:** The hardware cursor is drawn as a simple 4x4 white rectangle using `tgx_blit_rect`. This lacks the visual fidelity described in the project's "Gold Standard" goals.

### System Shell Limitations
- **File:** `kernel/vfs.c` / `src/chell.c`
- **Issue:** The shell commands (`ls`, `cat`) rely on `vfs_ls` and `vfs_cat` which return formatted strings rather than raw data streams, making piping or complex redirection impossible in the current architecture.
