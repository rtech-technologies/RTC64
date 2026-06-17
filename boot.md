# Sovereign RTC64 Boot Order & Architectural Initialization

The Sovereign RTC64 kernel follows a strict 8-phase executive boot sequence, modeled after the Windows NT executive initialization.

---

### Phase 0: The Bare-Metal Isolation Layer (Non-Preemptive)
During this phase, interrupts are disabled (`CLI`), and the system is in a strictly sequential, single-threaded state.

1. **Step 1: Bootloader Handoff & Memory Mapping**
   - Winload-style handoff from Limine.
   - Establish the Higher-Half Direct Map (HHDM).
   - Register the primary graphical framebuffer.
   - Activate Streaming SIMD Extensions (SSE2).

2. **Step 2: Physical Memory Matrix (PMM)**
   - Scan the Limine memory map.
   - Initialize the bit-packed Physical Memory Matrix.
   - Guard reserved kernel memory regions.

3. **Step 3: Executive Pool (Heap) Genesis**
   - Allocate 16MB for the primary kernel heap.
   - Initialize the TLSF allocator.

4. **Step 4: Architectural Table Setup**
   - Construct and load the Global Descriptor Table (GDT).
   - Configure the Task State Segment (TSS) with an Interrupt Stack Table (IST1).
   - Load the Interrupt Descriptor Table (IDT) with hardware exception gateways.

5. **Step 5: Timer Hardware Genesis**
   - Calibrate the Local APIC timer frequency using the PIT (Programmable Interval Timer).
   - Set the 100Hz periodic heartbeat (initially masked).

6. **Step 6: Driver Orchestration & Component Manager**
   - Probe the I/O Matrix and PCI bus.
   - Bind storage drivers (Ramdisk, NVMe, AHCI).
   - Initialize the Virtual File System (VFS) and FatFs.
   - Launch the USB Host Stack (CherryUSB).

---

### Phase 1: The Executive Subsystem Onboarding (Preemptive)
After Phase 0 completes, the system transitions to multi-tasking.

7. **The STI Handover**
   - Enable interrupts (`STI`).
   - The Local APIC timer starts triggering the preemptive scheduler.
   - Background services (COMPREC, System Monitor) start execution.

---

### Phase 7: User Land Pivot (WinPE Recovery Environment)
The final stage establishes the user-facing diagnostic environment.

8. **Environment Manager Startup**
   - Initialize the Nuklear GUI engine.
   - Pivot to the "Environment Manager" privileged task.
   - Launch the Graphical Shell and Desktop.
   - Execute the persistent Serial Diagnostic Shell on COM1.
