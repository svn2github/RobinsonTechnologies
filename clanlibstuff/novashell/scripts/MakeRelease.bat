REM ** Make sure american code page is used, otherwise the %DATE environmental var might be wrong
CHCP 437

cd ..
set d_fname=novashell_test.zip

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

REM ************* CHECKING FOR THE EXISTANCE OF WZZIP.EXE ON THE PATH, GIVE ERROR IF NOT FOUND *********
rem set error level to 0
ECHO:Y |FIND "Y" >NUL
echo %ERRORLEVEL%
wzzip.exe bogusfilename FileThatDoesntExistSoAnErrorWillBeCreated > NUL

REM For command.com
if "%ERRORLEVEL%" == "0" (
echo Error: wzzip.exe not found.
echo To use this batch file, you need to have Winzip 8 or newer installed and also the Winzip Command Line Support Addon.  
echo You can get this at http://download.winzip.com/wzcline.exe
echo You will need to copy wzzip.exe to your windows system32 directory or somewhere in your path so it's accessible from anywhere
source\util\beeper.exe /p
goto done
)

REM For cmd.com or something

if "%ERRORLEVEL%" == "9009" (
echo Error: wzzip.exe not found.
echo To use this batch file, you need to have Winzip 8 or newer installed and also the Winzip Command Line Support Addon.  
echo You can get this at http://download.winzip.com/wzcline.exe
echo You will need to copy wzzip.exe to your windows system32 directory or somewhere in your path so it's accessible from anywhere
source\util\beeper.exe /p
goto done
)
REM **************************

if exist %d_fname% del %d_fname%
wzzip %d_fname% bin -rp -x*.sup -x*.ilk -x*.zip -x*_debug.exe -x*.exp *.lib -xlog.txt -x@Scripts\RetailBuildExcludeList.txt -x*.sfk -x*.sfap0 -xdink.dat -xmap.dat

set C_FILENAME=%d_fname%
cd scripts
call FTPToSite.bat
pause