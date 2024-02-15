@echo off
if '%1'=='' goto USAGE

find /c "dist" %1

goto END

:USAGE
echo [USAGE] tcount.bat log
echo             log ... ant test log file name

:END
