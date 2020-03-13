#~/bin/bash

echo Configuring system...

#Configuration variables below

TARGET=i686-elf
ARCH=i386
DEFAULT_ASM=nasm
KERNEL_CFLAGS='-std=gnu99 -ffreestanding -O2 -Wall -Wextra' 
KERNEL_LINKS='-nostdlib  -lk -lgcc'
USER_CFLAGS='-std=gnu99 -O2 -Wall -Wextra' 
USER_LINKS='-lc -lgcc'
LIBK_CFLAGS="$KERNEL_CFLAGS -D__smplos_libk"
LIBC_CFLAGS="$KERNEL_CFLAGS -D__smplos_libc"
BUILDDIRS="libc os"
LOOPDEV1=/dev/loop18
LOOPDEV2=/dev/loop19

export PREFIX="$HOME/opt/cross"
export TARGET=i686-elf
export PATH="$PREFIX/bin:$PATH"

