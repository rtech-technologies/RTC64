#!/bin/bash
set -e

echo "=== Building R-TECH OS Skeleton ==="

echo "[1/2] Building Hosted SDL2 Environment..."
mkdir -p build
cd build
cmake ..
make
cd ..

echo "[2/2] Building Freestanding x86_64 Kernel..."
make -C kernel

echo "=== Build Complete! ==="
echo "Hosted Binary: build/nuklear_cherry_usb"
echo "Kernel Binary: kernel/kernel"
