# Sovereign RTC64 Kernel Repair Report

This document details the root causes and architectural solutions for the critical errors identified during the high-power hardening of the Sovereign RTC64 kernel.

## 1. The 32-bit Pointer Truncation Crisis
**Symptoms:** Kernel Page Fault (#PF) during hardware peripheral probe.
**Root Cause:** The kernel operates in HHDM at 0xffff800000000000. Hardware pointers were being truncated to 32-bit via (uint32_t) casts in the CherryUSB stack.
**Solution:** Patched external/CherryUSB/port/ehci/usb_ehci_reg.h to use uintptr_t.

## 2. Nested GPF via Stack Misalignment
**Symptoms:** GPF occurring inside exception handlers.
**Root Cause:** fxsave/fxrstor require 16-byte alignment.
**Solution:** Hardened kernel/isr_stubs.s with manual stack alignment before context saving.

## 3. Legacy 32-bit DMA Compatibility
**Symptoms:** DMA failures on EHCI/NVMe.
**Root Cause:** Legacy registers often support only 32-bit addresses.
**Solution:** Implemented pmm_alloc_low and pmm_alloc_blocks_low in kernel/pmm.c for the first 4GB of RAM.

## Summary
The system now follows a strict 8-phase boot sequence with 64-bit pointer integrity and stack alignment secured.
