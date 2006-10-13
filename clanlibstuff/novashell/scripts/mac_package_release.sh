#!/bin/bash

Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

echo Packing release
cd ../mac
mkdir dist

#clean out the old one
rm -R ./dist/novashell.app
#copy in the new one
echo Copying files to dist folder
cp -R build/Default/novashell.app ./dist

rm ./novashell.dmg
#hdiutil internet-enable -yes novashell.dmg
#return to the script directory

echo Creating dmg
make
echo Done
sh ../scripts/linux_upload.sh novashell.dmg
cd ../scripts