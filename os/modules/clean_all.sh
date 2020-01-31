input="targets.conf"
while IFS= read -r var
do
  echo "Cleaning $var..."
  cd $var
  make clean
  cd $HOME/SmplOSV2/os/modules
done < "$input"
rm -rf bin
mkdir bin
cd ramdisk_modules
./clean_all.sh
