CC = gcc
LD = ld

CFLAGS = -Wall -Wextra -std=c11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pic -m64 -march=x86-64 -mcmodel=kernel -mno-red-zone -fno-asynchronous-unwind-tables \
         -I../include -I. -Icherryusb/common -Icherryusb/core -Icherryusb/class/hub \
         -Icherryusb/class/msc -Icherryusb/class/hid -Icherryusb/port/ehci \
         -include usb_config.h

LDFLAGS = -nostdlib -static -no-pie -z max-page-size=0x1000 -T linker.ld

OBJS = kernel.o app_ui.o nuklear_kernel_impl.o nk_software_renderer.o \
       usb_osal.o usbd_core.o usbh_core.o usb_hal_ports.o \
       storage.o input.o usb_hal.o \
       usbh_msc.o usbh_hid.o usbh_hub.o usb_hc_ehci.o \
       vfs.o scheduler.o i18n.o uac_policy.o tgx_impl.o tlsf_impl.o \
       math.o panic.o malloc_glue.o \
       drivers/pci.o drivers/xhci.o drivers/ehci.o drivers/nvme.o drivers/ahci.o

.PHONY: all clean bin

all: kernel

bin: $(OBJS)
	ar rcs rtech_gui.a $(OBJS)

kernel: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) -o kernel

math.o: math.c
	$(CC) $(CFLAGS) -c math.c -o math.o

# Core Kernel Components
kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

app_ui.o: ../src/app_ui.c
	$(CC) $(CFLAGS) -c ../src/app_ui.c -o app_ui.o

nuklear_kernel_impl.o: nuklear_kernel_impl.c
	$(CC) $(CFLAGS) -c nuklear_kernel_impl.c -o nuklear_kernel_impl.o

nk_software_renderer.o: ../src/nk_software_renderer.c
	$(CC) $(CFLAGS) -c ../src/nk_software_renderer.c -o nk_software_renderer.o

# Pro Logic Components
vfs.o: vfs.c
	$(CC) $(CFLAGS) -c vfs.c -o vfs.o

scheduler.o: scheduler.c
	$(CC) $(CFLAGS) -c scheduler.c -o scheduler.o

i18n.o: i18n.c
	$(CC) $(CFLAGS) -c i18n.c -o i18n.o

uac_policy.o: uac_policy.c
	$(CC) $(CFLAGS) -c uac_policy.c -o uac_policy.o

tgx_impl.o: tgx_impl.c
	$(CC) $(CFLAGS) -c tgx_impl.c -o tgx_impl.o

tlsf_impl.o: tlsf_impl.c
	$(CC) $(CFLAGS) -c tlsf_impl.c -o tlsf_impl.o

malloc_glue.o: malloc_glue.c
	$(CC) $(CFLAGS) -c malloc_glue.c -o malloc_glue.o

# Hardware Drivers
drivers/pci.o: drivers/pci.c
	$(CC) $(CFLAGS) -c drivers/pci.c -o drivers/pci.o

drivers/xhci.o: drivers/xhci.c
	$(CC) $(CFLAGS) -c drivers/xhci.c -o drivers/xhci.o

drivers/ehci.o: drivers/ehci.c
	$(CC) $(CFLAGS) -c drivers/ehci.c -o drivers/ehci.o

drivers/nvme.o: drivers/nvme.c
	$(CC) $(CFLAGS) -c drivers/nvme.c -o drivers/nvme.o

drivers/ahci.o: drivers/ahci.c
	$(CC) $(CFLAGS) -c drivers/ahci.c -o drivers/ahci.o

# HAL Components
storage.o: storage.c
	$(CC) $(CFLAGS) -c storage.c -o storage.o

input.o: input.c
	$(CC) $(CFLAGS) -c input.c -o input.o

usb_hal.o: usb_hal.c
	$(CC) $(CFLAGS) -c usb_hal.c -o usb_hal.o

# CherryUSB Stack
usb_osal.o: usb_osal.c
	$(CC) $(CFLAGS) -c usb_osal.c -o usb_osal.o

usbd_core.o: cherryusb/core/usbd_core.c
	$(CC) $(CFLAGS) -c cherryusb/core/usbd_core.c -o usbd_core.o

usbh_core.o: cherryusb/core/usbh_core.c
	$(CC) $(CFLAGS) -c cherryusb/core/usbh_core.c -o usbh_core.o

usb_hal_ports.o: usb_hal_ports.c
	$(CC) $(CFLAGS) -c usb_hal_ports.c -o usb_hal_ports.o

# CherryUSB Classes
usbh_msc.o: cherryusb/class/msc/usbh_msc.c
	$(CC) $(CFLAGS) -c cherryusb/class/msc/usbh_msc.c -o usbh_msc.o

usbh_hid.o: cherryusb/class/hid/usbh_hid.c
	$(CC) $(CFLAGS) -c cherryusb/class/hid/usbh_hid.c -o usbh_hid.o

usbh_hub.o: cherryusb/class/hub/usbh_hub.c
	$(CC) $(CFLAGS) -c cherryusb/class/hub/usbh_hub.c -o usbh_hub.o

# CherryUSB Port (EHCI for QEMU)
usb_hc_ehci.o: cherryusb/port/ehci/usb_hc_ehci.c
	$(CC) $(CFLAGS) -c cherryusb/port/ehci/usb_hc_ehci.c -o usb_hc_ehci.o

clean:
	rm -f kernel $(OBJS) rtech_gui.a
