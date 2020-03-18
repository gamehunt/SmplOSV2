#!/bin/bash
set -e 
. ./config.sh
export TARGET=$TARGET
export ARCH=$ARCH
echo Set up target ${TARGET}
export KERNEL_CFLAGS=$KERNEL_CFLAGS
export LIBK_CFLAGS=$LIBK_CFLAGS
export LIBC_CFLAGS=$LIBC_CFLAGS
echo Compiling with:
export CC=${TARGET}-gcc
export AR=${TARGET}-ar
export AS=${TARGET}-as
export LD=${TARGET}-ld
echo $CC
echo $AR
echo $AS
echo $LD
export D_ASM=${DEFAULT_ASM}
echo Default assembler: $D_ASM 
export SYSROOT="$HOME/SmplOSV2/sysroot"
export SYSINC="/include"
export SYSLIB="/lib"
export KERNEL_LINKS="$KERNEL_LINKS"
export CC="$CC --sysroot=$SYSROOT"

if echo "$TARGET" | grep -Eq -- '-elf($|-)'; then
	export CC="$CC -isystem=$SYSINC"	
fi

echo Final CC for kernel: $CC $KERNEL_CFLAGS
if [ ! -d "$SYSROOT" ] 
then
	echo Creating sysroot...
	mkdir $SYSROOT
	mkdir $SYSINC
	mkdir $SYSLIB
	mkdir $SYSROOT/boot
fi
