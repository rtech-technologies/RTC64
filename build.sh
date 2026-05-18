#!/bin/bash
set -e

echo "=== Baking Real Bare-Metal Environment (RTECH OSx2) ==="

mkdir -p external
mkdir -p include/external
mkdir -p iso_root/boot/sys

# 1. Get the Real Limine Binaries
if [ ! -d "external/limine" ]; then
    echo "[1/7] Fetching Static Limine Assets..."
    git clone https://github.com/limine-bootloader/limine.git external/limine --branch=v5.x-branch-binary --depth=1
    make -C external/limine
fi
cp external/limine/limine.h include/

# 2. Fetch the Real CherryUSB Stack
if [ ! -d "external/CherryUSB" ]; then
    echo "[2/7] Fetching Genuine CherryUSB Stack..."
    git clone https://github.com/cherry-embedded/CherryUSB.git external/CherryUSB --depth=1
    rm -rf external/CherryUSB/.git
fi

# 3. Fetch stb_sprintf.h
if [ ! -f "include/external/stb_sprintf.h" ]; then
    echo "[3/7] Fetching stb_sprintf..."
    curl -L -o include/external/stb_sprintf.h https://raw.githubusercontent.com/nothings/stb/master/stb_sprintf.h
fi

# 4. Fetch FatFs (Using stm32duino mirror)
if [ ! -d "external/fatfs" ]; then
    echo "[4/7] Fetching FatFs..."
    git clone https://github.com/stm32duino/FatFs.git external/fatfs --depth=1
fi

# 5. Fetch qoi.h
if [ ! -f "include/external/qoi.h" ]; then
    echo "[5/7] Fetching qoi.h..."
    curl -L -o include/external/qoi.h https://raw.githubusercontent.com/phoboslab/qoi/master/qoi.h
fi

# 6. Fetch lwIP
if [ ! -d "external/lwip" ]; then
    echo "[6/7] Fetching lwIP..."
    git clone --depth 1 https://git.savannah.gnu.org/git/lwip.git external/lwip
fi

# 7. Fetch wolfSSL
if [ ! -d "external/wolfssl" ]; then
    echo "[7/7] Fetching wolfSSL..."
    git clone --depth 1 https://github.com/wolfSSL/wolfssl.git external/wolfssl
fi

echo "Environment Armed. Ready for 'make'."
