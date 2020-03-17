CFLAGS:=$(CFLAGS) $(USER_FLAGS)
CPPFLAGS:=$(CPPFLAGS) -I$(SYSROOT)/include
LIBS:
all: shell.smp install
shell.smp:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) -m elf_i386 main.o -o shell.smp --sysroot=/home/user/SmplOSV2/sysroot -lc
install:
	cp shell.smp $(HOME)/SmplOSV2/os/usr/bin/shell.smp
clean:
	rm -rf $(HOME)/SmplOSV2/os/usr/bin/shell.smp
	rm main.o
	rm -rf shell.smp
