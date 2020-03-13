 echo 'Building userspace apps...'
input="targets.conf"
while IFS= read -r var
do
  echo "Building $var..."
  cd $var
  make clean
  make all
  cd $HOME/SmplOSV2/os/modules
done < "$input"
cp -a $HOME/SmplOSV2/os/usr/bin/. $HOME/SmplOSV2/sysroot/usr/bin/
