CFLAGS:=$(CFLAGS) -ffreestanding -nostdlib -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: test.smod install
test.smod:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) -r main.o -o test.smod 
install:
	cp test.smod $(HOME)/SmplOSV2/os/modules/ramdisk_modules/bin
clean:
	rm -rf $(HOME)/SmplOSV2/os/modules/ramdisk_modules/bin/test.smod
	rm main.o
	rm -rf test.smod
