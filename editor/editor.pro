
TEMPLATE = app
DEPENDPATH =  .. .
INCLUDEPATH = .. ../libvpsc ../libcola ../libtopology ../libproject \
	../libavoid ../libogdf ../libdunnartcanvas ../libdunnartcanvas/qtpropertybrowser
DEPENDPATH += ..
TARGET = Dunnart

CONFIG += link_pkgconfig qt thread warn_off
QT += xml svg
PKGCONFIG += 

include(../common_options.qmake)

DESTDIR = $$DUNNARTBASE/build

arcadia {

# Prevent the app from being packaged as an OSX app bundle.
CONFIG -= app_bundle

LIBS += -L${DESTDIR}

}
else:macx {

DESTDIR = $$DUNNARTBASE
RC_FILE = Dunnart.icns

QMAKE_INFO_PLIST = Dunnart_Info.plist

LIBS += -L../Dunnart.app/Contents/Frameworks

}
else {

LIBS += -L${DESTDIR}

}

LIBS += -ldunnartcanvas

# The linker on OS X Tiger requires that we resupply these.
LIBS += -ltopology -lcola -lvpsc -logdf -lavoid

win32 {
CONFIG += windows

#QMAKE_POST_LINK  = "copy_win32_deps.bat ${DESTDIR}"

QMAKE_CLEAN = dunnart-win.zip

dunnart-win.zip.target = dunnart-win.zip
dunnart-win.zip.commands = package_and_upload_win.bat
dunnart-win.zip.depends = $$DESTDIR_TARGET

#POST_TARGETDEPS += dunnart-win.zip
QMAKE_EXTRA_TARGETS += dunnart-win.zip
}

macx  {

QMAKE_CLEAN = dunnart-osx.tar.gz

dunnart-osx.tar.gz.target = dunnart-osx.tar.gz
dunnart-osx.tar.gz.commands = ./package_and_upload_osx.sh
dunnart-osx.tar.gz.depends = $$DESTDIR_TARGET

QMAKE_EXTRA_TARGETS += dunnart-osx.tar.gz

}

linux-g++ {

EXTRA_BINFILES += Dunnart.sh
for(FILE,EXTRA_BINFILES){
    QMAKE_POST_LINK += $$quote(cp $${FILE} $${DESTDIR}$$escape_expand(\n\t))
}

QMAKE_CLEAN = dunnart-linux.tar.gz

dunnart-linux.tar.gz.target = dunnart-linux.tar.gz
dunnart-linux.tar.gz.commands = ./package_and_upload_linux.sh
dunnart-linux.tar.gz.depends = $$DESTDIR_TARGET

QMAKE_EXTRA_TARGETS += dunnart-linux.tar.gz

}


# Input
SOURCES += \ 
	editor.cpp \
	getopt.cpp \
	mainwindow.cpp \
    application.cpp
HEADERS += \
	getopt.h \
	mainwindow.h \
    application.h
