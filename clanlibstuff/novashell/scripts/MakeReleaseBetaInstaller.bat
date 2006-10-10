REM ** Make sure american code page is used, otherwise the %DATE environmental var might be wrong
CHCP 437

call DeleteGarbageFiles.bat

cd ..
SET C_FILENAME=NovashellInstaller.exe

call vnet.bat
set C_TARGET_EXE=bin\game.exe

REM erase it so we know it got built right
del %C_TARGET_EXE% > NUL

REM Compile everything 
devenv source\novashell.sln /build "Static MT Release"

REM make it smaller
REM upx.exe %C_TARGET_EXE%

REM Make sure the file compiled ok
if not exist %C_TARGET_EXE% beeper.exe /p

//installer

REM get version information from the source code
echo Grabbing version # information from source code.

ctoenv.exe source\main.cpp "m_engineVersion = " C_VERSION /r
if errorlevel 1 beeper.exe /p
call setenv.bat
del setenv.bat

ctoenv.exe source\main.cpp "m_engineVersionString = \"" C_TEXT_VERSION_TEMP
if errorlevel 1  beeper.exe /p
call setenv.bat
del setenv.bat
SET C_TEXT_VERSION=%C_TEXT_VERSION_TEMP% Alpha
REM done with temp var, kill it
SET C_TEXT_VERSION_TEMP=
echo Building installer: %C_FILENAME% %C_TEXT_VERSION%


cd win
..\..\..\util\NSIS\makensis.exe novashell.nsi
cd ..

Rem Check for error during installer packing
if not exist %C_FILENAME% beeper.exe /p

cd scripts
call FTPToSite.bat
pause