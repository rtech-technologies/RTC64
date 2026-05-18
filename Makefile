CC = gcc
LD = ld

CFLAGS = -Wall -Wextra -std=c11 -ffreestanding -fno-stack-protector \
         -fno-stack-check -fno-lto -fno-pic -m64 -march=x86-64 -mcmodel=kernel \
         -I./external/limine \
         -I./external/CherryUSB/common \
         -I./external/CherryUSB/core \
         -I./external/CherryUSB/core/host \
         -I./external/FatFs/include \
         -I./external/TLSF \
         -I./external/lwip/src/include \
         -I./external/wolfssl \
         -I./include \
         -I./include/external \
         -DFS_FATFS_WINDOW_ALIGNMENT=4

LDFLAGS = -nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 -T boot/linker.ld

KERNEL_OBJS = kernel/unice64/limine_reqs.o \
              kernel/unice64/main.o \
              kernel/libs/services.o \
              kernel/libs/vga_serial.o \
              kernel/libs/bump_alloc.o \
              kernel/libs/arc_mem.o \
              kernel/libs/vdisk.o \
              kernel/libs/usb_xhci.o \
              kernel/libs/ahci.o \
              kernel/libs/vfs.o \
              kernel/libs/rsl_string.o \
              kernel/libs/console.o \
              kernel/libs/rsl_commands.o \
              kernel/libs/panic.o \
              kernel/libs/libc_fatfs.o \
              kernel/libs/math_stubs.o \
              external/FatFs/ff.o \
              external/TLSF/tlsf.o

.PHONY: all clean environment iso run

all: environment kernel.elf

environment:
	chmod +x build.sh
	./build.sh

kernel.elf: $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $(KERNEL_OBJS) -o kernel.elf

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

iso: kernel.elf
	mkdir -p iso_root/boot/sys
	cp kernel.elf iso_root/boot/sys/kernel.elf
	echo "/R-TECH OS" > iso_root/boot/limine.conf
	echo "PROTOCOL=limine" >> iso_root/boot/limine.conf
	echo "KERNEL_PATH=boot:///boot/sys/kernel.elf" >> iso_root/boot/limine.conf
	echo "COMMENT=Entering the Bare-Metal Estate." >> iso_root/boot/limine.conf
	cp external/limine/limine-bios.sys iso_root/boot/
	cp external/limine/limine-bios-cd.bin iso_root/boot/
	-xorriso -as mkisofs -b boot/limine-bios-cd.bin \
		-no-emul-boot -boot-load-size 4 -boot-info-table \
		--efi-boot boot/limine-bios-cd.bin \
		-efi-boot-part --efi-boot-image --protective-msdos-label \
		iso_root -o os.iso
	-./external/limine/limine bios-install os.iso

run: iso
	qemu-system-x86_64 -m 256M -cdrom os.iso -serial stdio

clean:
	rm -f $(KERNEL_OBJS) kernel.elf os.iso
