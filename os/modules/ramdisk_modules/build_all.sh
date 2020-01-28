echo 'Building ramdisk modules...'
input="targets.conf"
while IFS= read -r var
do
  echo "Building $var..."
  cd $var
  make clean
  make all
  cd $HOME/SmplOSV2/os/modules/ramdisk_modules
done < "$input"
cp -a $HOME/SmplOSV2/os/modules/ramdisk_modules/bin/. $HOME/SmplOSV2/ramdisk
echo 'Finished!'
