#first update clanlib
cd ../../ClanLib-0.8
echo Updating ClanLib from SVN
svn update
make install

cd ../novashell
echo Updating project from SVN
svn update

cd SharedLib
svn update

#move back
cd ../scripts
