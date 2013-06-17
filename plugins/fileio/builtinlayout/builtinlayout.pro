
QT           += xml svg
TEMPLATE      = lib
CONFIG       += qt plugin
TARGET        = $$qtLibraryTarget(org.dunnart.BuiltinLayoutFileIOPlugin)

include(../../../common_options.qmake)
include(../fileio_plugin_options.pri)

HEADERS       =
SOURCES       = plugin.cpp

