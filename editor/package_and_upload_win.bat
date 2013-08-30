copy C:\Qt\Qt5.1.0\Tools\mingw48_32\bin\libgcc_s_dw2-1.dll ..\build

copy "C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\libwinpthread-1.dll" ..\build
copy "C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\libstdc++-6.dll" ..\build
copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\icuin51.dll ..\build
copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\icudt51.dll ..\build
copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\icuuc51.dll ..\build
copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\Qt5Core.dll ..\build
copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\Qt5Widgets.dll ..\build
copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\Qt5Gui.dll ..\build
copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\Qt5Svg.dll ..\build
copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\Qt5Xml.dll ..\build
copy C:\Qt\Qt5.1.0\5.1.0\mingw48_32\bin\Qt5PrintSupport.dll ..\build

xcopy /y C:\Qt\Qt5.1.0\5.1.0\mingw48_32\plugins\imageformats ..\build\plugins\imageformats\
xcopy /y C:\Qt\Qt5.1.0\5.1.0\mingw48_32\plugins\iconengines ..\build\plugins\iconengines\

del /q ..\build\*.a

"C:\Program Files\7-Zip\7z.exe" a dunnart-win.zip ..\build ..\examples

pscp -i generator-private.ppk dunnart-win.zip mwybrow@bruce.infotech.monash.edu.au:./WWW/dunnart/nightlies/

