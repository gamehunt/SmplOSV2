CFLAGS:=$(CFLAGS) -ffreestanding -nostdlib -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS) -I$(SYSROOT)/include
LIBS:
all: ps2.smod install
ps2.smod:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) -m elf_i386 -r main.o -o ps2.smod 
install:
	cp ps2.smod $(HOME)/SmplOSV2/os/modules/bin/ps2.smod
clean:
	rm -rf $(HOME)/SmplOSV2/os/modules/bin/ps2.smod
	rm main.o
	rm -rf ps2.smod
