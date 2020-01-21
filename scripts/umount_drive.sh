#!/bin/bash
source config.sh
sudo losetup -d $LOOPDEV1 
sudo losetup -d $LOOPDEV2
sudo umount /mnt/smplos
