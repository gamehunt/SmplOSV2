CFLAGS:=$(CFLAGS) $(USER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: cserv.smp install
cserv.smp:
	$(CC) -o cserv.smp main.c  $(CFLAGS) $(CPPFLAGS) -lgdi
install:
	cp cserv.smp $(HOME)/SmplOSV2/os/usr/bin/cserv.smp
clean:
	-rm -rf $(HOME)/SmplOSV2/os/usr/bin/cserv.smp
	-rm main.o
	-rm -rf cserv.smp
