#!/bin/sh

# macx-llvm
# macx-g++42


if [ "$1" == "clang" ]; then
	qmake -spec macx-clang-libc++ -recursive -config "$2 usemactoolbar" *.pro
else
	qmake -spec macx-g++ -recursive -config "$@" *.pro
fi


