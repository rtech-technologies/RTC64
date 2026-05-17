#!/bin/bash
set -e

echo "=== R-TECH OS Setup & Build ==="

# 1. Setup Dependencies
mkdir -p external
if [ ! -d "external/CherryUSB" ]; then
    echo "[1/4] Cloning CherryUSB..."
    git clone https://github.com/cherry-embedded/CherryUSB.git external/CherryUSB
fi

if [ ! -d "external/limine" ]; then
    echo "[2/4] Cloning Limine..."
    git clone https://github.com/limine-bootloader/limine.git external/limine --depth=1
    cd external/limine
    ./bootstrap
    ./configure --enable-bios --enable-bios-cd --enable-uefi-x86_64 --enable-uefi-ia32 LD_FOR_TARGET=ld OBJCOPY_FOR_TARGET=objcopy OBJDUMP_FOR_TARGET=objdump READELF_FOR_TARGET=readelf
    make
    cp bin/* .
    cd ../..
fi

# Link for kernel build
rm -f kernel/cherryusb
ln -sf ../external/CherryUSB kernel/cherryusb

# 2. Build Hosted SDL2 Environment
echo "[3/4] Building Hosted SDL2 Environment..."
mkdir -p build_sdl
cd build_sdl
cmake ..
make
cd ..

# 3. Build Freestanding x86_64 Kernel
echo "[4/4] Building Freestanding x86_64 Kernel..."
make -C kernel clean
make -C kernel

echo "=== Build Complete! ==="
echo "Hosted Binary: build_sdl/nuklear_cherry_usb"
echo "Kernel Binary: kernel/kernel"
