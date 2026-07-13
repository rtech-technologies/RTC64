#!/bin/bash
# Sovereign Unified UEFI/BIOS Bootable ISO Generator
set -e

# Configuration
KERNEL="kernel/kernel"
LIMINE_DIR="external/limine"
ISO_DIR="iso_root"
ISO_FILE="os.iso"

echo "=== Creating Bare-Metal Compatible Bootable ISO ==="

# Check requirements
if [ ! -f "$KERNEL" ]; then
    echo "Error: Kernel binary not found at target location: $KERNEL"
    exit 1
fi

# Prepare ISO directory structure
rm -rf "$ISO_DIR"
mkdir -p "$ISO_DIR/boot"
mkdir -p "$ISO_DIR/boot/limine"
mkdir -p "$ISO_DIR/EFI/BOOT"

# Copy kernel & ramdisk
cp "$KERNEL" "$ISO_DIR/boot/kernel.elf"
if [ -f "kernel/ramdisk.img" ]; then
    cp "kernel/ramdisk.img" "$ISO_DIR/boot/ramdisk.img"
fi

# Copy Limine config
cp kernel/limine.cfg "$ISO_DIR/limine.conf"
cp kernel/limine.cfg "$ISO_DIR/limine.cfg"
cp kernel/limine.cfg "$ISO_DIR/boot/limine.conf"
cp kernel/limine.cfg "$ISO_DIR/boot/limine.cfg"
cp kernel/limine.cfg "$ISO_DIR/boot/limine/limine.conf"
cp kernel/limine.cfg "$ISO_DIR/boot/limine/limine.cfg"

# Copy Limine bios-boot components
cp "$LIMINE_DIR/limine-bios.sys" "$ISO_DIR/boot/"
cp "$LIMINE_DIR/limine-bios-cd.bin" "$ISO_DIR/boot/"
cp "$LIMINE_DIR/limine-bios.sys" "$ISO_DIR/boot/limine/"
cp "$LIMINE_DIR/limine-bios-cd.bin" "$ISO_DIR/boot/limine/"

# Copy Limine uefi-boot components for bare-metal portability
cp "$LIMINE_DIR/limine-uefi-cd.bin" "$ISO_DIR/boot/"
cp "$LIMINE_DIR/limine-uefi-cd.bin" "$ISO_DIR/boot/limine/"
cp "$LIMINE_DIR/BOOTX64.EFI" "$ISO_DIR/EFI/BOOT/"
cp "$LIMINE_DIR/BOOTIA32.EFI" "$ISO_DIR/EFI/BOOT/"

# Create the Hybrid UEFI/BIOS ISO using xorriso
xorriso -as mkisofs \
    -b boot/limine-bios-cd.bin \
    -no-emul-boot -boot-load-size 4 -boot-info-table \
    --efi-boot boot/limine-uefi-cd.bin \
    -efi-boot-part --efi-boot-image \
    -o "$ISO_FILE" "$ISO_DIR"

# Install Limine deployment tool for BIOS compatibility
"$LIMINE_DIR/limine" bios-install "$ISO_FILE"

echo "=== Universal UEFI/BIOS Bootable ISO Created: $ISO_FILE ==="
