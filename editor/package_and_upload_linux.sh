#!/bin/sh -x

export PATH=/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin

( 
cd ..

for QTLIB in QtCore QtGui QtSvg QtXml
do
	cp /usr/lib/lib$QTLIB.so.4 build/
done 


rm -rf dunnart-linux.tar.gz

tar zcf dunnart-linux.tar.gz build examples

scp dunnart-linux.tar.gz mwybrow@bruce.infotech.monash.edu.au:./WWW/dunnart/nightlies/

rm -rf build
)
