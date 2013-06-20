#!/bin/sh -x

MACPORTS=/opt/local
QT=$HOME/Qt5.1.0/5.1.0-rc1/clang_64

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
