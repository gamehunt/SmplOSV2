ALL_OBJS=\
fb.o\

.PHONY: all clean install
.SUFFIXES: .o .c .asm

all: fb.a install

clean:
	-rm -f fb.a
	-rm -f $(HOME)/SmplOSV2/libs/bin/libfb.a
	-rm -f $(ALL_OBJS) *.o */*.o */*/*.o
	-rm -f $(ALL_OBJS:.o=.d) *.d */*.d */*/*.d

install:
	cp fb.a $(HOME)/SmplOSV2/libs/bin/libfb.a
	rsync -a include/ ../include

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 -Iinclude $(LIBC_CFLAGS) $(CPPFLAGS)
 
.asm.o:
	nasm -f elf32 $< -o $@
	
fb.a:$(ALL_OBJS)
	$(AR) rcs $@ $(ALL_OBJS)


