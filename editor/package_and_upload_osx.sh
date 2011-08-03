#!/bin/sh -x

MACPORTS=/opt/local-macports-with-a-really-very-quite-long-directory-name
QT=/usr/local/Qt4.7

export PATH=$QT/bin:/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin

( 
cd ..

macdeployqt Dunnart.app

for LIB in Dunnart.app/Contents/Frameworks/libdunnartcanvas.1.0.0.dylib Dunnart.app/Contents/PlugIns/*.dylib
do
	for QTLIB in QtCore QtGui QtSvg QtXml
	do
		/usr/bin/install_name_tool -change $QT/lib/$QTLIB.framework/Versions/4/$QTLIB @executable_path/../Frameworks/$QTLIB.framework/Versions/4/$QTLIB $LIB
	done 
done


rm -rf dunnart-osx.tar.gz

tar zcf dunnart-osx.tar.gz Dunnart.app examples

scp dunnart-osx.tar.gz mwybrow@bruce.infotech.monash.edu.au:./WWW/dunnart/nightlies/

rm -rf Dunnart.app
)
