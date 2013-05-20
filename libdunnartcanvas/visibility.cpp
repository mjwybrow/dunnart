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
 * Author(s): Michael Wybrow  <http://michael.wybrow.info/>
*/


#include <cstdlib>
#include <cassert>
#include <map>
#include <utility>

#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/visibility.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/interferencegraph.h"
#include "libavoid/vertices.h"
#include "libavoid/viscluster.h"
#include "libdunnartcanvas/cluster.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/connector.h"

#include "libavoid/router.h"

namespace dunnart {


using namespace Avoid;


//===========================================================================
//  Visibility Graph code:


// ConnPairSet is defined in interferencegraph.h
typedef std::map<unsigned int, int> TallyMap;

// -===============

static int calcConnectorIntersections(Canvas *canvas, PtOrderMap *ptOrders, 
        ConnPairSet *touchingConns, TallyMap *tallyMap, 
        ConnPairSet *crossingConnectors, Connector *queryConn,
        PointSet *crossingPoints = NULL);


int noOfConnectorCrossings(Connector *conn)
{
    PointSet crossingPoints;
    return calcConnectorIntersections(conn->canvas(), NULL, NULL, NULL, 
            NULL, conn, &crossingPoints);
}


void nudgeConnectors(Canvas *canvas, const double nudgeDist, 
        const bool displayUpdate)
{
    PtOrderMap ptOrds;
    calcConnectorIntersections(canvas, &ptOrds, NULL, NULL, NULL, NULL);

    // Do the actual nudging.
    for (int dim = 0; dim < 2; ++dim)
    {
        PtOrderMap::iterator finish = ptOrds.end();
        for (PtOrderMap::iterator it = ptOrds.begin(); it != finish; ++it)
        {
            //const VertID& ptID = it->first;
            PtOrder& order = it->second;
            PointRepVector pointsOrder = order.sortedPoints(dim);

            printf("Nudging[%d] ", dim);
            int count = 0;
            for (PointRepVector::iterator curr = pointsOrder.begin();
                    curr != pointsOrder.end(); ++curr)
            {
                if (curr->first == NULL)
                {
                    continue;
                }
                Point *connPt = curr->first;

                if (count == 0)
                {
                    printf("%g, %g : ", connPt->x, connPt->y);
                }
                const VertID id(connPt->id, connPt->vn,
                        VertID::PROP_ConnPoint);
                id.print(); printf(" ");
               
                if (count > 0)
                {
                    // If not the first point.
                    printf("%lX ", (long) connPt);
                    printf("%g, %g : ", connPt->x, connPt->y);
                    if (dim == 0)
                    {
                        connPt->x += nudgeDist * (count);
                    }
                    else
                    {
                        connPt->y += nudgeDist * (count);
                    }
                }
                count++;
            }
            printf("\n");
        }
    }

    if (displayUpdate)
    {
        QList<CanvasItem *> canvas_items = canvas->items();
        for (int i = 0; i < canvas_items.size(); ++i)
        {
            Connector *conn =
                    dynamic_cast<Connector *> (canvas_items.at(i));
            if (conn)
            {
                conn->applyNewRoute(conn->avoidRef->displayRoute());
                conn->update();
            }
        }
    }
}


#if 0
// ADS used in colorInteferingConnectors() for debug
static void conn_set_red(Conn *conn)
{
    conn->colour = QColor(255,0,0);
    conn->update();
}

static void conn_set_green(Conn *conn)
{
    conn->colour = QColor(0,255,0);
    conn->update();
}
#endif


#if ADS_DEBUG
// ADS used in calcConnectorIntersections for debug
static void identifyPointWithCircle(double xc, double yc, int r, int g, int b)
{
    int cox = canvas->get_offabsxpos(), coy = canvas->get_offabsypos();
    int cx = canvas->get_xpos(), cy = canvas->get_ypos();
    SDL_Rect crect = { cx + 2, cy + 2,
                       canvas->get_width() - 4, canvas->get_height() - 4 };
    SDL_SetClipRect(screen, &crect);
    Avoid::Point p(xc+cox,yc+coy); // does same as unoffsetPoint() (canvas.cpp)
    filledCircleRGBA(screen,(int)p.x,(int)p.y,
                     4,r,g,b,80);
    SDL_Flip(screen);
    SDL_Rect wholeCanvasRect;    
    wholeCanvasRect.x = canvas->get_xpos();
    wholeCanvasRect.y = canvas->get_ypos();
    wholeCanvasRect.w = canvas->get_width();
    wholeCanvasRect.h = canvas->get_height();

    canvasRect = wholeCanvasRect;
    SDL_SetClipRect(screen, NULL);
}
#endif /* ADS_DEBUG */


//
// reset colors of connectors to the saved value that is stored when 
// color is set by set_value for interfering connectors. This is so
// that connectors that were colored because they inteferered, and have
// been re-routed so that they no longer interfere have their colours 
// restored to what they were before changed by colorInterferingConnectors().
//
static void resetConnectorColors(Canvas *canvas)
{
    QList<CanvasItem *> canvas_items = canvas->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
        if (!conn)
        {
            continue;
        }
        conn->restoreColour();
    }
}

