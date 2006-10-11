#!/bin/bash

packedfilename=novashell_test.tar.gz

echo "Packing final release... $packedfilename"

cd ..
cd bin
rm log.txt
cd ..

unlink $packedfilenames

cd bin
tar cvfz ../$packedfilename novashell worlds base
cd ..

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
