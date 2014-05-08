#!/bin/sh -x

MACPORTS=/opt/local

# Qt SDK path.  Also need to update editor/package_and_upload_osx.sh file.
QT=$HOME/Qt5.2.1/5.2.1/clang_64

export PATH=$QT/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin:$MACPORTS/bin

cd $HOME/build/dunnart

rm -f  dunnart-osx.tar.gz

git pull

./qmake.sh clang nightlybuild
#qmake dunnart.pro -r -config release -spec win32-g++

make

if [ -f Dunnart.app/Contents/MacOS/Dunnart ]
then	
	cd editor

	make dunnart-osx.tar.gz
fi
