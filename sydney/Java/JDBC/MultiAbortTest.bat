@echo off
echo ant testcompile
call ant testcompile
echo start sydney service - 1
net start sydserver
echo execute java ManyClientsAbort (about 30 seconds...)
%JAVA_HOME%\bin\java -classpath "../../JDBC/lib/sydney.jar;build" ManyClientsAbort
echo stop sydney service - 1
net stop sydserver
echo start sydney service - 2
net start sydserver
echo drop database TEST
pushd bin
call sqli.bat -remote localhost 54321 -sql "drop database TEST"
popd
echo stop sydney service - 2
net stop sydserver
