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

#ifndef NOGRAPHVIZ

#include <sstream>
#include <map>

#include <gvc.h>

#include "libdunnartcanvas/graphdata.h"

#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/graphvizlayout.h"
#include "libvpsc/rectangle.h"
#include "libtopology/topology_graph.h"

#include "libavoid/router.h"

#ifndef ND_coord

// Older versions of graphviz, need to use ND_coord_i and 
// convert the format to boxf.
#define ND_coord ND_coord_i
#ifndef B2BF
    boxf B2BF(box const b) {
      boxf bf;
      bf.LL.x = b.LL.x;
      bf.LL.y = b.LL.y;
      bf.UR.x = b.UR.x;
      bf.UR.y = b.UR.y;
    }
#endif

#else 

// For newer versions of graphviz, ND_coord exists and 
// the box will already be boxf.
#define B2BF(a) a

#endif

namespace dunnart {

struct Transform {
	boxf inbb, outbb;
	double xscale, yscale;
	Transform(boxf inbb, QRectF pageBoundary) : inbb(inbb) {
		outbb.UR.x=pageBoundary.right();
		outbb.UR.y=pageBoundary.bottom();
		outbb.LL.x=pageBoundary.left();
		outbb.LL.y=pageBoundary.top();
		xscale = (double)(outbb.UR.x - outbb.LL.x) 
		       / (double)(inbb.UR.x - inbb.LL.x);
		yscale = (double)(outbb.UR.y - outbb.LL.y) 
		       / (double)(inbb.UR.y - inbb.LL.y);
	}
	void apply(pointf const &in, pointf &out) {
		out.x = (outbb.LL.x+xscale*(in.x-inbb.LL.x));
		out.y = (outbb.UR.y-yscale*(in.y-inbb.LL.y));
	}
	void apply(point const &in, pointf &out) {
		out.x = (outbb.LL.x+xscale*(in.x-inbb.LL.x));
		out.y = (outbb.UR.y-yscale*(in.y-inbb.LL.y));
	}
};

typedef std::map<Agnode_t*,int> NodeMap;
class VisibilityGraph {
public:
    VisibilityGraph(Agraph_t* gv, GraphData& gd, NodeMap& nodeMap) 
        : gv(gv), gd(gd), nodeMap(nodeMap), 
          router(new Avoid::Router(Avoid::PolyLineRouting)) {
    }
    ~VisibilityGraph() {
        delete router;
        //for_each(shapeRefs.begin(),shapeRefs.end(),cola::delete_object());
    }
    /**
     * add v's neighbours to visibility graph
     * search left and right in GD_rank(gv)[ND_rank(u)].v for 
     * first non-virtual node in each direction (if any)
     */
    void addRankNeighbours(const Agnode_t* v) {
        rank_t& rank = GD_rank(gv)[ND_rank(v)];
        int vPos = ND_order(v);
        double rBoundary=DBL_MAX, lBoundary=-DBL_MAX;
        // search right
        for (int j = vPos+1; j < rank.n; ++j) {
            Agnode_t* u = rank.v[j];
            if(ND_node_type(u)==0) {
                assert(nodeMap.find(u)!=nodeMap.end());
                int id=nodeMap[u];
                rBoundary=addEndlessRectToRight(id,gd.rs[id]);
                break;
            }
        }
        // search left
        for (int j = vPos-1; j >= 0; --j) {
            Agnode_t* u = rank.v[j];
            if(ND_node_type(u)==0) {
                assert(nodeMap.find(u)!=nodeMap.end());
                int id=nodeMap[u];
                lBoundary=addEndlessRectToLeft(id,gd.rs[id]);
                break;
            }
        }
        assert(lBoundary<rBoundary);
    }
    /**
     * route cola::Edge e around the rectangles in the visibility graph
     */
    void routeAround(unsigned i) {
        const cola::Edge e=gd.edges[i];
        unsigned connID = i + gd.rs.size() + 1;
        vpsc::Rectangle* r0=gd.rs[e.first], *r1=gd.rs[e.second];

        Avoid::Point srcPt(r0->getCentreX(),r0->getCentreY());
        Avoid::Point dstPt(r1->getCentreX(),r1->getCentreY());
        assert(dstPt.y>srcPt.y);
        Avoid::ConnRef *connRef = new Avoid::ConnRef(router, srcPt, dstPt,
                connID);
        router->processTransaction();
        const Avoid::PolyLine& route = connRef->route();
        std::vector<topology::EdgePoint*> eps;
        eps.push_back( new topology::EdgePoint( gd.topologyNodes[e.first], 
                    topology::EdgePoint::CENTRE));
        double y,lastY=gd.rs[e.first]->getCentreY();
        for(size_t j=1;j<route.size()-1;j++) {
            const Avoid::Point& p = route.ps[j];
            const unsigned nodeID=p.id-1;
            topology::Node* node=gd.topologyNodes[nodeID];
            topology::EdgePoint::RectIntersect ri;
            switch(p.vn) {
                case 0: ri=topology::EdgePoint::BR; 
                        y=gd.rs[nodeID]->getMinY();
                        break;
                case 1: ri=topology::EdgePoint::TR; 
                        y=gd.rs[nodeID]->getMaxY();
                        break;
                case 2: ri=topology::EdgePoint::TL;
                        y=gd.rs[nodeID]->getMaxY();
                        break;
                case 3: ri=topology::EdgePoint::BL; 
                        y=gd.rs[nodeID]->getMinY();
                        break;
                default: ri=topology::EdgePoint::CENTRE;
            }
            assert(y>lastY);
            lastY=y;
            eps.push_back(new topology::EdgePoint(node,ri));
        }
        eps.push_back(new topology::EdgePoint(gd.topologyNodes[e.second],
                    topology::EdgePoint::CENTRE));
        y=gd.rs[e.second]->getCentreY();
        assert(y>lastY);

        gd.topologyRoutes.push_back(new topology::Edge(i,gd.getIdealEdgeLength(i), eps));
        delete connRef;
    }
private:
    double addEndlessRectToRight(unsigned id, const vpsc::Rectangle* o) {
        double lBoundary=o->getMinX();
        vpsc::Rectangle r(lBoundary,DBL_MAX,o->getMinY(),o->getMaxY());
        addRect(id, &r);
        return lBoundary;
    }
    double addEndlessRectToLeft(unsigned id, const vpsc::Rectangle* o) {
        double rBoundary=o->getMaxX();
        vpsc::Rectangle r(-DBL_MAX,rBoundary,o->getMinY(),o->getMaxY());
        addRect(id, &r);
        return rBoundary;
    }
    void addRect(unsigned id, const vpsc::Rectangle* r) {
        // Create the ShapeRef:
        Avoid::Polygon shapePoly(4);
        // AntiClockwise!
        shapePoly.ps[0] = Avoid::Point(r->getMaxX(),r->getMinY());
        shapePoly.ps[1] = Avoid::Point(r->getMaxX(),r->getMaxY());
        shapePoly.ps[2] = Avoid::Point(r->getMinX(),r->getMaxY());
        shapePoly.ps[3] = Avoid::Point(r->getMinX(),r->getMinY());
        unsigned int shapeID = id + 1;
        Avoid::ShapeRef *shapeRef = 
                new Avoid::ShapeRef(router, shapePoly, shapeID);
        // ShapeRef constructor makes a copy of polygon so we can free it:
        router->addShape(shapeRef);
        shapeRefs.push_back(shapeRef);
    }
    Agraph_t* gv;
    GraphData& gd;
    NodeMap& nodeMap;
    Avoid::Router *router;
    std::vector<ShapeRef*> shapeRefs;
};
GraphvizLayout::GraphvizLayout(GraphData& g) : sep(NULL) {
	GVC_t *gvc = gvContext();
	Agraph_t* gv = agopen((char *) "g",AGDIGRAPH);
	unsigned n = g.getNodeCount(), m = g.getEdgeCount();
	std::vector<Agnode_t*> nodes(n);
	std::vector<Agedge_t*> edges(m);
    NodeMap nodeMap;
	for(unsigned i=0;i<n;i++) {
        std::ostringstream s; s << i;
        char ss[s.str().length()+1];
        strcpy(ss,s.str().c_str());
		nodes[i]=agnode(gv,ss);
        nodeMap[nodes[i]]=i;
	}
	for(unsigned i=0;i<m;i++) {
		edges[i]=agedge(gv, nodes[g.getConnStart(i)],nodes[g.getConnEnd(i)]);
	}
	gvLayout(gvc,gv,(char *) "dot");

	Transform transform(B2BF(GD_bb(gv)),g.pageBoundary);

	for(unsigned i=0;i<n;i++) {
		Agnode_t* v = nodes[i];
		pointf p;
		transform.apply(ND_coord(v),p);
        printf("v[%d]=(%g,%g)\n",i,p.x,p.y);
		g.rs[i]->moveCentreX(p.x);
		g.rs[i]->moveCentreY(p.y);
	}
    for(unsigned i=0;i<m;i++) {
        Agedge_t* e = edges[i];
        Agnode_t* u = e->tail;
        printf("u=%p, rank=%d\n",u,ND_rank(u));
        VisibilityGraph visibilityGraph(gv, g, nodeMap);
        //visibilityGraph.addRankNeighbours(u);
        Agnode_t* v=NULL;
        int lastRank = ND_rank(u);
        if(ED_to_virt(e)) {
            assert(ED_to_virt(e)->tail==u);
            v = ED_to_virt(e)->head;
            while(ND_node_type(v)==1) {
                printf("v=%p, rank=%d, type=%d\n",v,ND_rank(v),ND_node_type(v));
                assert(v!=NULL);
                assert(ND_rank(v)==lastRank+1);
                lastRank=ND_rank(v);
                visibilityGraph.addRankNeighbours(v);
                Agedge_t* out = *(ND_out(v).list);
                v = out->head;
            }
        }
        printf("v=%p, rank=%d\n",v,ND_rank(v));
        assert(v!=NULL);
        assert(ND_rank(v)==lastRank+1);
        assert(ND_node_type(v)==0);
        //visibilityGraph.addRankNeighbours(v);
        printf("\n");
        visibilityGraph.routeAround(i);
    }

    // Generate alignments for each level rank.
    // XXX: This creates problems (may crash/fail) if there are existing
    //      alignments relationships for any of the nodes.
    for (int i = GD_minrank(gv); i <= GD_maxrank(gv); i++) {
        CanvasItemList rankObjList;
        for (int j = 0; j < GD_rank(gv)[i].n; j++) {
            Agnode_t* n = GD_rank(gv)[i].v[j];
            assert(ND_order(n)==j);
            if(ND_node_type(n)==0) {
                rankObjList.push_back(g.getShape(nodeMap[n]));
            }
        }
        if (!rankObjList.empty())
        {
            // XXX: This is probably unsafe from within this thread.
            bool recordUndo = false;
            Guideline *guide =  createAlignment(ALIGN_MIDDLE, rankObjList, recordUndo);
            if (guide)
            {
                guideList.push_back(guide);
                g.guideToAlignmentConstraint(guide);
            }
        }
    }

    if ((int) guideList.size() > 1)
    {
        bool recordUndo = false;
        bool sortGuidelines = false;
        sep=createSeparation(SEP_VERTICAL, guideList, recordUndo, sortGuidelines);
        g.separationToMultiSeparationConstraint(sep);
        // QT Actions& actions = getActions();
        // QT actions.clear();
    }
	
    /* Free layout data */
    gvFreeLayout(gvc, gv);
    /* Free graph structures */
    agclose(gv);
    /* free context */
    gvFreeContext(gvc);
}
GraphvizLayout::~GraphvizLayout() {
    CanvasItemSet emptySet;
    if(sep) {
        sep->deactivateAll(emptySet);
    }
    for(QList<CanvasItem*>::iterator i=guideList.begin();i!=guideList.end();++i) {
        Guideline* g=dynamic_cast<Guideline*>(*i);
        assert(g);
        g->deactivateAll(emptySet);
        g->setVisible(false);
    }
}


}
#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

