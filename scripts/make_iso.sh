#!/bin/bash
set -e
mkdir -p iso_root/boot/limine
cp kernel/kernel iso_root/boot/kernel.elf
cp kernel/limine.cfg iso_root/boot/limine.cfg
cp kernel/boot.png iso_root/boot.png
cp external/limine/limine-bios.sys iso_root/boot/limine/
cp external/limine/limine-bios-cd.bin iso_root/boot/limine/
xorriso -as mkisofs -b boot/limine/limine-bios-cd.bin -no-emul-boot -boot-load-size 4 -boot-info-table iso_root -o os.iso
./external/limine/limine bios-install os.iso
