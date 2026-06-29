# Sovereign RTC64 Kernel - Security & Architectural Audit

This document lists every identified flaw, potential vulnerability, and architectural weakness discovered during the industrial audit of the NEONT executive core.

## 1. Critical Initialization & Boot Flaws
- **[Limine] Missing Request Markers:** The kernel is missing `LIMINE_REQUESTS_START_MARKER` and `LIMINE_REQUESTS_END_MARKER`. Without these, the bootloader fails to populate responses for HHDM, Memmap, and Framebuffer, resulting in a system-wide null-pointer cascade.
- **[Boot] HHDM Dependency:** The kernel assumes `hhdm_offset` is always valid. If the HHDM request fails, the kernel continues execution with `hhdm_offset = 0`, leading to incorrect virtual-to-physical mapping and Page Faults during driver initialization.
- **[PMM] Unsafe Bitmap Placement:** `pmm_init` does not verify if `hhdm_offset` is non-zero before calculating the virtual address for the physical bitmap.
- **[Heap] Silent Genesis Failure:** If `pmm_alloc_blocks` fails during heap creation, `hal_malloc_init` is called with a NULL pointer. This results in `global_tlsf_control` remaining NULL, causing all subsequent `malloc` calls to return NULL without error.

## 2. Memory Management & Safety
- **[PMM] Inefficient Block Search:** `pmm_alloc_blocks` always starts searching from page 0 rather than using `pmm_last_alloc`, leading to $O(N)$ performance degradation.
- **[PMM] Lack of Initialization Guards:** Core functions like `pmm_alloc` do not verify if `pmm_bitmap` is non-null, leading to early boot null-pointer dereferences.
- **[Scrubbing] Direct PMM Leak:** While `tlsf_free` zeroes memory, memory requested via `pmm_alloc` directly (for DMA) is not zeroed, potentially leaking physical frame data between drivers.

## 3. Concurrency & Synchronization
- **[VFS] Thread-Unsafe Resolver:** `vfs_resolve` uses a `static char res[256]` buffer. Concurrent calls from different tasks (e.g., Shell and App) will result in path corruption.
- **[Serial] Incomplete Syscall Locking:** The `SYS_SERIAL_WRITE` syscall routes directly to `serial_write`, bypassing the spinlock used by `serial_printf`.
- **[Storage] IO Race Conditions:** `ahci_io_wrapper` and `nvme_io_wrapper` use fixed command slots without per-device locking. Concurrent IO requests will corrupt the hardware command rings.

## 4. Hardware Drivers
- **[AHCI] Virtual Address DMA:** The AHCI driver passes virtual buffers directly to the PRDT. The SATA controller (requiring physical addresses) will DMA into incorrect memory.
- **[EHCI] Incomplete Initialization:** The EHCI driver fails to initialize the Periodic or Asynchronous list base registers. USB transfers will never be processed by the hardware.
- **[PS/2] Blocking IO:** `ps2_wait_read` and `ps2_wait_write` are infinite loops without timeouts. A hardware failure will hang the entire kernel.
- **[xHCI] Slot Config Race:** The xHCI driver reads `HCSPARAMS1` and writes `CONFIG` without ensuring the controller is in a stable state for configuration.

## 5. Scheduling & ABI
- **[Scheduler] Task Limit:** The kernel is hard-coded to 16 tasks. `scheduler_spawn` fails silently when the limit is reached.
- **[Scheduler] Delayed Stack Audit:** Stack overflow detection (canary check) only occurs in the Idle task once per second, leaving a large window for memory corruption.
- **[Syscall] Pointer Validation:** VFS syscalls do not verify if provided buffer pointers belong to the calling task's memory space.

## 6. Graphics & UI
- **[VGA] Dirty Rect Missing:** `vga_log` performs a full-screen `memset` on wrap-around, causing visible flicker. No line-scrolling is implemented.
- **[Input] Coordinate Clamping:** `hal_input_push_event` accumulates mouse coordinates without clamping to screen dimensions, allowing the cursor to move into "ghost" memory.
- **[Framebuffer] Unprotected Access:** Multiple tasks (Environment Manager and VGA Log) can write to the framebuffer simultaneously without synchronization.

## 7. Security & Policy
- **[UAC] Implementation Stubs:** `uac_request_permit` in `kernel/uac_policy.c` is currently a placeholder and does not actually perform security challenges or interrupt-based elevation.
- **[Panic] OSOD Recursion:** If `draw_glyph` or `memset` fails inside `osod_render`, the kernel may enter a recursive panic state without a hardware reset.
- **[VFS] FAT32 Auto-Format:** The VFS automatically formats uninitialized disks. This is a potential data-loss risk if a disk with a different filesystem is connected.

## 8. 64-bit Architectural Risks
- **[Boot] Limine Request Markers:** Mandatory `LIMINE_REQUESTS_START_MARKER` and `LIMINE_REQUESTS_END_MARKER` are missing from the kernel entry point, which may cause Limine to ignore the executive's hardware requests (Memmap, HHDM, Framebuffer) on some versions.
- **[MMIO] Hardcoded APIC Address:** `APIC_BASE` is hardcoded to `0xFEE00000`. Industrial kernels should discover the local APIC address via the MP Table or ACPI MADT to ensure compatibility across diverse hardware.
- **[Paging] HHDM Assumption:** The kernel assumes a single `hhdm_offset` applies to all physical addresses. If the bootloader maps different regions with different offsets (unlikely but possible in Limine), the kernel logic will fail.
- **[CPU] Lack of Multi-core Support:** The kernel is strictly single-core. While spinlocks are implemented, they do not handle IPIs (Inter-Processor Interrupts) or cache coherency across multiple physical CPUs.
