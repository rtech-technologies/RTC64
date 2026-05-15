#!/bin/bash
set -e

echo "=== R-TECH OS Setup & Build ==="

SETUP_ONLY=false
if [ "$1" == "--setup-only" ]; then
    SETUP_ONLY=true
fi

# Dependency Installation
if [ -f /etc/debian_version ]; then
    echo "[0/4] Installing System Dependencies (Requires sudo)..."
    sudo apt-get update
    sudo apt-get install -y build-essential cmake libsdl2-dev git nasm xorriso qemu-system-x86
fi

# Limine Setup
if [ ! -d "external/limine" ]; then
    echo "[1/4] Cloning & Building Limine Bootloader..."
    mkdir -p external
    git clone https://github.com/limine-bootloader/limine.git external/limine --branch=v7.x-binary --depth=1
    # Build limine-deploy if it doesn't exist
    make -C external/limine
    cp external/limine/limine.h kernel/limine.h
fi

# CherryUSB Setup
if [ ! -d "external/CherryUSB" ]; then
    echo "[2/4] Cloning CherryUSB..."
    git clone https://github.com/cherry-embedded/CherryUSB.git external/CherryUSB --depth=1
fi

if [ "$SETUP_ONLY" = true ]; then
    echo "Setup Complete."
    exit 0
fi

echo "[3/4] Building Hosted SDL2 Environment..."
mkdir -p build
cd build
cmake ..
make
cd ..

echo "[4/4] Building Freestanding x86_64 Kernel..."
make -C kernel

echo "=== Build Complete! ==="
echo "Hosted Binary: build/nuklear_cherry_usb"
echo "Kernel Binary: kernel/kernel"
