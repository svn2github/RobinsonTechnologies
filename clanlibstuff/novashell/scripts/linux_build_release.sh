#!/bin/bash

Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}


echo Running from $PWD

sh linux_svn_update.sh

cd ..
make distclean
echo Setting up for retail build
mkdir optimized
cd 'optimized'

CXXFLAGS="-O2 -g0" && ../configure --prefix=/home/mrfun/dev/novashell

#erase file that was there
rm ../bin/novashell

#actually make it
make install

#return to the dir from whence we came
cd ../scripts

sh ./linux_media_update.sh

sh ./linux_package_release.sh

Pause
