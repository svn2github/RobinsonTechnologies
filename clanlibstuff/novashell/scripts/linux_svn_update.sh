#update both pieces that we check out
cd ../
echo Updating project from SVN
svn update

cd SharedLib
svn update

#move back
cd ../scripts
