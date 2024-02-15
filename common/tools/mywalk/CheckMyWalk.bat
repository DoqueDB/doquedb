@echo off
setlocal
set interval=%1
if '%interval' == '' set interval=15m

:Loop

MyWalkClient localhost 32100
MyWalkClient localhost 32101

sleep %interval%
goto Loop
endlocal
@echo on
