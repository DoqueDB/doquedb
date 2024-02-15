@echo off
setlocal

REM 
REM PHPをマウントして使う
REM

REM テスト環境
net use T: /delete
net use T: \\helios\sydney\data\dmtester\ProgramFiles\TEST
call conf.bat
set path=%installpath%;T:\PHP5;%path%
set TEST_PHP_EXECUTABLE=T:\php5\php.exe

echo "Server_PasswordFilePath=Sydney"
set regfile=Server_PasswordFilePath.reg
echo REGEDIT4 > %regfile%
echo. >> %regfile%
echo [HKEY_LOCAL_MACHINE\SOFTWARE\RICOH\TRMeister] >> %regfile%
echo "Server_PasswordFilePath"="Sydney" >> %regfile%
%systempath%\regedit /s %regfile%

REM PDOコピー
rmdir /s /q T:\TESTPDO
mkdir T:\TESTPDO
copy ..\..\..\PHP\PDO\c.O7\php_pdo_trmeister.dll T:\TESTPDO

REM メイン
net start sydserver
pushd ..\..\..\ModuleTest\PHP\PDO\UnitTest
call unittest.bat
cd ..\PDO_TEST
php -d open_basedir= -d safe_mode=0 -d output_buffering=0 run-tests.php trmeister_tests
popd
net stop sydserver

net use T: /delete

endlocal
@echo on
