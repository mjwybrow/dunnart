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
#include <QtGlobal>
#include <vector>
#include <algorithm>
#include <string>
//#include <tr1/functional>
#include <functional>
#include <iostream>

#include <ogdf/fileformats/GmlParser.h>
#include <ogdf/energybased/FMMMLayout.h>
#include "libdunnartcanvas/FMMLayout.h"
#include "libdunnartcanvas/gmlgraph.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/cluster.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libcola/shortest_paths.h"
#include "libcola/convex_hull.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/pluginshapefactory.h"

namespace dunnart {

namespace gml {

using namespace std;

Dim::Dim(double w, double h) : w(w), h(h) {}
Box::Box(double x, double y, double X, double Y) : x(x), y(y), X(X), Y(Y) {
}
Box::Box(double x, double y, const Dim d) :
    x(x), y(y), X(x+d.w), Y(y+d.h) {}

void Box::grow(const Box n) {
    x=std::min(x,n.x); y=std::min(y,n.y);
    X=std::max(X,n.X); Y=std::max(Y,n.Y);
}

double Box::width() const {
    assert((X-x)>0);
    return X-x;
}

double Box::height() const {
    assert((Y-y)>0);
    return Y-y;
}

void Box::getCentre(double& cx, double& cy) {
    cx=x+width()/2.0;
    cy=y+height()/2.0;
}

bool intervalOverlap(double a, double b, double c, double d) {
  return std::max(a, c) < std::min(b, d);
  //return (a>c&&a<d) || (b>c&&b<d) || (c>a&&c<b) || (d>a&&d<b);
}
bool Box::overlaps(const Box o) const {
    return intervalOverlap(x,X,o.x,o.X) && intervalOverlap(y,Y,o.y,o.Y);
}

Page::Page(Canvas *canvas) : canvas(canvas)
{ }

void Page::set(const Box n) {
    canvas->setPageRect(QRectF(n.x, n.y, n.width(), n.height())); 
}

COff::COff(int& x, int& y) : x(x), y(y) {}

void COff::set(int nx, int ny) {
    x = nx;
    y = ny;
}

Box COff::getView(int w, int h) {
    return Box(static_cast<double>(-x),static_cast<double>(-y),
               static_cast<double>(w-x),static_cast<double>(h-y));
}

static const QColor clusterColours[]={
    QColor(96,205,243,85),
    QColor(96,243,126,85),
    QColor(243,224,96,85),
    QColor(182,96,243,85),
    QColor(243,96,160,85),
    QColor(230,139,64,85)
};

const int clusterColoursN = sizeof(clusterColours) / sizeof(QColor);
 
Graph::Graph(Canvas *canvas, string gmlFile, Page page, COff canvasOffset)
    : canvas_(canvas),
      gmlFileName(gmlFile), 
      page(page), 
      canvasOffset(canvasOffset),
      GA(G), 
      shapes(G), 
      timeStamps(G), 
      connectors(G),
      edgeLengths(G), 
      gbounds(DBL_MAX,DBL_MAX,-DBL_MAX,-DBL_MAX),
      scale(2),
      canvasShapesLimit(30), 
      time(0),
      UML(true),
      UseClusters(canvas->useGmlClusters())
{
    canvas->setOptIdealEdgeLengthModifier(scale);
    canvas->setOptShapeNonoverlapPadding(4);
    GA.initAttributes(
            ogdf::GraphAttributes::nodeGraphics |
            ogdf::GraphAttributes::nodeLabel |
            ogdf::GraphAttributes::nodeType |
            ogdf::GraphAttributes::edgeType |
            ogdf::GraphAttributes::nodeTemplate |
            ogdf::GraphAttributes::nodeCluster |
            ogdf::GraphAttributes::nodeImageUrl);
    ifstream is(gmlFile.c_str());
    ogdf::GmlParser gml(is,true);
    if (gml.error()||!gml.read(G,GA)) {
        ogdf::String message = gml.errorString();
        int line = gml.getLineNumber();
        cerr << "ERROR READING GML FILE: " << gmlFile << endl;
        cerr << "Message: "<<message<<endl;
        cerr << "Line: "<<line<<endl;
        throw "FILE ERROR!";
    }
    ogdf::node v;
    shapes.fill(NULL);
    connectors.fill(NULL);
    edgeLengths.fill(1);
    timeStamps.fill(time);
    startNode=G.firstNode();
    forall_nodes(v,G) {
        shapes[v] = NULL;

        GA.x(v)*=scale; 
        GA.y(v)*=scale;
        GA.width(v)*=scale;
        GA.height(v)*=scale;
        string t(GA.templateNode(v).cstr());
        if(t.size()>=15 && t.substr(10,5)=="START") {
            startNode = v;
        }
        //printf("node %s is in cluster %s\n",GA.labelNode(v).cstr(),GA.clusterNode(v).cstr());
    }
    printf("Start node=%s\n",GA.labelNode(startNode).cstr());
    layout();
    forall_nodes(v,G) {
        gbounds.grow(Box(GA.x(v),GA.y(v),Dim(GA.width(v),GA.height(v))));
    }
    forall_nodes(v,G) {
        GA.x(v)-=gbounds.x;
        GA.y(v)-=gbounds.y;
    }
    gbounds.X-=gbounds.x;
    gbounds.Y-=gbounds.y;
    gbounds.x=0;
    gbounds.y=0;
    page.set(Box(0,0,Dim(gbounds.width(),gbounds.height())));

    // we need rubber band routing to preserve topology in makeFeasible
    canvas->setOptRubberBandRouting(true);

    // setup shortest paths
    unsigned n = G.numberOfNodes();
    shortestPathsMatrix = new int*[n];
    for(unsigned i=0;i<n;i++) {
        shortestPathsMatrix[i]=new int[n];
    }
    vector<cola::Edge> es;
    ogdf::edge e;
    forall_edges(e,G) {
        es.push_back(make_pair(e->source()->index(),e->target()->index()));
    }
    shortest_paths::johnsons(n,shortestPathsMatrix,es);
    canvas->setIdealConnectorLength(70);
}
Draw::Draw(ogdf::Graph& G, ogdf::GraphAttributes& GA, QPixmap *pixmap,
            const Box& bounds)
    : surface_(pixmap), 
      painter(surface_),
      sw(surface_->width()),
      sh(surface_->height()),
      colour(Qt::black),
      bounds(bounds)
{
    Q_UNUSED (G)
    Q_UNUSED (GA)
}
void Draw::polygon(double *dvx, double *dvy, int n) {
    QPointF *points = new QPointF[n];
    for (int i = 0; i < n; ++i)
    {
        points[i] = QPointF(dvx[i], dvy[i]);
    }
    painter.setPen(colour);
    painter.setBrush(QBrush(colour));
    painter.drawPolygon(points, n);
    delete [] points;
}
void Draw::box(const Box b) {
    painter.setPen(colour);
    painter.setBrush(QBrush(colour));
    painter.drawRect(QRectF(QPointF(x(b.x),y(b.y)), QPointF(x(b.X),y(b.Y))));
}
void Draw::line(double x1, double y1, double x2, double y2) {
    painter.setPen(colour);
    painter.drawLine(QPointF(x(x1),y(y1)), QPointF(x(x2),y(y2)));
}
void Draw::circle(double x1, double y1, int r) {
    painter.setPen(colour);
    painter.drawEllipse(QPointF(x(x1),y(y1)), r, r);
}
int Draw::x(double gx) {
    return static_cast<int>(sw*(gx - bounds.x)/bounds.width());
}
int Draw::y(double gy) {
    return static_cast<int>(sh*(gy - bounds.y)/bounds.height());
}
#if 0
void Graph::createOverviewWindow(QWidget** overviewDialog, int xtop, int ytop) {
    Q_UNUSED (overviewDialog)

    ow = new gml::OverviewWindow(xtop, ytop);
    //QT ow->setGlobalPointer(overviewDialog);
    int cx, cy, cw, ch;
    ow->getCanvasPosDim(cx,cy,cw,ch);
    selectOverview(cx, cy, false);
    expandNeighbours(startNode);
}
#endif

double Graph::dist(const ogdf::node v, const double cx, const double cy) const {
    double x=GA.x(v), y=GA.y(v);
    double dx=cx-x, dy=cy-y;
    return sqrt(dx*dx+dy*dy);
}


Canvas *Graph::canvas(void) const
{
    return canvas_;
}

struct LayoutCallback : public LayoutDoneCallback {
    LayoutCallback(Graph* g, bool beautify=false) : g(g), beautify(beautify) {
        printf("Creating gmlgraphexpl:LayoutCallback\n");
    }
    void notify() {
        GraphLayout* gl= g->canvas()->layout();
        if(!beautify) {
            printf("beautify...\n");
            delete layoutDoneCallback;
            layoutDoneCallback = new LayoutCallback(g,true);
            gl->topologyNodesCount = g->getCanvasShapesCount();
            g->createClusters();
            g->canvas()->interrupt_graph_layout();
        } else {
            printf("graphexpl: Layout done... updating overview...\n");
            g->relayoutOverview();
            gl->unlockAll(NULL);
            delete layoutDoneCallback;
            layoutDoneCallback = NULL;
        }
    }
    Graph* g;
    bool beautify;
};

unsigned Graph::getCanvasShapesCount() const {
    return canvasShapes.size();
}

#if 0
void Graph::drawConnectorGhosts(SDL_Surface *surface, int offx, int offy)
{
    int width = surface->w;
    int height = surface->h;
    /* The number of bytes used for every scanline. */
    int stride = width * 4;

    /* Free old pixel data and allocate new space */
    unsigned char *canvasData = (unsigned char *) calloc(stride * height, 1);

    /* Create a cairo surface for our allocated image. */
    /* We use the CAIRO_FORMAT_ARGB32 to support transparancy. */
    cairo_surface_t *cairo_surface =
            cairo_image_surface_create_for_data(canvasData,
                                            CAIRO_FORMAT_ARGB32, 
                                            width, height, stride);

    /* Create a cairo drawing context, normalize it and draw a clock. */
    /* Delete the context afterwards. */
    cairo_t *cr = cairo_create(cairo_surface);
    //cairo_scale(cr, surface->w, surface->h);
    
    //------------------------------------------
   
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(cr, 1);
 
    ogdf::edge e;
    forall_edges(e,G) {
        ogdf::node s=e->source(), t=e->target();
        ShapeObj *sS = shapes[s];
        ShapeObj *sT = shapes[t];
        if (((sS && !sS->isInactive() && !sS->isCollapsed()) && 
                    ((sT == NULL) || sT->isInactive())) ||
            ((sT && !sT->isInactive() && !sT->isCollapsed()) && 
                    ((sS == NULL) || sS->isInactive())))
        {
            double x1=GA.x(s), y1=GA.y(s),
                   x2=GA.x(t), y2=GA.y(t);
            double w1=GA.width(s), h1=GA.height(s),
                   w2=GA.width(t), h2=GA.height(t);
            if(sS && !sS->isInactive()) {
                sS->getPosAndSize(x1,y1,w1,h1);
            }
            if(sT && !sT->isInactive()) {
                sT->getPosAndSize(x2,y2,w2,h2);
            }
            //printf("%g %g %g %g\n", x1, y1, x2, y2);
            double iX1 = x1 + w1 /2;
            double iY1 = y1 + h1 / 2;
            double iX2 = x2 + w2 / 2;
            double iY2 = y2 + h2 / 2;
            //aalineRGBA(surface, iX1, iY1, iX2, iY2, 255, 128, 0, 128);
    
            bool srcIsActive = (sS && !sS->isInactive());
            double sX = (srcIsActive) ? iX1 : iX2;
            double sY = (srcIsActive) ? iY1 : iY2;
            double oSX = sX, oSY = sY;
            double tX = (srcIsActive) ? iX2 : iX1;
            double tY = (srcIsActive) ? iY2 : iY1;

            // Vector
            Avoid::Point vec(tX - oSX, tY - oSY);
            // Length is the dot product
            double length = sqrt(vec.x * vec.x + vec.y * vec.y);
            // Calculate unit vector
            Avoid::Point unitVec(vec.x / length, vec.y / length);
 
            // Point very far away for intersection
            Avoid::Point farVec(unitVec.x *1000, unitVec.y * 1000);
            // Translate back to find point
            double farX = sX + farVec.x;
            double farY = sY + farVec.y;
            ShapeObj *sShape = (srcIsActive) ? sS : sT;
            Avoid::Polygon *poly = sShape->poly(0);
            for (size_t i = 0; i < poly->size(); ++i)
            {
                if (segmentIntersectPoint(Avoid::Point(sX, sY), 
                            Avoid::Point(farX, farY), poly->ps[i], 
                            poly->ps[(i + 1) % poly->size()], &sX, &sY))
                {
                    break;
                }
            }
            delete poly;

            double segmentLength = 30;
            // Extend out
            Avoid::Point newVec(unitVec.x * segmentLength, unitVec.y * segmentLength);
            // Translate back to find point
            tX = sX + newVec.x;
            tY = sY + newVec.y;
            
            sX += offx;
            sY += offy;
            oSX += offx;
            oSY += offy;
            tX += offx;
            tY += offy;
            cairo_pattern_t *gradient = 
                cairo_pattern_create_radial(sX, sY, segmentLength/3.0, sX, sY, segmentLength);
            cairo_pattern_add_color_stop_rgba(gradient, 0, 200./255., 40./255., 16./255, 1);
            cairo_pattern_add_color_stop_rgba(gradient, 1, 200./255., 40./255., 16./255, 0);           
            
            //cairo_set_source_rgba(cr, 0.2, 0.2, 1, 0.6);
            cairo_set_source(cr, gradient);
            cairo_move_to(cr, oSX, oSY);
            cairo_line_to(cr, tX, tY);
            cairo_stroke(cr);

            cairo_pattern_destroy(gradient); 

        }
    }

    //------------------------------------------

    cairo_destroy(cr);

    /* We stored our image in ARGB32 format. We have to create a mask */
    /* for this format to tell SDL about our data layout. */
    QColor rmask = 0x00ff0000;
    QColor gmask = 0x0000ff00;
    QColor bmask = 0x000000ff;
    QColor amask = 0xff000000;

    /* Free old surface and create a new one from our pixel data. */
    SDL_Surface *canvasSurface =
        SDL_CreateRGBSurfaceFrom((void *) canvasData, width, height, 32,
                                 stride, rmask, gmask, bmask, amask);

    /* Blit the clock to the screen and refresh */
    SDL_BlitSurface(canvasSurface, NULL, surface, NULL);
    //SDL_FastBlit(canvasSurface, surface, 0, 0, 0, 0, width, height);
    SDL_FreeSurface(canvasSurface);
    free(canvasData);
}
#endif

void Graph::restartLayout(ogdf::node centre) {
    qDebug("Graph::restartLayout()");
    layoutDoneCallback = new LayoutCallback(this);
    //fully_restart_graph_layout(NULL);
    ShapeObj* sh = shapes[centre];
    canvas()->layout()->lockShape(sh);
    canvas()->restart_graph_layout();

    // Centre first view.
    QList<QGraphicsView *> views = canvas()->views();
    views.first()->centerOn(sh->centrePos());
}


Cluster *newClusterWrapper(NodeList& memberIdList, 
        const ogdf::NodeArray<ShapeObj*>& shapes)
{
    CanvasItemList memberList;

    for(NodeList::iterator i=memberIdList.begin(); i!=memberIdList.end();++i)
    {
        if (shapes[*i] && !(shapes[*i]->isInactive()))
        {
            memberList.push_back(shapes[*i]);
        }
    }
    
    if (memberList.empty())
    {
        return NULL;
    }
    return newCluster(memberList);
}


void Graph::createClusters() {
    if(UseClusters) {
        int col=0;
        for(map<string,NodeList>::iterator i=canvasClustersMap.begin();
                i!=canvasClustersMap.end();++i,++col)
        {
            col = col % clusterColoursN;
            Cluster* c=newClusterWrapper(i->second, shapes);
            if (c)
            {
                c->setFillColour(clusterColours[col]);
                canvasClusters.push_back(c);
            }
        }
    }
}
struct CompareNodes {
    CompareNodes(int* d, ogdf::NodeArray<int>& timeStamps)
        : d(d), timeStamps(timeStamps) {}
    bool operator() (const ogdf::node& u, const ogdf::node& v) {
        int du=d[u->index()], dv=d[v->index()];
        if(du==dv) {
            return timeStamps[u]>timeStamps[v];
        }
        return du<dv;
    }
    int* d;
    ogdf::NodeArray<int>& timeStamps;
};
struct IsVisible {
    IsVisible(const ShapeObj* sh, const ogdf::NodeArray<ShapeObj*>& shapes) 
        : sh(sh), shapes(shapes) {}
    bool operator()(const ogdf::node& v) {
        ShapeObj* vsh = shapes[v];
        if(sh==vsh) {
            assert(!sh->isInactive()); // because we are searching canvas list
            return true;
        }
        return false;
    }
    const ShapeObj* sh;
    const ogdf::NodeArray<ShapeObj*>& shapes;
};
struct NotInNeighbourhood {
    NotInNeighbourhood(vector<ogdf::node>& vs, ogdf::NodeArray<ShapeObj*>& shapes,
            int limit, CanvasItemList& toRemove)
        : vs(vs), shapes(shapes), limit(limit), toRemove(toRemove) {}
    bool operator() (ShapeObj* sh) {
        bool remove = true;
        vector<ogdf::node>::iterator first = vs.begin(),
                                     last = first + limit,
                                     result = last;
        result = find_if(first,last,IsVisible(sh,shapes));
        if(result!=last) {
            remove = false;
        } else {
            toRemove.push_back(sh);
        }
        return remove;
    }
    vector<ogdf::node>& vs;
    ogdf::NodeArray<ShapeObj*>& shapes;
    int limit;
    CanvasItemList& toRemove;
};
struct AddToCanvas {
    AddToCanvas(Graph& g) : g(g) { }
    void operator() (ogdf::node& v) {
        g.createShape(v);
    }
    Graph& g;
};
struct AddClusterDetails {
    AddClusterDetails(Graph& g) : g(g) { }
    void operator() (ogdf::node& v) {
        g.storeClusterInfo(v);
    }
    Graph& g;
};
/**
 * expand neighbourhood around node closest to position x,y
 */
void Graph::expandNeighbours(double x, double y) {
    double minDist = DBL_MAX;
    ogdf::node v, closest=NULL;
    printf("expanding neighbours around %f,%f\n",x,y);
    forall_nodes(v,G) {
        double d=dist(v,x,y);
        if(d<minDist) {
            closest = v;
            minDist = d;
        }
    }
    expandNeighbours(closest);
}
void Graph::expandNeighbours(ogdf::node centre) {
    qDebug("expandNeighbours setting interrupt...");
    GraphLayout* gl=canvas()->layout();
    gl->setInterruptFromDunnart();
    gl->unlockAll(NULL);
    // vs is a list of nodes sorted such that nodes with shortest path lengths
    // from centre and most recent time stamps are at the front
    vector<ogdf::node> vs;
    ogdf::node v;
    forall_nodes(v,G) {
        vs.push_back(v);
    }
    int *d=shortestPathsMatrix[centre->index()];
    sort(vs.begin(),vs.end(),CompareNodes(d,timeStamps));
    // the first canvasShapesLimit nodes in vs are the neighbourhood
    // we wish to show in the detailed canvas.  We remove the remaining
    // shapes from the canvas.
    CanvasItemList toRemove;
    if(UseClusters){
        for(CanvasItemList::iterator i=canvasClusters.begin();
                i!=canvasClusters.end();++i)
        {
            //QT (*i)->routerRemove();
            (*i)->setVisible(false);
        }
    }
    canvasShapes.remove_if(NotInNeighbourhood(
                vs,shapes,canvasShapesLimit,toRemove));
    //QT removeFromCanvas(toRemove);
    // the shapes still on the canvas at this point are the intersection
    // between the previous neighbourhood and the new neighbourhood.
    // We apply topology preserving layout initially only between shapes in
    // this neighbourhood.
    gl->topologyNodesCount = canvasShapes.size();
    // now we add the remaining primary nodes to the canvas with an
    // updated timestamp.
    time++;
    if(UseClusters) {
        canvasClustersMap.clear();
    }
    vector<ogdf::node>::iterator endIt = (vs.size() <= canvasShapesLimit) ?
            vs.end() : (vs.begin() + canvasShapesLimit);
    for_each(vs.begin(),endIt,AddToCanvas(*this));
    for_each(vs.begin(),vs.end(),AddClusterDetails(*this));
    ogdf::edge e;
    forall_edges(e,G) {
        // create connector if both ends are on the canvas
        createConnector(e);
    }
    restartLayout(centre);
}
void Graph::expandNeighbours(ShapeObj* shape) {
    Cluster *cluster = dynamic_cast<Cluster *> (shape);
    if (cluster)
    {
        // Clusters can't be focal nodes.
        return;
    }
    printf("Expand neighbours!\n");
    ogdf::node centre = nodes[shape];
    expandNeighbours(centre);
}
void Graph::drawOverviewOverlay() {
    // ow->updateCanvasPosOverlay();
}

// XXX: This is pretty inefficient at the moment.
QColor Graph::getNodeColor(const ogdf::node v)
{
    QColor col = QColor(0,0,0,85);
    if(!UseClusters)
    {
        return col;
    }
    bool found = false;
    int colN = 0;
    for(map<string,NodeList>::iterator i=canvasClustersMap.begin();
            i!=canvasClustersMap.end();++i,++colN)
    {
        for (NodeList::iterator n = i->second.begin();
                n != i->second.end(); ++n)
        {
            if (*n == v)
            {
                colN = colN % clusterColoursN;
                col = clusterColours[colN];
                found = true;
                break;
            }
        }
        if (found)
        {
            break;
        }
    }
    return col;
}


void Graph::drawOverview() {
#if 0
    assert(ow!=NULL);
    ow->clearSurface();
    ow->gmlGraph = this;
    QPixmap *overview = ow->getSurface();

    Draw draw(G, GA, overview, gbounds);
    
    if(UseClusters)
    {
        int col = 0;
        for(map<string,NodeList>::iterator i=canvasClustersMap.begin();
                i!=canvasClustersMap.end();++i,++col)
        {
            std::vector<unsigned> hullIndexes;
            int totalPoints = i->second.size() * 4;
            std::valarray<double> xValues(totalPoints);
            std::valarray<double> yValues(totalPoints);
            std::valarray<int> ids(totalPoints);
            std::valarray<int> vns(totalPoints);
            int index = 0;
    
            for (NodeList::iterator n = i->second.begin();
                    n != i->second.end(); ++n)
            {
                double& nx = GA.x(*n);
                double& ny = GA.y(*n);
                double& nw = GA.width(*n);
                double& nh = GA.height(*n);
                
                xValues[index] = nx;
                yValues[index] = ny;
                ++index;
         
                xValues[index] = nx + nw;
                yValues[index] = ny;
                ++index;

                xValues[index] = nx;
                yValues[index] = ny + nh;
                ++index;
         
                xValues[index] = nx + nw;
                yValues[index] = ny + nh;
                ++index;
            }

            hull::convex(xValues, yValues, hullIndexes);

            double vx[hullIndexes.size()];
            double vy[hullIndexes.size()];
    
            index = 0;
            for (std::vector<unsigned>::iterator curr = hullIndexes.begin();
                    curr != hullIndexes.end(); ++curr)
            {
                vx[index] = xValues[*curr];
                vy[index] = yValues[*curr];
                index++;
            }

            col = col % clusterColoursN;
            draw.colour = clusterColours[col];
            //printf("%s: %d\n", i->first.c_str(), (int) i->second.size());
            draw.polygon(vx, vy, hullIndexes.size());
            draw.colour = Qt::black;
        }
    }

    ogdf::node v;
    forall_nodes(v,G) {
        draw.colour = getNodeColor(v);
        // Turn off transparency:
        draw.colour.setAlpha(255);
#if 0
        ShapeObj* sh=shapes[v];
        if (sh && !sh->isInactive())
        {
            // Colour nodes in the primary graph red.
            draw.colour = Qt::black;
        }
#endif
        draw.box(Box(GA.x(v),GA.y(v),Dim(GA.width(v),GA.height(v))));
        draw.colour = Qt::black;
    }
    ogdf::edge e;
    forall_edges(e,G) {
        ogdf::node s=e->source(), t=e->target();
        double x1=GA.x(s), y1=GA.y(s),
               x2=GA.x(t), y2=GA.y(t);
        double w1=GA.width(s), h1=GA.height(s),
               w2=GA.width(t), h2=GA.height(t);
        QColor sCol = getNodeColor(s);
        QColor tCol = getNodeColor(t);
        draw.colour = QColor(50,50,50,85);
        if (sCol == tCol)
        {
            draw.colour = sCol;
        }
        draw.colour.setAlpha(255);
        draw.line(x1+w1/2.0,y1+h1/2.0,x2+w2/2.0,y2+h2/2.0);
        draw.colour = Qt::black;
    }

    ow->updateWindowFromSurface();
#endif
}


void Graph::createShape(ogdf::node v) {
    createShape(v,Box(GA.x(v)-gbounds.x,GA.y(v)-gbounds.y,
                      Dim(GA.width(v),GA.height(v))));
}

/*
  // QT
void setupUMLClass(ClassShape* c, string s) {
    bool store_undo = false;
    istringstream ss(s);
    ss.ignore(1); // ignore "{"
    string name;
    string attributes;
    string methods;
    getline(ss,name,'|');
    getline(ss,attributes,'|');
    getline(ss,methods,'}');
    name=name.substr(0,name.length()-2);
    vector<QString> lines;
    lines.push_back(name.c_str());
    c->update_contents(EDIT_CLASS_NAME,lines, store_undo);
    lines.clear();
    istringstream as(attributes);
    string a;
    while(getline(as,a,'\\')) {
        lines.push_back(a.substr(a.find_first_not_of("#* ")).c_str());
        as.ignore(1); // ignore l
    }
    c->update_contents(EDIT_ATTRIBUTES,lines, store_undo);
    lines.clear();
    istringstream ms(methods);
    while(getline(ms,a,'\\')) {
        lines.push_back(a.substr(a.find_first_not_of("#* ")).c_str());
        ms.ignore(1); // ignore l
    }
    c->update_contents(EDIT_METHODS,lines, store_undo);
}
*/
/**
 * Create a shape for node v with position and dimensions specified by b.
 * If a shape already exists for node v and it is inactive then it is 
 * returned to the canvas.
 * @param v corresponding gml node
 * @param b bounding box for shape
 */
void Graph::createShape(ogdf::node v, const Box b) {
    PluginShapeFactory *factory = sharedPluginShapeFactory();
    ShapeObj* sh=shapes[v];
    if(sh==NULL) {
        string templ(GA.templateNode(v).cstr());
        if(GA.shapeNode(v)==ogdf::GraphAttributes::oval) {
            sh = factory->createShape("flowEndOProc");
            sh->setCentrePos(QPointF(b.x, b.y));
            sh->setLabel(GA.labelNode(v).cstr());
        }
        else if(templ.substr(0,9)=="UML:CLASS") {
            sh = factory->createShape("umlClass");
            sh->setCentrePos(QPointF(b.x, b.y));
            // QT setupUMLClass(c,GA.labelNode(v).cstr());
        }
        else {
            sh = new RectangleShape();
            sh->setCentrePos(QPointF(b.x, b.y));
            sh->setLabel(GA.labelNode(v).cstr());
        }

        // we want shapes in the canvas list in the order that they are created
        string imageUrl(GA.imageUrlNode(v).cstr());
        if(!imageUrl.empty()) {
            string imagePath(gmlFileName,0,gmlFileName.size()-4);
            string imageFile=imagePath+"_images/"+imageUrl+".gif";
            //QT sh->setDecorativeImageFile(imageFile);
        }
        int smallW = 30, smallH = 20;
        //QT sh->determine_small_dimensions(&smallW, &smallH);
        int width = smallW, height = smallH;
        GA.width(v)=width;
        sh->setPosAndSize(QPointF(b.x, b.y), QSizeF(width, height));
        shapes[v] = sh;
        nodes[sh] = v;
        canvasShapes.push_back(sh);
        timeStamps[v]=time;
    } else if(sh->isInactive()) {
        CanvasItemList col;
        col.push_back(sh);
        returnToCanvas(col);
        canvasShapes.push_back(sh);
        timeStamps[v]=time;
    } else {
        printf("tried createShape for one already on the canvas!\n");
    }
    //QT sh->bring_to_back();
}

void Graph::storeClusterInfo(ogdf::node v)
{
    if (!UseClusters)
    {
        return;
    }
    string clusterName(GA.clusterNode(v).cstr());
    if (clusterName.size() > 0)
    {
        map<string,NodeList >::iterator i = 
                canvasClustersMap.find(clusterName);
        if (i == canvasClustersMap.end())
        {
            canvasClustersMap[clusterName]=NodeList();
        }
        canvasClustersMap[clusterName].push_back(v);
    }
}

bool Graph::isVisible(ogdf::node v, const Box& view) {
    const Box s(GA.x(v)-gbounds.x, GA.y(v)-gbounds.y, 
                Dim(GA.width(v), GA.height(v)));
    return view.overlaps(s);
}

void Graph::createConnector(ogdf::edge e) {
    Connector* conn = connectors[e];
    ShapeObj *start = shapes[e->source()], *end = shapes[e->target()];
    if(start!=NULL && end!=NULL 
     && !start->isInactive() && !end->isInactive()) {
        if(conn==NULL) {
            Connector* conn = new Connector();
            conn->initWithConnection(start,end);
            conn->setObeysDirectedEdgeConstraints(false);
            connectors[e] = conn;
            if(UML) {
                if(GA.type(e)==ogdf::Graph::generalization) {
                conn->setArrowHeadType(Connector::triangle_outline);
                conn->setDotted(0);
                } else {
                    conn->setDotted(1);
                }
            }
        } else if(conn->isInactive()) {
            CanvasItemList col;
            col.push_back(conn);
            returnToCanvas(col);
        } else {
            printf("tried createConnector for one already on canvas!\n");
        }
    }
}

void setupFMM(ogdf::FMMLayout& f, double scale) {
    f.unitEdgeLength(scale*100); 
    f.qualityVersusSpeed(ogdf::FMMLayout::qvsGorgeousAndEfficient);
    //setting options for the force calculation step
    //f.forceModel(f.fmFruchtermanReingold);
    f.forceModel(f.fmNew);
    f.springStrength(10);
    f.repForcesStrength(0.01);
    f.repulsiveForcesCalculation(f.rfcNMM);
    //f.repulsiveForcesCalculation(f.rfcExact);
    f.stopCriterion(f.scFixedIterationsOrThreshold);
    f.threshold(0.01);
    f.fixedIterations(30);
    f.forceScalingFactor(0.05);
    f.coolTemperature(false);
    f.coolValue(0.99);
   
    //setting options for postprocessing
    f.fineTuningIterations(10);
    f.fineTuneScalar(0.2);
    f.adjustPostRepStrengthDynamically(false);
    f.postSpringStrength(10.0);
    f.postStrengthOfRepForces(0.01);

    //setting options for different repulsive force calculation methods
    f.frGridQuotient(2); 
    f.nmTreeConstruction(f.rtcSubtreeBySubtree);
    f.nmSmallCell(f.scfIteratively);
    f.nmParticlesInLeaves(25); 
    f.nmPrecision(4);   
}

void setupFMMM(ogdf::FMMMLayout& f, double scale) {
    f.unitEdgeLength(scale*100); 
    f.qualityVersusSpeed(ogdf::FMMMLayout::qvsGorgeousAndEfficient);
    //setting options for the force calculation step
    f.forceModel(f.fmFruchtermanReingold);
    f.springStrength(1);
    f.repForcesStrength(1);
    f.repulsiveForcesCalculation(f.rfcNMM);
    //repulsiveForcesCalculation(rfcExact);
    f.stopCriterion(f.scFixedIterationsOrThreshold);
    f.threshold(0.01);
    f.fixedIterations(30);
    f.forceScalingFactor(0.05);
    f.coolTemperature(false);
    f.coolValue(0.99);
   
    //setting options for postprocessing
    f.fineTuningIterations(10);
    f.fineTuneScalar(0.2);
    f.adjustPostRepStrengthDynamically(true);
    f.postSpringStrength(1.0);
    f.postStrengthOfRepForces(1);

    //setting options for different repulsive force calculation methods
    f.frGridQuotient(2); 
    f.nmTreeConstruction(f.rtcSubtreeBySubtree);
    f.nmSmallCell(f.scfIteratively);
    f.nmParticlesInLeaves(25); 
    f.nmPrecision(4);   
}

void Graph::layout() {
    //FMMLayout fmm;
    //setupFMM(fmm);
    ogdf::FMMMLayout fmm;
    setupFMMM(fmm,scale);
 
    fmm.call(GA);
    printf("Layout took: %f seconds\n",fmm.getCpuTime());
    /*
    ogdf::FMMLayout fm;
    setupFMM(fm,scale);
    for(unsigned i=0;i<10;++i) {
        fm.call(GA);
        printf("Relayout took: %f seconds\n",fm.getCpuTime());
    }
    */
}
void Graph::relayoutOverview() {
    ogdf::FMMLayout fmm;
    setupFMM(fmm,scale);
    ogdf::node v;
    forall_nodes(v,G) {
        ShapeObj* sh=shapes[v];
        if(sh!=NULL) {
            QPointF pos = sh->centrePos();
            GA.type(v)=ogdf::FMMLayout::FIXED;
            GA.x(v)=pos.x();
            GA.y(v)=pos.y();
        } else {
            GA.type(v)=ogdf::FMMLayout::UNFIXED;
        }
    }
    fmm.call(GA);
    printf("Relayout took: %f seconds\n",fmm.getCpuTime());
    drawOverview();
    // Update the ghost connector directions.
}



} // namespace gml

}
// vim: filetype=cpp:cindent:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :

