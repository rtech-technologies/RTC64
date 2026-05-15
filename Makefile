.PHONY: all clean hosted kernel setup iso run

all: setup hosted kernel iso

setup:
	chmod +x build.sh
	./build.sh --setup-only || true

hosted:
	mkdir -p build
	cd build && cmake .. && make

kernel:
	$(MAKE) -C kernel

iso: kernel
	chmod +x scripts/make_iso.sh
	./scripts/make_iso.sh

run: iso
	qemu-system-x86_64 -cdrom os.iso -m 512M -M q35 -device qemu-xhci -device usb-kbd -device usb-tablet

clean:
	rm -rf build iso_root os.iso
	$(MAKE) -C kernel clean
