#!/bin/bash
set -e

echo "=== Baking Real Bare-Metal Environment ==="

mkdir -p external
mkdir -p iso_root/boot/sys

# 1. Get the Real Limine Binaries (Hardened acquisition)
if [ ! -d "external/limine" ]; then
    echo "[1/2] Fetching Static Limine Assets..."
    git clone https://github.com/limine-bootloader/limine.git external/limine --branch=v8.x-binary --depth=1
    echo "Building Limine deployment tool..."
    make -C external/limine limine
fi

# 2. Fetch the Real CherryUSB Source Tree
if [ ! -d "external/CherryUSB" ]; then
    echo "[2/2] Fetching Genuine CherryUSB Stack..."
    git clone https://github.com/cherry-embedded/CherryUSB.git external/CherryUSB --depth=1
    # Remove .git to ensure tracking by parent repo if needed,
    # but the directive was "include real source files".
    rm -rf external/CherryUSB/.git
fi

echo "Environment Armed. Ready for 'make'."
