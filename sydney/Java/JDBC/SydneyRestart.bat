@echo off
echo ant testcompile
call ant testcompile
net start sydserver
%JAVA_HOME%\bin\java -classpath "../../JDBC/lib/sydney.jar;build" SydneyRestart
net stop sydserver
