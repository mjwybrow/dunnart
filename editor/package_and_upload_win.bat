copy C:\QtSDK\mingw\bin\libgcc_s_dw2-1.dll ..\build
copy C:\QtSDK\mingw\bin\mingwm10.dll ..\build

copy C:\QtSDK\Desktop\Qt\4.7.2\mingw\bin\QtCore4.dll ..\build
copy C:\QtSDK\Desktop\Qt\4.7.2\mingw\bin\QtGui4.dll ..\build
copy C:\QtSDK\Desktop\Qt\4.7.2\mingw\bin\QtSvg4.dll ..\build
copy C:\QtSDK\Desktop\Qt\4.7.2\mingw\bin\QtXml4.dll ..\build

xcopy /y C:\QtSDK\Desktop\Qt\4.7.2\mingw\plugins\imageformats ..\build\plugins\imageformats\
xcopy /y C:\QtSDK\Desktop\Qt\4.7.2\mingw\plugins\iconengines ..\build\plugins\iconengines\

del /q ..\build\*.a

"C:\Program Files\7-Zip\7z.exe" a dunnart-win.zip ..\build ..\examples

pscp -i generator-private.ppk dunnart-win.zip mwybrow@bruce.infotech.monash.edu.au:./WWW/dunnart/nightlies/

