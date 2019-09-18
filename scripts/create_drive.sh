dd if=/dev/zero of=../disk.img bs=512 count=1048576
fdisk ../disk.img <<< $'n\n\n\n\n\n\n\na\nw\n'
sudo losetup /dev/loop0 ../disk.img
sudo losetup /dev/loop1 ../disk.img -o 1048576
sudo mkdosfs -F32 -f 2 /dev/loop1
sudo mkdir /mnt/smplos
sudo mount /dev/loop1 /mnt/smplos
sudo grub-install --root-directory=/mnt/smplos --no-floppy --modules="normal part_msdos ext2 multiboot" /dev/loop0
sync
sleep 3
