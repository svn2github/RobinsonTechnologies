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

#emergency copy system while clanlib.org is down

#First check to see if our network drive is available

if [ -d /Volumes/PROJECTS/clanlibstuff/novashell/bin/base ] 
then

 echo Located network drive.
 else
  echo -e "Network drive not available!!  Use Ctrl-K from finder to mount PROJECTS and CLANLIB\a"
  Pause
fi

cd /Volumes/CLANLIB
echo "Exporting CLANLIB..."
svn export --force ./ ~/dev/Clanlib-0.8

cd ~/dev/ClanLib-0.8
#svn update

echo Building clanlib...
xcodebuild -project ClanLibUniversalBinaries.xcodeproj -configuration Deployment
if [ $? -ne 0 ]
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