
TEMPLATE = lib
TARGET = topology

CONFIG += shared


INCLUDEPATH += .. .
INCLUDEPATH += .. ../libvpsc

include(../common_options.qmake)
CONFIG -= qt

win32 {
LIBS += -Wl,--allow-shlib-undefined
}
else {
LIBS += -Wl,-undefined -Wl,dynamic_lookup
}
LIBS += -L$$DESTDIR -lvpsc

# Input
SOURCES += topology_constraints_constructor.cpp topology_graph.cpp topology_constraints.cpp compute_forces.cpp \
              resize.cpp
HEADERS += topology_constraints.h topology_graph.h topology_log.h util.h

