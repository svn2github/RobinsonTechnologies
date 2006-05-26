#!/bin/bash

packedfilename=novashell_test.tar.gz

echo "Packing final release... $packedfilename"

cd ..
mkdir dist
cd bin
rm log.txt

cd ..
rm -f 
tar cvfz $packedfilename bin

if [ -f $packedfilename ] 
then

 echo FTPing file
	
   sh ./scripts/linux_upload.sh $packedfilename

 else
  echo -e "Error with gzipping or something. \a"
  Pause
 exit 1;
fi

cd scripts
