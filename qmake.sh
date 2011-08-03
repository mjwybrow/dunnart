#!/bin/sh

# macx-llvm
# macx-g++42
# ~/mkspecs/unsupported/macx-clang

if [ "$1" == "clang" ]; then

	qmake -spec ~/mkspecs/unsupported/macx-clang -recursive -config "$2" *.pro
	(
	cd libogdf
	qmake -spec macx-g++42 -recursive -config "$2" *.pro
	)
else
	qmake -spec macx-g++42 -recursive -config "$@" *.pro
fi


