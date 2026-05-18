#!/bin/bash
set -e

echo "=== Baking Real Bare-Metal Environment (RTECH OSx2) ==="

mkdir -p external
mkdir -p include/external
mkdir -p iso_root/boot/sys

# 1. Get the Real Limine Binaries
if [ ! -d "external/limine" ]; then
    echo "[1/8] Fetching Static Limine Assets..."
    git clone https://github.com/limine-bootloader/limine.git external/limine --branch=v5.x-branch-binary --depth=1
    make -C external/limine
fi
cp external/limine/limine.h include/

# 2. Fetch the Real CherryUSB Stack
if [ ! -d "external/CherryUSB" ]; then
    echo "[2/8] Fetching Genuine CherryUSB Stack..."
    git clone https://github.com/cherry-embedded/CherryUSB.git external/CherryUSB --depth=1
    rm -rf external/CherryUSB/.git
fi

# 3. Fetch stb_sprintf.h
if [ ! -f "include/external/stb_sprintf.h" ]; then
    echo "[3/8] Fetching stb_sprintf..."
    curl -Lo include/external/stb_sprintf.h https://raw.githubusercontent.com/nothings/stb/master/stb_sprintf.h
fi

# 4. Fetch FatFs (Using stable Zephyr mirror)
if [ ! -d "external/FatFs" ]; then
    echo "[4/8] Fetching FatFs..."
    git clone https://github.com/zephyrproject-rtos/fatfs.git external/FatFs --depth=1
fi

# 5. Fetch Nuklear
if [ ! -f "include/nuklear.h" ]; then
    echo "[5/8] Fetching Nuklear..."
    curl -Lo include/nuklear.h https://raw.githubusercontent.com/Immediate-Mode-UI/Nuklear/master/nuklear.h
fi

# 6. Fetch TLSF
if [ ! -d "external/TLSF" ]; then
    echo "[6/8] Fetching TLSF..."
    git clone https://github.com/mattconte/tlsf.git external/TLSF --depth=1
fi

# 7. Fetch lwIP
if [ ! -d "external/lwip" ]; then
    echo "[7/8] Fetching lwIP..."
    git clone --depth 1 https://git.savannah.gnu.org/git/lwip.git external/lwip
fi

# 8. Fetch wolfSSL
if [ ! -d "external/wolfssl" ]; then
    echo "[8/8] Fetching wolfSSL..."
    git clone --depth 1 https://github.com/wolfSSL/wolfssl.git external/wolfssl
fi

# Fetch stb_truetype
if [ ! -f "include/external/stb_truetype.h" ]; then
    curl -Lo include/external/stb_truetype.h https://raw.githubusercontent.com/nothings/stb/master/stb_truetype.h
fi

echo "Environment Armed. Ready for 'make'."
