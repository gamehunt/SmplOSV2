CFLAGS:=$(CFLAGS) $(USER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: cserv_tst.smp install
cserv_tst.smp:
	i686-smplos-g++ -o cserv_tst.smp main.cpp  $(CFLAGS) $(CPPFLAGS) -lcserv
install:
	cp cserv_tst.smp $(HOME)/SmplOSV2/os/usr/bin/ctst.smp
clean:
	-rm -rf $(HOME)/SmplOSV2/os/usr/bin/ctst.smp
	-rm main.o
	-rm -rf cserv_tst.smp
