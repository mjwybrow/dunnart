
TEMPLATE = lib
TARGET = dunnartcanvas

CONFIG += shared

DEPENDPATH +=  .. ../libogdf .
INCLUDEPATH += .. ../libogdf .

CONFIG += link_pkgconfig qt thread
QT += xml svg
PKGCONFIG += 

include(../common_options.qmake)

include(qtpropertybrowser/qtpropertybrowser.pri)

graphviz {
	PKGCONFIG += libgvc
}
!graphviz {
	DEFINES += NOGRAPHVIZ
}

win32 {
LIBS += -Wl,--export-all-symbols -Wl,--no-whole-archive
}
LIBS += -L${DESTDIR} -lavoid -lvpsc -ltopology -lcola -logdf

# Input
RESOURCES += \
	libdunnartcanvas.qrc
FORMS = \
	ui/LayoutProperties.ui \
	ui/ZoomLevel.ui \
	ui/createalignment.ui \
	ui/createdistribution.ui \
	ui/createseparation.ui \
	ui/connectorproperties.ui \
    ui/createtemplate.ui \
    ui/shapepickerdialog.ui
SOURCES += \ 
	FMMLayout.cpp \
	align3.cpp \
	oldcanvas.cpp \
	canvasitem.cpp \
	cluster.cpp \
	connector.cpp \
	distribution.cpp \
	expand_grid.cpp \
	freehand.cpp \
	graphdata.cpp \
	graphlayout.cpp \
	graphvizlayout.cpp \
	guideline.cpp \
	indicator.cpp \
	interferencegraph.cpp \
	nearestpoint.cpp \
	placement.cpp \
	polygon.cpp \
	separation.cpp \
	shape.cpp \
	textshape.cpp \
	undo.cpp \
	visibility.cpp \
	canvas.cpp \
	canvasview.cpp \
	utility.cpp \
	gmlgraph.cpp \
	template-constraints.cpp \
	templates.cpp \
	relationship.cpp \
	handle.cpp \
	svgshape.cpp \
	canvastabwidget.cpp \
	ui/layoutproperties.cpp \
	ui/zoomlevel.cpp \
	ui/createalignment.cpp \
	ui/createtemplate.cpp \
	ui/createdistribution.cpp \
	ui/createseparation.cpp \
	ui/connectorproperties.cpp \
	ui/propertieseditor.cpp \
    pluginshapefactory.cpp \
    ui/shapepickerdialog.cpp \
    connectorhandles.cpp \
    ui/undohistorydialog.cpp
HEADERS += \
	FMMLayout.h \
	align3.h \
	oldcanvas.h \
	canvasitem.h \
	cluster.h \
	connector.h \
	distribution.h \
	expand_grid.h \
	freehand.h \
	gmlgraph.h \
	graphdata.h \
	graphlayout.h \
	graphvizlayout.h \
	guideline.h \
	indicator.h \
	instrument.h \
	interferencegraph.h \
	nearestpoint.h \
	placement.h \
	polygon.h \
	separation.h \
	shape.h \
	shared.h \
	textshape.h \
	undo.h \
	visibility.h \
	canvas.h \
	canvasview.h \
	template-constraints.h \
	templates.h \
	utility.h \
	githash.h \
	relationship.h \
	handle.h \
	svgshape.h \
	canvastabwidget.h \
	ui/layoutproperties.h \
	ui/zoomlevel.h \
	ui/createalignment.h \
	ui/createtemplate.h \
	ui/createdistribution.h \
	ui/createseparation.h \
	ui/connectorproperties.h \
	ui/propertieseditor.h \
    pluginshapefactory.h \
    ui/shapepickerdialog.h \
    connectorhandles.h \
    ui/undohistorydialog.h


githash.target = githash.h
win32 {
githash.commands = write_gitver.bat $$githash.target
}
else {
githash.commands = @ver=`git show --abbrev-commit | grep "^commit" | cut -f2 -d\' \'`; echo \"`cat .hash`define GITHASH \\\"\$\$ver\\\"\" > $$githash.target;
}
githash.depends = dummy

QMAKE_EXTRA_TARGETS += githash dummy


PRE_TARGETDEPS += githash.h
