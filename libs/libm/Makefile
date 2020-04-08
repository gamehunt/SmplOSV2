ALL_OBJS=\
libm.o\

.PHONY: all clean install
.SUFFIXES: .o .c .asm

all: libm.a install

clean:
	-rm -f libm.a
	-rm -f $(HOME)/SmplOSV2/libs/bin/libm.a
	-rm -f $(ALL_OBJS) *.o */*.o */*/*.o
	-rm -f $(ALL_OBJS:.o=.d) *.d */*.d */*/*.d

install:
	cp libm.a $(HOME)/SmplOSV2/libs/bin/libm.a
	rsync -a include/ ../include

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 -Iinclude $(LIBC_CFLAGS) $(CPPFLAGS)
 
.asm.o:
	nasm -f elf32 $< -o $@
	
libm.a:$(ALL_OBJS)
	$(AR) rcs $@ $(ALL_OBJS)


