CFLAGS:=$(CFLAGS) $(USER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: ls.smp install
ls.smp:
	$(CC) -o ls.smp main.c  $(CFLAGS) $(CPPFLAGS)
install:
	cp ls.smp $(HOME)/SmplOSV2/os/usr/bin/ls.smp
clean:
	-rm -rf $(HOME)/SmplOSV2/os/usr/bin/ls.smp
	-rm main.o
	-rm -rf ls.smp
