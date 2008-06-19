#!/bin/bash

Pause()
{
    key=""
    echo -n Hit any key to continue....
    stty -icanon
    key=`dd count=1 2>/dev/null`
    stty icanon
}

echo
echo -n This script will build the novashell executable.  First, make sure you have the latest Clanlib 0.8.X from svn, and have done a "make install" with it.
echo
Pause
cd clanlibstuff/novashell/scripts
sh linux_build_release.sh

echo
echo -n "Well, if everything worked, you have the novashell bin sitting in the clanlibstuff/novashell/bin dir.  You'll need to download one of compiled distributions"
echo -n "from RTsoft to get the graphics/data to really test it though."
echo
Pause



