ALL_OBJS=\
cserv.o\
widgets/widget.o\
widgets/debug.o\

.PHONY: all clean install
.SUFFIXES: .o .c .cpp .asm

all: cserv.a install

clean:
	-rm -f cserv.a
	-rm -f $(HOME)/SmplOSV2/libs/bin/libcserv.a
	-rm -f $(ALL_OBJS) *.o */*.o */*/*.o
	-rm -f $(ALL_OBJS:.o=.d) *.d */*.d */*/*.d

install:
	cp cserv.a $(HOME)/SmplOSV2/libs/bin/libcserv.a
	rsync -a include/ ../include/

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 -Iinclude -Iinclude/cserv $(LIBC_CFLAGS) $(CPPFLAGS)

.cpp.o:
	i686-smplos-g++ -MD -c $< -o $@ -std=gnu11 -Iinclude -Iinclude/cserv -I../fb/include $(LIBC_CFLAGS) $(CPPFLAGS) -lfb
 
.asm.o:
	nasm -f elf32 $< -o $@
	
cserv.a:$(ALL_OBJS)
	$(AR) rcs $@ $(ALL_OBJS)


