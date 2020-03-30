ALL_OBJS=\
string/strlen.o \
string/memcpy.o \
string/memset.o \
string/memmove.o \
string/memcmp.o \
string/strcat.o \
string/strcmp.o \
string/substr.o \
string/strcpy.o \
string/strchr.o \
string/strtok.o \
stdio/putchar.o \
stdio/puts.o \
stdio/vprintf.o \
stdio/printf.o \
stdio/vsprintf.o \
stdio/sprintf.o \
ctype/isdigit.o \
stdlib/abort.o \
stdlib/alloc.o \
stdlib/atoi.o \
time/clock.o \
time/time.o \



LIBK_OBJS=$(ALL_OBJS:.o=.libk.o)
LIBC_OBJS=$(ALL_OBJS)\
libc_init.o\
syscall.o \
stdio/stdio.o\
dirent/dirent.o \
unistd/exec.o \
unistd/getcwd.o \
unistd/chdir.o \
stdlib/env.o \

CRT_OBJS=crt0.o\
crti.o\
crtn.o\

.PHONY: all clean install
.SUFFIXES: .o .libk.o .c .asm

all: libk.a libc.a crt

clean:
	rm -f libc.a
	rm -f libk.a
	rm $(SYSROOT)$(SYSLIB)/libk.a
	rm $(SYSROOT)$(SYSLIB)/libc.a
	rm -f $(LIBC_OBJS) $(LIBK_OBJS) *.o */*.o */*/*.o
	rm -f $(LIBC_OBJS:.o=.d) $(LIBK_OBJS:.o=.d) *.d */*.d */*/*.d
	rm -f $(CRT_OBJS)
install:
	cp libk.a $(SYSROOT)$(SYSLIB)/libk.a
	cp libc.a $(SYSROOT)$(SYSLIB)/libc.a
	cp crt0.o $(SYSROOT)$(SYSLIB)/crt0.o
	cp crti.o $(SYSROOT)$(SYSLIB)/crti.o
	cp crtn.o $(SYSROOT)$(SYSLIB)/crtn.o


install-headers:
	rsync -a include/ $(SYSROOT)$(SYSINC)/

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 -Iinclude $(LIBC_CFLAGS) $(CPPFLAGS)
 
.asm.o:
	nasm -f elf32 $< -o $@
 
.c.libk.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 -Iinclude $(LIBK_CFLAGS)
 
.asm.libk.o:
	nasm -f elf32 $< -o $@
	
libc.a:$(LIBC_OBJS)
	$(AR) rcs $@ $(LIBC_OBJS)

libk.a:$(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

crt:$(CRT_OBJS)

