
set PATH=%PATH%;"C:\Users\mwybrow\AppData\Local\Programs\Git\bin"

git show --abbrev-commit | grep "^commit" | cut -f2 -d' ' > value
COPY /Y .init+value initvar.bat
call initvar.bat 

echo #define GITHASH "%GITVER%" > %1
DEL /Q value initvar.bat
