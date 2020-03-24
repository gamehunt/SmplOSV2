CFLAGS:=$(CFLAGS) -ffreestanding -nostdlib -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS) 
LIBS:
all: test_disk.smod install
test_disk.smod:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) -r main.o -o test_disk.smod 
install:
	cp test_disk.smod $(HOME)/SmplOSV2/os/modules/bin/tdsk.smod
clean:
	rm -rf $(HOME)/SmplOSV2/os/modules/bin/tdsk.smod
	rm main.o
	rm -rf test_disk.smod
