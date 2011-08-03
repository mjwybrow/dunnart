
TEMPLATE = lib
TARGET = vpsc

CONFIG += shared

DEPENDPATH += ..
INCLUDEPATH += ..

include(../common_options.qmake)
CONFIG -= qt

# Input
SOURCES += block.cpp blocks.cpp rectangle.cpp solve_VPSC.cpp constraint.cpp variable.cpp
HEADERS += block.h blocks.h cbuffer.h constraint.h csolve_VPSC.h exceptions.h isnan.h linesegment.h mosek_quad_solve.h pairing_heap.h placement_SolveVPSC.h rectangle.h removeoverlap.h solve_VPSC.h variable.h assertions.h

