#!/bin/sh -x

export PATH=/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin

cd /home/mjwybrow/ws-git/dunnart

rm -f  dunnart-linux.tar.gz

git pull

qmake dunnart.pro -r -config release

make

cd editor

make dunnart-linux.tar.gz

