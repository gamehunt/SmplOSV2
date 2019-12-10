#!/bin/bash
source config.sh
losetup $LOOPDEV1 ../disk.img
losetup $LOOPDEV2 ../disk.img -o 1048576
mount $LOOPDEV2 /mnt/smplos/
