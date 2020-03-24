CFLAGS:=$(CFLAGS) -ffreestanding -nostdlib -Wall -Wextra -O0
CPPFLAGS:=$(CPPFLAGS)
LIBS:
all: test.smp install
test.smp:
	$(CC) -c main.c $(CFLAGS) $(CPPFLAGS) 
	$(LD) main.o -o test.smp
install:

clean:
	rm main.o
	rm test.smp
