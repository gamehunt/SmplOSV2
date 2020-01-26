#!/bin/bash

./build.sh
./create_ramdisk.sh
sudo rsync -a ../sysroot/ /mnt/smplos/
sudo cp ../ramdisk.rmdsk /mnt/smplos/boot
sync
sleep 3
qemu-system-i386 -d cpu_reset -m 256 -drive format=raw,file=../disk.img
