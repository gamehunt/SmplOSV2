CFLAGS:=$(CFLAGS) -ffreestanding -nostdlib -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: qvideo.smod install
qvideo.smod:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) -r main.o -o qvideo.smod 
install:
	cp qvideo.smod $(HOME)/SmplOSV2/os/modules/bin/qvideo.smod
clean:
	rm -rf $(HOME)/SmplOSV2/os/modules/bin/qvideo.smod
	rm main.o
	rm -rf qvideo.smod
