# Sovereign RTC64 Hybrid Boot Architecture

To build a Windows-style executive boot sequence for Sovereign RTC64, we mirror the architectural rigour of the Windows NT startup process (the NTOSKRNL/SMSS model) while maintaining the Phase 0-7 hardware-focused foundation.

## Part 1: Phase 0 Hardware Foundation (The "POST" and Loader Layer)
This is the low-level hardware environment setup—the equivalent of the firmware and boot manager (BOOTMGR) initialization before the kernel takes command.

- **Phase 0: Bare-Metal Isolation** (Interrupts CLI, initial stack setup).
    - **Step 1: Bootloader Handoff** (Limine environment discovery, HHDM/Framebuffer registration).
    - **Step 2: Memory Matrix** (PMM initialization—the system’s physical memory map).
    - **Step 3: Kernel Heap Genesis** (Executive Pool creation for the life of the kernel).
    - **Step 4: Architecture Frame Setup** (GDT/IDT/TSS—the CPU security boundaries).
    - **Step 5: Entropy and Security Activation** (Local APIC/Timer calibration).
    - **Step 6: Hardware Peripheral I/O Probe** (Bus enumeration, driver orchestration).
    - **Step 7: Subsystem Threading** (Scheduler initialization for the first system tasks).

## Part 2: Phases 1–7 Executive Startup (The "Windows-Style" Sequence)
Once the hardware is stable (post-Step 7), the Sovereign kernel transitions into the Executive mode, mirroring the stages Windows uses to bring up its internal subsystems (NTOSKRNL → SMSS → Wininit).

| Sovereign Phase | Windows Equivalent | Description |
| :--- | :--- | :--- |
| Phase 1 | Kernel Initialization | Hardware interrupts enabled; PnP Manager and Executive components start. |
| Phase 2 | Namespace Initialization | Virtual File System (VFS) and Object Manager startup. |
| Phase 3 | Security Monitor (SRM) | Security Reference Monitor (Local Security Authority/lsass.exe equivalent). |
| Phase 4 | Power & I/O Manager | Finalizing storage stacks, IRP stability, and mounting volumes. |
| Phase 5 | Session Manager (smss.exe) | Spawning the initial executive sessions and user-mode environment boundaries. |
| Phase 6 | Service Control Manager | Starting core background services (e.g., COMPREC, Drivers, Diagnostic Shell). |
| Phase 7 | Environment Manager | Spawning the User-mode GUI/Console (The "WinPE" shell/Environment Manager). |

## Implementation Strategy
- **Strict Serial Logging**: serial_printf calls at every sub-step to diagnose exact crash points.
- **HHDM Pivot**: Framebuffer access must use the HHDM virtual offset for professional high-power operation.
- **Unified Control**: The system transitions mathematically from raw silicon to a structured WinPE-like executive shell.
