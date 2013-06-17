
QT           += xml svg
TEMPLATE      = lib
CONFIG       += qt plugin
TARGET        = $$qtLibraryTarget(org.dunnart.BasicShapesPlugin)

include(../../../common_options.qmake)
include(../shape_plugin_options.pri)

HEADERS       = \
    diamond.h \
    roundedrect.h \
    ellipse.h
SOURCES       = plugin.cpp \
    diamond.cpp \
    roundedrect.cpp \
    ellipse.cpp

