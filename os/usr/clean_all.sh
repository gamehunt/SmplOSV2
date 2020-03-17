input="targets.conf"
while IFS= read -r var
do
  echo "Cleaning $var..."
  cd $var
  make clean
  cd $HOME/SmplOSV2/os/usr
done < "$input"
rm -rf bin
rm -rf ../../sysroot/usr/bin
mkdir ../../sysroot/usr/bin
mkdir bin
