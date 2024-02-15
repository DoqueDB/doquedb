@echo off
setlocal

set dumpOn=0
set processID=0

:AnalyzeOption

if '%1' == '/dump' goto SetDump
if '%1' == '' goto DoMain

set processID=%1
shift /1
goto AnalyzeOption

:SetDump
set dumpOn=1
shift /1
goto AnalyzeOption

:DoMain

if %processID% == 0 goto End

if %dumpOn% == 1 goto DumpOn

MyWalk.exe /port 32100 /sum %processID%
goto End

:DumpOn

MyWalk.exe /port 32100 /dump 32101 /sum %processID%
goto End

:End
endlocal
@echo on
