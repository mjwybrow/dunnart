
QT           += xml svg network
TEMPLATE      = lib
CONFIG       += qt plugin
TARGET        = $$qtLibraryTarget(application_garuda)

include(../../../common_options.qmake)
include(../application_plugin_options.pri)

HEADERS       = \
    appgarudaclient.h \
    openincompatiblesoftwarewidget.h \
    qt-json/json.h
SOURCES       = plugin.cpp \
    appgarudaclient.cpp \
    openincompatiblesoftwarewidget.cpp \
    qt-json/json.cpp

FORMS += \
    openincompatiblesoftwarewidget.ui

