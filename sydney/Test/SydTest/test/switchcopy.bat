echo off
REM Win98/Win2k両用xcopyコマンド。パスはフルパスでなければならない。

if '%OS%' == 'Windows_NT' xcopy /s /q /e /k /x /i %1 %2
if not '%OS%' == 'Windows_NT' xcopy /s /q /e /k /i %1 %2
echo on
