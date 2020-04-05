CFLAGS:=$(CFLAGS) $(USER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: term.smp install
term.smp:
	$(CC) -o term.smp main.c  $(CFLAGS) $(CPPFLAGS) -lgdi -lkbd
install:
	cp term.smp $(HOME)/SmplOSV2/os/usr/bin/term.smp
clean:
	-rm -rf $(HOME)/SmplOSV2/os/usr/bin/term.smp
	-rm main.o
	-rm -rf term.smp
