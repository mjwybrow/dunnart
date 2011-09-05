
QT           += xml svg
TEMPLATE      = lib
CONFIG       += qt plugin
TARGET        = $$qtLibraryTarget(shapes_basic)

include(../../../common_options.qmake)
include(../shape_plugin_options.pri)

HEADERS       = \
    diamond.h \
    roundedrect.h
SOURCES       = plugin.cpp \
    diamond.cpp \
    roundedrect.cpp

