@echo off
setlocal enabledelayedexpansion

REM 子サーバのセットアップ
REM 1. 子サーバの設定ファイルからレジストリパスを取得する
REM 2. 子サーバのレジストリ情報を初期化したレジストリファイルを生成し、システムに取り込む
REM 3. 子サーバのDBを初期化する

set conf_file=cascade_for_windows.conf

if "%1" == "" (
  if not exist "%conf_file%" (
    echo Default File not found: %conf_file%
    echo Usage: %0 [cascade conf file]
    goto :END
  )
) else (
  if not exist "%1" (
    echo File not found: %1
    goto :END
  ) else (
    set conf_file=%1
  )
)

set systempath=%SystemRoot%

REM 子サーバの設定ファイル中で、レジストリパスが書かれている項目名
set RegPathPropertyName=ConfPath

set /A CascadeID=0

REM *************************************** メインループ ************************************************
REM 子サーバの設定ファイルを一行ずつ読み込む
for /f "tokens=1* delims=-: " %%a in (%conf_file%) do (
  REM 退避用のレジストリファイル
  set RegFile=
  REM レジストリが書き込まれているシステム上のパス
  set RegPath=

  if '%%a' == '%RegPathPropertyName%' (
    set RegPath=%%b
    set RegFile=cascade!CascadeID!_default.reg
  )

  if not '!RegPath!' == '' (
    if exist !RegFile! (
      move /y !RegFile! %TEMP%
      @echo Existing !RegFile! was moved to %TEMP%.
    )
    echo REGEDIT4 > !RegFile!
    echo. >> !RegFile!
    echo [!RegPath!] >> !RegFile!
    REM 子サーバはユーザ認証を無効化する必要がある
    echo "Server_PasswordFilePath"="Sydney" >> !RegFile!
    REM 子サーバのSydTestメッセージストリームを設定する
    echo "SydTest_MessageOutputError"="1" >> !RegFile!
    echo "SydTest_MessageOutputInfo"="1" >> !RegFile!
    echo "SydTest_MessageOutputTime"="1" >> !RegFile!
    REM 再起動までデフォルトで５分からの変更
    echo "Admin_ReplicatorWaitTime"="1000" >> !RegFile!
    %systempath%\regedit /s !RegFile!
    del !RegFile!
    %systempath%\regedit /e !RegFile! !RegPath!

    set /a CascadeID+=1
  )
)
REM ************************************** メインループここまで ********************************************

:END

endlocal
@echo on
