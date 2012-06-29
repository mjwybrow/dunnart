
TEMPLATE = lib
TARGET = topology

CONFIG += shared


INCLUDEPATH += .. .
INCLUDEPATH += .. ../libvpsc ../libavoid ../libcola

include(../common_options.qmake)
CONFIG -= qt

LIBS += -L$$DESTDIR -lvpsc -lcola -lavoid

# Input
SOURCES += \
	topology_constraints_constructor.cpp \
	topology_graph.cpp \
	topology_constraints.cpp \
	compute_forces.cpp \
	resize.cpp \
	cola_topology_addon.cpp \
	orthogonal_topology.cpp

HEADERS += \
	topology_constraints.h \
	topology_graph.h \
	topology_log.h \
	util.h \
	cola_topology_addon.h \
	orthogonal_topology.h

