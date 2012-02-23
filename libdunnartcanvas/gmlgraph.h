/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2007-2008  Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA  02110-1301, USA.
 * 
 * 
 * Author(s): Tim Dwyer
 *            Michael Wybrow  <http://michael.wybrow.info/>
*/

#ifndef GMLGRAPH_H
#define GMLGRAPH_H

#include <QDockWidget>
#include <QLabel>
#include <QPainter>
#include <QColor>

#include <map>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/GraphAttributes.h>

#include "libdunnartcanvas/canvasitem.h"

namespace dunnart {


class Canvas;
class ShapeObj;
class Connector;

namespace gml {

struct Dim {
    double w, h;
    Dim(double w, double h);
};
struct Box {
    double x, y, X, Y;
    Box(double x, double y, double X, double Y);
    Box(double x, double y, const Dim dim);
    void grow(const Box);
    double width() const;
    double height() const;
    void getCentre(double& x, double& y);
    bool overlaps(const Box) const;
};

struct Page {
    Canvas *canvas;
    Page(Canvas *canvas);
    void set(const Box b);
};

struct COff {
    int &x, &y;
    COff(int& x, int& y);
    Box getView(int w, int h);
    void set(int x, int y);
};
struct Draw {
    Draw(ogdf::Graph& G, ogdf::GraphAttributes& GA, QPixmap *pixmap,
            const Box& bounds);
    void box(const Box b);
    void line(double x1, double y1, double x2, double y2);
    void circle(double x, double y, int r);
    void polygon(double *dvx, double *dvy, int n);
    int x(double gx);
    int y(double gy);
    QPixmap* surface_;
    QPainter painter;
    const double sw, sh;
    QColor colour;
    Box bounds;
};


#include <list>
typedef std::list<ogdf::node > NodeList;

class Graph {
public:
    Graph(Canvas *canvas, std::string gmlFile, Page page, COff coff);
    void relayoutOverview();
    unsigned getCanvasShapesCount() const;
    void expandNeighbours(ShapeObj* shape);
    void expandNeighbours(double x, double y);
    void createClusters();
    void createShape(ogdf::node v);
    void createShape(ogdf::node v, const Box);
    void storeClusterInfo(ogdf::node v);
    void createConnector(ogdf::edge e);
    void drawConnectorGhosts(QPixmap *image, int offx, int offy);
    void drawOverviewOverlay();
    Canvas *canvas(void) const;
protected:
    QColor getNodeColor(const ogdf::node v);
    void expandNeighbours(ogdf::node centre);
    void layout();
    void layoutVisible(Box& view);
    void drawOverview();
    void restartLayout(ogdf::node centre);
    bool isVisible(ogdf::node v, const Box& view);
    Canvas *canvas_;
    std::string gmlFileName;
    Page page;
    COff canvasOffset;
    ogdf::Graph G;
    ogdf::GraphAttributes GA;
    ogdf::NodeArray<ShapeObj*> shapes;
    ogdf::NodeArray<int> timeStamps;
    ogdf::EdgeArray<Connector*> connectors;
    ogdf::EdgeArray<int> edgeLengths;
    ogdf::node startNode;
    Box gbounds;
    double scale;
    std::map<ShapeObj*,ogdf::node> nodes;
    std::map<std::string,NodeList > canvasClustersMap;
    CanvasItemList canvasClusters;
    std::list<ShapeObj*> canvasShapes;
    const unsigned canvasShapesLimit;
    int time;
    int **shortestPathsMatrix;
    bool UML;
    bool UseClusters;
private:
    double dist(const ogdf::node v, const double cx, const double cy) const;
};

} // namespace gml

}
#endif // GMLGRAPH_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

