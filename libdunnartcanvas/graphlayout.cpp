/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2008  Monash University
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

#include <QCoreApplication>

#include "libdunnartcanvas/canvas.h"

#include "libdunnartcanvas/graphdata.h"

#include "libcola/cola.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/cluster.h"
#include "libtopology/topology_graph.h"
#include "libvpsc/rectangle.h"
#include "libdunnartcanvas/templates.h"
#include "libdunnartcanvas/template-constraints.h"
#include "libdunnartcanvas/canvasview.h"

#ifdef CONSTRAINT_DEBUG
    #define ConstraintDebug(a) qDebug(a)
#else
    #define ConstraintDebug(a) while(0) { }
#endif

namespace dunnart {

// The order that we process returned PosInfos in.  Order matters because the
// guideline length with depend on the positions of the aligned shapes, etc.
enum PosInfoProcessOrder 
{
    PosInfoProcessOrderShape,
    PosInfoProcessOrderGuideline,
    PosInfoProcessOrderDistribution,
    PosInfoProcessOrderSeparation,
    PosInfoProcessOrderOther
};

using namespace std;
using cola::delete_object;

PosInfos debugHUDPositions;

class LayoutThread : public QThread
{
    public:
        LayoutThread(GraphLayout *layout)
            : layout(layout)
        { }
        void run(void)
        {
            layout->initThread();
        }
    private:
        GraphLayout *layout;
};


GraphLayout::GraphLayout(Canvas *canvas) 
    : mode(ORGANIC),
      optimizationMethod(MAJORIZATION),
      graph_layout_iterations(100),
      runLevel(0),
      topologyNodesCount(10000),
      m_canvas(canvas),
      m_graph(NULL),
      running(false),
      retPositionsHandled(true),
      outputDebugFiles(false),
      positionChangesFromDunnart(false),
      interruptFromDunnart(true),
      freeShiftFromDunnart(false),
      restartFromDunnart(false),
      askedToFinish(false),
      m_layout_thread(NULL)
{
    m_layout_thread = new LayoutThread(this);
    m_layout_thread->start();
}


GraphLayout::~GraphLayout()
{
    // Tell the thread to exit and then wait for it.
    m_layout_signal_mutex.lock();
    askedToFinish = true;
    m_layout_signal_mutex.unlock();

    m_layout_wait_condition.wakeAll();
    m_layout_thread->wait();

    delete m_graph;
}

GraphData *GraphLayout::getGraphData(void)
{
    return m_graph;
}


// The following structure hierarchy is used for interthread communication

PosInfo::PosInfo()
    : debugHUD(false),
      processOrder(PosInfoProcessOrderOther),
      HUDTimer(0)
{
}

PosInfo::PosInfo(bool debugHUD, unsigned short HUDTimer)
    : debugHUD(debugHUD),
      processOrder(PosInfoProcessOrderOther),
      HUDTimer(HUDTimer)
{
}


/**
 * used to report a conflicting (or unsatisfiable) constraint condition.
 */
struct ConflictPosInfo : PosInfo {
    CanvasItem *obj1;
    CanvasItem *obj2;
    ConflictPosInfo(CanvasItem *c1, CanvasItem *c2):
        obj1(c1),
        obj2(c2) { }
    void process(Canvas *canvas)
    {
        Q_UNUSED (canvas)

        if (obj1)
        {
            obj1->setConstraintConflict(true);
        }
        if (obj2)
        {
            obj2->setConstraintConflict(true);
        }
    }
};
/**
 * causes dunnart to draw a circle at a specified location for debug purposes.
 */
struct TracePointPosInfo : PosInfo {
    TracePointPosInfo(QPointF p):
        PosInfo(true,100),p(p) { }
    void process(Canvas *canvas)
    {
        Q_UNUSED (canvas)
    }
#ifdef DEBUG_OVERLAY
    void myProcessHUD(SDL_Surface *sur)
    {
        filledCircleRGBA(sur, (int)x, (int)y, 1, 0, 0, 255, 80);
    }
#endif
    QPointF p;
};
/**
 * causes dunnart to draw a line at a specified location for debug purposes.
 */
struct TraceLinePosInfo : PosInfo {
    TraceLinePosInfo(QPointF p0, QPointF p1, unsigned colour=0):
        PosInfo(true,30),p0(p0),p1(p1),colour(colour) { }
    void process(Canvas *canvas)
    {
        Q_UNUSED (canvas)
    }
#ifdef DEBUG_OVERLAY
    void myProcessHUD(SDL_Surface *sur)
    {
        int r=colour==0?255:0;
        int g=colour==1?255:0;
        int b=colour==2?255:0;
        lineRGBA(sur, (int)x0, (int)y0, (int)x1, (int)y1, r, g, b, 80);
    }
#endif
    QPointF p0, p1;
    unsigned colour;
};

/**
 * communicate position update info for a dunnart Shape from layout.
 */
struct ShapePosInfo : PosInfo {
    ShapeObj* shapePtr;
    QRectF shapeRect;
    bool locked;
    bool resized;
    ShapePosInfo(ShapeObj *p, double x, double y):
        shapePtr(p),
        locked(false),
        resized(false) 
    { 
        shapeRect = shapePtr->shapeRect(avoidBuffer);
        shapeRect.moveCenter(QPointF(x, y));
        processOrder = PosInfoProcessOrderShape;
    }

