# Nuklear CherryUSB OS Skeleton

This project provides a GUI skeleton using the Nuklear immediate mode library, integrated with the CherryUSB stack, and prepared for a kernel environment using the Limine bootloader.

## Project Structure

- `src/`: Hosted application source code (SDL2/OpenGL3).
- `kernel/`: Basic kernel skeleton and Limine bootloader configuration.
- `include/`: Shared headers, including the service plugin architecture.
- `external/CherryUSB/`: The CherryUSB stack.

## Service Plugin Architecture

The system uses a `service_table_t` (defined in `include/services.h`) to link hardware or OS services (like Storage, Clock, etc.) to the GUI.

If a service is not provided (NULL pointer in the table), the corresponding GUI elements are automatically grayed out and disabled.

## How to Build and Test

### Single-Command Build (All Targets)

To build both the hosted SDL2 application and the freestanding kernel at once:
```bash
make
```
or
```bash
chmod +x build.sh
./build.sh
```

### Hosted Application (Linux/macOS/Windows)

To test the GUI on your OS:

1. **Requirements**: SDL2, OpenGL, CMake, GCC/Clang.
2. **Build**:
   ```bash
   make hosted
   ```
3. **Run**:
   ```bash
   ./build/nuklear_cherry_usb
   ```

### Kernel (x86_64)

To build the skeleton kernel:

1. **Requirements**: GCC (x86_64), GNU LD.
2. **Build**:
   ```bash
   make kernel
   ```
3. **Testing in QEMU**:
   To test the kernel, you would typically create an ISO image using `limine` and `xorriso`, then run:
   ```bash
   qemu-system-x86_64 -cdrom image.iso
   ```
   *Note: A full ISO creation script requires the `limine` binary and `xorriso` which are not included in this minimal skeleton.*

## Plugging in Features

To add a feature (e.g., a real storage driver):

1. Implement the `service_t` interface.
2. Assign your implementation to the `g_services` table.
3. The GUI will automatically enable the relevant buttons.
