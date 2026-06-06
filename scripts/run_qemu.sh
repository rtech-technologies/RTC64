#!/bin/sh

set -e

echo "Preparing to run QEMU..."

# If xorriso is available, create the ISO and boot via Limine.
if command -v xorriso >/dev/null 2>&1; then
    echo "xorriso found: creating ISO and booting via Limine"
    make iso
    if command -v qemu-system-x86_64 >/dev/null 2>&1; then
        qemu-system-x86_64 -m 512M -cdrom os.iso -boot d -device qemu-xhci -device usb-kbd -device usb-mouse -serial stdio "$@"
    else
        echo "QEMU not found. ISO created at os.iso. Install qemu-system-x86_64 to run it."
        exit 1
    fi
else
    echo "xorriso not found — attempting to launch QEMU directly with kernel ELF (no ISO/limine)"
    if command -v qemu-system-x86_64 >/dev/null 2>&1; then
        qemu-system-x86_64 -kernel kernel/kernel -m 512M -serial stdio -device qemu-xhci -device usb-kbd -device usb-mouse "$@"
    else
        echo "QEMU not found. Install xorriso and qemu-system-x86_64 to create ISO and run."
        exit 1
    fi
fi
