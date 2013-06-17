
QT_CONFIG -= no-pkg-config

TEMPLATE = subdirs

SUBDIRS = builtinsvg \
            builtingml \
            builtinlayout

packagesExist(libcgraph) {
    message("Has libcgraph")
    SUBDIRS += graphviz
}
else {
    message("No libcgraph")
}

CONFIG += ordered
