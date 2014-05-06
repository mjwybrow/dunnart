TEMPLATE = lib
TARGET = cola
CONFIG += shared
DEPENDPATH += .. .
INCLUDEPATH += .. \
    ../libvpsc
include(../common_options.qmake)
CONFIG -= qt

#LIBS += -Wl,-undefined -Wl,dynamic_lookup

LIBS += -L$$DESTDIR -lvpsc

# Input
SOURCES += cola.cpp \
    colafd.cpp \
    conjugate_gradient.cpp \
    gradient_projection.cpp \
    straightener.cpp \
    cycle_detector.cpp \
    connected_components.cpp \
    convex_hull.cpp \
    cluster.cpp \
    compound_constraints.cpp \
    max_acyclic_subgraph.cpp \
    output_svg.cpp \
    cc_clustercontainmentconstraints.cpp \
    cc_nonoverlapconstraints.cpp \
    box.cpp
HEADERS += cola.h \
    cluster.h \
    commondefs.h \
    compound_constraints.h \
    gradient_projection.h \
    max_acyclic_subgraph.h \
    sparse_matrix.h \
    straightener.h \
    output_svg.h \
    shortest_paths.h \
    cc_clustercontainmentconstraints.h \
    cc_nonoverlapconstraints.h \
    unused.h \
    box.h \
    config.h