    /**
     * This shape is locked at xPos, yPos and graphlayout can't change it.
     */
    ShapePosInfo(ShapeObj* s, bool resized=false):
        shapePtr(s),
        locked(true),
        resized(resized)
    {
        processOrder = PosInfoProcessOrderShape,
        assert(shapePtr!=NULL);
        shapeRect = shapePtr->shapeRect(avoidBuffer);
    }
    void process(Canvas *canvas)
    {
        Q_UNUSED (canvas)

        ConstraintDebug("**  SHAPE\n");
        if (!locked)
        {
            //QPointF diff = QPointF(centreX, centreY) - shapePtr->pos();
            //qDebug("Moving shape: (%g,%g)  %g\n", centreX, centreY,
            //        diff.manhattanLength());

            shapePtr->cmd_setCentrePos(shapeRect.center());
        }
    }
    void fixGraphLayoutPosition(GraphData*, cola::Locks&, cola::Resizes&);
};

/**
 * Functor for use in forEach loop over the points in a topology::Edge to copy
 * each EdgePoint into an Avoid::PolyLine
 */
struct CopyClusterVertex {
    CopyClusterVertex(GraphLayout& gl, vector<Avoid::Point>& points) 
        : gl(gl), points(points), i(0) {}
    void operator() (const topology::EdgePoint* p) {
        points[i].x = p->posX();
        points[i].y = p->posY();
        points[i].id = gl.getGraphData()->getShape(p->node->id)->internalId();
        points[i].vn = p->rectIntersectAsVertexNumber();
        ++i;
    }
    GraphLayout& gl;
    vector<Avoid::Point>& points;
    unsigned i;
};
/**
 * communicate position update info for a dunnart Cluster from layout.
 */
struct ClusterPosInfo : PosInfo
{
    ClusterPosInfo(GraphLayout& gl, Cluster *dc, topology::Edge const *e)
        : clusterPtr(dc),
          hasBoundary(true),
          hasFixedSizeAndPosition(false)
    {
        unsigned n = e->nSegments + 1;
        points.resize(n);
        e->forEachEdgePoint(CopyClusterVertex(gl, points));
    }
    ClusterPosInfo(Cluster *dc, double x, double y)
        : clusterPtr(dc),
          hasBoundary(false),
          hasFixedSizeAndPosition(true),
          centrePos(x, y)
    { 
        points.clear();
    }
    ClusterPosInfo(Cluster *dc)
        : clusterPtr(dc),
          hasBoundary(false),
          hasFixedSizeAndPosition(false)
    { 
        points.clear();
    }
    void process(Canvas *canvas)
    {
        Q_UNUSED (canvas)

        if (hasBoundary)
        {
            /*
            printf("Have boundary: ");
            for(unsigned i=0;i<x.size();i++) {
                printf("(%f,%f)",x[i],y[i]);
            }
            printf("\n");
            */
            clusterPtr->setNewBoundary(points);
        } 
        else if (hasFixedSizeAndPosition)
        {
            clusterPtr->setCentrePos(centrePos);
        }
        else
        {
            //printf("No boundary!\n");
            clusterPtr->recomputeBoundary();
        }
    }

