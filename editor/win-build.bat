set PATH=C:\QtSDK\mingw\bin;C:\QtSDK\Desktop\Qt\4.8.1\mingw\bin;AppData\Local\Programs\Git\bin;%SystemRoot%\system32;%SystemRoot%;%SystemRoot%\System32\Wbem;%SYSTEMROOT%\System32\WindowsPowerShell\v1.0\;C:\Program Files (x86)\Microsoft Application Virtualization Client;C:\Program Files (x86)\QuickTime\QTSystem\;C:\Program Files (x86)\Google\Google Apps Sync\;C:\Program Files (x86)\PuTTY

cd \ws-git\dunnart\editor

pageant.exe generator-private.ppk

del /q dunnart-win.zip

cd ..


set GIT_SSH=C:\Program Files (x86)\PuTTY\plink.exe

git pull

qmake dunnart.pro -r -config nightlybuild -spec win32-g++

mingw32-make

cd editor

IF EXIST ..\build\Dunnart.exe mingw32-make dunnart-win.zip

