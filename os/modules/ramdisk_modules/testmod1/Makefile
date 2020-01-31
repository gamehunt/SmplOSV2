CFLAGS:=$(CFLAGS) -ffreestanding -nostdlib -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS) -I$(SYSROOT)/usr/include -I../../../kernel/include
LIBS:
all: test1.smod install
test1.smod:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) -m elf_i386 -r main.o -o test1.smod 
install:
	cp test1.smod $(HOME)/SmplOSV2/os/modules/ramdisk_modules/bin
clean:
	rm -rf $(HOME)/SmplOSV2/os/modules/ramdisk_modules/bin/test1.smod
	rm main.o
	rm -rf test1.smod
