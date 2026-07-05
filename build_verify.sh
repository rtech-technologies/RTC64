#!/bin/bash
echo "=== RTC64 Industrial Build Verification ==="
make clean > /dev/null
make all 2>&1 | tee build_dry.log
if [ -f kernel/kernel ]; then echo "  OK: kernel/kernel exists"; else echo "  FAIL: kernel/kernel missing"; fi
if [ -f kernel/ramdisk.img ]; then echo "  OK: kernel/ramdisk.img exists"; else echo "  FAIL: kernel/ramdisk.img missing"; fi
ls -R iso_root/boot/sys/
echo "=== Verification Complete ==="
