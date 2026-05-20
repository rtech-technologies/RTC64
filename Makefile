CC = gcc
LD = ld

CFLAGS = -Wall -Wextra -std=c11 -ffreestanding -fno-stack-protector \
         -fno-stack-check -fno-lto -fno-pic -m64 -march=x86-64 -mcmodel=kernel \
         -mno-red-zone -fno-asynchronous-unwind-tables \
         -I./include -I./kernel -I./kernel/drivers \
         -I./kernel/cherryusb/common \
         -I./kernel/cherryusb/core \
         -I./kernel/cherryusb/class/msc \
         -I./kernel/cherryusb/class/hid \
         -I./kernel/cherryusb/class/hub \
         -I./kernel/fatfs \
         -include kernel/usb_config.h -DKERNEL_MODE

LDFLAGS = -nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 -T kernel/linker.ld

# All Source Objects
KERNEL_OBJS = kernel/kernel.o src/app_ui.o src/chell.o src/lab.o src/installer.o \
              kernel/nuklear_kernel_impl.o src/nk_software_renderer.o \
              kernel/usb_osal.o kernel/usb_hal_ports.o kernel/storage.o \
              kernel/input.o kernel/usb_hal.o kernel/vfs.o \
              kernel/scheduler.o kernel/i18n.o kernel/uac_policy.o \
              kernel/tgx_impl.o kernel/tlsf_impl.o kernel/math.o \
              kernel/panic.o kernel/malloc_glue.o kernel/storage_hal.o \
              kernel/panic_hal.o kernel/diskio_impl.o kernel/ffsystem_impl.o \
              kernel/ff_partitions.o kernel/fatfs/ff.o kernel/fatfs/ffunicode.o \
              kernel/drivers/pci.o kernel/drivers/xhci.o kernel/drivers/ehci.o \
              kernel/drivers/nvme.o kernel/drivers/ahci.o \
              kernel/cherryusb/core/usbd_core.o \
              kernel/cherryusb/core/usbh_core.o \
              kernel/cherryusb/class/msc/usbh_msc.o \
              kernel/cherryusb/class/hid/usbh_hid.o \
              kernel/cherryusb/class/hub/usbh_hub.o \
              kernel/cherryusb/port/ehci/usb_hc_ehci.o

.PHONY: all clean iso run environment

all: environment hdd.img kernel/kernel iso

environment:
	chmod +x build.sh
	./build.sh

hdd.img:
	chmod +x scripts/gen_disk.sh
	./scripts/gen_disk.sh

kernel/kernel: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $(KERNEL_OBJS) -o kernel/kernel

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

iso: kernel/kernel
	chmod +x scripts/make_iso.sh
	./scripts/make_iso.sh

QEMU = qemu-system-x86_64
QEMU_FLAGS = -m 512M -cdrom os.iso -boot d -device qemu-xhci -device usb-kbd -device usb-mouse -serial stdio

run: all
	$(QEMU) $(QEMU_FLAGS) $(EXTRA_QEMU_FLAGS) -drive file=hdd.img,format=raw,if=none,id=dr0 -device nvme,drive=dr0,serial=1234

clean:
	rm -rf $(KERNEL_OBJS) kernel/kernel os.iso hdd.img iso_root/
