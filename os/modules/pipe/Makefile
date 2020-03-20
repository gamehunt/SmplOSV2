CFLAGS:=$(CFLAGS) -ffreestanding -nostdlib -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS) -I$(SYSROOT)/usr/include -I../../../kernel/include
LIBS:
all: pipe.smod install
pipe.smod:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) -m elf_i386 -r main.o -o pipe.smod 
install:
	cp pipe.smod $(HOME)/SmplOSV2/os/modules/bin/pipe.smod
clean:
	rm -rf $(HOME)/SmplOSV2/os/modules/bin/pipe.smod
	rm main.o
	rm -rf pipe.smod