    // The dunnart cluster shape pointer.
    Cluster *clusterPtr;
    // Points comprising the cluster boundary.
    bool hasBoundary;
    vector<Avoid::Point> points;
    // Used to return the position of fixed size parent shapes.
    bool hasFixedSizeAndPosition;
    QPointF centrePos;
};


/**
 * Functor for use in forEach loop over the points in a topology::Edge to copy
 * each EdgePoint into an Avoid::PolyLine
 */
struct CopyPoint {
    CopyPoint(const GraphData* g, Avoid::PolyLine& r) : g(g), r(r), i(0) {}
    void operator() (const topology::EdgePoint* p) {
        assert(i<(unsigned)r.size());
        Avoid::Point& q = r.ps[i++];
        q.x = p->posX();
        q.y = p->posY();
        q.id = g->getShape(p->node->id)->internalId();
        q.vn = p->rectIntersectAsVertexNumber();
    }
    const GraphData* g;
    Avoid::PolyLine& r;
    unsigned i;
};
/**
 * communicate path update info for a dunnart Conn from layout.
 */
struct ConnPosInfo : PosInfo {
    Avoid::PolyLine route;
    Connector* conn;
    ConnPosInfo(const GraphData* g, Connector* conn, cola::Edge const& e,
            double x0, double y0, double x1, double y1)
        : conn(conn)
    {
        route = Avoid::PolyLine(2);
        Avoid::Point& p=route.ps[0];
        p.x=x0;
        p.y=y0;
        p.id=g->getShape(e.first)->internalId();
        p.vn=4;
        
        Avoid::Point& p2=route.ps[1];
        p2.x=x1;
        p2.y=y1;
        p2.id=g->getShape(e.second)->internalId();
        p2.vn=4;
    }
    ConnPosInfo(const GraphData* g, Connector* conn,topology::Edge const *e)
            : conn(conn) 
    {
        // Copy the new route into dunnart:
        route = Avoid::PolyLine(e->nSegments+1);
        e->forEachEdgePoint(CopyPoint(g,route));
    }
    ~ConnPosInfo() {
    }
    void process(Canvas *canvas)
    {
        Q_UNUSED (canvas)

        //printf("ConnPosInfo::process()\n");
        bool updateLibavoid = true;
        conn->applyNewRoute(route, updateLibavoid);
    }
};

/**
 * communicate position update info for a dunnart Guideline corresponding to a set of layout constraints from layout.
 */
struct GuidelinePosInfo : PosInfo {
    Guideline *guidePtr;
    double gPos;
    bool hasPos;
    GuidelinePosInfo(Guideline *g, double gp): guidePtr(g), gPos(gp)
    {
        processOrder = PosInfoProcessOrderGuideline;
        hasPos = true;
    }
    GuidelinePosInfo(Guideline *g): guidePtr(g)
    {
        processOrder = PosInfoProcessOrderGuideline;
        hasPos = false;
    }
    void process(Canvas *canvas)
    {
        Q_UNUSED (canvas)

        ConstraintDebug("**  GUIDELINE\n");
        guidePtr->updateFromLayout(gPos, hasPos);
    }
    void fixGraphLayoutPosition(GraphData*,cola::Locks&,cola::Resizes&);
};

/**
 * communicate position update info for a dunnart Template 
 * corresponding to a set of layout constraints from layout.
 */
struct TemplatePosInfo : PosInfo {
    Template *templatePtr;
    double templatePos;
    TemplatePosInfo(Template *lt, double gp)
        : templatePtr(lt), 
          templatePos(gp) 
    { }
    void process(Canvas *canvas)
    {
        Q_UNUSED (canvas)

        templatePtr->updatePositionFromSolver(templatePos, false);
    }
    void fixGraphLayoutPosition(GraphData* g,cola::Locks&,cola::Resizes&) 
    {
        LinearTemplateConstraint *ltc 
                = dynamic_cast<LinearTemplateConstraint *>
                        (g->getConstraint(templatePtr));
        BranchedTemplateConstraint *btc 
                = dynamic_cast<BranchedTemplateConstraint *>
                        (g->getConstraint(templatePtr));
        if(ltc) {
          ltc->fixPos(templatePos);
        }
        else if(btc) {
          btc->fixPos(templatePos);
        }
    }
};

/**
 * communicate position update info for a dunnart Distribution corresponding to a set of layout constraints from layout.
 */
struct DistributionPosInfo : PosInfo {
    Distribution *distroPtr;
    double dSep;
    DistributionPosInfo(Distribution *d, double ds): distroPtr(d), dSep(ds)
    {
        processOrder = PosInfoProcessOrderDistribution;
    }
    void process(Canvas *canvas)
    {
        Q_UNUSED (canvas)

        ConstraintDebug("**  DISTRIBUTION\n");
        distroPtr->updateFromLayout(dSep);
    }
    void fixGraphLayoutPosition(GraphData*,cola::Locks&,cola::Resizes&);
};

/**
 * communicate position update info for a dunnart Separation corresponding to a set of layout constraints from layout.
 */
struct SeparationPosInfo : PosInfo {
    Separation *sepPtr;
    double sSep; 
    SeparationPosInfo(Separation *s, double ss): sepPtr(s), sSep(ss)
    {
        processOrder = PosInfoProcessOrderSeparation;
    }
    void process(Canvas *canvas)
    {
        Q_UNUSED (canvas)

        ConstraintDebug("**  SEPARATION\n");
        sepPtr->updateFromLayout(sSep);
    }
    void fixGraphLayoutPosition(GraphData*,cola::Locks&,cola::Resizes&);
};

/**
 * communicate overrun of page bounds from layout
 */
struct PageBoundsPosInfo : PosInfo {
    double minX, maxX, minY, maxY;
    PageBoundsPosInfo(double minX, double maxX, double minY, double maxY) 
        : minX(minX), maxX(maxX), minY(minY), maxY(maxY) {}
    void process(Canvas *canvas)
    {
        double page_buffer = canvas->visualPageBuffer();
        canvas->setExpandedPage(QRectF(
                QPointF(minX - page_buffer, minY - page_buffer),
                QPointF(maxX + page_buffer, maxY + page_buffer)));
    }
};

/**
 * called during alt-dragging with false to prevent layout from occurring
 * during the drag.  When the drag is completed (i.e. alt released) it is
 * called again with value=true.
 * interruptFromDunnart is set true at completion of drag to trigger layout
 * GraphLayout::initialise (otherwise false because nothing should happen).
 */
void GraphLayout::setLayoutSuspended(bool value)
{
    m_layout_signal_mutex.lock();
    if(value) {
        interruptFromDunnart = false;
    }
    freeShiftFromDunnart = value;
    m_layout_signal_mutex.unlock();
}


bool GraphLayout::isFreeShiftFromDunnart(void)
{
    m_layout_signal_mutex.lock();
    bool result = freeShiftFromDunnart;
    m_layout_signal_mutex.unlock();

    return result;
}


void GraphLayout::setInterruptFromDunnart(void)
{
    m_layout_signal_mutex.lock();
    interruptFromDunnart = true;
    positionChangesFromDunnart = false;
    m_layout_signal_mutex.unlock();
}

void GraphLayout::setRestartFromDunnart(void)
{
    m_layout_signal_mutex.lock();
    //interruptFromDunnart = true;
    positionChangesFromDunnart = true;
    m_layout_signal_mutex.unlock();
}


void ShapePosInfo::fixGraphLayoutPosition(GraphData* g,cola::Locks& locks, cola::Resizes& resizes) {
    unsigned id=g->getNodeID(shapePtr);
    locks.push_back(cola::Lock(id, shapeRect.center().x(), shapeRect.center().y()));
    if(resized) {
        resizes.push_back(cola::Resize(id, shapeRect.x(), shapeRect.y(),
                shapeRect.width(), shapeRect.height()));
    }
}

void GuidelinePosInfo::fixGraphLayoutPosition(GraphData* g,cola::Locks&,cola::Resizes&) {
    cola::AlignmentConstraint *ac 
        = static_cast<cola::AlignmentConstraint *>(g->getConstraint(guidePtr));
    assert(ac != NULL);
    ac->fixPos(gPos);
}

void DistributionPosInfo::fixGraphLayoutPosition(GraphData* g,cola::Locks&,cola::Resizes&) {
    cola::DistributionConstraint *dc 
        = static_cast<cola::DistributionConstraint *>(g->getConstraint(distroPtr));
    assert(dc != NULL);
    dc->setSeparation(dSep);
}

void SeparationPosInfo::fixGraphLayoutPosition(GraphData* g,cola::Locks&,cola::Resizes&) {
    cola::MultiSeparationConstraint *msc 
        = static_cast<cola::MultiSeparationConstraint *>(g->getConstraint(sepPtr));
    assert(msc != NULL);
    msc->setSeparation(sSep);
}

/**
 * Functor passed to cola for call-back, executed before each iteration of layout.
 * We use to read the list of fixed position shapes, i.e. the shapes which should be
 * treated by layout as constrained to the current position.
 */
struct PreIteration : cola::PreIteration {
    PreIteration(GraphLayout& gl) : cola::PreIteration(gl.locks, gl.resizes), gl(gl) {}
    bool operator()() {
        gl.m_layout_signal_mutex.lock();
        changed=gl.positionChangesFromDunnart;
        bool interrupt = gl.interruptFromDunnart | gl.freeShiftFromDunnart;
        gl.m_layout_signal_mutex.unlock();
        if (interrupt) { 
            printf("User interrupt detected in PreIteration!\n");
            return false; 
        }
        gl.m_changed_list_mutex.lock();
        gl.locks.clear();
        gl.resizes.clear();
        for (PosInfos::iterator p=gl.fixedPositions.begin();
                p!=gl.fixedPositions.end();p++) {
            (*p)->fixGraphLayoutPosition(gl.m_graph,locks,resizes);
        }
        gl.m_changed_list_mutex.unlock();
        return true;
    }
    GraphLayout& gl;
};

/**
 * creates the correct type of PosInfo object for a cola::CompoundConstraint.
 * @return the PosInfo object or NULL if no constraint is applicable.
 */
PosInfo* returnPosInfoFactory(cola::CompoundConstraint *c) {
    if(cola::AlignmentConstraint *ac = dynamic_cast<cola::AlignmentConstraint*>(c)) {
        if(ac->isFixed()) {
            ac->unfixPos();
            return new GuidelinePosInfo((Guideline*)ac->guide);
        } 
        if(ac->guide==NULL) { // no gui object associated with this alignment
            return NULL;
        }
        return new GuidelinePosInfo((Guideline*)ac->guide, ac->position());
    }
    if(cola::DistributionConstraint *dc = dynamic_cast<cola::DistributionConstraint*>(c)) {
        return new DistributionPosInfo((Distribution *)dc->indicator, dc->sep);
    }
    if(cola::MultiSeparationConstraint *sc = dynamic_cast<cola::MultiSeparationConstraint*>(c)) {
        return new SeparationPosInfo((Separation *)sc->indicator, sc->sep);
    }
    if(LinearTemplateConstraint *ltc = dynamic_cast<LinearTemplateConstraint*>(c)) {
        if(ltc->isFixed) {
            ltc->unfixPos();
            return NULL;
        } 
        return new TemplatePosInfo((LinearTemplate *)ltc->indicator, ltc->position);
    }
    if(BranchedTemplateConstraint *btc = dynamic_cast<BranchedTemplateConstraint*>(c)){
        if(btc->isFixed) {
            btc->unfixPos();
            return NULL;
        }
        return new TemplatePosInfo((BranchedTemplate *)btc->indicator, btc->position);
    }

    return NULL;
}
/**
 * A functor that is called at the end of each iteration of cola::ConstrainedMajorizationLayout.
 * Must invoke the default TestConvergence functor.
 */
class PostIteration : public cola::TestConvergence {
public:
    PostIteration(GraphLayout& gl) 
        : cola::TestConvergence(1e-5, gl.graph_layout_iterations),
          n(gl.m_graph->getNodeCount()),
          gl(gl) { }
    /**
     * Called by cola::ConstrainedMajorizationLayout after each layout iteration.
     * Returns new layout by populating the list gl.retPositions list with PosInfo objects.
     * @param new_stress stress level after last iteration
     * @param X node coordinates after last move
     * @param Y node coordinates after last move
     * @return result of TestConvergence functor call
     */
    bool operator()(const double new_stress,
                valarray<double> & X, valarray<double> & Y) {
        gl.m_layout_signal_mutex.lock();
        bool interrupt = gl.interruptFromDunnart | gl.freeShiftFromDunnart;
        gl.m_layout_signal_mutex.unlock();
        if(interrupt||gl.restartFromDunnart) {
            reset();
        }
        if (interrupt)
        {
            printf("User interrupt detected in PostIteration!\n");
            gl.m_layout_signal_mutex.lock();
            gl.interruptFromDunnart = true;
            if (gl.freeShiftFromDunnart)
            {
                gl.positionChangesFromDunnart=false;
            }
            gl.m_layout_signal_mutex.unlock();
            return true;
        }
        
        //dumpScreenshot();
        
        bool ready = false;
        
        while (!ready)
        {
            gl.m_return_positions_mutex.lock();
            ready = gl.retPositionsHandled;
            gl.m_return_positions_mutex.unlock();

            gl.m_layout_signal_mutex.lock();
            bool finish = gl.askedToFinish;
            gl.m_layout_signal_mutex.unlock();
            if (finish)
            {
                return true;
            }
        }

        gl.m_return_positions_mutex.lock();
        gl.retPositionsHandled = false;
        for_each(gl.retPositions.begin(),gl.retPositions.end(),
                delete_object());
        gl.retPositions.clear();
        for (unsigned i = 0; i < n; i++) {
            ShapeObj* shape = gl.m_graph->getShape(i);
            if (shape && (gl.fixedShapeLookup.find(shape) == 
                          gl.fixedShapeLookup.end())) 
            {
                gl.retPositions.push_back(
                        new ShapePosInfo(shape, X[i], Y[i]));
            }
        }

        bool pageBoundChange = false;
        double pbx=0, pby=0, pbX=0, pbY=0;
        // update guide positions
        for(cola::CompoundConstraints::iterator i = gl.m_graph->ccs.begin();
                i != gl.m_graph->ccs.end(); ++i)
        {
            cola::CompoundConstraint* c = *i;
            if (cola::PageBoundaryConstraints* pc =
                    dynamic_cast<cola::PageBoundaryConstraints*>(c))
            {
                pageBoundChange=true;
                pbx = pc->getActualLeftMargin(vpsc::HORIZONTAL);
                pbX = pc->getActualRightMargin(vpsc::HORIZONTAL);
                pby = pc->getActualLeftMargin(vpsc::VERTICAL);
                pbY = pc->getActualRightMargin(vpsc::VERTICAL);
            }
            if(PosInfo* pi = returnPosInfoFactory(c)) {
                gl.retPositions.push_back(pi);
            }
        }
        bool unsatisfiedConstraintsExist=false;
        for(cola::UnsatisfiableConstraintInfos::iterator i=gl.unsatisfiableX.begin();
                i!=gl.unsatisfiableX.end();i++) {
            gl.showUnsatisfiable(*i);
            unsatisfiedConstraintsExist=true;
            delete *i;
        }
        gl.unsatisfiableX.clear();
        for(cola::UnsatisfiableConstraintInfos::iterator i=gl.unsatisfiableY.begin();
                i!=gl.unsatisfiableY.end();i++) {
            gl.showUnsatisfiable(*i);
            unsatisfiedConstraintsExist=true;
            delete *i;
        }
        gl.unsatisfiableY.clear();
        if(pageBoundChange) {
            gl.retPositions.push_back(
                    new PageBoundsPosInfo(pbx,pbX,pby,pbY));
        }

        // Update cluster boundaries and connector routes
        if(gl.runLevel==1) {
            for(topology::Edges::iterator i=gl.m_graph->topologyRoutes.begin();
                    i!=gl.m_graph->topologyRoutes.end();++i) {
                topology::Edge* e = *i;
                assert(e->firstSegment->start->node->id
                        <gl.m_graph->topologyNodesCount);
                assert(e->lastSegment->end->node->id
                        <gl.m_graph->topologyNodesCount);
                if(!e->cycle()) {
                    gl.retPositions.push_back( new
                            ConnPosInfo(gl.m_graph,
                                (Connector*)gl.m_graph->conn_vec[e->id], e));
                } else {
                    Cluster* c=gl.m_graph->dunnartClusters[e->id];
                    gl.retPositions.push_back( new ClusterPosInfo(gl, c, e) );
                }
                //for(unsigned j=0;j<gl.graph->topologyRoutes[i]->debugLines.size();j++) {
                    //straightener::DebugLine &l=gl.graph->topologyRoutes[i]->debugLines[j];
                    //gl.retPositions.push_back(
                            //new TraceLinePosInfo(l.x0,l.y0,l.x1,l.y1,l.colour));
                //}
                //gl.graph->topologyRoutes[i]->debugLines.clear();
            }
            for (unsigned i = 0; i < gl.m_graph->dunnartClusters.size(); ++i)
            {
                Cluster* c = gl.m_graph->dunnartClusters[i];
                if (c && c->rectangular)
                {
                    gl.retPositions.push_back( new ClusterPosInfo(c) );
                }
            }
            if (gl.m_canvas->optPreserveTopology())
            {
                for(unsigned i=0; i<gl.m_graph->edges.size(); ++i) {
                    cola::Edge& e = gl.m_graph->edges[i];
                    unsigned u=e.first, v=e.second;
                    if(u>=gl.m_graph->topologyNodesCount
                       ||v>=gl.m_graph->topologyNodesCount) {
                        gl.retPositions.push_back( new
                            ConnPosInfo(gl.m_graph,
                                (Connector*)gl.m_graph->conn_vec[i], e,
                                X[u],Y[u],X[v],Y[v]));
                    }
                }
            }
        } 
        gl.m_return_positions_mutex.unlock();
        QCoreApplication::postEvent(gl.m_canvas, new LayoutUpdateEvent(),
                Qt::LowEventPriority);

        if(unsatisfiedConstraintsExist) return true;
        //printf("Stress=%f\n",new_stress);
        //SDL_Delay(3000);
        bool converged = TestConvergence::operator()(new_stress,X,Y);
        /*
        if(iterations<10) { // sometimes layout stops too early without this
            converged = false;
        }
        */
        return converged;
        //return true;
        //return false;
    }
private:
    unsigned n;
    GraphLayout& gl;
};



int GraphLayout::initThread()
{
    bool changes = false;
    bool firstRun = true;
    do
    {
        if (!changes)
        {
            m_layout_signal_mutex.lock();
            if (freeShiftFromDunnart)
            {
                //QT SDLGui::postUserEvent(USEREVENT_LAYOUT_FREESHIFT);
            }
            else if(!firstRun) 
            {
                QCoreApplication::postEvent(m_canvas,
                        new LayoutFinishedEvent(), Qt::LowEventPriority);
            }
            
            firstRun = false;
            running = false;
            m_layout_signal_mutex.unlock();

            m_layout_mutex.lock();
            //printf("Thread waiting...\n");
            m_layout_wait_condition.wait(&m_layout_mutex);
            //printf("Thread resumed...\n");
            m_layout_mutex.unlock();
        }

        m_layout_signal_mutex.lock();
        if (askedToFinish)
        {
            // The thread should finish here, so exit from the loop.
            m_layout_signal_mutex.unlock();
            break;
        }
        positionChangesFromDunnart = false;
        bool currInterrupt = interruptFromDunnart;
        if (!freeShiftFromDunnart)
        {
            running = true;
            interruptFromDunnart = false;
            m_layout_signal_mutex.unlock();
            run(currInterrupt);
        }
        else
        {
            m_layout_signal_mutex.unlock();
        }
    
        m_layout_signal_mutex.lock();
        changes = positionChangesFromDunnart;
        m_layout_signal_mutex.unlock();
    }
    while (1);

    return EXIT_SUCCESS;

}

struct CmpPosInfoPtrs
{
    bool operator()(const PosInfo *a, const PosInfo *b)
    {
        return a->processOrder < b->processOrder;
    }
};
/**
 * called by the GUI thread to handle changes in position of objects from the layout thread
 */
int GraphLayout::processReturnPositions()
{
    PosInfos returnPositions;

    m_return_positions_mutex.lock();
    returnPositions = retPositions;
    retPositions.clear();
    retPositionsHandled = true;
    m_return_positions_mutex.unlock();

    int movesCount = returnPositions.size();
    //qDebug() << "processReturnPositions: retPositions.size() = " <<
    //        returnPositions.size();
    returnPositions.sort(CmpPosInfoPtrs());

    ConstraintDebug("\n*******START**********\n");
    m_canvas->m_processing_layout_updates = true;
    while (!returnPositions.empty())
    {
        PosInfo* info = returnPositions.front();
        info->process(m_canvas);
        returnPositions.pop_front();
#ifdef DEBUG_OVERLAY
        if (info->debugHUD)
        {
            debugHUDPositions.push_back(info);
        }
        else
        {
            delete info;
        }
#else
        delete info;
#endif
    }
    m_canvas->m_processing_layout_updates = false;

    // Update selection cue after nodes have moved.
    bool computePositions = true;
    m_canvas->repositionAndShowSelectionResizeHandles(computePositions);

    ConstraintDebug("********END***********\n\n");
    //redraw_connectors(NULL);
    return movesCount;
}


void GraphLayout::lockShape(ShapeObj* shape)
{
    QSet<ShapeObj*>::iterator si = lockedShapes.find(shape);
    if(si==lockedShapes.end()) {
        lockedShapes.insert(shape);
    }
}


void GraphLayout::addLockedShapesToFixedList(void) 
{
    CObjList list;
    QList<CanvasItem *> canvas_items = m_canvas->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        if (ShapeObj *shape = isShapeForLayout(canvas_items.at(i))) 
        {
            if (shape->hasLockedPosition())
            {
                list.push_back(shape);
            }
        }
    }
    addToFixedList(list);
}