//
// Color connectors that are intersecting or shared path ('nudgable').
// We use graph coloring algorithm (Welsh-Powell) to color all
// intersecting/shared path connectors different colors.
//
void colourInterferingConnectors(Canvas *canvas)
{
    ConnPairSet touchingConns;
    ConnPairSet crossingConns;

    calcConnectorIntersections(canvas, NULL, &touchingConns, NULL, 
            &crossingConns, NULL);
    ConnPairSet::iterator pathFinish = touchingConns.end();

#if 0
    // ADS begin old debug line coloring code
    printf("colorInterferingConnectors start\n");
    for (ConnPairSet::iterator it = touchingConns.begin(); 
            it != pathFinish; ++it)
    {
        Conn *conn1 = it->first;
        Conn *conn2 = it->second;
        printf("colorInterferingConnectors shared path %d,%d\n",
               conn1->get_ID(),
               conn2->get_ID());
        conn_set_red(conn1); conn_set_red(conn2);
    }
    for (ConnPairSet::iterator iter = crossingConns.begin();
         iter != crossingConns.end(); ++iter)
    {
        Conn *conn1 = iter->first;
        Conn *conn2 = iter->second;
        printf("colorInterferingConnectors crossing connectors %d,%d\n",
               conn1->get_ID(),
               conn2->get_ID());
        conn_set_green(conn1); conn_set_green(conn2);
    }
    printf("colorInterfering Connectors end\n");
    // ADS end old debug line coloring code
#endif

    // build inteference graph for crossings and shared paths
    ConnPairSet interferingConns = crossingConns; // start with crossings
    // then add shared paths
    for (ConnPairSet::iterator it = touchingConns.begin(); 
            it != pathFinish; ++it)
    {
        interferingConns.insert(*it);
    }
    interference_graph::InterferenceGraph *intgraph = NULL;
    intgraph = new interference_graph::InterferenceGraph(interferingConns);
    // color the graph so adjacent nodes have different colors
    int num_colors = intgraph->color_graph();

    std::cout << "colourInterferingConnectors: required " << num_colors << 
        " colours." << std::endl;

    // reset colors to values they had before any previous call here
    resetConnectorColors(canvas);

    // and now set connector colors according to the graph coloring
    // by using the color number as index into list of suitable line colors.
    const QList<QColor> connectorColours =
            canvas->interferingConnectorColours();
    if (num_colors > connectorColours.size())
    {
        std::cerr << "Too many colors required\n";
        // ADS TODO: need to decide what to do about this; just give up,
        // or generate colors rather than use fixed list?
    }
    else
    {
        for (interference_graph::NodePtrList::iterator iter = 
                 intgraph->nodes->begin();
             iter != intgraph->nodes->end(); ++iter)
        {
            Connector *conn = (*iter)->conn;
            conn->overrideColour(connectorColours[(*iter)->colornum-1]);
        }
    }
    delete intgraph;
}

static int calcConnectorIntersections(Canvas *canvas, PtOrderMap *ptOrders,
        ConnPairSet *touchingConns, TallyMap *tallyMap,
        ConnPairSet *crossingConns, Connector *queryConn,
        PointSet *crossingPoints)
{
    Q_UNUSED (ptOrders)

    int crossingsN = 0;

    // Do segment splitting.
    QList<CanvasItem *> canvas_items = canvas->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        Point lastInt(INFINITY, INFINITY);
        Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
        if (!conn)
        {
            continue;
        }
        
        for (int j = (i + 1); j < canvas_items.size(); ++j)
        {
            Point lastInt2(INFINITY, INFINITY);
            Connector *conn2 = dynamic_cast<Connector *> (canvas_items.at(j));
            if (!conn2)
            {
                continue;
            }
            
            if (queryConn && (queryConn != conn) && (queryConn != conn2))
            {
                // Querying, and neither of these are the query connector.
                continue;
            }

            if (conn->internalId() == conn2->internalId())
            {
                continue;
            }
            
            Avoid::Polygon& route = conn->avoidRef->displayRoute();
            Avoid::Polygon& route2 = conn2->avoidRef->displayRoute();
            splitBranchingSegments(route2, true, route);
        }
    }

    for (int i = 0; i < canvas_items.size(); ++i)
    {
        Point lastInt(INFINITY, INFINITY);
        Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
        if (!conn)
        {
            continue;
        }
        
        for (int j = (i + 1); j < canvas_items.size(); ++j)
        {
            Point lastInt2(INFINITY, INFINITY);
            Connector *conn2 = dynamic_cast<Connector *> (canvas_items.at(j));
            if (!conn2)
            {
                continue;
            }
            
            if (queryConn && (queryConn != conn) && (queryConn != conn2))
            {
                // Querying, and neither of these are the query connector.
                continue;
            }

            if (conn->internalId() == conn2->internalId())
            {
                continue;
            }
            
            Avoid::Polygon& route = conn->avoidRef->displayRoute();
            Avoid::Polygon& route2 = conn2->avoidRef->displayRoute();
            //bool checkForBranchingSegments = false;
            int crossings = 0;
            bool touches = false;
            ConnectorCrossings cross(route2, true, route);
            cross.crossingPoints = crossingPoints;
            for (size_t i = 1; i < route.size(); ++i)
            {
                const bool finalSegment = ((i + 1) == route.size());
                cross.countForSegment(i, finalSegment);

                crossings += cross.crossingCount;
                touches |= (cross.crossingFlags & CROSSING_TOUCHES);
            }
            if (touchingConns && touches)
            {
                // Add to the list of touching connectors.
                touchingConns->insert(std::make_pair(conn, conn2));
            }
            assert(crossings <= 2);
            if (crossings > 0)
            {

                if (tallyMap)
                {
                    (*tallyMap)[conn->internalId()]++;
                    (*tallyMap)[conn2->internalId()]++;
                }
                if (crossingConns)
                {
                    crossingConns->insert(std::make_pair(conn, conn2));
                }
                crossingsN += crossings;
            }
        }
    }
