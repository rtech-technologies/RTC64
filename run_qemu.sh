#!/bin/bash

# This script provides instructions and a command template to run the kernel in QEMU.
# It assumes you have 'xorriso' and 'mtools' installed to create an ISO,
# and 'limine' binaries available.

echo "To boot the kernel, you need to create a bootable ISO image."
echo "1. Download Limine binaries (limine-bios.sys, limine-cd.bin, limine-cd-efi.bin)."
echo "2. Create a folder structure:"
echo "   iso_root/"
echo "   ├── boot/"
echo "   │   └── kernel (from kernel/kernel)"
echo "   └── limine.conf (from kernel/limine.conf)"
echo "3. Copy Limine binaries to iso_root/."
echo "4. Run xorriso to create the ISO:"
echo "   xorriso -as mkisofs -b limine-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table iso_root -o image.iso"
echo "5. Install Limine to the ISO:"
echo "   ./limine bios-install image.iso"
echo ""
echo "Once you have 'image.iso', run it with QEMU:"
echo "qemu-system-x86_64 -cdrom image.iso -m 512M"

# If the environment had QEMU and the ISO was ready, we would run:
# qemu-system-x86_64 -kernel kernel/kernel -m 512M
