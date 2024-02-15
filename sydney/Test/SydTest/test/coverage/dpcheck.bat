@echo off
setlocal

REM テストに失敗している結果を削除
pushd single\normal
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
cd ..\except
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
popd

pushd multi\normal
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
cd ..\except
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
popd

pushd recovery\normal
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
cd ..\except
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
popd

pushd single_eu\normal
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
cd ..\..\single_eu_utf8\normal
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
cd ..\..\single_ja\normal
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
cd ..\..\single_ja_utf8\normal
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
cd ..\..\single_utf8\normal
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
cd ..\..\single_zh_utf8\normal
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
popd

pushd single_dist\normal
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
cd ..\except
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
popd

pushd recovery\normal
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
cd ..\except
for %%F in (*) do if %%~zF lss 1024 ( del %%~nF.* )
popd

grep -i 'MemoryLeaks=' */*/*.xml | grep -v '"0"'
grep -i 'TotalBytesLeaked=' */*/*.xml | grep -v '"0"'
grep -i 'ResourceLeaks=' */*/*.xml | grep -v '"0"'
grep -i 'InterfaceLeaks=' */*/*.xml | grep -v '"0"'
grep -i 'DotNETLeaks=' */*/*.xml | grep -v '"0"'
grep -i 'Errors=' */*/*.xml | grep -v '"0"'
grep -i 'DotNETPerfItems=' */*/*.xml | grep -v '"0"'

endlocal
@echo on

::
:: Copyright (c) 2001, 2023 Ricoh Company, Ltd.
:: All rights reserved.
::

