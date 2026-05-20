#!/bin/bash
# Generate a blank 64MB disk for RTC64 Sovereign testing
dd if=/dev/zero of=hdd.img bs=1M count=64
echo "Generated blank 64MB disk: hdd.img"
echo "To use with QEMU, add: -drive file=hdd.img,format=raw,if=none,id=dr0 -device nvme,drive=dr0,serial=1234"
