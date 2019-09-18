echo Configuring system...

#Configuration variables below

TARGET=i686-elf
ARCH=i386
DEFAULT_ASM=nasm
KERNEL_CFLAGS='-std=gnu99 -ffreestanding -O2 -Wall -Wextra' 
KERNEL_LINKS='-nostdlib  -lk -lgcc'
LIBK_CFLAGS="$KERNEL_CFLAGS -D__smplos_libk"
LIBC_CFLAGS="$KERNEL_CFLAGS -D__smplos_libc"
BUILDDIRS="libc os"
