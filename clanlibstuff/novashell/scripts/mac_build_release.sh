Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

sh mac_svn_update.sh

cd ../mac
echo Building universal release game binaries

xcodebuild -target novashell_release
if [ $? -ne 0 ]
then
echo -e "Error building game, check it out. \a"
Pause
exit
else
echo "Success!"
fi 

#First check to see if our network drive is available

if [ -d /Volumes/PROJECTS/clanlibstuff/novashell/bin/media ] 
then

 echo Located network drive.
 else
  echo -e "Network drive not available!!  Turn it on now and hit a key! \a"
  Pause
fi

#refresh media data from the main server (first deleting the old stuff)
rm ./build/Default/novashell.app/log.txt
rm -R ./build/Default/novashell.app/Contents/Resources/media
rm -R ./build/Default/novashell.app/Contents/Resources/profiles
mkdir ./build/Default/novashell.app/Contents/Resources
mkdir ./build/Default/novashell.app/Contents/Resources/media
#copy the new one
cp -R /Volumes/PROJECTS/clanlibstuff/novashell/bin/media build/Default/novashell.app/Contents/Resources

cd ../scripts

