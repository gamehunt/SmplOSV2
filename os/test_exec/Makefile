CFLAGS:=$(CFLAGS) -ffreestanding -nostdlib -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS) -I$(SYSROOT)/usr/include -I../../../kernel/include
LIBS:
all: test.smp install
test.smp:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) -m elf_i386 main.o -o test.smp
install:

clean:
	rm main.o
	rm test.smp
