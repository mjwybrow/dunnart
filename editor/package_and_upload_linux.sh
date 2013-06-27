#!/bin/sh -x

export PATH=/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin

( 
cd ..

for QTLIB in Qt5Core Qt5Gui Qt5Network Qt5PrintSupport Qt5Svg Qt5Widgets Qt5Xml
do
	cp /usr/lib/i386-linux-gnu/lib$QTLIB.so.5 build/
done 


rm -rf dunnart-linux.tar.gz

tar zcf dunnart-linux.tar.gz build examples

scp dunnart-linux.tar.gz mwybrow@bruce.infotech.monash.edu.au:./WWW/dunnart/nightlies/

rm -rf build
)
