CFLAGS:=$(CFLAGS) -ffreestanding -nostdlib -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS) -I$(SYSROOT)/usr/include -I../../../kernel/include
LIBS:
all: test2.smod install
test2.smod:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) -m elf_i386 -r main.o -o test2.smod 
install:

clean:

