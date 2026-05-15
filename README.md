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

### 🚀 One-Command Demo
To automatically install dependencies, build the OS, and launch it in QEMU:
```bash
make run
```

### 📦 Standalone Binary for Other OSs
To generate a statically linked library (`rtech_gui.a`) that you can link into **your own operating system**:
```bash
make bin
```
The result will be at `dist/rtech_gui.a`. You can then link this into your project and call `ui_render` and `nk_sw_render`.

### 🖥️ Hosted Development (Linux/macOS)
To test the GUI quickly on your current OS using SDL2:
1. **Build**: `make hosted`
2. **Run**: `./build/nuklear_cherry_usb`

### 🛠️ Kernel Development
To build only the freestanding x86_64 kernel:
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
