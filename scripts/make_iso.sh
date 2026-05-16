#!/bin/bash
set -e

# Configuration
KERNEL="kernel/kernel"
LIMINE_DIR="external/limine"
ISO_DIR="iso_root"
ISO_FILE="os.iso"

echo "=== Creating Bootable ISO ==="

# Check requirements
if [ ! -f "$KERNEL" ]; then
    echo "Error: Kernel not found at $KERNEL. Build it first."
    exit 1
fi

if [ ! -d "$LIMINE_DIR" ]; then
    echo "Error: Limine not found at $LIMINE_DIR. Run setup first."
    exit 1
fi

# Prepare ISO directory structure
rm -rf "$ISO_DIR"
mkdir -p "$ISO_DIR/boot"
mkdir -p "$ISO_DIR/boot/limine"
mkdir -p "$ISO_DIR/EFI/BOOT"

# Copy kernel
cp "$KERNEL" "$ISO_DIR/boot/"

# Copy Limine config and binaries
cp kernel/limine.conf "$ISO_DIR/boot/limine/"
cp "$LIMINE_DIR/limine-bios.sys" "$ISO_DIR/boot/limine/"
cp "$LIMINE_DIR/limine-bios-cd.bin" "$ISO_DIR/boot/limine/"
# cp "$LIMINE_DIR/limine-uefi-cd.bin" "$ISO_DIR/boot/limine/"
cp "$LIMINE_DIR/BOOTX64.EFI" "$ISO_DIR/EFI/BOOT/"
cp "$LIMINE_DIR/BOOTIA32.EFI" "$ISO_DIR/EFI/BOOT/"

# Create the ISO
# Using paths relative to ISO_DIR for -b and --efi-boot
xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin \
    -no-emul-boot -boot-load-size 4 -boot-info-table \
    -o os.iso "$ISO_DIR"

# Install Limine deployment tool
"$LIMINE_DIR/limine" bios-install "$ISO_FILE"

echo "=== ISO Created: $ISO_FILE ==="
