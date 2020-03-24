CFLAGS:=$(CFLAGS) -ffreestanding -nostdlib -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS) 
LIBS:
all: ata.smod install
ata.smod:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) -r main.o -o ata.smod
install:
	cp ata.smod $(HOME)/SmplOSV2/os/modules/ramdisk_modules/bin
clean:
	rm -rf $(HOME)/SmplOSV2/os/modules/ramdisk_modules/bin/ata.smod
	rm main.o
	rm -rf ata.smod
