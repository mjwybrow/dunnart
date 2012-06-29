
TEMPLATE = lib
TARGET = avoid

CONFIG += shared

INCLUDEPATH += ..
DEPENDPATH += ..

include(../common_options.qmake)
CONFIG -= qt

# Input
SOURCES += connector.cpp geometry.cpp geomtypes.cpp graph.cpp makepath.cpp orthogonal.cpp router.cpp shape.cpp timer.cpp vertices.cpp viscluster.cpp visibility.cpp vpsc.cpp connend.cpp connectionpin.cpp junction.cpp obstacle.cpp \
    mtst.cpp \
    hyperedge.cpp \
    hyperedgetree.cpp \
    scanline.cpp
HEADERS += assertions.h connector.h debug.h geometry.h geomtypes.h graph.h libavoid.h makepath.h orthogonal.h router.h shape.h timer.h vertices.h viscluster.h visibility.h vpsc.h connend.h connectionpin.h junction.h obstacle.h \
    mtst.h \
    hyperedge.h \
    hyperedgetree.h \
    scanline.h
