# R-TECH™ Sovereign RTC64 Operating System
## Technical Specification & Architectural Blueprint (NEONT)

Sovereign RTC64 is a high-performance, hardened x86_64 higher-half kernel designed for extreme stability, security, and professional diagnostics. It follows a strict 8-phase executive boot sequence and implements advanced architectural safeties to mitigate 100+ common OS error categories.

---

### 1. System Specifications
- **Architecture:** x86_64 (AMD64) 64-bit Long Mode.
- **Kernel Mapping:** Higher-half virtual allocation at `0xffffffff80000000`.
- **Boot Protocol:** Limine (Revision 3) with full anchor/marker compliance.
- **Preemption:** 100Hz Local APIC Timer (10ms heartbeat).
- **Memory:** Bit-packed Matrix PMM + TLSF Executive Pool (16MB Genesis Heap).
- **Security:** UAID/UPID Process Sandboxing, Task State Segment (TSS) Isolation.
- **Failure Recovery:** IST1-isolated Double Fault handling, Recursive Panic detection.
- **Graphics:** SSE2-accelerated software rasterization on a linear graphical framebuffer.

---

### 2. External Modules (Subsystem Components)
The Sovereign RTC64 kernel integrates several professional-grade external libraries to provide low-level logic:
- **Limine:** Transitions the CPU to 64-bit mode, parses the ELF kernel, and provides the initial HHDM (Higher-Half Direct Map) and memory map.
- **Nuklear:** An immediate-mode GUI toolkit used for all graphical applications and windows.
- **CherryUSB:** A bare-metal USB Host/Device stack providing xHCI and EHCI controller support.
- **FatFs:** An industry-standard FAT12/16/32 filesystem implementation.
- **TLSF (Two-Level Segregated Fit):** A high-speed, constant-time memory allocator for the kernel heap.
- **TGX:** A primitive graphics library used for software rendering of geometric shapes and UI components.
- **STB Image:** Used for loading and processing graphical assets.

---

### 3. R-TECH™ Original Implementation (Core Kernel Logic)

