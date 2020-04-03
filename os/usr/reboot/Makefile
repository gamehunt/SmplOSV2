CFLAGS:=$(CFLAGS) $(USER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: reboot.smp install
reboot.smp:
	$(CC) -o reboot.smp main.c  $(CFLAGS) $(CPPFLAGS)
install:
	cp reboot.smp $(HOME)/SmplOSV2/os/usr/bin/reboot.smp
clean:
	-rm -rf $(HOME)/SmplOSV2/os/usr/bin/reboot.smp
	-rm main.o
	-rm -rf reboot.smp
