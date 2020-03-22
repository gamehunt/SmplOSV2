input="targets.conf"
while IFS= read -r var
do
  echo "Cleaning $var..."
  cd $var
  make clean
  cd $HOME/SmplOSV2/libs
done < "$input"
rm -rf bin
rm -rf include
rm -rf ../sysroot/lib
rm -rf ../sysroot/include
mkdir ../sysroot/include
mkdir ../sysroot/lib
mkdir bin
mkdir include
