CFLAGS:=$(CFLAGS) $(USER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: init.smp install
init.smp:
	$(CC) -o init.smp main.c  $(CFLAGS) $(CPPFLAGS)
install:
	cp init.smp $(HOME)/SmplOSV2/os/usr/bin/init.smp
clean:
	-rm -rf $(HOME)/SmplOSV2/os/usr/bin/init.smp
	-rm main.o
	-rm -rf init.smp
