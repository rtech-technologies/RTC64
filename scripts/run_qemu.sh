#!/bin/sh

set -e

echo "Preparing to run QEMU (developer-friendly)..."

# Developer-friendly defaults. By default we boot `kernel/kernel` directly
# so you can iterate without needing to build an ISO. To force ISO boot, set
# USE_ISO=1 in the environment.
QEMU_CMD="qemu-system-x86_64"
if ! command -v ${QEMU_CMD} >/dev/null 2>&1; then
    echo "qemu-system-x86_64 not found. Install QEMU to run the VM." >&2
    exit 1
fi

QEMU_ARGS="-m 2048M -smp 2 -serial stdio -device qemu-xhci -device usb-kbd -device usb-mouse -no-reboot"

# Enable KVM when available for faster iteration
if [ -e /dev/kvm ] && ${QEMU_CMD} --version >/dev/null 2>&1; then
    QEMU_ARGS="$QEMU_ARGS -enable-kvm"
fi

# Provide a GDB stub when DEBUG_GDB=1
if [ "${DEBUG_GDB}" = "1" ]; then
    echo "Enabling GDB stub on tcp:1234 (-s -S)"
    QEMU_ARGS="$QEMU_ARGS -S -gdb tcp::1234"
fi

if [ "${USE_ISO}" = "1" ]; then
    # Create ISO if possible and boot via Limine
    if command -v xorriso >/dev/null 2>&1; then
        echo "Creating ISO via limine..."
        make iso
        echo "Booting ISO..."
        echo "${QEMU_CMD} ${QEMU_ARGS} -cdrom os.iso $@"
        exec ${QEMU_CMD} ${QEMU_ARGS} -cdrom os.iso "$@"
    else
        echo "USE_ISO=1 requested but xorriso not found. Install xorriso or unset USE_ISO." >&2
        exit 1
    fi
else
    # Boot kernel directly for fast development iterations
    if [ ! -f kernel/kernel ]; then
        echo "kernel/kernel not found — run 'make' first" >&2
        exit 1
    fi
    echo "Booting kernel directly for development (fast iteration)"
    echo "${QEMU_CMD} ${QEMU_ARGS} -kernel kernel/kernel $@"
    exec ${QEMU_CMD} ${QEMU_ARGS} -kernel kernel/kernel "$@"
fi
