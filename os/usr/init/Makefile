CFLAGS:=$(CFLAGS) $(USER_FLAGS)
CPPFLAGS:=$(CPPFLAGS) -I$(SYSROOT)/include
LIBS:
all: init.smp install
init.smp:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) -m elf_i386 main.o -o init.smp --sysroot=/home/user/SmplOSV2/sysroot -lc
install:
	cp init.smp $(HOME)/SmplOSV2/os/usr/bin/init.smp
clean:
	rm -rf $(HOME)/SmplOSV2/os/usr/bin/init.smp
	rm main.o
	rm -rf init.smp
