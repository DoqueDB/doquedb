@echo off

set TEMPFILENAME=%TEMP%\chkdll

if not exist %1 goto error1

dumpbin.exe /exports %1|perl -ne "print if (m/[0-9]+[ ]+[0-9A-Z]+[ ]+[0-9A-Z]+[ ]+.*/)"|perl -pe "s/([0-9]+)[ ]+[0-9A-Z]+[ ]+[0-9A-Z]+[ ]+(.*)/$2/">%TEMPFILENAME%-1
if not exist %2 goto error2
dumpbin.exe /exports %2|perl -ne "print if (m/[0-9]+[ ]+[0-9A-Z]+[ ]+[0-9A-Z]+[ ]+.*/)"|perl -pe "s/([0-9]+)[ ]+[0-9A-Z]+[ ]+[0-9A-Z]+[ ]+(.*)/$2/">%TEMPFILENAME%-2
fc %TEMPFILENAME%-1 %TEMPFILENAME%-2
goto end
:error1
echo.
echo %1が存在しません
goto end
:error2
echo.
echo %2が存在しません
goto end
:end
