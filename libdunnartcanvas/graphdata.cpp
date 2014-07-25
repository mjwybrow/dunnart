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
/**
 * GraphLayout constructs a cola graph representation from dunnart objects and maintains the
 * graph structure and various mapping lookups using the methods defined here
 */

#include <algorithm>

#include "libavoid/libavoid.h"

// dunnart object definitions:
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/cluster.h"
#include "libdunnartcanvas/handle.h"
// cola object definitions:
#include "libcola/cola.h"
#include "libtopology/topology_graph.h"
#include "libvpsc/rectangle.h"
#include "libdunnartcanvas/graphlayout.h"
#ifndef NOGRAPHVIZ
#include "libdunnartcanvas/graphvizlayout.h"
#endif
#include "libdunnartcanvas/graphdata.h"
#include "libdunnartcanvas/templates.h"
#include "libdunnartcanvas/template-constraints.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/stronglyconnectedcomponents.h"

namespace dunnart {

using namespace std;
using namespace vpsc;

// Convenience structure for levelAssignmentTraverse algorithm.
struct NodeLevelInfo
{
        NodeLevelInfo()
            : isRoot(true),
              sighted(false),
              level(0),
              parentVisits(0)
        {
        }

    std::set<unsigned> children;
    std::set<unsigned> parents;
    bool isRoot;
    bool sighted;
    unsigned level;
    unsigned parentVisits;
};

// This is similar to Kahn's 1962 topological sorting algorithm.
// It is O(|V|+|E|).  It starts from each root, incrementing the levels
// as it moves down the tree.  It waits when it gets to a node with multiple
// parents and waits until it has been visited from every parent, and then
// continues from there with the maximum level.
// It also keeps track of the highestLevel, for later convenience.
static void levelAssignmentTraverse(unsigned ind, unsigned newLevel,
        unsigned& maxLevel, std::vector<NodeLevelInfo>& nodeInfo)
{
    // Set the node level.
    nodeInfo[ind].level = qMax(nodeInfo[ind].level, newLevel);
    maxLevel = qMax(maxLevel, nodeInfo[ind].level);
    nodeInfo[ind].parentVisits++;

    if (nodeInfo[ind].parentVisits < nodeInfo[ind].parents.size())
    {
        // Stop until we've reached this from every parent.
        return;
    }

    // Recurse to set node levels of children.
    for (std::set<unsigned>::iterator it = nodeInfo[ind].children.begin();
            it != nodeInfo[ind].children.end(); ++it)
    {
        levelAssignmentTraverse(*it, nodeInfo[ind].level + 1, maxLevel,
                nodeInfo);
    }
}


GraphData::GraphData(Canvas *canvas, bool ignoreEdges, 
        GraphLayout::Mode mode, bool beautify, unsigned topologyNodesCount) 
    : canvas_(canvas),
      topologyNodesCount(topologyNodesCount) 
{
    Q_UNUSED (beautify)

    pageBoundary = QRectF();
    // Note that in Dunnart the coordinates of shapes are their top-left corners
    // while in constrained_majorization_layout we use the centres.

    Canvas::LayeredAlignment layeredAlignment =
            canvas_->optLayeredAlignmentPosition();

    // create nodes
    QList<CanvasItem *> canvasObjects = canvas->items();
    double xMin = DBL_MAX, xMax = -DBL_MAX;
    double yMin = DBL_MAX, yMax = -DBL_MAX;
    for(int i = 0; i < canvasObjects.size(); ++i)
    {
        if (ShapeObj *shape = isShapeForLayout(canvasObjects.at(i)))
        {
            size_t nodeIndex = shapeToNode(shape);

            double centreX = rs[nodeIndex]->getCentreX();
            double centreY = rs[nodeIndex]->getCentreY();
            xMax = qMax(xMax, centreX);
            xMin = qMin(xMin, centreX);
            yMax = qMax(yMax, centreY);
            yMin = qMin(yMin, centreY);
        }
    }

    if (!ignoreEdges)
    {
        // If the nodes are all clumped at one position, then the layout
        // won't be able to separate them, thus we distribute them a small
        // amount.
        if (xMax == xMin)
        {
            for (size_t i = 0; i < rs.size(); ++i)
            {
                double jiggle = ((double) i) - (rs.size() / 2);
                //qDebug("Clumped: offset X: %g", jiggle);
                rs[i]->offset(jiggle, 0.0);
            }
        }
        if (yMax == yMin)
        {
            for (size_t i = 0; i < rs.size(); ++i)
            {
                double jiggle = ((double) i) - (rs.size() / 2);
                //qDebug("Clumped: offset Y: %g", jiggle);
                rs[i]->offset(0.0, jiggle);
            }
        }
    }

    // Determine cluster heirarchy for shape-based clusters.
    QSet<cola::Cluster *> allShapeClusters;
    for (int i = 0; i < canvasObjects.size(); ++i)
    {
        // Compute cola::cluster objects and their child nodes.
        if (ShapeObj *shape = isShapeForLayout(canvasObjects.at(i)))
        {
            QList<ShapeObj *> children = shape->containedShapes();
            if (!children.empty())
            {
                cola::RectangularCluster *c = new cola::RectangularCluster(snMap[shape]);
                double buffer = canvas_->optShapeNonoverlapPadding();
                c->setMargin(buffer);

                // If the shape has containment padding specified, use it.
                if (!shape->containmentPadding().isNull())
                {
                    QMarginsF padding = shape->containmentPadding();
                    cola::Box containmentPadding(padding.left(), padding.right(), padding.top(), padding.bottom());
                    c->setPadding(containmentPadding);
                }

                rectClusterShapeMap[shape] = c;
                allShapeClusters.insert(c);
                dunnartClusters.push_back(NULL);
                rootNodes.erase(snMap[shape]);
                for (int j = 0; j < children.size(); ++j)
                {
                    // Only add as nodes, if they themselves contain
                    // no children.
                    unsigned nodeID = snMap[children.at(j)];
                    c->nodes.push_back(nodeID);
                    rootNodes.erase(nodeID);
                }
            }
        }
    }
    for (int i = 0; i < canvasObjects.size(); ++i)
    {
        // Assign some cola::cluster objects as child clusters of other
        // cola::cluster objects.
        if (ShapeObj *shape = isShapeForLayout(canvasObjects.at(i)))
        {
            QList<ShapeObj *> children = shape->containedShapes();
            if (!children.empty())
            {
                cola::Cluster *p = rectClusterShapeMap[shape];
                for (int j = 0; j < children.size(); ++j)
                {
                    if (rectClusterShapeMap.contains(children.at(j)))
                    {
                        cola::Cluster *c = rectClusterShapeMap[children.at(j)];
                        p->clusters.push_back(c);
                        allShapeClusters.remove(c);
                    }
                }
            }
        }
    }
    for (QSet<cola::Cluster *>::iterator c = allShapeClusters.begin(); 
            c != allShapeClusters.end(); ++c)
    {
        // And put remaining cola::clusters at children of the root cluster.
        clusterHierarchy.clusters.push_back(*c);
    }

    // create edges
    vector<Distribution*> distrolist;
    vector<Separation*> separationlist;
    if (!ignoreEdges)
    {
        for (int i = 0; i < canvasObjects.size(); ++i)
        {
            if (Connector *conn = dynamic_cast<Connector *> (canvasObjects.at(i)))
            {    
                connectorToEdge(conn);
            }
        }
        setupMultiEdges();

        if ((mode == GraphLayout::FLOW)  || (mode == GraphLayout::LAYERED))
        {
            // Direction.  Default down.
            vpsc::Dim dimension = vpsc::YDIM;
            bool flipped = false;
            int direction = canvas_->optFlowDirection() % 4;
            if (direction == Canvas::FlowUp)
            {
                flipped = true;
            }
            else if (direction == Canvas::FlowRight)
            {
                dimension = vpsc::XDIM;
            }
            else if (direction == Canvas::FlowLeft)
            {
                dimension = vpsc::XDIM;
                flipped = true;
            }

            // Set up downward edge constraints.
            // In the case that the graph isn't a DAG, i.e., has cycles, we
            // don't constrain any of the edges that form a cycle.  We do
            // this by computing the strongly connected components for the
            // Graph and only add a downward edge constraint between two
            // nodes (connector endpoints) if they are not part of the same
            // strongly connected component.
            SCCDetector cycleDetector;
            QVector<int> sccIndexes =
                    cycleDetector.stronglyConnectedComponentIndexes(this);
            std::vector<NodeLevelInfo> nodeInfo(shape_vec.size());

            for (uint i = 0; i < conn_vec.size(); ++i)
            {
                if ( ! conn_vec[i]->isDirected() ||
                     ! conn_vec[i]->obeysDirectedEdgeConstraints() )
                {
                    // Don't constrain undirected edges, or those
                    // ignoring downward edge constraints.
                    conn_vec[i]->setHasDownwardConstraint(false);
                    continue;
                }

                cola::Edge edge = edges[i];
                if (sccIndexes[edge.first] != sccIndexes[edge.second])
                {
#ifdef DIRECTED_CONSTRAINT_DEBUG
                    cout << "generating directed constraint for conn("
                            << conn_vec[i]->getId() << ")" << endl;
#endif
                    unsigned firstIndex = (flipped) ? edge.second : edge.first;
                    unsigned secondIndex = (flipped) ? edge.first : edge.second;

                    if (mode == GraphLayout::FLOW)
                    {
                        ccs.push_back(new cola::SeparationConstraint(dimension,
                                firstIndex, secondIndex,
                                (canvas_->m_ideal_connector_length *
                                 canvas_->optIdealEdgeLengthModifier() *
                                 canvas_->m_flow_separation_modifier)));
                    }
                    else if (mode == GraphLayout::LAYERED)
                    {
                        // Save the hierarchy information so we can align the
                        // layers together.
                        nodeInfo[firstIndex].children.insert(secondIndex);
                        nodeInfo[secondIndex].parents.insert(firstIndex);
                        nodeInfo[firstIndex].sighted = true;
                        nodeInfo[secondIndex].sighted = true;
                        nodeInfo[secondIndex].isRoot = false;
                    }
                    conn_vec[i]->setHasDownwardConstraint(true);
                }
                else
                {
                    conn_vec[i]->setHasDownwardConstraint(false);
                }
            }

            // Apply level constraints.
            if (mode == GraphLayout::LAYERED)
            {
                // Assign levels for each node.
                unsigned maxLevel = 0;
                for (size_t ind = 0; ind < shape_vec.size(); ++ind)
                {
                    if (nodeInfo[ind].isRoot && nodeInfo[ind].sighted)
                    {
                        levelAssignmentTraverse(ind, nodeInfo[ind].level + 1,
                                maxLevel, nodeInfo);
                    }
                }

                // Create list of nodes in each level and remember the size
                // of the largest
                std::vector<std::set<unsigned> > levelLists(maxLevel);
                std::vector<double> levelShapeLength(maxLevel, 0.0);
                for (size_t ind = 0; ind < shape_vec.size(); ++ind)
                {
                    // Clear the layer channel padding information.
                    shape_vec[ind]->setProperty("layeredStartChannelPadding",
                            QVariant());
                    shape_vec[ind]->setProperty("layeredEndChannelPadding",
                            QVariant());
                    shape_vec[ind]->setProperty("layeredEndConnectorGroup",
                            QVariant());
                    // Store level information.
                    if (nodeInfo[ind].level > 0)
                    {
                        size_t level = nodeInfo[ind].level - 1;
                        levelLists[level].insert(ind);
                        levelShapeLength[level] = qMax(
                                levelShapeLength[level],
                                rs[ind]->length(dimension));
                    }
                }

                // Align each level.
                cola::AlignmentConstraint *prevLevelAlignment = NULL;
                for (size_t level = 0; level < maxLevel; ++level)
                {
                    // Set of nodes in this level.
                    std::set<unsigned>& nodes = levelLists[level];

                    int prevSources = 0;
                    int currSources = 0;
                    if (level > 0)
                    {
                        // Find the number of connector sources on each side
                        // of the channel between levels.  The nodes if the
                        // previous level are in "prevNodes" and the nodes of
                        // this level are in "nodes".
                        for (std::set<unsigned>::const_iterator it = nodes.begin();
                                it != nodes.end(); ++it)
                        {
                            if (nodeInfo[*it].parents.size() > 0)
                            {
                                currSources++;
                            }
                        }
                        std::set<unsigned>& prevNodes = levelLists[level - 1];
                        for (std::set<unsigned>::const_iterator it = prevNodes.begin();
                                it != prevNodes.end(); ++it)
                        {
                            if (nodeInfo[*it].children.size() > 0)
                            {
                                prevSources++;
                            }
                        }

                        // Based on the numer of sources on the smaller side,
                        // give an ordering to the nodes where these
                        // connectors terminate.  For example in a directed
                        // tree flowing to th left, three nodes on one level
                        // might point to seven nodes, so each of the seven
                        // nodes would be given a group based on the three
                        // sources [0-2].  This is not the best solution, but
                        // it is fairly simple and easy to compute.
                        if (flipped)
                        {
                            if (prevSources < currSources)
                            {
                                // Connectors point to nodes in "nodes".
                                int number = 0;
                                for (std::set<unsigned>::const_iterator it = prevNodes.begin();
                                        it != prevNodes.end(); ++it)
                                {
                                    if (nodeInfo[*it].children.size() > 0)
                                    {
                                        shape_vec[*it]->setProperty("layeredEndConnectorGroup",
                                                QVariant(number++));
                                    }
                                }
                            }
                            else
                            {
                                // Connectors point to nodes in "prevNodes".
                                int number = 0;
                                for (std::set<unsigned>::const_iterator it = nodes.begin();
                                        it != nodes.end(); ++it)
                                {
                                    for (std::set<unsigned>::const_iterator ch =
                                            nodeInfo[*it].parents.begin();
                                            ch != nodeInfo[*it].parents.end(); ++ch)
                                    {
                                        shape_vec[*ch]->setProperty("layeredEndConnectorGroup",
                                                QVariant(number));
                                    }
                                    if (nodeInfo[*it].parents.size() > 0)
                                    {
                                        number++;
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (prevSources > currSources)
                            {
                                // Connectors point to nodes in "prevNodes".
                                int number = 0;
                                for (std::set<unsigned>::const_iterator it = nodes.begin();
                                        it != nodes.end(); ++it)
                                {
                                    if (nodeInfo[*it].parents.size() > 0)
                                    {
                                        shape_vec[*it]->setProperty("layeredEndConnectorGroup",
                                                QVariant(number++));
                                    }
                                }
                            }
                            else
                            {
                                // Connectors point to nodes in "nodes".
                                int number = 0;
                                for (std::set<unsigned>::const_iterator it = prevNodes.begin();
                                        it != prevNodes.end(); ++it)
                                {
                                    for (std::set<unsigned>::const_iterator ch =
                                            nodeInfo[*it].children.begin();
                                            ch != nodeInfo[*it].children.end(); ++ch)
                                    {
                                        shape_vec[*ch]->setProperty("layeredEndConnectorGroup",
                                                QVariant(number));
                                    }
                                    if (nodeInfo[*it].children.size() > 0)
                                    {
                                        number++;
                                    }
                                }                            }
                        }
                    }

                    // Set layer channel padding information for each shape on
                    // this level, and add it to the alignment relationship.
                    // The channel padding information is used for centring
                    // all connectors at the same point between levels.
                    cola::AlignmentConstraint *alignment =
                            new cola::AlignmentConstraint(dimension);
                    for (std::set<unsigned>::const_iterator it = nodes.begin();
                            it != nodes.end(); ++it)
                    {
                        double offset = 0;
                        double modifier = (flipped) ? -1 : 1;
                        double distToEdge = rs[*it]->length(dimension) / 2.0;
                        double startPadding = 0.0;
                        double endPadding = 0.0;
                        if (layeredAlignment == Canvas::ShapeStart)
                        {
                            offset = modifier * distToEdge;
                            startPadding = distToEdge;
                            endPadding = levelShapeLength[level] - distToEdge;
                        }
                        else if (layeredAlignment == Canvas::ShapeEnd)
                        {
                            offset = modifier * -distToEdge;
                            startPadding = levelShapeLength[level] - distToEdge;
                            endPadding = distToEdge;
                        }
                        else
                        {
                            startPadding = levelShapeLength[level] / 2.0;
                            endPadding = levelShapeLength[level] / 2.0;
                        }
                        shape_vec[*it]->setProperty("layeredStartChannelPadding",
                                QVariant(startPadding));
                        shape_vec[*it]->setProperty("layeredEndChannelPadding",
                                QVariant(endPadding));

                        alignment->addShape(*it, offset);
                    }
                    ccs.push_back(alignment);

                    if (prevLevelAlignment)
                    {

                        // Determine padding between this level and previous.
                        double padding = 0;
                        if (layeredAlignment == Canvas::ShapeMiddle)
                        {
                            padding = (levelShapeLength[level] / 2.0) +
                                    (levelShapeLength[level - 1] / 2.0);
                        }
                        else if (layeredAlignment == Canvas::ShapeStart)
                        {
                            padding = levelShapeLength[level];
                        }
                        else if (layeredAlignment == Canvas::ShapeEnd)
                        {
                            padding = levelShapeLength[level - 1];
                        }

                        // Create a separation constraint between this level's
                        // alignment and the one for the previous level.
                        ccs.push_back(new cola::SeparationConstraint(dimension,
                                prevLevelAlignment, alignment, padding +
                                (canvas_->m_ideal_connector_length *
                                 canvas_->optIdealEdgeLengthModifier() *
                                 canvas_->m_flow_separation_modifier)));
                    }
                    prevLevelAlignment = alignment;
                }

/*
                // Code for aligning just children of each parent node.
                for (size_t ind = 0; ind < childLists.size(); ++ind)
                {
                    std::set<unsigned>& children = childLists[ind];
                    if (children.empty())
                    {
                        continue;
                    }

                    cola::AlignmentConstraint *alignment =
                            new cola::AlignmentConstraint(dimension);
                    for (std::set<unsigned>::iterator it = children.begin();
                            it != children.end(); ++it)
                    {
                        if (parentCount[*it] == 1)
                        {
                            alignment->addShape(*it, 0);
                        }
                    }
                    ccs.push_back(alignment);
                }
                */
            }
        }
    }

    // get the corners of the page
    pageBoundary = canvas->pageRect();
    
    // adjust boundary for page_margin
    double page_buffer = canvas->visualPageBuffer();
    QPointF margin(page_buffer, page_buffer);
    pageBoundary.setTopLeft(pageBoundary.topLeft() + margin);
    pageBoundary.setBottomRight(pageBoundary.bottomRight() - margin);

    generateRectangleConstraints(canvasObjects);

    for (int i = 0; i < canvasObjects.size(); ++i)
    {
        QGraphicsItem *co = canvasObjects.at(i);
        Guideline *guide = dynamic_cast<Guideline *> (co);
        Distribution *distro = dynamic_cast<Distribution *> (co);
        Cluster *cluster = dynamic_cast<Cluster *> (co);
        Separation *separation = dynamic_cast<Separation *> (co);
        if (guide) {
            guideToAlignmentConstraint(guide);
        } else if (distro) {
            // process distribution constraints later once we've setup all the
            // alignment constraints
            distrolist.push_back(distro);
        } else if (separation) {
            separationlist.push_back(separation);
        } else if (cluster && !cluster->isCollapsed()) {
            dunnartClusterToCluster(cluster);
        } 
    }
    setUpRootCluster();
    // distribution constraints contain a list of pairs of alignment guidelines
    for(vector<Distribution*>::iterator i=distrolist.begin();i!=distrolist.end();i++)
    {
        distroToDistributionConstraint(*i);
    }
    for(vector<Separation*>::iterator i=separationlist.begin(); 
            i!=separationlist.end(); i++) {
        separationToMultiSeparationConstraint(*i);
    }

    // Templates rely on the other constraints have been handled
    // (and had variables assigned for them).
    QList<CanvasItem *> canvas_items = canvas_->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        LinearTemplate *linear = 
                dynamic_cast<LinearTemplate *> (canvas_items.at(i));
        BranchedTemplate *branched =
                dynamic_cast<BranchedTemplate *> (canvas_items.at(i));
        if (linear)
        {
            linearTemplateToConstraints(linear);
        }
        else if (branched)
        {
            branchedTemplateToConstraints(branched);
        }
    }

    //qDebug("Total connector crossings=%d", noOfConnectorCrossings());
    
    if (canvas_->optFitWithinPage())
    {
        cerr << "Extracting page boundaries: tl.x=" << 
                pageBoundary.left() << " tl.y=" << 
                pageBoundary.top() << " br.x=" << 
                pageBoundary.right() << " br.y=" << 
                pageBoundary.bottom() << endl;
        cola::PageBoundaryConstraints* pbc =
                new cola::PageBoundaryConstraints(
                        pageBoundary.left(),
                        pageBoundary.right(),
                        pageBoundary.top(),
                        pageBoundary.bottom());

        // for each shape on the canvas create a PageBoundaryConstraint that 
        // has the shape's center position (its vertex pos) and the offset 
        // from that pos being dependant on the shape's dimensions
        //shape_select(lastFreehand);
        for(int i = 0; i < canvasObjects.size(); ++i)
        {
            if (ShapeObj *shape = isShapeForLayout(canvasObjects.at(i)))
            {
                double w = shape->width();
                double h = shape->height();  
                // add the offsets for this shape
                // there is an offset for:
                //   + pos left from center = w /2
                //   + pos right from center = -w/2
                //   + pos top from center = h/2
                //   + pos bottom from center = -h/2
                pbc->addShape(getNodeID(shape), w/2, h/2);
            }
        }
        ccs.push_back(pbc);
    }

#if 0
#ifndef NOGRAPHVIZ
        {
            graphvizLayout=auto_ptr<GraphvizLayout>(new GraphvizLayout(*this));
        }
#endif

    qDebug("GraphData ctor done: ccs=%d, rs=%d, "
           "topologyNodes=%d, topologyRoutes=%d", (int) ccs.size(),
           (int) rs.size(), (int) topologyNodesCount,
           (int) topologyRoutes.size());
#endif

    qDebug("GraphData ctor done: ccs=%d, rs=%d",
           (int) ccs.size(), (int) rs.size());
}


void resolveOverlappingRectangles(Rectangles &rs, vpsc::Dim dim) {
    unsigned n=rs.size();
    //qDebug("resolveOverlappingRectangles: n=%d",n);
    Variables vs(n);
    unsigned i=0;
    for(Variables::iterator v=vs.begin();v!=vs.end();++v,++i) {
        *v=new Variable(i,0,1);
    }
    Constraints cs;
    if(dim==vpsc::HORIZONTAL) {
        generateXConstraints(rs,vs,cs,false);
    } else {
        generateYConstraints(rs,vs,cs);
    }
    try {
        vpsc::IncSolver vpsc(vs,cs);
        vpsc.solve();
    } catch (char *str) {
        std::cerr<<str<<std::endl;
        for(vpsc::Rectangles::iterator r=rs.begin();r!=rs.end();++r) {
            std::cerr << **r <<std::endl;
        }
    }
    vpsc::Rectangles::iterator r=rs.begin();
    for(vpsc::Variables::iterator v=vs.begin();v!=vs.end();++v,++r) {
        assert((*v)->finalPosition==(*v)->finalPosition);
        if(dim==vpsc::HORIZONTAL) {
            (*r)->moveCentreX((*v)->finalPosition);
        } else {
            (*r)->moveCentreY((*v)->finalPosition);
        }
    }
    assert(r==rs.end());
    for_each(cs.begin(),cs.end(),delete_object());
    for_each(vs.begin(),vs.end(),delete_object());
}

/**
 * Functor for use in forEach loop over the points in a topology::Edge to copy
 * each EdgePoint into an Avoid::PolyLine
 */
struct GDCopyClusterVertex {
    GDCopyClusterVertex(GraphData *gd, vector<Avoid::Point>& points) 
        : gd(gd), points(points), i(0) {}
    void operator() (const topology::EdgePoint* p) {
        assert(gd != NULL);
        points[i].x = p->posX();
        points[i].y = p->posY();
        points[i].id = gd->getShape(p->node->id)->internalId();
        points[i].vn = p->rectIntersectAsVertexNumber();
        ++i;
    }
    GraphData *gd;
    vector<Avoid::Point>& points;
    unsigned i;
};


void GraphData::generateRoutes() {
    // find feasible topologyRoutes for edges
    Avoid::Router *router = new Avoid::Router(Avoid::PolyLineRouting);
    // We need to use rubber-banding for generating topologyRoutes.
    bool generate_topologyRoutes_with_rubber_banding = true;
    if (generate_topologyRoutes_with_rubber_banding)
    {
        router->RubberBandRouting=true;
    }
    // Use rotational sweep for point visibility
    router->UseLeesAlgorithm = true;
    // Don't use invisibility graph.
    router->InvisibilityGrph = false;
    std::map<unsigned int, unsigned> idMap;
    for(unsigned i=0;
            i<min(topologyNodesCount,(unsigned int)shape_vec.size());++i) {
        vpsc::Rectangle* r=rs[i];
        double x=r->getMinX();
        double X=r->getMaxX();
        double y=r->getMinY();
        double Y=r->getMaxY();
        // Create the ShapeRef:
        Avoid::Polygon shapePoly(4);
        // AntiClockwise!
        shapePoly.ps[0] = Avoid::Point(X,y);
        shapePoly.ps[1] = Avoid::Point(X,Y);
        shapePoly.ps[2] = Avoid::Point(x,Y);
        shapePoly.ps[3] = Avoid::Point(x,y);
        unsigned int shapeID = shape_vec[i]->internalId();
        idMap[shapeID] = i;
#ifdef PATHDEBUG
        qDebug("Shape[%3d]:  %g. %g - %g %g", shapeID, x, y, X, Y);
#endif
        new Avoid::ShapeRef(router, shapePoly, shapeID);
    }
    // process transaction so shapes are added to the router before
    // we try and reference them with the cluster boundaries.
    router->processTransaction();
    for(unsigned i=0; i < topologyRoutes.size(); ++i)
    {
        topology::Edge *e = topologyRoutes[i];

        // Add cluster to libavoid:
        vector<Avoid::Point> points;
        points.resize(e->nSegments + 1);
        e->forEachEdgePoint(GDCopyClusterVertex(this, points));
        Avoid::Polygon clusterPoly(e->nSegments);
        for (unsigned int n = 0; n < e->nSegments; ++n)
        {
            clusterPoly.ps[n] = points[n];
        }
        unsigned int clusterID = dunnartClusters[i]->internalId();
        new Avoid::ClusterRef(router, clusterPoly, clusterID);
    }
 
    for(unsigned i=0;i<conn_vec.size();++i) {
        cola::Edge e=edges[i];
        if(e.first>=topologyNodesCount||e.second>=topologyNodesCount)
            continue;
        Avoid::ConnRef *connRef;
        unsigned int connID = conn_vec[i]->internalId();
        vpsc::Rectangle* r0=rs[e.first], *r1=rs[e.second];
        Avoid::Point srcPt(r0->getCentreX(),r0->getCentreY());
        Avoid::Point dstPt(r1->getCentreX(),r1->getCentreY());
#ifdef PATHDEBUG
        qDebug("=Conn[%3d]================================================",
                connID);
#endif
        const Avoid::PolyLine& oldRoute = conn_vec[i]->avoidRef->route();
        if (generate_topologyRoutes_with_rubber_banding && !oldRoute.empty()) {
            Avoid::Point srcPt(r0->getCentreX(),r0->getCentreY());
            Avoid::Point dstPt(oldRoute.ps[1].x,oldRoute.ps[1].y);
            connRef = new Avoid::ConnRef(router, srcPt, dstPt, connID);
            connRef->setSourceEndpoint(srcPt);
            for (size_t j = 1; j < oldRoute.size(); ++j) {
                const Avoid::Point& ep=oldRoute.ps[j];
                dstPt.x=ep.x;
                dstPt.y=ep.y;
#ifdef PATHDEBUG
                qDebug("Trace(%2lu):  [%d %d] %g, %g", j, ep.id, ep.vn,
                        ep.x, ep.y);
#endif
                connRef->makePathInvalid();
                bool exists = false;
                size_t last = (oldRoute.size() - 1);
                if (j < last)
                {
                    // Tracing: Use the ID of the point.
                    exists = connRef->setEndpoint(Avoid::VertID::tar,
                            Avoid::VertID(ep.id, ep.vn,  
                                Avoid::VertID::PROP_ConnPoint), &dstPt);
                }
                if (!exists)
                {
                    if (j == last)
                    {
                        // Use updated position for shape, since it might
                        // have been moved by makeFeasible().
                        dstPt.x = r1->getCentreX();
                        dstPt.y = r1->getCentreY();
                    }
                    // Last one, or now non-existant point, use a normal point.
                    connRef->setEndpoint(Avoid::VertID::tar, dstPt);
                }
                router->processTransaction();
            }
        } else {
            Avoid::Point srcPt(r0->getCentreX(),r0->getCentreY());
            Avoid::Point dstPt(r1->getCentreX(),r1->getCentreY());
            connRef = new Avoid::ConnRef(router, srcPt, dstPt, connID);
            router->processTransaction();
        }
        const Avoid::PolyLine& route = connRef->route();
        vector<topology::EdgePoint*> eps;
        //cout << "addToPath(vs[" << e.first << "],EdgePoint::CENTRE);" << endl;
        assert(e.first<topologyNodesCount);
        eps.push_back( new topology::EdgePoint( topologyNodes[e.first], 
                    topology::EdgePoint::CENTRE));
        for(size_t j=1;j<route.size()-1;j++) {
            const Avoid::Point& p = route.ps[j];
            const unsigned nodeID = idMap[p.id];
            assert(nodeID!=e.first);
            assert(nodeID!=e.second);
            //qDebug("  (%f,%f)", r->xs[j], r->ys[j]);
            //cout << "addToPath(vs[" << nodeID << "],";
            assert(nodeID<topologyNodesCount);
            topology::Node* node=topologyNodes[nodeID];
            topology::EdgePoint::RectIntersect ri;
            switch(p.vn) {
                case 0: ri=topology::EdgePoint::BR; 
                        //cout << "EdgePoint::BR);" << endl;
                        break;
                case 1: ri=topology::EdgePoint::TR; 
                        //cout << "EdgePoint::TR);" << endl;
                        break;
                case 2: ri=topology::EdgePoint::TL;
                        //cout << "EdgePoint::TL);" << endl;
                        break;
                case 3: ri=topology::EdgePoint::BL; 
                        //cout << "EdgePoint::BL);" << endl;
                        break;
                default: ri=topology::EdgePoint::CENTRE;
            }
            eps.push_back(new topology::EdgePoint(node,ri));
        }
        assert(e.second<topologyNodesCount);
        eps.push_back(new topology::EdgePoint(topologyNodes[e.second],
                    topology::EdgePoint::CENTRE));
        //cout << "addToPath(vs[" << e.second << "],EdgePoint::CENTRE);" << endl;
        topologyRoutes.push_back(new topology::Edge(i,edgeLengths[i], eps));
    }
    assert(topology::assertNoSegmentRectIntersection(topologyNodes,topologyRoutes));
    assert(topology::assertConvexBends(topologyRoutes));
    delete router;
}
/**
 * We construct the constraints as we create the graph, so they need to be cleaned up
 */
GraphData::~GraphData() {
    for_each(ccs.begin(),ccs.end(),delete_object());
    for_each(topologyRoutes.begin(),topologyRoutes.end(),delete_object());
}

/**
 * Either constructs or gets an existing dummy/real node for the specified node
 * handle.  Specifically, if the handle indicates the centre, return the actual
 * node corresponding to nodeID, otherwise return a dummy node correctly
 * constrainted relative to the real node for the specified port.
 * @param nodeID node index
 * @param handleFlags specifies a port for the node
 * @return ID of the port node
 */
unsigned GraphData::getConnectionPoint(const CPoint& connPointInfo)
{
    unsigned nodeID = getNodeID(connPointInfo.shape);
    unsigned pinClassID = connPointInfo.pinClassID;

    // Just treat all connections like centre connections for the moment.
    return nodeID;

    if(pinClassID == CENTRE_CONNECTION_PIN)
    {
        return nodeID;
    }

    double handleSize=0.1;
    double x,X,y,Y,cx,cy;
    vpsc::Rectangle *r=rs[nodeID];
    x = connPointInfo.x - handleSize/2.0;
    y = connPointInfo.y - handleSize/2.0;
    X = x + handleSize;
    Y = y + handleSize;
    cx = r->getCentreX() - connPointInfo.x;
    cy = r->getCentreY() - connPointInfo.y;

    // create dummy node for start handle and constraints to
    // position dummy node relative to centre of node
    unsigned dummyNodeID=rs.size();
    rs.push_back(new vpsc::Rectangle(x,X,y,Y,true));
    ccs.push_back(new cola::SeparationConstraint(vpsc::XDIM, nodeID,
                dummyNodeID, cx, true));
    ccs.push_back(new cola::SeparationConstraint(vpsc::YDIM, nodeID,
                dummyNodeID, cy, true));
    addEdge(nodeID,dummyNodeID,fabs(cx+cy));

    return dummyNodeID;
}

/**
 * creates a cola graph node from a dunnart Shape.
 * @param shape the Dunnart shape for which the cola graph node is created
 */
size_t GraphData::shapeToNode(ShapeObj* shape) {
    assert(shape != NULL);
    double g=0;
    double buffer = shape->canvas()->optShapeNonoverlapPadding();
    QRectF rect = shape->shapeRect(buffer + g);
    unsigned nodeID=rs.size();
    snMap[shape]=nodeID;
    rootNodes.insert(nodeID);
    bool allowOverlap = false;
    if(rect.width() < 1.0) {
        qWarning("dummy node, size<1 found - allowing overlap");
        allowOverlap=true;
    }
    vpsc::Rectangle *r = new vpsc::Rectangle(rect.left(), rect.right(),
            rect.top(), rect.bottom(), allowOverlap);
    qWarning("Node id=%d, (x,y)=(%f,%f), (w,h)=%f,%f",
            shape->internalId(),r->getCentreX(), r->getCentreY(),
            r->width(), r->height());

    rs.push_back(r);

    if(nodeID<topologyNodesCount) {
        // create topology reps for the first topologyNodesCount nodes only.
        topologyNodes.push_back(new topology::Node(nodeID,r));
    }

    shape_vec.push_back(shape);

    return rs.size() - 1;
}
void promotedups(cola::Cluster* c)
{
    Q_UNUSED (c)

    // NodeFrqMap<Node,int> fm
    // NodeClusterMap<Node,Cluster> cm
    // for each cluster d in c:
    //   promotedups(d)
    //     for node n in d
    //     fm[n]++
    // for each node n where fm[n]>1
    //   move n from cm[n] into c
}
/**
 * Creates cola clusterHierarchy for a dunnart Cluster.
 * @param cluster the dunnart representation
 */
void GraphData::dunnartClusterToCluster(Cluster* cluster) {
    assert(!cluster->isCollapsed());

    ShapeList& l=cluster->getMembers();
    cola::Cluster *c = NULL;
    if (cluster->rectangular) {
        cola::RectangularCluster *rectC = new cola::RectangularCluster();
        double buffer = canvas_->optShapeNonoverlapPadding();
        rectC->setMargin(buffer * 2);
        c = rectC;
    } else {
        c = new cola::ConvexCluster();
    }
    clusterHierarchy.clusters.push_back(c);
    dunnartClusters.push_back(cluster);
    for(ShapeList::iterator i = l.begin();
            i!=l.end();i++) {
        unsigned nodeID = snMap[*i];
        c->nodes.push_back(nodeID);
        rootNodes.erase(nodeID);
    }

#if 0
    if (cluster->isCollapsed())
    {
        double clx, cly, clw, clh;
        cluster->getPosAndSize(clx, cly, clw, clh);

        double clcx = clx + (clw / 2);
        double clcy = cly + (clh / 2);

        // Cluster is collapsed, so create constraints to keep the member 
        // objects positioned relative to each other.

        cola::AlignmentConstraint *vac = new cola::AlignmentConstraint(clcx);
        vac->guide = NULL;
        ccsx.push_back(vac);
        qDebug("Collapsed cluster:");

        cola::AlignmentConstraint *hac = new cola::AlignmentConstraint(clcy);
        hac->guide = NULL;
        ccsy.push_back(hac);

        qDebug("Cluster centre: %g %g\n", clcx, clcy);

        ShapeList& members = cluster->getMembers();
        ClusterShapeMap& smallShapeInfo = cluster->getSmallShapeInfo();
        for (ShapeList::iterator curr = members.begin(); 
            curr != members.end(); ++curr)
        {
            double offx, offy, sw, sh;
            smallShapeInfo[(*curr)->getId()]->getOffsetAndSize(offx, offy,
                    sw, sh);
            qDebug("Shape id: %d", snMap[*curr]);
            qDebug("Shape offset: %g %g", offx, offy);
            qDebug("Shape size: %g %g", sw, sh);
            qDebug("Shape pos: %g %g", clcx - offx, clcy - offy);
            double sx, sy, sw2, sh2;
            (*curr)->getPosAndSize(sx, sy, sw2, sh2);
            qDebug("Real Shape size: %g %g", sw2, sh2);
            qDebug("Real Shape pos: %g %g", sx, sy);
            
            vac->offsets.push_back(make_pair(snMap[(*curr)],-offx));
            hac->offsets.push_back(make_pair(snMap[(*curr)],-offy));
        }
    }
#endif
}


void GraphData::setUpRootCluster() {
    clusterHierarchy.nodes.resize(rootNodes.size());
    copy(rootNodes.begin(),rootNodes.end(),clusterHierarchy.nodes.begin());
}
/**
 * Creates cola edge for a dunnart Conn.
 * @param conn the dunnart representation
 * @param mode the layout mode may affect the constraints/type of edge created
 */
void GraphData::connectorToEdge(Connector* conn)
{
    conn_vec.push_back(conn);
    QPair<CPoint, CPoint> connpts = conn->get_connpts();
    if(!connpts.first.shape||!connpts.second.shape) {
        qWarning("dangling connector!");
        return;
    }
    unsigned a=getConnectionPoint(connpts.first);
    unsigned b=getConnectionPoint(connpts.second);
#if 0
    if (conn->orthogonalConstraint != Conn::NONE)
    {
        //qDebug("Creating orthogonalConstraint");
        cola::OrthogonalEdgeConstraint *oec = 
                new cola::OrthogonalEdgeConstraint(a, b);
        if (conn->orthogonalConstraint == Conn::HORIZONTAL)
        {
            ccsy.push_back(oec);
            orthogonalEdgeCountY++;
        }
        else
        {
            ccsx.push_back(oec);
            orthogonalEdgeCountX++;
        }
    }
#endif

    double idealLength = conn->idealLength();
    addEdge(a,b,idealLength);

    // dump dot format
    //qDebug("%d->%d;",a,b);
}


/** once edges are loaded the following detects multi-edges and sets up the
 * connector so that they are rendered with offsets.
 */
void GraphData::setupMultiEdges() {
    list<Connector*> conList(conn_vec.begin(),conn_vec.end());
    list<list<Connector*> > conDups;
    for(list<Connector*>::iterator i=conList.begin();i!=conList.end();i++) {
        (*i)->m_is_multiedge=false;
    }
    for(list<Connector*>::iterator i=conList.begin();i!=conList.end();)
    {
        QPair<ShapeObj *, ShapeObj *> p1 = (*i)->getAttachedShapes();
        list<Connector*> dups;
        dups.push_back(*i);
        for(list<Connector*>::iterator j=++i;j!=conList.end();j++) {
            if(!(*j)->m_is_multiedge && p1.first && p1.second) {
                QPair<ShapeObj *, ShapeObj *> p2 = (*j)->getAttachedShapes();
                if(   (p1.first==p2.first && p1.second==p2.second)
           || (p1.first==p2.second && p1.second==p2.first)) {
                    (*j)->m_is_multiedge=true;
                    dups.push_back(*j);
                }
            }
        }
        if(dups.size()>1) { 
            conDups.push_back(dups);
        }
    }
    for(list<list<Connector*> >::iterator i=conDups.begin();i!=conDups.end();i++) {
        qDebug("Duplicates: ");
        unsigned ctr=0;
        for(list<Connector*>::iterator j=i->begin();j!=i->end();j++) {
            QPair<ShapeObj *, ShapeObj *> p = (*j)->getAttachedShapes();
            qDebug("+-(%d, %d)", p.first->internalId(), p.second->internalId());
            (*j)->m_multiedge_size=i->size();
            (*j)->m_multiedge_index=ctr++;
        }
    }
}


void GraphData::generateRectangleConstraints(
        QList<CanvasItem *>& canvasChildren)
{
    if (!canvas_->m_rectangle_constraint_test)
    {
        return;
    }

    std::vector<unsigned> idList;
    for (int i = 0; i < canvasChildren.size(); ++i)
    {
        if (ShapeObj *shape = isShapeForLayout(canvasChildren.at(i)))
        {
            if (shape->internalId() <= 1)
            {
                continue;
            }
            idList.push_back(snMap[shape]);
        }
    }
    RectangleConstraint *rc = new RectangleConstraint(
            300, 600, 200, 500, idList);
    ccs.push_back(rc);
}


/**
 * Creates cola::AlignmentConstraint for a dunnart Guideline.
 * @param guide the dunnart representation
 */
void GraphData::guideToAlignmentConstraint(Guideline* guide) {
    vpsc::Dim dim = 
            (guide->get_dir()==GUIDE_TYPE_HORI) ? vpsc::YDIM : vpsc::XDIM;
    cola::AlignmentConstraint *ac = 
            new cola::AlignmentConstraint(dim, guide->position());
    ccMap[guide]=ac;
    ac->indicator=(void*)guide;
    ccs.push_back(ac);
    for(RelsList::iterator i = guide->relationships.begin();
            i!=guide->relationships.end();i++) {
        double offset = 0;
        ShapeObj* s = (*i)->shape;
        // if s is null then it's a distribution constraint
        // which we handle separately below
        if(s!=NULL) {
            double width = s->width();
            double height = s->height();
            
            switch((*i)->type) {
                case ALIGN_RIGHT:
                    offset = -width/2.0; break;
                case ALIGN_LEFT:
                    offset = width/2.0; break;
                case ALIGN_BOTTOM:
                    offset = -height/2.0; break;
                case ALIGN_TOP:
                    offset = height/2.0; break;
                default: // ALIGN_CENTER, ALIGN_MIDDLE: offset=0
                    break;
            }
            ac->addShape(snMap[s], offset);
        }
    }
}


void GraphData::linearTemplateToConstraints(LinearTemplate* templatPtr)
{
    // Find the cola::CompoundConstraint representing the alignment.
    cola::AlignmentConstraint *ac = 
           dynamic_cast<cola::AlignmentConstraint *> (ccMap[templatPtr->guide]);

    // Store the IDs of all shapes attached to the guideline.
    std::list<unsigned> idList = ac->subConstraintObjIndexes();

    // Set the constraint as either being horizontal or vertical, depending
    // on the orientation of the dunnart guideline object.
    vpsc::Dim dim = (templatPtr->guide->get_dir() == GUIDE_TYPE_HORI) ?
            vpsc::YDIM : vpsc::XDIM;
    
	double separation = 0;
	double minAttached, maxAttached;
	int nodes;
        templatPtr->guide->rangeOfAttachedShapes(minAttached, maxAttached, nodes);
    if (templatPtr->_explicitNewBranchSpan)
    {
        separation = templatPtr->_explicitNewBranchSpan;
        templatPtr->_explicitNewBranchSpan = 0;
    }
    else if (nodes > 1 ) {
      separation = (maxAttached - minAttached) / (nodes - 1);
    }

    // Create the CompoundConstraint for the Linear Template.
    LinearTemplateConstraint *ltc = new LinearTemplateConstraint(dim, ac,
            idList, separation);
    // Add it to the map.
    ccMap[templatPtr] = ltc;
    // Set a pointer in the compound constraint back to the Dunnart object.
    ltc->indicator = (void*) templatPtr;

    ccs.push_back(ltc);
}

// Create compound constraints for branched template
void GraphData::branchedTemplateToConstraints(BranchedTemplate* templatPtr)
{
    std::list<unsigned> idList;
    // generate the list of alignment constraints for all the guides
    // in the branched template
    ACList aclist;  

    // Find the cola::CompoundConstraint representing the alignment of the 
    // guides
    for (GuidelineList::iterator currguide = templatPtr->guidelist.begin(); 
            currguide != templatPtr->guidelist.end(); ++currguide)
    {
        if (dynamic_cast<Guideline *> (*currguide)) 
        {
            Guideline* guide = (Guideline*)(*currguide);
            cola::AlignmentConstraint *ac = 
                    dynamic_cast<cola::AlignmentConstraint *> (ccMap[guide]);
    
            // Store the IDs of all shapes attached to the guideline.
            std::list<unsigned> idList = ac->subConstraintObjIndexes();
            aclist.push_back(ac);
        }
    }

    // create a list of id lists for the BranchedTemplateConstraint
    std::vector< std::list<unsigned> > listOfIdLists;
    //  std::vector<double> separations;
    double sep = 0;
    int longest_branch_index = -1;
    int centreguide_index = -1; int count = 0;
    for (GuidelineList::iterator curr = templatPtr->guidelist.begin(); 
            curr != templatPtr->guidelist.end(); ++curr) 
    {
        if (dynamic_cast<Guideline *> (*curr)) 
        {
            Guideline* guide = (Guideline*)(*curr);
            double minAttached, maxAttached;
            int nodes;
            guide->rangeOfAttachedShapes(minAttached, maxAttached, nodes);

            // Create a list of the IDs of all shapes attached to the 
            // current guide
            cola::AlignmentConstraint *ac = 
                    dynamic_cast<cola::AlignmentConstraint *> (ccMap[guide]);
            std::list<unsigned> idList = ac->subConstraintObjIndexes();
            listOfIdLists.push_back(idList); // store the list of IDs

            // check if this is the centre guide, and if it is, keep its index:
            if (guide == templatPtr->centreguide) 
            { 
                centreguide_index = count;
            }
            // otherwise, calculate the max branch separation:
            else if (nodes > 1) 
            {
                if ((maxAttached - minAttached) > sep) 
                {
                    sep = (maxAttached - minAttached);
                    longest_branch_index = count;
                }
            }
            count++;
        }
      
    }

    // Determine primary dimension.
    vpsc::Dim dim = (templatPtr->centreguide->get_dir() == GUIDE_TYPE_HORI) ?
            vpsc::YDIM : vpsc::XDIM;

    if (templatPtr->_explicitNewBranchSpan)
    {
        sep = templatPtr->_explicitNewBranchSpan;
        templatPtr->_explicitNewBranchSpan = 0;
    }
    // Create the CompoundConstraint for the guides in the Branched Template, 
    // both in the perpendicular dimension to the guideline and the same
    // dimension.
    BranchedTemplateConstraint *btc = new BranchedTemplateConstraint(dim, 
          aclist, templatPtr->getPos(), 50, listOfIdLists, sep, 
          centreguide_index, longest_branch_index);
    // Add it to the map.
    ccMap[templatPtr] = btc;
    // Set a pointer in the compound constraint back to the Dunnart object.
    btc->indicator = (void*) templatPtr;
    // add the compound constraint to the solver:
    ccs.push_back(btc);
}


/**
 * Creates cola::DistributionConstraint for a dunnart Distribution.
 * @param distro the dunnart representation
 */
void GraphData::distroToDistributionConstraint(Distribution* distro) 
{
    if (distro->relationships.size() < 1)
    {
        // Empty distribution.
        return;
    }
    dirctn d = distro->relationships.front()->guide->get_dir();
    vpsc::Dim dim = (d==GUIDE_TYPE_HORI) ? vpsc::YDIM : vpsc::XDIM;
    cola::DistributionConstraint* dc = new cola::DistributionConstraint(dim);
    dc->indicator = (void *) distro;
    ccMap[distro]=dc;
    dc->sep = distro->getSeparation();
    for (RelsList::iterator i = distro->relationships.begin();
            i!=distro->relationships.end();i++) 
    {
        Guideline* g1 = (*i)->guide;
        Guideline* g2 = (*i)->guide2;
        cola::AlignmentConstraint *ac1 =
                (cola::AlignmentConstraint *) ccMap[g1];
        cola::AlignmentConstraint *ac2 = 
                (cola::AlignmentConstraint *) ccMap[g2];
        assert(ac1!=NULL);
        assert(ac2!=NULL);
        dc->addAlignmentPair(ac1, ac2);
        d=g1->get_dir();
    }
    ccs.push_back(dc);
}
/**
 * Creates cola::MultiSeparationConstraint for a dunnart Separation.
 * @param sep the dunnart representation
 */
void GraphData::separationToMultiSeparationConstraint(Separation* sep)
{
    if (sep->relationships.empty())
    {
        // Empty distribution.
        return;
    }
    dirctn d = sep->relationships.front()->guide->get_dir();
    vpsc::Dim dim = (d==GUIDE_TYPE_HORI) ? vpsc::YDIM : vpsc::XDIM;
    bool equality = false; // Distributions have replaced equality separations.
    cola::MultiSeparationConstraint* c =
            new cola::MultiSeparationConstraint(dim, sep->gap, equality);
    c->indicator = (void *) sep;
    ccMap[sep]=c;
    for(RelsList::iterator i = sep->relationships.begin();
            i!=sep->relationships.end();i++) {
        Guideline* g1 = (*i)->guide;
        Guideline* g2 = (*i)->guide2;
        cola::AlignmentConstraint *ac1 =
                (cola::AlignmentConstraint *) ccMap[g1];
        cola::AlignmentConstraint *ac2 =
                (cola::AlignmentConstraint *) ccMap[g2];
        assert(ac1!=NULL);
        assert(ac2!=NULL);
        c->addAlignmentPair(ac1, ac2);
        d=g1->get_dir();
    }
    ccs.push_back(c);
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

