#!/bin/bash
set -e

# Configuration
KERNEL="kernel/kernel"
LIMINE_DIR="external/limine"
ISO_DIR="iso_root"
ISO_FILE="os.iso"

echo "=== Creating Bootable ISO ==="

# 1. Prepare ISO directory structure
rm -rf "$ISO_DIR"
mkdir -p "$ISO_DIR/boot"
mkdir -p "$ISO_DIR/EFI/BOOT"

# 2. Copy Kernel
if [ -f "$KERNEL" ]; then
    cp "$KERNEL" "$ISO_DIR/boot/kernel.elf"
else
    echo "Error: $KERNEL not found!"
    exit 1
fi

# 3. Copy Limine Configuration (Put in /boot/ as requested, and / for safety)
cp kernel/limine.conf "$ISO_DIR/boot/limine.conf"
cp kernel/limine.conf "$ISO_DIR/limine.conf"

# 4. Copy Limine Binaries
if [ -d "$LIMINE_DIR" ]; then
    cp "$LIMINE_DIR/limine-bios.sys" "$ISO_DIR/boot/"
    cp "$LIMINE_DIR/limine-bios-cd.bin" "$ISO_DIR/boot/"
    cp "$LIMINE_DIR/BOOTX64.EFI" "$ISO_DIR/EFI/BOOT/"
    cp "$LIMINE_DIR/BOOTIA32.EFI" "$ISO_DIR/EFI/BOOT/"
else
    echo "Error: Limine assets not found in $LIMINE_DIR. Run build.sh first."
    exit 1
fi

# 5. Build ISO
xorriso -as mkisofs -b boot/limine-bios-cd.bin \
    -no-emul-boot -boot-load-size 4 -boot-info-table \
    -o "$ISO_FILE" "$ISO_DIR"

# 6. Install Limine MBRs
"$LIMINE_DIR/limine" bios-install "$ISO_FILE"

echo "=== ISO Created Successfully ==="
