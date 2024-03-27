@echo off
setlocal
set TOOLSDIR=%~d0%~p0\..\..\..\..\common\tools\build
set MAKEFILE=%~d0%~p0\makefile_all.windows

if not "%*" == "" (nmake /F %MAKEFILE% TOOLSDIR="%TOOLSDIR%" TARGETS="%*" RECONF_TARGET="%1" all) else (nmake /F %MAKEFILE% TOOLSDIR="%TOOLSDIR%" all)
endlocal
@echo on
