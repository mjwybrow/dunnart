
QT           += xml svg
TEMPLATE      = lib
CONFIG       += qt plugin
TARGET        = $$qtLibraryTarget(fileio_graphviz)

include(../../../common_options.qmake)
include(../fileio_plugin_options.pri)


CONFIG += link_pkgconfig
PKGCONFIG += libcgraph


HEADERS       =
SOURCES       = plugin.cpp

