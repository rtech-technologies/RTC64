# RTC64 Full-System Structural Audit & Risk Matrix

## System Risk Matrix

| Interaction | Risk Level | Description | Mitigation Strategy |
| :--- | :---: | :--- | :--- |
| **FatFs <-> HAL Storage** | High | `diskio_impl.c` passes raw `storage_device_t` pointers. Invalid `pdrv` indices in `disk_read/write` can lead to NULL dereference or out-of-bounds access if HAL state isn't synchronized. | Implement boundary checks in `hal_storage_get_device` and ensure `pdrv` mapping is immutable after registration. |
| **USB ISR <-> VFS Refresh** | Medium | `usbh_msc_run` triggers `vfs_refresh_mounts`. If an interrupt occurs during a VFS traversal in the UI, the mount list could be corrupted. | Use a 'dirty' flag for hotplug events. Defer VFS list reconstruction to the start of the main executive loop, outside the render window. |
| **Ring 0 / UI Context** | Low | Currently, the UI runs in Ring 0 with full kernel privileges. A malicious UI element or bug could overwrite the GDT or Page Tables. | (Future) Implement Ring 3 separation. Current mitigation: Use `const` pointers for static descriptor tables and enable WP (Write Protect) bit in CR0. |
| **FatFs Resource Leaks** | Medium | `f_mount` is called with `opt=1`. If a USB device is yanked, `usbh_msc_stop` does not currently call `f_unmount`, leaving orphaned FatFs objects in the VFS mount table. | Update `usbh_msc_stop` to explicitly signal VFS to unmount and free the `FATFS` structure associated with the device index. |
| **Init Sequence Race** | High | GUI initialization depends on the font engine and heap. If `nk_init` is called before `hal_malloc_init`, the system will crash immediately. | Enforce a strict dependency-ordered boot chain (see below). |

## Hardened Unified Boot Sequence Strategy

To ensure system stability, the following sequence must be strictly followed in `kernel_main`:

1. **Phase 1: Processor Prep**
   - SSE Enable (CR0/CR4)
   - GDT Load & Segment Reload (Ensures stable stack/code base)
2. **Phase 2: Memory Sovereignty**
   - `hal_malloc_init` (Required for all subsequent dynamic allocations)
3. **Phase 3: Hardware Discovery**
   - `hal_storage_init` / `hal_input_init` (Clean registries)
   - `pci_scan` (Identify MMIO bases for XHCI/EHCI/NVMe)
4. **Phase 4: Logical Services**
   - `vfs_init` (Empty mount tables)
   - `scheduler_init` (Ready for tasking)
5. **Phase 5: Peripheral Activation**
   - `hal_usb_init` (Starts CherryUSB Host Stack, triggers enumeration)
6. **Phase 6: UI Subsystem**
   - Font Baking & Nuklear Context Init
   - Style Application
7. **Phase 7: Executive Loop**
   - Event Polling -> Logic Update -> UI Render -> FB Flush
