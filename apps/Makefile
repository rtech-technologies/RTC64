CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pic -m64 -march=x86-64 -I../include -I../kernel -DRSL_IMPLEMENTATION
LDFLAGS = -nostdlib -static -m elf_x86_64 -T app.ld

APPS = shell.bin

all: $(APPS)

shell.bin: crt0.o shell.o user_rsl.o
	ld $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.s
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

clean:
	rm -f *.o *.bin
