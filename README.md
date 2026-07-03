# RTC64 Flagship Operating System

RTC64 is the flagship operating system platform for R-TECH™, built as a professional x86_64 OS with a custom kernel, Limine bootloader support, and a Linux-compatible driver layer for modern hardware.

This repository is the foundation for the RTC64 OS line: a fully bootable image, a reliable kernel driver stack, compatibility wrappers for real-world PCI drivers, and production-grade build tooling for ISO deployment.

## Why RTC64

RTC64 is designed as a flagship OS, not a hobby project. It delivers:

- A bootable, production-quality x86_64 kernel image
- A native PCI subsystem with robust device initialization
- APIC-based interrupt handling and Linux-style IRQ compatibility
- A Linux-compatible driver path for porting real hardware drivers
- USB and networking service scaffolding for long-term use
- ISO generation and bootloader installation ready for deployment

## What’s Included

- `kernel/`: Core OS kernel, drivers, PCI subsystem, interrupt handling, and boot metadata.
- `include/`: Shared interfaces, Linux compatibility headers, and platform abstractions.
- `external/CherryUSB/`: USB host and class drivers for professional device support.
- `scripts/`: Build and ISO generation helpers.

## Linux Compatibility Layer

RTC64 includes a kernel-side compatibility layer for Linux-style drivers, including:

- `pci_register_driver` / `pci_unregister_driver`
- `linux_compat_probe_pci_device`
- `pci_enable_device`, `pci_disable_device`, `pci_ioremap_bar`, `pci_set_master`
- `request_irq` / `free_irq`
- Basic `net_device` / `sk_buff` stubs for Ethernet driver integration

This approach preserves RTC64’s native PCI stack while exposing familiar driver interfaces for porting real-world code.

## Build Requirements

Required tools:

- `gcc`
- `make`
- `python3`
- `xorriso`
- `qemu-system-x86_64` (recommended for testing)
- `git`

The repository contains `external/limine`, so Limine is built in-tree and no separate Limine install is required.

## Build and Test

### Build and Run the OS in QEMU
```bash
make run
```

This builds the kernel, creates `os.iso`, and launches QEMU.

### Build Kernel and ISO Only
```bash
make kernel
```

Output files:

- `kernel/kernel`
- `kernel/ramdisk.img`
- `os.iso`

### Run the ISO Manually
```bash
qemu-system-x86_64 -m 512M -cdrom os.iso -boot d -device qemu-xhci -device usb-kbd -device usb-mouse -serial stdio
```

### Clean Build Artifacts
```bash
make clean
```

## Deployment for Permanent Use

RTC64 is intended to serve as a production OS image. After validating the kernel and hardware support, the generated `os.iso` can be deployed to real systems or used as a production VM image.

**Write to USB** (replace `/dev/sdX` with the correct device):
```bash
sudo dd if=os.iso of=/dev/sdX bs=4M status=progress conv=fsync
sync
```

> Warning: This command will overwrite the target device. Verify the device path carefully.

## Recommended Production Workflow

1. Build and validate the OS in QEMU.
2. Port, test, and certify drivers with the Linux compatibility layer.
3. Add persistent storage support, boot configuration, and system services.
4. Create a release ISO and test it on target hardware.
5. Keep separate development and production image sets.

## Production-Ready Focus Areas

To make RTC64 a complete flagship OS, the next professional milestones are:

- Full PCI and device driver coverage for target hardware
- Robust interrupt, DMA, and power management support
- Reliable storage, filesystem, and persistence layers
- Production Ethernet/network stack and NIC drivers
- User-facing install, configuration, and boot management

## Project Structure

- `src/`: Application and UI source code.
- `kernel/`: Kernel sources, drivers, and platform runtime.
- `include/`: Public headers and compatibility wrappers.
- `external/CherryUSB/`: USB host stack and class drivers.
- `scripts/`: ISO and ramdisk generation scripts.

## Moving Toward a Complete OS

Current areas for continued professional development include:

- PCI and hardware driver maturity
- interrupt, DMA, and power management
- storage, filesystem, and persistence
- networking and Ethernet driver support
- system configuration and install paths

RTC64 is structured as a professional OS platform, and these components are the next steps toward a production-quality system.
