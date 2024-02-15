@echo off
setlocal

REM tools/setup/restore.batのかわり
REM tools/setup/package/restore.batを一部修正

REM テストスクリプトのある場所から呼び出すことを想定
call ..\..\conf.bat

set savedir=save
if not '%1' == '' set savedir=%1

pushd %databasepath%
rmdir /s /q data > NUL 2>&1
rmdir /s /q system > NUL 2>&1
mkdir data
mkdir system
xcopy /s /q /e /k /x %savedir%\data data
xcopy /s /q /e /k /x %savedir%\system system
popd

endlocal
@echo on
