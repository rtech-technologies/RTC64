#!/bin/bash
set -e

echo "=== R-TECH™ OS Setup & Build ==="

# 1. Setup Dependencies
mkdir -p external

# Limine: Use binary branch for pre-compiled bootloader assets
if [ ! -d "external/limine" ]; then
    echo "[1/4] Fetching Limine Binary Assets..."
    git clone https://github.com/limine-bootloader/limine.git external/limine --branch=v5.x-branch-binary --depth=1

    # Build the host-side deployment tool
    echo "Building Limine deployment tool..."
    make -C external/limine
fi

# 2. Build Hosted SDL2 Environment
echo "[2/4] Building Hosted SDL2 Environment..."
mkdir -p build_sdl
cd build_sdl
cmake ..
make
cd ..

# 3. Build Freestanding x86_64 Kernel
echo "[3/4] Building Freestanding x86_64 Kernel..."
make -C kernel clean
make -C kernel

echo "=== Build Complete! ==="
echo "Hosted Binary: build_sdl/nuklear_cherry_usb"
echo "Kernel Binary: kernel/kernel"