void GraphLayout::lockUnselectedShapes(QWidget ** c)
{
    Q_UNUSED (c)

    // selection is globally defined in canvas
    set<ShapeObj*> selected;
    QList<CanvasItem *> selected_items = 
            m_canvas->selectedItems();
    for (int i = 0; i < selected_items.size(); ++i)
    {
        if (ShapeObj *shape = isShapeForLayout(selected_items.at(i))) 
        {
            selected.insert(shape);
        }
    }
    QList<CanvasItem *> canvas_items = m_canvas->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        if (ShapeObj *shape = isShapeForLayout(canvas_items.at(i))) 
        {
            set<ShapeObj*>::iterator i = selected.find(shape);
            // anything not in the selection is locked, everything else
            // is unlocked
            if(i==selected.end()) {
                QSet<ShapeObj*>::iterator si = lockedShapes.find(shape);
                if(si==lockedShapes.end()) {
                    lockedShapes.insert(shape);
                    shape->setLockedPosition(true);
                }
            } else {
                lockedShapes.remove(shape);
                shape->setLockedPosition(false);
            }
        }
    }
}

void GraphLayout::unlockAll(QWidget ** c)
{
    Q_UNUSED (c)

    QList<CanvasItem *> canvas_items = m_canvas->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        if (ShapeObj *shape = isShapeForLayout(canvas_items.at(i))) 
        {
            lockedShapes.remove(shape);
            shape->setLockedPosition(false);
        }
    }
}

