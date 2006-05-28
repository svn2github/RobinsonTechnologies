#!/bin/bash

Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

echo Updating Clanlib from SVN
cd ../../../../dev/ClanLib-0.8
svn update
echo Building clanlib...
BuildLogFileName=./build.txt
rm $BuildLogFileName
xcodebuild -configuration Deployment | tee $BuildLogFileName | cat

#scan build log for errors
if fgrep 'error:' $BuildLogFileName > /dev/null
then
echo -e "Error building clanlib. \a"
Pause
exit
else
echo "Success!"
fi 
cd ../../rtsvn
echo Updating project from SVN
svn update
cd clanlibstuff/novashell/scripts