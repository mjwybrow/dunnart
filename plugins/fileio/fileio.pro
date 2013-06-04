

TEMPLATE = subdirs

SUBDIRS = builtinsvg \
            builtingml \
            builtinlayout

packagesExist(libcgraph) {
    SUBDIRS += graphviz
}

CONFIG += ordered
