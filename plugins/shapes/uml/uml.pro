
QT           += xml svg
TEMPLATE      = lib
CONFIG       += qt plugin
TARGET        = $$qtLibraryTarget(shapes_uml)

include(../../../common_options.qmake)
include(../shape_plugin_options.pri)

HEADERS       = umlclass.h
SOURCES       = plugin.cpp umlclass.cpp
