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

#ifndef GRAPHDATA_H
#define GRAPHDATA_H

#include <QRectF>
#include <vector>
#include <map>
#include <memory>

#include "libdunnartcanvas/graphlayout.h"
#include "libcola/cola.h"

namespace topology {
    class Node;
    class Edge;
}

namespace dunnart {


class Shape;
class LinearTemplate;
class BranchedTemplate;
class CPoint;
#ifndef NOGRAPHVIZ
class GraphvizLayout;
#endif
class Canvas;
/**
 * GraphLayout constructs a cola graph representation from dunnart objects and maintains the
 * graph structure and various mapping lookups here
 */
class GraphData {
public:
    GraphData(Canvas *canvas, bool ignoreEdges, GraphLayout::Mode mode, 
            bool beautify, unsigned topologyNodesCount);
    void generateRoutes();
    ~GraphData(); 
    /** once edges are loaded the following detects multi-edges and sets up the
     * connector so that they are rendered with offsets.
     */
    void setupMultiEdges();
    /** return pointer to dunnart shape based on ID.  If called with the ID of a
     * dummy node it returns NULL.
     */
    ShapeObj* getShape(unsigned i) const {
        if(i<shape_vec.size()) {
            return shape_vec[i];
        }
        return NULL;
    }
    /// get ID from shape reference
    unsigned getNodeID(ShapeObj* shape) const {
        std::map<ShapeObj*, unsigned>::const_iterator i=snMap.find(shape);
        assert(i!=snMap.end());
        return i->second;
    }
    /// gets a cola constraint from the dunnart Indicator reference
    cola::CompoundConstraint* getConstraint(Indicator* ind) const {
        std::map<Indicator*, cola::CompoundConstraint*>::const_iterator i=ccMap.find(ind);
        assert(i!=ccMap.end());
        return i->second;
    }
    // the following getters return the graph data in the
    // most suitable format for cola layout algorithm constructors.
    cola::RootCluster* getRootCluster() {
        // return clusterHierarchy if one is present
        if(clusterHierarchy.clusters.size()>0) {
            return &clusterHierarchy;
        }
        return NULL;
    }
    void getEdgeLengths(std::vector<double> &elengths) {
        elengths = edgeLengths;
    }
    unsigned getNodeCount() {
        return rs.size();
    }
    unsigned getEdgeCount() {
        return edges.size();
    }
    unsigned getConnStart(unsigned i) {
        return edges[i].first;
    }
    unsigned getConnEnd(unsigned i) {
        return edges[i].second;
    }
    double getIdealEdgeLength(unsigned i) {
        return edgeLengths[i];
    }
    Canvas *canvas_;
    // The following members form the basic graph and constraint definitions
    //! Bounding boxes for each of the real nodes in the graph.
    std::vector<vpsc::Rectangle*> rs;
    //! list of edge pairs
    std::vector<cola::Edge> edges;
    //! Horizontal and vertical compound constraints 
    cola::CompoundConstraints ccs; 
    //! list of path routings associated with each edge
    std::vector<topology::Edge*> topologyRoutes;
    //! nodes used in routings
    std::vector<topology::Node*> topologyNodes;
    //! pageBoundary updated to current canvas page by apply
    QRectF pageBoundary;
    //! topology routes only considered for the first topologyNodesCount nodes
    unsigned topologyNodesCount;
    // the xxxxToXxxx methods create cola objects for dunnart graph objects
    size_t shapeToNode(ShapeObj* shape);
    void connectorToEdge(Connector* conn);
    void dunnartClusterToCluster(Cluster* cluster);
    void guideToAlignmentConstraint(Guideline* guide);
    void generateRectangleConstraints(QList<CanvasItem *>& canvasChildren);
    void distroToDistributionConstraint(Distribution* distro);
    void separationToMultiSeparationConstraint(Separation* sep);
    void linearTemplateToConstraints(LinearTemplate* templatPtr);
    void branchedTemplateToConstraints(BranchedTemplate* templatPtr);
    cola::RootCluster clusterHierarchy;
private:
    unsigned addEdge(unsigned u, unsigned v, double l) {
        //printf("edges[%d]=Edge(%d,%d);\n",edges.size(),u,v);
        unsigned id = edges.size();
        edges.push_back(std::make_pair(u,v));
        edgeLengths.push_back(l);
        return id;
    }
    void setUpRootCluster();
    unsigned getConnectionPoint(const CPoint& connPointInfo);
    std::map<ShapeObj*, unsigned> snMap;
    QMap<ShapeObj*, cola::Cluster *> rectClusterShapeMap;
    std::map<Indicator*,cola::CompoundConstraint*> ccMap;
    std::map<cola::Cluster*,Cluster*> clusterMap;
    std::vector<ShapeObj*> shape_vec;
    std::set<unsigned> rootNodes;
    std::vector<Connector*> conn_vec;
    std::vector<double> edgeLengths;
    std::vector<Cluster*> dunnartClusters;
    unsigned orthogonalEdgeCountX, orthogonalEdgeCountY;
#ifndef NOGRAPHVIZ
    std::auto_ptr<GraphvizLayout> graphvizLayout;
#endif
    friend class PostIteration;
    friend class SCCDetector;
};

}
#endif // GRAPHDATA_H

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

