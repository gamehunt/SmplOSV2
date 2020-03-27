ALL_OBJS=\
gdi.o\

.PHONY: all clean install
.SUFFIXES: .o .c .asm

all: gdi.a install

clean:
	rm -f gdi.a
	rm -f $(HOME)/SmplOSV2/libs/bin/libgdi.a
	rm -f $(ALL_OBJS) *.o */*.o */*/*.o
	rm -f $(ALL_OBJS:.o=.d) *.d */*.d */*/*.d

install:
	cp gdi.a $(HOME)/SmplOSV2/libs/bin/libgdi.a
	rsync -a include/ ../include

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 -Iinclude $(LIBC_CFLAGS) $(CPPFLAGS)
 
.asm.o:
	nasm -f elf32 $< -o $@
	
gdi.a:$(ALL_OBJS)
	$(AR) rcs $@ $(ALL_OBJS)


