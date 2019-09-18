set -e
. ./setupenv.sh

for DIR in $BUILDDIRS; do
	echo Cleaning $DIR 
	(cd ../$DIR && make clean)
done
