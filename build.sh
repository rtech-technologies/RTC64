#!/bin/bash
set -e

echo "=== R-TECH™ OS Setup & Build ==="

# 1. Setup Dependencies
mkdir -p external

# Limine: Use a static Point Release Tarball
LIMINE_VERSION="12.2.0"
if [ ! -f "external/limine/limine" ]; then
    echo "[1/4] Fetching Limine Release ${LIMINE_VERSION}..."
    mkdir -p external/limine
    curl -L "https://github.com/Limine-Bootloader/Limine/releases/download/v${LIMINE_VERSION}/limine-binary.tar.gz" -o external/limine.tar.gz
    tar -xzf external/limine.tar.gz -C external/limine --strip-components=1
    rm external/limine.tar.gz

    # Compile the deploy tool from the binary release source
    gcc external/limine/limine.c -o external/limine/limine
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
