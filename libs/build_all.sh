 echo 'Building libraries...'
input="targets.conf"
while IFS= read -r var
do
  echo "Building $var..."
  cd $var
  make clean
  make all
  cd $HOME/SmplOSV2/libs
done < "$input"
cp -a $HOME/SmplOSV2/libs/bin/. $HOME/SmplOSV2/sysroot/lib
cp -a $HOME/SmplOSV2/libs/include/. $HOME/SmplOSV2/sysroot/include
