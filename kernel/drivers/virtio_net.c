/* Minimal virtio-net skeleton driver (scaffold)
 * Registers and logs virtio-net devices detected by PCI scan.
 * Real virtio implementation to be added iteratively.
 */
#include <stdint.h>
#include <stddef.h>
#include "pro_os.h"
#include "hal.h"
#include "serial.h"

void virtio_net_init(uint8_t bus, uint8_t slot, uint8_t func) {
    serial_printf("[VIRTIO] Initializing virtio-net at %02x:%02x.%x\n", bus, slot, func);

    /* Probe common config registers via PCI config space */
    uint32_t vendor_device = pci_read_config(bus, slot, func, 0);
    uint16_t vendor = vendor_device & 0xFFFF;
    uint16_t device = (vendor_device >> 16) & 0xFFFF;
    serial_printf("[VIRTIO] PCI VENDOR=%04x DEVICE=%04x\n", vendor, device);

    /* Read BAR0 (legacy I/O/MMIO for virtio devices) */
    uint64_t bar0 = pci_get_bar(bus, slot, func, 0);
    serial_printf("[VIRTIO] BAR0 = %p\n", (void*)bar0);

    /* TODO: map MMIO region or set up IO access, negotiate features, and wire into network stack */

    /* Temporary: announce the device presence to higher-level services (if any) */
    return;
}
