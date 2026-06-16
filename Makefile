CC = gcc
LD = ld

CFLAGS = -Wall -Wextra -Werror -std=c11 -ffreestanding -fno-stack-protector \
         -fno-stack-check -fno-lto -fno-pic -m64 -march=x86-64 -mcmodel=kernel \
         -mno-red-zone -fno-asynchronous-unwind-tables \
         -I./include -I./kernel -I./kernel/drivers -I./kernel/fatfs \
         -I./external/limine \
         -I./external/CherryUSB/common \
         -I./external/CherryUSB/core \
         -I./external/CherryUSB/class/msc \
         -I./external/CherryUSB/class/hid \
         -I./external/CherryUSB/class/hub \
         -include kernel/usb_config.h -DKERNEL_MODE

LDFLAGS = -nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 -T kernel/linker.ld

# All Source Objects
KERNEL_OBJS = kernel/kernel.o src/app_ui.o src/chell.o src/lab.o src/installer.o \
              kernel/nuklear_kernel_impl.o kernel/stb_image_impl.o \
              src/nk_software_renderer.o kernel/syscall.o kernel/sys_shell.o \
              kernel/usb_osal.o \
              kernel/usb_hal_ports.o kernel/storage.o kernel/input.o \
              kernel/usb_hal.o kernel/vfs.o kernel/scheduler.o \
              kernel/serial.o kernel/i18n.o kernel/uac_policy.o kernel/tgx_impl.o \
              kernel/tlsf_impl.o kernel/math.o kernel/panic.o \
              kernel/gdt.o kernel/interrupts.o kernel/isr_stubs.o \
              kernel/apic.o kernel/pmm.o \
              kernel/comprec.o kernel/cm.o \
              kernel/malloc_glue.o kernel/storage_hal.o kernel/panic_hal.o \
              kernel/diskio_impl.o kernel/ffsystem_impl.o \
              kernel/fatfs/ff.o kernel/fatfs/ffunicode.o \
              kernel/drivers/pci.o kernel/drivers/xhci.o kernel/drivers/ehci.o \
              kernel/drivers/nvme.o kernel/drivers/ahci.o kernel/drivers/ramdisk.o \
              kernel/drivers/rtc.o \
              external/CherryUSB/core/usbd_core.o \
              external/CherryUSB/core/usbh_core.o \
              external/CherryUSB/class/msc/usbh_msc.o \
              external/CherryUSB/class/hid/usbh_hid.o \
              external/CherryUSB/class/hub/usbh_hub.o \
              external/CherryUSB/port/ehci/usb_hc_ehci.o

.PHONY: all clean environment iso run

all: environment kernel/kernel iso

environment:
	chmod +x build.sh
	./build.sh

kernel/kernel: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $(KERNEL_OBJS) -o kernel/kernel

$(KERNEL_OBJS): environment

%.o: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

iso: kernel/kernel
	mkdir -p iso_root/boot/sys
	cp kernel/kernel iso_root/boot/sys/kernel.elf
	# Modern and Legacy config support at root and /boot/
	cp kernel/limine.cfg iso_root/limine.conf
	cp kernel/limine.cfg iso_root/limine.cfg
	cp kernel/limine.cfg iso_root/boot/limine.conf
	cp kernel/limine.cfg iso_root/boot/limine.cfg
	cp external/limine/limine-bios.sys iso_root/boot/
	cp external/limine/limine-bios-cd.bin iso_root/boot/
	xorriso -as mkisofs -b boot/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		iso_root -o os.iso
	./external/limine/limine bios-install os.iso

QEMU = qemu-system-x86_64
QEMU_FLAGS = -m 512M -cdrom os.iso -boot d -device qemu-xhci -device usb-kbd -device usb-mouse -serial stdio

run: all
	$(QEMU) $(QEMU_FLAGS) $(EXTRA_QEMU_FLAGS)

clean:
	rm -rf $(KERNEL_OBJS) kernel/kernel os.iso iso_root/limine.conf iso_root/limine.cfg iso_root/boot/sys/kernel.elf iso_root/boot/limine.conf iso_root/boot/limine.cfg iso_root/boot/limine-bios.sys iso_root/boot/limine-bios-cd.bin
