CFLAGS:=$(CFLAGS) $(USER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: login.smp install
login.smp:
	$(CC) -o login.smp main.c  $(CFLAGS) $(CPPFLAGS)
install:
	cp login.smp $(HOME)/SmplOSV2/os/usr/bin/login.smp
clean:
	-rm -rf $(HOME)/SmplOSV2/os/usr/bin/login.smp
	-rm main.o
	-rm -rf login.smp
