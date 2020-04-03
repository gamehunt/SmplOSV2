CFLAGS:=$(CFLAGS) $(USER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: shutdown.smp install
shutdown.smp:
	$(CC) -o shutdown.smp main.c  $(CFLAGS) $(CPPFLAGS)
install:
	cp shutdown.smp $(HOME)/SmplOSV2/os/usr/bin/shutdown.smp
clean:
	-rm -rf $(HOME)/SmplOSV2/os/usr/bin/shutdown.smp
	-rm main.o
	-rm -rf shutdown.smp