#### A. Executive Core (`kernel/`)
- **`kernel.c`**: The **High-Power Executive Initialization Sequence**. It orchestrates the 8-phase boot process, establishing the HHDM, initializing the PMM and Executive Heap, loading architectural tables, and pivoting to the User Land Environment Manager.
- **`scheduler.c`**: Meaty preemptive scheduler. It manages tasks using a stable ID system and a `TASK_DEAD` state to prevent handle invalidation. It performs full architectural state saving (15 GPRs, CRs, Segments) and **512-byte SSE/FPU context switching** via `FXSAVE`.
- **`pmm.c`**: Physical Memory Manager. Employs a bit-packed matrix to track every 4KB page. Includes `pmm_alloc_low` to force allocations below 4GB for legacy hardware and DMA contexts.
- **`panic.c`**: Monolithic Panic Engine. Renders the **Orange Screen of Death (#FF4500)**. It captures and bitmaps the CPU state, security UAID/UPID, and diagnostic register dumps to both the framebuffer and COM1. Features a recursive guard to detect "Double Panics."
- **`gdt.c`**: 64-bit Global Descriptor Table implementation. Crucially implements the **Task State Segment (TSS)** and configures **IST1 (Interrupt Stack Table 1)** for Double Fault isolation.
- **`interrupts.c`**: Central IDT manager. Routes hardware exceptions and IRQs. Includes defensive EOI logic to acknowledge unhandled non-spurious interrupts, preventing "Interrupt Storms."
- **`isr_stubs.s`**: Optimized assembly entry points. Ensures **16-byte stack alignment** for every interrupt to prevent General Protection Faults during SSE state restoration.
- **`apic.c`**: Local APIC driver. Configures the system timer tick and implements the **Spurious Interrupt Handler (Vector 255)** to filter hardware noise.
- **`usb_osal.c`**: Sovereign's "Meaty" OSAL for CherryUSB. Implements counting semaphores, mutexes, and real-time timers driven by the system tick via `hal_get_uptime_ms()`.
- **`vfs.c`**: Virtual File System. Provides a unified namespace (`/mnt/`) and deep path translation between Sovereign virtual paths and physical FatFs drive strings.
- **`sys_shell.c`**: Persistent serial diagnostic shell on COM1 (115200 baud). Provides real-time metrics for memory, tasks, and PCI hardware.
- **`input.c`**: Kernel event broker. Manages a circular buffer for mouse/keyboard events. Translates raw USB HID packets into GUI-ready coordinates.
- **`uac_policy.c`**: User Account Control engine. Enforces security boundaries and audits privilege elevation requests based on UAID and UPID identifiers.
- **`syscall.c`**: Native system call dispatcher, providing applications with access to executive services via numeric identifiers.
- **`nuklear_kernel_impl.c`**: Freestanding libc implementation. Features **SSE2-optimized `memset` and `memcpy`** using 128-bit XMM registers for maximum GUI performance.
- **`malloc_glue.c`**: Hardened bridge for the TLSF allocator. Includes null-pointer guards and **Integer Overflow protection** in `calloc`.
- **`storage_hal.c`**: Unified storage device registry, managing the lifecycle of disks detected during the PCI scan.
- **`panic_hal.c`**: Hardware bridge that extracts Limine framebuffer metadata for use by the graphical panic renderer.
- **`diskio_impl.c`**: Low-level implementation layer mapping FatFs disk operations to the Sovereign Storage HAL.
- **`ffsystem_impl.c`**: Operating system interface for FatFs, mapping its memory management to the kernel heap.
- **`tgx_impl.c`**: Backend implementation for TGX primitives on the Sovereign linear graphical framebuffer.
- **`i18n.c`**: System-wide translation engine for internationalized UI and error reporting.
- **`serial.c`**: Low-level UART driver for COM1, used for exhaustive logging and the diagnostic shell.

#### B. Hardware Architecture Drivers (`kernel/drivers/`)
- **`pci.c`**: High-power PCI/PCI-e discovery engine. Scans the I/O matrix and maintains a global registry of all detected hardware.
- **`xhci.c`**: USB 3.0 controller driver. Implements 64-bit physical address management for the Device Context Base Address Array Pointer (DCBAAP).
- **`ehci.c`**: USB 2.0 controller driver. Features robust port power-up sequences and hardware-level reset monitoring.
- **`nvme.c`**: Meaty NVMe driver. Manages admin and I/O queues for high-speed SSD storage.
- **`ahci.c`**: Modern SATA/AHCI driver with PRDT-based DMA support for disk Read/Write operations.
- **`ramdisk.c`**: Volatile memory-backed storage driver for recovery and installation scenarios.
- **`rtc.c`**: Driver for the CMOS Real-Time Clock, providing precise hardware-level time tracking.

#### C. User Land Applications (`src/`)
- **`app_ui.c`**: The **Environment Manager (smss.exe/explorer.exe equivalent)**. Implements the Windows PE-style recovery desktop, window management, and taskbar.
- **`chell.c`**: The **Graphical Application Shell**. Provides a terminal-focused interface for VFS navigation, device management, and system auditing.
- **`lab.c`**: **Sovereign Lab**. A full diagnostic suite for real-time hardware testing and PCI device enumeration.
- **`main.c`**: Standard entry point for hosted-build simulation (used for UI development outside the kernel).
- **`nk_software_renderer.c`**: High-power software rasterizer that converts Nuklear UI draw commands into linear pixel data.
- **`installer.c`**: Installation Wizard module for environment configuration.

---

### 4. Hardware Safety & Error Mitigation (Section 4 Compliance)
Sovereign RTC64 is hardened against critical failure modes:
- **Double Fault Isolation:** IDT Vector 8 uses a dedicated TSS IST stack, preventing triple faults when the primary kernel stack is compromised.
- **Recursive Panic Guard:** Detects if a fault occurs during the reporting of another fault, halting the system safely.
- **Integer Overflow Protection:** Memory allocation arithmetic is verified before execution to prevent wrapped-integer buffer overflows.
- **Interrupt Storm Guard:** Unhandled IRQs are acknowledged and silenced to prevent CPU starvation.

---

### 5. Deployment and Build
- **Linker Logic:** `kernel/linker.ld` enforces RX/RW page-boundary separation for improved security.
- **ISO Generation:** The Makefile produces `os.iso` using `xorriso` and `mtools`, installing the Limine BIOS stages for wide hardware compatibility.

*Documented by Sovereign - R-TECH™ High-Power Computing Division.*
