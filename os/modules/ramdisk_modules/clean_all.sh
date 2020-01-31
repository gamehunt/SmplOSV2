input="targets.conf"
while IFS= read -r var
do
  echo "Cleaning $var..."
  cd $var
  make clean
  cd $HOME/SmplOSV2/os/modules/ramdisk_modules
done < "$input"
rm -rf bin
mkdir bin
cd $HOME/SmplOSV2
rm -rf ramdisk
mkdir ramdisk 
