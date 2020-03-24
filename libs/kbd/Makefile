ALL_OBJS=\
kbd.o\

.PHONY: all clean install
.SUFFIXES: .o .c .asm

all: kbd.a install

clean:
	rm -f kbd.a
	rm -f $(HOME)/SmplOSV2/libs/bin/libkbd.a
	rm -f $(ALL_OBJS) *.o */*.o */*/*.o
	rm -f $(ALL_OBJS:.o=.d) *.d */*.d */*/*.d

install:
	cp kbd.a $(HOME)/SmplOSV2/libs/bin/libkbd.a
	rsync -a include/ ../include

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 -Iinclude $(LIBC_CFLAGS) $(CPPFLAGS)
 
.asm.o:
	nasm -f elf32 $< -o $@
	
kbd.a:$(ALL_OBJS)
	$(AR) rcs $@ $(ALL_OBJS)