void GraphLayout::addToFixedList(CObjList & objList) {
    for(CObjList::iterator i=objList.begin(); i!=objList.end();i++) {
        Guideline *guide = dynamic_cast<Guideline *> (*i);
        Distribution *distro = dynamic_cast<Distribution *> (*i);
        Separation *separation = dynamic_cast<Separation *> (*i);
        Template *templatPtr = dynamic_cast<Template *> (*i);
       
        if (ShapeObj *shape = isShapeForLayout(*i)) 
        {
            if(fixedShapeLookup.find(shape)==fixedShapeLookup.end()) {
                ShapePosInfo* spi=new ShapePosInfo(shape);
                fixedShapeLookup[shape]=spi;
                fixedPositions.push_back(spi);
            }
        }
        else if (guide) {
            fixedPositions.push_back(new GuidelinePosInfo(guide, guide->position()));
        }
        else if (templatPtr) {
            fixedPositions.push_back(new TemplatePosInfo(templatPtr, 
                        templatPtr->getPos()));
        }
        else if (distro)
        {
            fixedPositions.push_back(
                    new DistributionPosInfo(distro, distro->space));
        }
        else if (separation)
        {
            fixedPositions.push_back(
                    new SeparationPosInfo(separation, separation->gap));
        }
    }
}
void GraphLayout::addToResizedList(CObjList & objList) {
    for(CObjList::iterator i=objList.begin(); i!=objList.end();i++) {
        if (ShapeObj *shape = isShapeForLayout(*i)) 
        {
            map<ShapeObj*,ShapePosInfo*>::iterator i=fixedShapeLookup.find(shape);
            if(i==fixedShapeLookup.end()) {
                ShapePosInfo* spi=new ShapePosInfo(shape, true);
                fixedShapeLookup[shape]=spi;
                fixedPositions.push_back(spi);
            } else {
                i->second->resized=true;
            }
        }
    }
}

