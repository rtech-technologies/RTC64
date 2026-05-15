.PHONY: all clean hosted kernel setup

all: setup hosted kernel

setup:
	chmod +x build.sh
	./build.sh --setup-only || true

hosted:
	mkdir -p build
	cd build && cmake .. && make

kernel:
	$(MAKE) -C kernel

clean:
	rm -rf build
	$(MAKE) -C kernel clean
