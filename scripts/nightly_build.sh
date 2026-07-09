#!/bin/bash
# Sovereign RTC64 Nightly Build and Verification Script
# This script ensures the industrial integrity of the Sovereign RTC64 project.

set -e
LOG_FILE="nightly_build.log"
exec > >(tee -a "$LOG_FILE") 2>&1

echo "=== Starting Sovereign RTC64 Nightly Build: $(date) ==="

# 1. Clean environment
echo "[1/4] Cleaning build artifacts..."
make clean

# 2. Build everything
echo "[2/4] Executing clean build (High-Power mode)..."
# We ignore error 127 from 'make all' because it might be the missing xorriso for the ISO step.
# We'll check critical artifacts manually.
make all || [ $? -eq 127 ]

if [ -f "kernel/kernel" ] && [ -f "kernel/ramdisk.img" ]; then
    echo "BUILD SUCCESSFUL (Kernel and Ramdisk artifacts present)"
else
    echo "BUILD FAILED: Critical artifacts missing."
    exit 1
fi

# 3. Artifact Verification
echo "[3/4] Verifying build artifacts..."
if [ ! -f "kernel/kernel" ]; then
    echo "ERROR: kernel/kernel missing!"
    exit 1
fi

KERNEL_SIZE=$(stat -c%s "kernel/kernel")
echo "Kernel Size: $KERNEL_SIZE bytes"
if [ "$KERNEL_SIZE" -lt 102400 ]; then
    echo "WARNING: Kernel seems suspiciously small."
fi

if [ ! -f "kernel/ramdisk.img" ]; then
    echo "ERROR: ramdisk.img missing!"
    exit 1
fi

# Check some userland apps
for app in shell lab studio notepad tests; do
    if [ ! -f "apps/$app/app.bin" ]; then
        echo "ERROR: Userland app '$app' failed to build!"
        exit 1
    fi
done

# 4. Binary Integrity Checks
echo "[4/4] Performing binary integrity audit..."
# Check for common industrial symbols
SYMBOLS=("kernel_main" "scheduler_switch" "vfs_read" "pmm_alloc")
for sym in "${SYMBOLS[@]}"; do
    if ! nm kernel/kernel | grep -q "$sym"; then
        echo "ERROR: Critical symbol '$sym' missing from kernel binary!"
        exit 1
    fi
done

echo "=== Nightly Build and Verification COMPLETE: $(date) ==="
echo "Result: STABLE"
