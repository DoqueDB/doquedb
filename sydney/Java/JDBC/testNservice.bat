@echo off
setlocal

echo start sydney server
net start sydserver
time /t
call testN.bat %1 %3
time /t
echo wait %2 [hour]
perl sleep_hour.pl %2
time /t
echo stop sydney server
net stop sydserver
time /t
