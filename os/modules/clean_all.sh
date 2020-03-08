input="targets.conf"
while IFS= read -r var
do
  echo "Cleaning $var..."
  cd $var
  make clean
  cd $HOME/SmplOSV2/os/modules
done < "$input"
rm -rf bin
rm -rf ../../sysroot/bin/modules
mkdir ../../sysroot/bin/modules
mkdir bin
cd ramdisk_modules
./clean_all.sh
