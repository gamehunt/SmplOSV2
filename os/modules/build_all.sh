 echo 'Building modules...'
input="targets.conf"
while IFS= read -r var
do
  echo "Building $var..."
  cd $var
  make clean
  make all
  cd $HOME/SmplOSV2/os/modules
done < "$input"
cp -a $HOME/SmplOSV2/os/modules/bin/. $HOME/SmplOSV2/bin/modules/
cd ramdisk_modules
./build_all.sh
