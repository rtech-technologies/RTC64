CC = gcc
LD = ld

CFLAGS = -Wall -Wextra -std=c11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fPIE -m64 -march=x86-64 \
         -I../include -I. -I../external/CherryUSB/common -I../external/CherryUSB/core -I../external/CherryUSB/class/hub

LDFLAGS = -nostdlib -static -m elf_x86_64 -z max-page-size=0x1000 -T linker.ld

OBJS = kernel.o app_ui.o nuklear_kernel_impl.o nk_software_renderer.o \
       usb_osal.o usbd_core.o usbh_core.o usb_stubs.o

.PHONY: all clean

all: kernel

kernel: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o kernel

bin: $(OBJS)
	ar rcs rtech_gui.a $(OBJS)

usb_stubs.o: usb_stubs.c
	$(CC) $(CFLAGS) -c usb_stubs.c -o usb_stubs.o

usb_osal.o: usb_osal.c
	$(CC) $(CFLAGS) -c usb_osal.c -o usb_osal.o

usbd_core.o: ../external/CherryUSB/core/usbd_core.c
	$(CC) $(CFLAGS) -c ../external/CherryUSB/core/usbd_core.c -o usbd_core.o

usbh_core.o: ../external/CherryUSB/core/usbh_core.c
	$(CC) $(CFLAGS) -c ../external/CherryUSB/core/usbh_core.c -o usbh_core.o

kernel.o: kernel.c
	$(CC) $(CFLAGS) -c kernel.c -o kernel.o

app_ui.o: ../src/app_ui.c
	$(CC) $(CFLAGS) -c ../src/app_ui.c -o app_ui.o

nuklear_kernel_impl.o: nuklear_kernel_impl.c
	$(CC) $(CFLAGS) -c nuklear_kernel_impl.c -o nuklear_kernel_impl.o

nk_software_renderer.o: ../src/nk_software_renderer.c
	$(CC) $(CFLAGS) -c ../src/nk_software_renderer.c -o nk_software_renderer.o

clean:
	rm -f kernel $(OBJS) rtech_gui.a
