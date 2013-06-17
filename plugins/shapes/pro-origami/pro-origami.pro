
QT           += xml svg
TEMPLATE      = lib
CONFIG       += qt plugin
TARGET        = $$qtLibraryTarget(org.dunnart.ProOrigamiShapesPlugin)

include(../../../common_options.qmake)
include(../shape_plugin_options.pri)

HEADERS       = strand.h helix.h
SOURCES       = strand.cpp helix.cpp plugin.cpp

