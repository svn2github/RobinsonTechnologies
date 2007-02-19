REM FTP our API reference files to the server, create dirs if need be

call win_make_docs.bat
REM get our ftp logon info
call ../../../SetFTPLogonInfo.bat

ncftpput -u %_FTP_USER_% -p %_FTP_PASS_% -R %_FTP_SITE_% /www/novashell/docs api

REM remove environmental vars we used
set C_FILENAME=
call ../../../KillFTPLogonInfo.bat
