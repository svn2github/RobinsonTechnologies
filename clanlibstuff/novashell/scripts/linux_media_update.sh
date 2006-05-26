#!/bin/bash

#erase the old stuff
echo Erasing local media directory
rm -r ../bin/media

echo Coping media tree from master server
cp -r /media/projects/clanlibstuff/novashell/bin/media ../bin
