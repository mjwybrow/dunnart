copy C:\Qt\Qt5.1.0\Tools\mingw48_32\bin\libgcc_s_dw2-1.dll ..\build

copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\QtCore4.dll ..\build
copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\QtGui4.dll ..\build
copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\QtSvg4.dll ..\build
copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\QtXml4.dll ..\build

xcopy /y C:\Qt\Qt5.1.0\5.1.0\mingw48_32\plugins\imageformats ..\build\plugins\imageformats\
xcopy /y C:\Qt\Qt5.1.0\5.1.0\mingw48_32\plugins\iconengines ..\build\plugins\iconengines\

del /q ..\build\*.a

"C:\Program Files\7-Zip\7z.exe" a dunnart-win.zip ..\build ..\examples

pscp -i generator-private.ppk dunnart-win.zip mwybrow@bruce.infotech.monash.edu.au:./WWW/dunnart/nightlies/

