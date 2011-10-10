#!/bin/sh -x

MACPORTS=/opt/local-macports-with-a-really-very-quite-long-directory-name
QT=/usr/local/Qt4.7

export PATH=$QT/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin:$MACPORTS/bin

cd /Users/mjwybrow/ws-git/dunnart

rm -f  dunnart-osx.tar.gz

git pull

./qmake.sh nightlybuild
#qmake dunnart.pro -r -config release -spec win32-g++

make

if [ -f Dunnart.app/Contents/MacOS/Dunnart ]
then	
	cd editor

	make dunnart-osx.tar.gz
fi