void GraphLayout::apply(bool ignoreEdges)
{
    this->ignoreEdges=ignoreEdges;
    m_changed_list_mutex.lock();
    // tell layout thread whatever has changed
    Actions& actions = m_canvas->getActions();
    for_each(fixedPositions.begin(),fixedPositions.end(),delete_object());
    fixedPositions.clear();
    fixedShapeLookup.clear();

    addToFixedList(actions.moveList);
    addToResizedList(actions.resizeList);

    CObjList lockedShapesList;
    foreach (ShapeObj *shape, lockedShapes)
    {
        lockedShapesList.push_back(shape);
    }
    addToFixedList(lockedShapesList);
    addLockedShapesToFixedList();
    m_changed_list_mutex.unlock();

    m_layout_signal_mutex.lock();
    if (running)
    {
        positionChangesFromDunnart = true;
        m_layout_signal_mutex.unlock();
    }
    else
    {
        m_layout_signal_mutex.unlock();
        // Wake the layout thread up:
        m_layout_wait_condition.wakeAll();
    }
}


void GraphLayout::setLayoutMode(Mode newMode)
{
    mode = newMode;
    printf("Layout Mode set to %d\n", (int) mode);
}

void GraphLayout::setOptimizationMethod(OptimizationMethod newOM)
{
    optimizationMethod = newOM;
    string s = "Majorization";
    if(optimizationMethod == STEEPESTDESCENT) {
        s = "Steepest Descent";
    }
    cout << "Optimization method set to " << s << endl;
}

