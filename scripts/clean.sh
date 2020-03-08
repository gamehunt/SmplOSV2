#!/bin/bash

set -e
. ./setupenv.sh

for DIR in $BUILDDIRS; do
	echo Cleaning $DIR 
	(cd ../$DIR && make clean)
done

rm ../ramdisk.rmdsk 
sudo rm -rf /mnt/smplos/bin
sudo rm -rf /mnt/smplos/include
sudo rm -rf /mnt/smplos/lib