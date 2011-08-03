
QT           += xml svg
TEMPLATE      = lib
CONFIG       += qt plugin
TARGET        = $$qtLibraryTarget(shapes_basic)

include(../../../common_options.qmake)
include(../shape_plugin_options.pri)

HEADERS       = \
    diamond.h \
    ellipse.h \
    userinteraction.h \
    inputoutput.h
SOURCES       = plugin.cpp \
    diamond.cpp \
    ellipse.cpp \
    userinteraction.cpp \
    inputoutput.cpp

