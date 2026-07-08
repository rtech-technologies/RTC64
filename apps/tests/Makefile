CC = gcc
LD = ld
LIB_DIR = ../lib
CFLAGS = -Wall -Wextra -Werror -std=c11 -ffreestanding -fno-stack-protector -fno-stack-check -fno-lto -fno-pic -m64 -march=x86-64 -I../../include -I../../kernel -I$(LIB_DIR)
LDFLAGS = -nostdlib -static -m elf_x86_64 -T $(LIB_DIR)/app.ld

OBJS = app.o $(LIB_DIR)/crt0.o $(LIB_DIR)/user_rsl.o

all: app.bin

app.bin: $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o app.bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(LIB_DIR)/%.o: $(LIB_DIR)/%.c
	$(CC) $(CFLAGS) -DRSL_IMPLEMENTATION -c $< -o $@

$(LIB_DIR)/%.o: $(LIB_DIR)/%.s
	$(CC) $(CFLAGS) -x assembler-with-cpp -c $< -o $@

clean:
	rm -f *.o app.bin
