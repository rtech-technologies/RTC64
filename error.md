# Sovereign RTC64 Critical Error Audit

This document details every potential architectural or logical error that could prevent the Sovereign RTC64 kernel from booting.

## 1. Physical Memory Manager (PMM) Failures
- **Error:** Memory Map Missing.
- **Cause:** Limine fails to provide a valid memory map response.
- **Result:** Kernel cannot track usable physical pages; `pmm_alloc` returns NULL.
- **Impact:** CRITICAL. Immediate system hang.

## 2. Kernel Heap (TLSF) Exhaustion
- **Error:** Out of Memory (OOM) during Stage 0.
- **Cause:** Subsystems (USB, FatFs) allocate more than the initial 16MB heap.
- **Result:** Subsystems return error codes or NULL pointers.
- **Impact:** CRITICAL. Subsystems like VFS will fail to initialize, causing a cascading failure.

## 3. Interrupt Handling & IDT
- **Error:** Missing Exception Handler for Page Fault.
- **Cause:** Attempting to access an unmapped or non-canonical address before the IDT is fully mapped.
- **Result:** Triple fault and instant reboot.
- **Impact:** CRITICAL.

## 4. GDT/Stack Alignment
- **Error:** Misaligned Stack during ISR.
- **Cause:** The `cpu_state` struct or the stack pointer is not 16-byte aligned before `fxsave`/`fxrstor`.
- **Result:** General Protection Fault (#GP) during the interrupt return or context switch.
- **Impact:** CRITICAL.

## 5. Storage Driver Timeout
- **Error:** Controller Not Ready (NVMe/AHCI).
- **Cause:** Hardware fails to respond within the 1,000,000-iteration loop limit during initialization.
- **Result:** `nvme_init` or `ahci_init` returns failure.
- **Impact:** MAJOR. VFS will not mount the system drive, and boot assets will be unavailable.

## 6. USB Stack Deadlock
- **Error:** CherryUSB Queue Overflow.
- **Cause:** Excessive input events generated before the Environment Manager task is spawned to drain the queue.
- **Result:** Memory corruption or dropped input events.
- **Impact:** MODERATE. UI will appear frozen.

## 7. Scheduler Context Corruption
- **Error:** Invalid Stack Pointer in Task.
- **Cause:** `scheduler_add_task` initializes the RIP or RSP with incorrect values.
- **Result:** Immediate crash upon the first task switch.
- **Impact:** CRITICAL.

## 8. SSE Hardware Activation
- **Error:** Invalid Opcode (#UD).
- **Cause:** Calling `memset` or `memcpy` (which use XMM registers) before `init_sse()` is executed.
- **Result:** System crash.
- **Impact:** CRITICAL.
