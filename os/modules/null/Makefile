CFLAGS:=$(CFLAGS) -ffreestanding -nostdlib -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: null.smod install
null.smod:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) -r main.o -o null.smod 
install:
	cp null.smod $(HOME)/SmplOSV2/os/modules/bin/null.smod
clean:
	rm -rf $(HOME)/SmplOSV2/os/modules/bin/null.smod
	rm main.o
	rm -rf null.smod
