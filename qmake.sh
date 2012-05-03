#!/bin/sh

# macx-llvm
# macx-g++42

if [ "$1" == "clang" ]; then

	qmake -spec unsupported/macx-clang -recursive -config "$2" *.pro
	
	# There is some clang linking bug that causes issues with OGDF,
	# so use GCC for that instead.
	(
	cd libogdf
	qmake -spec macx-g++42 -recursive -config "$2" *.pro
	)
else
	qmake -spec macx-g++42 -recursive -config "$@" *.pro
fi


