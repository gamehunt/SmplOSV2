CFLAGS:=$(CFLAGS) $(USER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: tty.smp install
tty.smp:
	$(CC) -o tty.smp main.c  $(CFLAGS) $(CPPFLAGS) -lgdi
install:
	cp tty.smp $(HOME)/SmplOSV2/os/usr/bin/tty.smp
clean:
	-rm -rf $(HOME)/SmplOSV2/os/usr/bin/tty.smp
	-rm main.o
	-rm -rf tty.smp
