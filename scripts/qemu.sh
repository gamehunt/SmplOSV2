#!/bin/bash

./build.sh
sudo rsync -a ../sysroot/ /mnt/smplos/
sync
sleep 3
qemu-system-i386 -d cpu_reset -m 256 -drive format=raw,file=../disk.img
