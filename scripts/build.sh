set -e
. ./setupenv.sh

./clean.sh || true

for DIR in $BUILDDIRS; do
	echo Installing headers for $DIR 
	(cd ../$DIR && make install-headers)
done

for DIR in $BUILDDIRS; do
	echo Building $DIR 
	(cd ../$DIR && make all && make install)
done


