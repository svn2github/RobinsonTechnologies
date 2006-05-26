#!/bin/bash

cd ..
mkdir dist
cd bin
rm log.txt
strip -s novashell
#upx-ucl -q --best novashell
cd ..
tar cvfz novashell.tar.gz bin

echo Packing final release and FTPing or whatever
cd scripts
