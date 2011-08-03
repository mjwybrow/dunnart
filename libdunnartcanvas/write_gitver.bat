
set PATH=%PATH%;"C:\Program Files\Git\bin"

git show --abbrev-commit | grep "^commit" | cut -f2 -d' ' > value
COPY /Y .init+value initvar.bat
initvar.bat 

echo #define GITHASH "%GITVER%" > %1
DEL /Q value initvar.bat
