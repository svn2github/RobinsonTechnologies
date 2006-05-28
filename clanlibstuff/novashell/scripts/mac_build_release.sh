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

cd ../scripts

