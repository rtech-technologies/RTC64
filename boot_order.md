# Sovereign RTC64 Boot Order Documentation

The Sovereign RTC64 kernel follows a strict 8-step boot sequence modeled after the Windows Executive initialization process.

## Phase 0: The Bare-Metal Isolation Layer
During this phase, hardware interrupts are strictly disabled (`cli`). Execution remains confined to a single CPU core.

### Step 1: The Bootloader Handoff and Registry Mapping
- **Action:** Intercept control from Limine and parse response pointers.
- **Purpose:** Store addresses for physical memory, the framebuffer, and assets in global kernel structures before any executive logic runs.

### Step 2: The Core Memory Matrix Allocation
- **Action:** Pass the memory map to the Physical Memory Manager (PMM).
- **Purpose:** Establish page tracking and memory matrix stability before high-level C functions are called.

### Step 3: The Critical Kernel Heap Genesis
- **Action:** Initialize the kernel heap allocator (TLSF).
- **Purpose:** Provide dynamic memory support for subsystems like CherryUSB and FatFs.

### Step 4: The Hardware Architecture Frame Setup
- **Action:** Load GDT, construct IDT, and map exception handlers.
- **Purpose:** Ensure the CPU can handle faults and interrupts safely before unmasking interrupt lines.

## Phase 1: The Executive Subsystem Onboarding
Hardware interrupts are safely enabled (`sti`). The system clock starts, and core OS services activate.

### Step 5: The Entropy and Security Activation
- **Action:** Initialize Local APIC timer and unmask interrupts.
- **Purpose:** Establish a secure and stable time base and interrupt environment.

### Step 6: The Hardware Peripheral I/O Probe
- **Action:** Initialize CherryUSB core and probe host controllers (xHCI/EHCI).
- **Purpose:** Load boot-start drivers and discover peripherals without freezing the execution pipeline.

## User Space: The Environment Management Hand-off
The kernel transitions execution to an isolated, unprivileged Environment Manager.

### Step 7: The Session Manager Pivot (smss.exe Equivalent)
- **Action:** Spawn the "Environment Manager" task.
- **Purpose:** Isolate the graphics and user interface layers from the core kernel space.

### Step 8: The Graphics Subsystem and Input Loop Launch
- **Action:** Execute the Nuklear GUI loop inside the Environment Manager.
- **Purpose:** Sequential polling of inputs and pixel array drawing directly onto the framebuffer.
