# Sovereign OS Code Audit & Error Report

This document outlines identified bugs, architectural flaws, and potential instabilities discovered during a comprehensive audit of the RTC64 kernel.

## 1. High Severity: Architectural & Memory Safety

### 1.1 64-bit Pointer Truncation in EHCI Driver
*   **Location:** `kernel/cherryusb/port/ehci/usb_hc_ehci.h`, `usb_hc_ehci.c`
*   **Issue:** Macros such as `EHCI_PTR2ADDR` cast 64-bit virtual pointers to `uint32_t`. In a higher-half kernel (mapped at `0xffffffff80000000`), this truncates the address, resulting in invalid memory access by the HBA or Page Faults when the kernel attempts to dereference truncated pointers.
*   **Consequence:** Immediate Page Fault (Vector 0x0E) or silent memory corruption during USB initialization.

### 1.2 Fragile Panic Argument Parsing
*   **Location:** `kernel/panic.c` ( `panic_printf` )
*   **Issue:** Manual stack/register parsing using `__asm__ volatile ("movq %%rbp, %0" : "=r"(args))` is highly dependent on compiler behavior (`-fno-omit-frame-pointer`). If optimized, the argument pointers will be incorrect.
*   **Consequence:** Garbage output on the panic screen, making debugging impossible.

### 1.3 Kernel Heap Exhaustion
*   **Location:** `kernel/kernel.c`, `kernel/malloc_glue.c`
*   **Issue:** The kernel uses a fixed 16MB heap. There is no monitoring or protection against heap exhaustion during intensive UI rendering (Nuklear) or large file operations.
*   **Consequence:** System hang or silent allocation failure leading to NULL pointer dereferences.

## 2. Medium Severity: Driver & Subsystem Logic

### 2.1 Skeletal XHCI/EHCI Implementation
*   **Location:** `kernel/drivers/xhci.c`, `kernel/drivers/ehci.c`
*   **Issue:** These drivers perform a hardware reset but do not actually implement the Command/Event ring logic or bridge the controller to the CherryUSB host stack.
*   **Consequence:** USB 3.0 (xHCI) devices will not be detected or functional despite being found during PCI scan.

### 2.2 VFS Mount Table Inconsistency
*   **Location:** `kernel/vfs.c` ( `vfs_refresh_mounts` )
*   **Issue:** The mount logic does not handle device removal or re-ordering. If `hal_storage_get_device_count()` changes, the indices in the `VolToPart` table (FatFs) and the VFS mount points may become misaligned.
*   **Consequence:** Attempting to access `nvme0` might result in accessing a USB drive or a "Drive Not Found" error.

### 2.3 `vsnprintf` Limitations
*   **Location:** `kernel/nuklear_kernel_impl.c`
*   **Issue:** Missing support for field width (`%08x`), length modifiers (`%hu`, `%hhd`), and floating point.
*   **Consequence:** Hard-to-read logs and incorrect hardware address reporting.

## 3. Low Severity: UI & Maintenance

### 3.1 Hardcoded Drive Formatting
*   **Location:** `src/app_ui.c`, `src/installer.c`
*   **Issue:** `f_mkfs` uses a hardcoded `FF_MAX_SS` (sector size). This may fail on drives with non-standard sector sizes (e.g., 4K native).
*   **Consequence:** Drive formatting fails in the installer for certain hardware.

### 3.2 USB Polling Latency
*   **Location:** `kernel/usb_hal.c`
*   **Issue:** Entire USB stack relies on `hal_usb_poll()` called from the main UI loop.
*   **Consequence:** UI lag can cause USB timeouts or dropped packets, especially for HID devices (keyboard/mouse).
