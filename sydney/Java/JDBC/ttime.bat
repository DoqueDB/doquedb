@echo off
if '%1'=='' goto USAGE

grep Total %1

goto END

:USAGE
echo [USAGE] ttime.bat log
echo             log ... ant test log file name

:END
