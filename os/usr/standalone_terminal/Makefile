CFLAGS:=$(CFLAGS) $(USER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: sterm.smp install
sterm.smp:
	$(CC) -o sterm.smp main.c  $(CFLAGS) $(CPPFLAGS) -lgdi -lkbd
install:
	cp sterm.smp $(HOME)/SmplOSV2/os/usr/bin/sterm.smp
clean:
	-rm -rf $(HOME)/SmplOSV2/os/usr/bin/sterm.smp
	-rm main.o
	-rm -rf sterm.smp
