CFLAGS:=$(CFLAGS) $(USER_FLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: shell.smp install
shell.smp:
	$(CC) -o shell.smp main.c $(CFLAGS) $(CPPFLAGS) 
install:
	cp shell.smp $(HOME)/SmplOSV2/os/usr/bin/shell.smp
clean:
	-rm -rf $(HOME)/SmplOSV2/os/usr/bin/shell.smp
	-rm main.o
	-rm -rf shell.smp
