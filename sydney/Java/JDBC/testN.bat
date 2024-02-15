@echo off
setlocal

set cnt=1
if '%1'=='' goto START
set cnt=%1
:START
FOR /L %%i IN (1,1,%cnt%) DO call ant %2test
