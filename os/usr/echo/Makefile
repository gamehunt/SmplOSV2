CFLAGS:=$(CFLAGS) $(USER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: echo.smp install
echo.smp:
	$(CC) -o echo.smp main.c  $(CFLAGS) $(CPPFLAGS)
install:
	cp echo.smp $(HOME)/SmplOSV2/os/usr/bin/echo.smp
clean:
	-rm -rf $(HOME)/SmplOSV2/os/usr/bin/echo.smp
	-rm main.o
	-rm -rf echo.smp
