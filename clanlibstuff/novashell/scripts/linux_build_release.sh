#!/bin/bash

filename="novashell"

Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

echo Running from $PWD - building $filename

sh linux_svn_update.sh

cd ..
make distclean
echo Setting up for retail build
mkdir optimized
#figure out the bin path ourselves
binoutputdir=$PWD

cd 'optimized'

CXXFLAGS="-O2 -g0" && ../configure --prefix=$binoutputdir

#erase file that was there
rm -f ../bin/$filename

#actually make it
make install


#success?

if [ -f ../bin/$filename ] 
then

 echo Successfully built it.
  cd ../bin  
  strip -s $filename

 else
  echo -e "Error building executable! \a"
  Pause
 exit 1;
fi

#return to the dir from whence we came
cd ../scripts

sh ./linux_media_update.sh

sh ./linux_package_release.sh

Pause
