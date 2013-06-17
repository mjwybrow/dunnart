
QT           += xml svg
TEMPLATE      = lib
CONFIG       += qt plugin
TARGET        = $$qtLibraryTarget(org.dunnart.SBGNShapesPlugin)

include(../../../common_options.qmake)
include(../shape_plugin_options.pri)

HEADERS       = pdepn.h \
    pdunspecifiedepn.h \
    pdsourcesink.h \
    pdsimplechemepn.h \
    pdmacromolepn.h \
    pdnucleicepn.h \
    pdcomplexepn.h \
    pdperturbingepn.h \
    pdphenotypeprocessnode.h \
    pdlogicaloperators.h \
    pdprocessnode.h
SOURCES       = pdepn.cpp \
    plugin.cpp \
    pdunspecifiedepn.cpp \
    pdsourcesink.cpp \
    pdsimplechemepn.cpp \
    pdmacromolepn.cpp \
    pdnucleicepn.cpp \
    pdcomplexepn.cpp \
    pdperturbingepn.cpp \
    pdphenotypeprocessnode.cpp \
    pdlogicaloperators.cpp \
    pdprocessnode.cpp
