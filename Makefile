.PHONY: all clean hosted kernel

all: hosted kernel

hosted:
	mkdir -p build
	cd build && cmake .. && make

kernel:
	$(MAKE) -C kernel

clean:
	rm -rf build
	$(MAKE) -C kernel clean
