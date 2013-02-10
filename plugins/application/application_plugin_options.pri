INCLUDEPATH  += . $$DUNNARTBASE $$DUNNARTBASE/libdunnartcanvas
DEPENDPATH  += . $$DUNNARTBASE $$DUNNARTBASE/libdunnartcanvas

LIBDESTDIR = $$DUNNARTBASE/build 
DESTDIR       = $$DUNNARTBASE/build/plugins
RESOURCESDIR  = $$DUNNARTBASE/build

macx {
!arcadia {

LIBDESTDIR = $$DUNNARTBASE/Dunnart.app/Contents/Frameworks
DESTDIR       = $$DUNNARTBASE/Dunnart.app/Contents/PlugIns
RESOURCESDIR  = $$DUNNARTBASE/Dunnart.app/Contents/Resources

}
}
LIBS += -L$$LIBDESTDIR -ldunnartcanvas