void GraphLayout::initialise(void)
{
    printf("GraphLayout::initialise: runlevel=%d\n",runLevel);
    if (m_graph!=NULL)
    {
        delete m_graph;
    }
    bool beautify = (runLevel == 1) ? true : false;
    m_graph = new GraphData(m_canvas, ignoreEdges, mode,
            beautify, topologyNodesCount);
}


void GraphLayout::run(const bool shouldReinitialise)
{
    if (shouldReinitialise)
    {
        initialise();
    }

    //printf("Running graph layout...\n");
    assert(m_graph!=NULL);

    PreIteration preIter(*this);
    PostIteration postIter(*this);

    valarray<double> elengths;
    m_graph->getEdgeLengths(elengths);

    cola::ConstrainedFDLayout alg(m_graph->rs, m_graph->edges, 1.0,
            m_canvas->m_opt_prevent_overlaps, &elengths[0], postIter, 
            &preIter);
    alg.setConstraints(m_graph->ccs);
    alg.setClusterHierarchy(&(m_graph->clusterHierarchy));
    if (runLevel == 1)
    {
        if (shouldReinitialise)
        {
            alg.makeFeasible();
                    
            // Store topology after makeFeasible
            alg.getTopology(&(m_graph->topologyNodes), 
                    &(m_graph->topologyRoutes));

            if (m_canvas->m_opt_preserve_topology)
            {
                m_graph->generateRoutes();
            }
        }

        if (m_canvas->m_opt_preserve_topology)
        {
            alg.setTopology(&(m_graph->topologyNodes), 
                    &(m_graph->topologyRoutes));
        }
    }
    alg.setUnsatisfiableConstraintInfo(&unsatisfiableX,&unsatisfiableY);
    alg.run(true,true);
    //alg.outputInstanceToSVG();
}


void GraphLayout::setOutputDebugFiles(const bool value)
{
    outputDebugFiles = value;
}

void GraphLayout::showUnsatisfiable(cola::UnsatisfiableConstraintInfo* i)
{
    printf("WARNING: Unsatisfiable constraint:\n");
    printf("  left id=%d,right id=%d\n",i->vlid,i->vrid);

    ShapeObj *s1 = m_graph->getShape(i->vlid);
    ShapeObj *s2 = m_graph->getShape(i->vrid);
    if(s1 && s2) {
        retPositions.push_back(
                new TraceLinePosInfo(
                    s1->centrePos(), s2->centrePos(), 0));
    }
    
    retPositions.push_back(new ConflictPosInfo(s1, s2));
}

}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

