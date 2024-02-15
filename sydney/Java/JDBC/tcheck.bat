@echo off

setlocal

rem テンポラリファイル名
set TMP_LOG=tcheck_tmp.log

rem ログファイル名が指定されていなければ USAGE 出力
if '%1'=='' goto USAGE

rem ログファイルが存在しなければエラーメッセージ出力
if not exist %1 goto NOTHING

rem テンポラリファイルが存在すれば削除
if exist %TMP_LOG% del %TMP_LOG%

rem ひとつもテスト結果がログファイル内に出力されていなければ
rem そのログファイルは不正としてエラーメッセージ出力
grep "Errors: 0" %1 > %TMP_LOG% 2>&1
if errorlevel 1 goto ILLEGAL_LOG

rem テスト結果判定
grep "Failures: [1-9]" %1
set NOT_FAILURES=%errorlevel%
grep "Errors: [1-9]" %1
set NOT_ERRORS=%errorlevel%
if '%NOT_FAILURES%'=='0' goto END
if '%NOT_ERRORS%'=='0' goto END

rem すべてのエラーが正常終了
echo No error

goto END

:NOTHING
echo No such file %1

goto END

:ILLEGAL_LOG
echo Illegal log file %1

goto END

:USAGE
echo [USAGE] tcheck.bat log
echo             log ... ant test log file name

:END

if exist %TMP_LOG% del %TMP_LOG%