#if ADS_DEBUG
    if (crossingPoints)
    {
        for (PointSet::const_iterator i = crossingPoints->begin();
                i != crossingPoints->end(); ++i)
        {
            identifyPointWithCircle(i->x, i->y, 255, 0, 0);
        }
    }
#endif

    return crossingsN;
}


void reroute_all_connectors(Canvas *canvas)
{
    Avoid::Router *router = canvas->router();
    bool lastSimpleRouting = router->SimpleRouting;
    router->SimpleRouting = false;
    router->ClusteredRouting = canvas->avoidClusterCrossings();
    bool force = true;
    reroute_connectors(canvas, force);
    router->SimpleRouting = lastSimpleRouting;
    canvas->interrupt_graph_layout();
}


void redraw_connectors(Canvas *canvas)
{
    QList<CanvasItem *> canvas_items = canvas->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
        if (conn)
        {
            conn->reapplyRoute();
        }
    }
}


void reroute_connectors(Canvas *canvas, const bool force,
        const bool postProcessing)
{
    Avoid::Router *router = canvas->router();
    //qDebug("%d reroute_connectors(%d, %d)\n", (int) time(NULL),
    //       (int) force, (int) postProcessing);
    if (router->SimpleRouting)
    {
        router->processTransaction();
        QList<CanvasItem *> canvas_items = canvas->items();
        for (int i = 0; i < canvas_items.size(); ++i)
        {
            Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
            if (conn)
            {
                conn->forceReroute();
            }
        }
        return;
    }

    if (force)
    {
        //printf("+++++ Making all libavoid paths invalid\n");
        QList<CanvasItem *> canvas_items = canvas->items();
        for (int i = 0; i < canvas_items.size(); ++i)
        {
            Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));

            if (conn)
            {
                conn->avoidRef->makePathInvalid();
                conn->forceReroute();
            }
        }
    }
    bool changes = router->processTransaction();
    //router->outputInstanceToSVG("libavoid-debug-new");
    // Update connectors.
    if (changes)
    {
        int rconns = 0;
        QList<CanvasItem *> canvas_items = canvas->items();
        for (int i = 0; i < canvas_items.size(); ++i)
        {
            Connector *conn =
                    dynamic_cast<Connector *> (canvas_items.at(i));
            if (conn &&  (!(router->SelectiveReroute) ||
                     conn->avoidRef->needsRepaint() || force))
            {
                conn->updateFromLibavoid();
                rconns++;
            }
        }
        //printf("-- %3d conns rerouted! --\n", rconns);
        //fflush(stdout);
    }
    // -----------------
   
    if (postProcessing && canvas->avoidConnectorCrossings())
    {
        //SDL_Surface *sur = canvas->get_image(canvas->get_active_image_n());
        
        std::map<unsigned int, int> tallies;
        printf("Avoiding connector crossings...\n");

        calcConnectorIntersections(canvas, NULL, NULL, &tallies, NULL, NULL);
        
        TallyMap::iterator curr, finish = tallies.end();
        while (!tallies.empty())
        {
            unsigned int maxID = 0;
            int maxVal = 0;
            TallyMap::iterator maxIt;

            for (curr = tallies.begin(); curr != finish; ++curr)
            {
                if ((*curr).second > maxVal)
                {
                    maxVal = (*curr).second;
                    maxID  = (*curr).first;
                    maxIt  = curr;
                }
            }
            tallies.erase(maxIt);
            QList<CanvasItem *> canvas_items = canvas->items();
            for (int i = 0; i < canvas_items.size(); ++i)
            {
                Connector *conn = dynamic_cast<Connector *> (canvas_items.at(i));
                if (!conn || (maxID != conn->internalId()))
                {
                    continue;
                }
                conn->rerouteAvoidingIntersections();
            }
        }
    }

    if (postProcessing && canvas->optColourInterferingConnectors())
    {
        colourInterferingConnectors(canvas);
    }
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
