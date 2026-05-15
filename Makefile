.PHONY: all clean hosted kernel setup iso run bin

all: setup hosted kernel iso

bin: setup
	mkdir -p dist
	$(MAKE) -C kernel bin
	cp kernel/rtech_gui.a dist/
	@echo "Standalone binary created at dist/rtech_gui.a"

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

run: setup all
	qemu-system-x86_64 -cdrom os.iso -m 512M -M q35 -device qemu-xhci -device usb-kbd -device usb-tablet

clean:
	rm -rf build iso_root os.iso dist
	$(MAKE) -C kernel clean
