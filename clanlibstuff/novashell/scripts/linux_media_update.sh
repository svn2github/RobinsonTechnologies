#!/bin/bash

#erase the old stuff
echo Erasing local media directory
rm -r ../bin/base
rm -r ../bin/worlds

echo Coping media tree from master server
cp -r /media/projects/clanlibstuff/novashell/bin/base ../bin/base
cp -r /media/projects/clanlibstuff/novashell/bin/worlds ../bin
