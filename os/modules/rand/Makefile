CFLAGS:=$(CFLAGS) -ffreestanding -nostdlib -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: rand.smod install
rand.smod:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	nasm -f elf32 rdrand.asm
	$(LD) -r main.o rdrand.o -o rand.smod 
install:
	cp rand.smod $(HOME)/SmplOSV2/os/modules/bin/rand.smod
clean:
	rm -rf $(HOME)/SmplOSV2/os/modules/bin/rand.smod
	rm main.o
	rm -rf rand.smod
