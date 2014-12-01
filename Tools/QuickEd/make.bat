set OLDPATH=%PATH%
set PATH=%PATH:cygwin=bin%
make.exe %1 %2 %3
set PATH=%OLDPATH%