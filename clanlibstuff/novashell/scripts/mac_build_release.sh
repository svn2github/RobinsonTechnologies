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

BuildLogFileName=./build.txt
rm $BuildLogFileName
xcodebuild -target novashell_release | tee $BuildLogFileName | cat

#scan build log for errors
if fgrep 'error:' $BuildLogFileName > /dev/null
then
echo -e "Error building game, check it out. \a"
Pause
exit
else
echo "Success!"
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
