#!/bin/bash
set -e

echo "=== Preparing Sovereign OS Boot Environment ==="

mkdir -p external
mkdir -p iso_root/boot

# 1. Fetch Limine Binaries for ISO creation
if [ ! -d "external/limine" ]; then
    echo "[1/1] Fetching Limine Bootloader Assets..."
    git clone https://github.com/limine-bootloader/limine.git external/limine --branch=v5.x-branch-binary --depth=1
    echo "Building Limine tool..."
    make -C external/limine limine
fi

echo "Environment Ready."
