#!/bin/bash

set -e
. ./setupenv.sh

for DIR in $BUILDDIRS; do
	echo Cleaning $DIR 
	(cd ../$DIR && make clean)
done

rm ../os/ramdisk.rmdsk 
