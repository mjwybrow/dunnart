INCLUDEPATH  += . $$DUNNARTBASE $$DUNNARTBASE/libdunnartcanvas
DEPENDPATH  += . $$DUNNARTBASE $$DUNNARTBASE/libdunnartcanvas

LIBDESTDIR = $$DUNNARTBASE/build 
DESTDIR       = $$DUNNARTBASE/build/plugins

macx {
!arcadia {

LIBDESTDIR = $$DUNNARTBASE/Dunnart.app/Contents/Frameworks
DESTDIR       = $$DUNNARTBASE/Dunnart.app/Contents/PlugIns

}
}
LIBS += -L$$LIBDESTDIR -ldunnartcanvas

