#!/bin/bash
source config.sh
dd if=/dev/zero of=../disk.img bs=512 count=1048576
fdisk ../disk.img <<< $'n\n\n\n\n\n\n\na\nw\n'
#device = losetup -f
sudo losetup $LOOPDEV1 ../disk.img
#device_next = losetup -f
sudo losetup $LOOPDEV2 ../disk.img -o 1048576
sudo mkdosfs -F32 -f 2 $LOOPDEV2
sudo mkdir /mnt/smplos
sudo mount $LOOPDEV2 /mnt/smplos
sudo grub-install --target=i386-pc --root-directory=/mnt/smplos --no-floppy --modules="normal part_msdos ext2 multiboot" $LOOPDEV2 
sync
sudo cp grub.cfg /mnt/smplos/boot/grub/grub.cfg
sleep 3
