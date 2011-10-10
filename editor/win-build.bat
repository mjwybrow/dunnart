set PATH=C:\QtSDK\mingw\bin;C:\QtSDK\Desktop\Qt\4.7.2\mingw\bin;C:\Program Files\Git\bin;C:\Program Files\Parallels\Parallels Tools\Applications;C:\WINDOWS\system32;C:\WINDOWS;C:\WINDOWS\System32\Wbem

cd \ws-git\dunnart\editor

C:\WINDOWS\pageant.exe generator-private.ppk

del /q dunnart-win.zip

cd ..


set GIT_SSH=C:\WINDOWS\plink.exe

git pull

qmake dunnart.pro -r -config nightlybuild -spec win32-g++

mingw32-make

cd editor

IF EXIST ..\build\Dunnart.exe mingw32-make dunnart-win.zip

