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

#include <QGraphicsSceneWheelEvent>
#include <QByteArray>
#include <cassert>
#include <utility>

#include "libavoid/libavoid.h"
using Avoid::Polygon;
using Avoid::Point;

#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/visibility.h"
#include "libdunnartcanvas/cluster.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/utility.h"
#include "libdunnartcanvas/handle.h"
#include "libdunnartcanvas/connector.h"

#include "libcola/convex_hull.h"

namespace dunnart {

const QColor clusterFillCol = QColor(96, 205, 243, 85);

Cluster *newCluster(CanvasItemList& memberList, QString id)
{
    CanvasItemList members;

    members.clear();
    for (CanvasItemList::iterator curr = memberList.begin();
            curr != memberList.end(); ++curr)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (*curr);

        if (shape && shape->canBe(C_CLUSTERED))
        {
            members.push_back(shape);
        }
    }
    if (members.empty())
    {
        fprintf(stderr, "WARNING: newCluster called with no members.\n");
        return NULL;
    }
    Cluster *newC = new Cluster(members, id);

    return newC;
}


//===========================================================================
//  Cluster object code:


Cluster::Cluster(Canvas *canvas, const QDomElement& node, const QString& ns)
    : ShapeObj(),
      avoidClusterRef(NULL),
      rectangular(false),
      detailLevel(100)
{
    setZValue(ZORD_Cluster);
    
    optionalProp(node, x_rectangular, rectangular, ns);

    members.clear();

    QString value = nodeAttribute(node, ns, x_contains);
    if (!value.isNull())
    {       
        // Read a space separated list of shape IDs.  Then,
        // lookup each shape and add it to the members list.
        QStringList list = value.split(" ", QString::SkipEmptyParts);
        qDebug() << list;
        for (int i = 0; i < list.size(); ++i)
        {
            QString id = list.at(i);

            CanvasItem *cObj = canvas->getItemByID(id);
            if (cObj)
            {
                ShapeObj *shape = dynamic_cast<ShapeObj *> (cObj);
                if (shape)
                {
                    members.push_back(shape);
                }
            }
            else
            {
                fprintf(stderr, "WARNING: Cluster member (ID %s) not "
                        "found, ignoring.\n", qPrintable(id));
            }
        }
    }
    value = nodeAttribute(node, ns, x_fillCol);

    calculateBoundary();

    m_fill_colour = clusterFillCol;
    if (!value.isNull())
    {
        m_fill_colour = QColorFromRRGGBBAA(value.toLatin1().data());
    }
    m_stroke_colour = m_fill_colour;

    initWithXMLProperties(canvas, node, ns);
}


Cluster::Cluster(CanvasItemList& memberList, QString id)
    : ShapeObj(),
      avoidClusterRef(NULL),
      rectangular(false),
      detailLevel(100)
{
    m_string_id = id;
    m_stroke_colour = m_fill_colour = clusterFillCol;
    setZValue(ZORD_Cluster);

    members.clear();
    for (CanvasItemList::iterator curr = memberList.begin();
            curr != memberList.end(); ++curr)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (*curr);

        if (shape && shape->canBe(C_CLUSTERED))
        {
            members.push_back(shape);
        }
    }
    calculateBoundary();

    setPainterPath(buildPainterPath());
}


ShapeList& Cluster::getMembers(void)
{
    return members;
}


void Cluster::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (canvas() == NULL)
    {
        return;
    }

    if (event->delta() > 0)
    {
        this->changeDetailLevel(true);
    }
    else if (event->delta() < 0)
    {
        this->changeDetailLevel(false);
    }
    ShapeObj::wheelEvent(event);
}


static const double levelsOfDetail = 16;
static const double maxCollapsedClusterSize = 80;


void Cluster::changeDetailLevel(bool expand)
{
    QDomDocument doc("XML");
    double lastDetailLevel = detailLevel;
    bool shrink = !expand;
    
    if (expand)
    {
        detailLevel = 100;
    }
    else
    {
        detailLevel = 0;
    }

    if (lastDetailLevel == detailLevel)
    {
        // No change
        return;
    }

    QRectF clusterRect = shapeRect();
    //qDebug() << "-- clc " << clusterRect.center();

    if (shrink)
    {
        if ((clusterRect.center().x() < maxCollapsedClusterSize) &&
            (clusterRect.center().y() < maxCollapsedClusterSize))
        {
            // The cluster is already smaller than maxCollapsedClusterSize, 
            // so don't shrink it.
            detailLevel = 100;
            return;
        }
        m_expanded_size = clusterRect.size();

        ConnMultiset allConnMs;
        for (ShapeList::iterator curr = members.begin(); 
                curr != members.end(); ++curr)
        {
            QRectF shapeRect = (*curr)->shapeRect();
            //qDebug() << "-- rect " << shapeRect;
            QPointF offset = clusterRect.center() - shapeRect.center();
            //qDebug() << "-- offset " << offset;
            shapeRect.moveCenter(offset);

            largeShapeInfo[(*curr)->internalId()] = shapeRect;

            ConnMultiset connMs = (*curr)->getConnMultiset();
            allConnMs.insert(connMs.begin(), connMs.end());
        }
        ConnMultiset::iterator prev = allConnMs.begin();
        ConnMultiset::iterator curr = allConnMs.begin();
        for (; curr != allConnMs.end(); ++curr)
        {
            if (curr != prev)
            {
                if (*curr == *prev)
                {
                    // The two connector endpoints are both connected to
                    // shapes within the cluster, so add to internalConns 
                    // list.
                    internalConns.insert(*curr);
                }
            }
            prev = curr;
        }
    }

    double minProportion;
    if (m_expanded_size.width() > m_expanded_size.height())
    {
        minProportion = maxCollapsedClusterSize / m_expanded_size.width();
    }
    else
    {
        minProportion = maxCollapsedClusterSize / m_expanded_size.height();
    }
    // Shrink to at least half size.
    minProportion = std::min(minProportion, 0.5);
        
    double startProportion;
    double endProportion;
    double stepProportion;
    if (shrink)
    {
        startProportion = 1;
        endProportion = minProportion;
    }

    GraphLayout *gl = canvas()->layout();
    Avoid::Router *router = canvas()->router();
    if (expand)
    {
        startProportion = minProportion;
        endProportion = 1;
        
        gl->setInterruptFromDunnart();
        bool lastSimpleRouting = router->SimpleRouting;
        router->SimpleRouting = false;

        for (ShapeList::iterator curr = members.begin(); 
                curr != members.end(); ++curr)
        {
            (*curr)->setAsCollapsed(false);
            (*curr)->setVisible(true);
            (*curr)->routerAdd();
        }
        for (ConnSet::iterator curr = internalConns.begin(); 
                curr != internalConns.end(); ++curr)
        {
            (*curr)->setVisible(true);
        }
        for (XmlNodeList::iterator curr = expandedConnRoutes.begin();
                curr != expandedConnRoutes.end(); ++curr)
        {
            Connector *conn = curr->first;

            QPair<CPoint, CPoint> connpts = conn->get_connpts();
            ShapeObj *srcShape = connpts.first.shape;
            ShapeObj *dstShape = connpts.second.shape;

            if ((srcShape == this) || (dstShape == this))
            {
                // If the connector is still attached to the cluster
                // shape, then reroute it back to it's uncollapsed route.
                // QT conn->updateFromXmlRep(DELTA_REROUTE, curr->second);
            }
        }
        expandedConnRoutes.clear();

        internalConns.clear();
        /* QT
        Handle *cHand = selectHandle(HAN_CENTER | HAN_CONNPT);
        assert(cHand);
        cHand->setVisible(false);
        delete cHand;
        */

        router->SimpleRouting = lastSimpleRouting;
        // Make it appear as if nothing moved.
        canvas()->getActions().clear();
        routerRemove();
        m_is_collapsed = false;
        routerAdd();
        setZValue(ZORD_Cluster);
        // Restart graph layout so it no longer sees collapsed cluster
        // constraints.
        canvas()->restart_graph_layout();
    }
    stepProportion = (endProportion - startProportion) / levelsOfDetail;

    double proportion;
    for (int i = 1; i <= levelsOfDetail; ++i)
    {
        if (i == levelsOfDetail)
        {
            proportion = endProportion;
        }
        else
        {
            proportion = startProportion + (stepProportion * i);
        }
    
        for (ShapeList::iterator curr = members.begin(); 
            curr != members.end(); ++curr)
        {
            QRectF shapeRect = largeShapeInfo[(*curr)->internalId()];
            //qDebug() << "-- offset" << shapeRect.center();

            QSizeF fn = shapeRect.size() * (proportion - 0.02);
            QSizeF n = shapeRect.size() * proportion;
            if (proportion == 1)
            {
                fn = n;
            }
            //printf("-- nw %g nh %g\n", nw, nh);
            QPointF npos = clusterRect.center() - (shapeRect.center() * proportion);
            //printf("-- nx %g ny %g\n", nx, ny);
            (*curr)->setPosAndSize(npos, fn);
        }
        if (i != levelsOfDetail)
        {
            //QT do_response_tasks();
            gl->processReturnPositions();
        }
    }

    if (shrink)
    {
        gl->setInterruptFromDunnart();
        // Recompute boundary, since this is may not be done fast enough 
        // by the layout thread.
        recomputeBoundary();
        // Add center handle.
        routerRemove();
        m_is_collapsed = true;
        routerRemove();
        setZValue(ZORD_Shape);

        Handle *cHand = new Handle(this, HAN_CENTER | HAN_CONNPT,
                HPLACE_ZERO, 0, HPLACE_ZERO, 0);

        bool lastSimpleRouting = router->SimpleRouting;
        router->SimpleRouting = false;
        // Hide internal connectors.
        // Save the small shape offsets.
        for (ShapeList::iterator curr = members.begin(); 
                curr != members.end(); ++curr)
        {
            QRectF shapeRect = (*curr)->shapeRect();
            //qDebug() << "-- rect " << shapeRect;
            QPointF offset = clusterRect.center() - shapeRect.center();
            //qDebug() << "-- offset " << offset;
            shapeRect.moveCenter(offset);

            smallShapeInfo[(*curr)->internalId()] = shapeRect;

            ConnMultiset connSet = (*curr)->getConnMultiset();
            
            for (ConnMultiset::iterator ccurr = connSet.begin();
                    ccurr != connSet.end(); ++ccurr)
            {
                Connector *conn = (*ccurr);
                if (internalConns.find(conn) != internalConns.end())
                {
                    // It's an internal conn, so ignore.
                    continue;
                }

                QPair<CPoint, CPoint> connpts = conn->get_connpts();
                ShapeObj *srcShape = connpts.first.shape;
                ShapeObj *dstShape = connpts.second.shape;

                QDomElement undoNode = conn->to_QDomElement(XMLSS_INONE, doc);
                QDomElement node = doc.createElement("path");
                if (srcShape)
                {
                    if (srcShape == (*curr))
                    {
                        newProp(node, x_srcID, getIdString());
                        newProp(node, x_srcFlags, cHand->handleFlags());

                        
                        newProp(undoNode, x_srcID, srcShape->getIdString());
                        newProp(undoNode, x_srcFlags, connpts.first.pinClassID);
                    }
                }
                
                if (dstShape)
                {
                    if (dstShape == (*curr))
                    {
                        newProp(node, x_dstID, getIdString());
                        newProp(node, x_dstFlags, cHand->handleFlags());
                        
                        newProp(undoNode, x_dstID, dstShape->getIdString());
                        newProp(undoNode, x_dstFlags, connpts.second.pinClassID);
                    }
                }
                expandedConnRoutes.push_back(std::make_pair(conn, undoNode));

                // QT conn->updateFromXmlRep(DELTA_REROUTE, node);
            }
        }
        for (ConnSet::iterator curr = internalConns.begin(); 
                curr != internalConns.end(); ++curr)
        {
            if ((*curr)->avoidRef)
            {
                (*curr)->avoidRef->makeInactive();
            }
            (*curr)->setSelected(false);
            (*curr)->setVisible(false);
        }
        for (ShapeList::iterator curr = members.begin(); 
                curr != members.end(); ++curr)
        {
            (*curr)->routerRemove();
            (*curr)->setAsCollapsed(true);
            (*curr)->setVisible(false);
        }
        router->SimpleRouting = lastSimpleRouting;
        // Make it appear as if nothing moved.
        canvas()->getActions().clear();
        // Restart graph layout so it sees collapsed cluster constraints,
        canvas()->restart_graph_layout();
    }

    if (expand)
    {
        // Free the largeShapeInfo information.
        largeShapeInfo.clear();
        // Free the smallShapeInfo information.
        smallShapeInfo.clear();
        internalConns.clear();
        
        // Tell the layout to restart again, since then it will notice the
        // final sizes of shapes and therefore use correct cluster boundary
        // sizes.
        canvas()->interrupt_graph_layout();
    }
}

void Cluster::userMoveBy(qreal dx, qreal dy)
{
    // Move the contained shapes.
    Actions& actions = canvas()->getActions();
    ShapeList& members = getMembers();
    for (ShapeList::iterator curr = members.begin();
            curr != members.end(); ++curr)
    {
        if (!(*curr)->isSelected())
        {
            // Move the shape, if it is unselected.
            (*curr)->QGraphicsItem::moveBy(dx, dy);
            actions.moveList.push_back(*curr);
        }
    }
    moveBy(dx, dy);
}


void Cluster::routerAdd(void)
{
    if (!isCollapsed())
    {
        Avoid::Router *router = canvas()->router();

        assert(avoidClusterRef == NULL);
        Polygon poly(0);
        avoidClusterRef =
                new Avoid::ClusterRef(router, poly, internalId());
     }
     else
     {
         // If collapsed, treat as a normal shape.
         ShapeObj::routerAdd();
     }
 }


void Cluster::routerRemove(void)
{
    Avoid::Router *router = canvas()->router();
    if (!isCollapsed())
    {
        if (avoidClusterRef == NULL)
        {
            return;
        }

        // Delete shape
        router->deleteCluster(avoidClusterRef);
        avoidClusterRef = NULL;
    }
    else
    {
        // If collapsed, treat as a normal shape.
        ShapeObj::routerRemove();
    }
}


void Cluster::routerMove(void)
{
    if (!isCollapsed())
    {
        if (avoidClusterRef == NULL)
        {
            return;
        }

        Polygon *polygon = poly(0);
        avoidClusterRef->setNewPoly(*polygon);
        delete polygon;
    }
    else
    {
        // If collapsed, treat as a normal shape.
        ShapeObj::routerMove();
    }
}


void Cluster::routerResize(void)
{
    // Treat as a move.
    routerMove();
}



#if 0
static void changeClusterType(CanvasItem **c)
{
    Cluster *cluster = (Cluster *) (*c)->get_parent()->get_ident();

    cluster->rectangular = !(cluster->rectangular);

    restart_graph_layout(NULL);
}


static const int BUT_CMODE = 200;

void Cluster::addContextMenuItems(MenuItems& items)
{
    items.push_back(
            MenuItem(BUT_TYP_Button, BUT_CMODE, (rectangular) ? 
                    "Style: Rectangular": "Style: Convex",
                    "", NULL, null_callback));
    items.push_back(MenuSeparator());
    items.push_back(
            MenuItem(BUT_TYP_Button, BUT_GENERIC, (rectangular) ?
                    "Make Convex" : "Make Rectangular",
                    "", NULL, changeClusterType));
    items.push_back(MenuSeparator());
    
    CanvasItem::addContextMenuItems(items);
}


void Cluster::changeContextMenuState(Menu *menu)
{
    CanvasItem::changeContextMenuState(menu);
}
#endif


bool Cluster::canBe(const unsigned int flags)
{
    return flags & C_NONE;
}


void Cluster::calculateBoundary(void)
{
    if (rectangular)
    {
        double minX = DBL_MAX;
        double minY = DBL_MAX;
        double maxX = -DBL_MAX;
        double maxY = -DBL_MAX;
        for (ShapeList::iterator curr = members.begin();
                curr != members.end(); ++curr)
        {
            double shMinX, shMinY, shMaxX, shMaxY;
            //Avoid::Polygon *poly = (*curr)->shapePoly;
            Avoid::Polygon *poly = (*curr)->poly(routingBuffer);
            poly->getBoundingRect(&shMinX, &shMinY, &shMaxX, &shMaxY);

            minX = std::min(minX, shMinX);
            minY = std::min(minY, shMinY);
            maxX = std::max(maxX, shMaxX);
            maxY = std::max(maxY, shMaxY);
        }

        boundary.clear();
        boundary.resize(4);
        boundary[0].x = minX;
        boundary[0].y = minY;
        boundary[1].x = minX;
        boundary[1].y = maxY;
        boundary[2].x = maxX;
        boundary[2].y = maxY;
        boundary[3].x = maxX;
        boundary[3].y = minY;
        psn = 4;

        computeBoundingBox();
        return;
    }

    std::vector<unsigned> hullIndexes;

    int totalPoints = 0;
    for (ShapeList::iterator curr = members.begin();
            curr != members.end(); ++curr)
    {
        if (!(*curr)->canBe(C_CLUSTERED))
        {
            qFatal(
                   "Cluster::calculateBoundary(): object %d can't be clustered",
                   (*curr)->internalId());
        }
        Avoid::Polygon *poly = (*curr)->poly(avoidBuffer);

        totalPoints += poly->size();

        delete poly;
    }

    std::valarray<double> xValues(totalPoints);
    std::valarray<double> yValues(totalPoints);
    std::valarray<int> ids(totalPoints);
    std::valarray<int> vns(totalPoints);
    int index = 0;
    for (ShapeList::iterator curr = members.begin();
            curr != members.end(); ++curr)
    {
        Avoid::Polygon *poly = (*curr)->poly(avoidBuffer);

        for (size_t i = 0; i < poly->size(); ++i)
        {
            xValues[index + i] = poly->ps[i].x;
            yValues[index + i] = poly->ps[i].y;
            ids[index + i] = poly->id();
            vns[index + i] = i;
        }
        index += poly->size();

        delete poly;
    }

    hull::convex(xValues, yValues, hullIndexes);

    psn = hullIndexes.size();
    assert(psn != 0);
    
    boundary.resize(psn);
    index = 0;
    for (std::vector<unsigned>::iterator curr = hullIndexes.begin();
            curr != hullIndexes.end(); ++curr)
    {
        boundary[index].x = xValues[*curr];
        boundary[index].y = yValues[*curr];
        boundary[index].id = ids[*curr];
        boundary[index].vn = vns[*curr];
        index++;
    }

    computeBoundingBox();
}


void Cluster::setNewBoundary(std::vector<Avoid::Point>& points)
{
    if (m_is_collapsed)
    {
        return;
    }
    // QT restore_behind();

    psn = points.size() - 1; // libcola returns one point twice.
    assert(psn > 1);
    
    boundary.resize(psn);
    for (int i = 0 ; i < psn; ++i)
    {
        boundary[i] = points[i];
    }

    computeBoundingBox();

    routerResize();

    update();
}

// Called when a collapsed cluster is moved by the layout thread.
void Cluster::setCentrePos(const QPointF& newPos)
{
    // This is only used for collapsed clusters.
    assert(m_is_collapsed);

    // Adjust the boundary.
    QPointF diff = pos() - newPos;
    for (int i = 0 ; i < psn; ++i)
    {
        boundary[i].x -= diff.x();
        boundary[i].y -= diff.y();
    }
    // Move the shape.
    CanvasItem::setPos(newPos);

    routerResize();
}


void Cluster::computeBoundingBox(void)
{
    QPolygonF boundaryPoly;
    // Find the bounding box.
    for (int i = 0; i < psn; ++i)
    {
        boundaryPoly << QPointF(boundary[i].x, boundary[i].y);
    }

    QRectF boundingRect = boundaryPoly.boundingRect();
    setSize(boundingRect.size());
    ShapeObj::setCentrePos(boundingRect.center());
}


void Cluster::recomputeBoundary(void)
{
    calculateBoundary();

    // Visibility graph stuff:
    routerResize();

    update();
}


QPainterPath Cluster::buildPainterPath(void)
{
    QPainterPath painter_path;

    QPolygonF polygon;
    for (int i = 0; i < psn; ++i)
    {
        polygon << (QPointF(boundary[i].x, boundary[i].y) - pos());
    }
    
    painter_path.addPolygon(polygon);
    painter_path.closeSubpath();
    
    return painter_path;
}


#if 0
void Cluster::draw(QPixmap *surface, const int x, const int y,
        const int type, const int w, const int h)
{
    // QT
    if (!surface)
    {
        return;
    }
    
    // Get text size:
    
    //SDL_Color black = { 0x00, 0x00, 0x00, 0 };

    QColor strokeCol = QColor(255, 0, 0, 75);
    //boxColor(surface, x, y, x + w - 1, y + h - 1, strokeCol);
   
    strokeCol = QColor(0, 0, 0, 0);
    switch (type)
    {
        case SHAPE_DRAW_NORMAL:
            // Nothing to do.
            break;
        case SHAPE_DRAW_HIGHLIGHTED:
        case SHAPE_DRAW_LEAD_HIGHLIGHTED:
            strokeCol = QColor(0, 255, 255);
            break;
        case SHAPE_DRAW_OUTLINE:
            strokeCol = QColor(0, 0, 0, 128);
            break;
        default:
            break;
    }
    QColor fillCol = colour;
    if (isCollapsed())
    {
        // Turn off opacity.
        //QT fillCol |= 255;
    }

    Sint16 *xpsOff = (Sint16 *) calloc(psn, sizeof(Sint16));
    Sint16 *ypsOff = (Sint16 *) calloc(psn, sizeof(Sint16));
   
    for (int i = 0; i < psn; ++i)
    {
        xpsOff[i] = static_cast<Sint16> (boundary[i].x - fPosX + x);
        ypsOff[i] = static_cast<Sint16> (boundary[i].y - fPosY + y);
    }

    filledPolygonColor(surface, xpsOff, ypsOff, psn, fillCol);
    aapolygonColor(surface, xpsOff, ypsOff, psn, strokeCol);

    std::free(xpsOff);
    std::free(ypsOff);

    double clx, cly, clw, clh;
    getPosAndSize(clx, cly, clw, clh, false);
    
    double clcx = clx + (clw / 2);
    double clcy = cly + (clh / 2);
    // Draw small members
    if (isCollapsed())
    {
        for (ShapeList::iterator curr = members.begin(); 
                curr != members.end(); ++curr)
        {
            double ox, oy, w, h;
            smallShapeInfo[(*curr)->get_ID()]->getOffsetAndSize(ox, oy, w, h);
            ox += (w/2);
            oy += (h/2);
            w += (2 * HANDLE_PADDING);
            h += (2 * HANDLE_PADDING);
            (*curr)->draw(surface, 
                    (int) (clcx - ox - HANDLE_PADDING + 1 + cxoff), 
                    (int) (clcy - oy  - HANDLE_PADDING + 1 + cyoff), 
                    SHAPE_DRAW_NORMAL, (int) w, (int) h);
        }
    }
}
#endif


void Cluster::set_label(const char *l)
{
    Q_UNUSED (l)
    // Do nothing
    return;
}


void Cluster::write_svg_path(QDomElement& node)
{
    bool perform_rounding = true;
    double rounding_dist = 5.0;
    Avoid::Polygon route = 
            Polygon(avoidClusterRef->polygon());

    if (perform_rounding)
    {
        bool closed = true;
        route = route.curvedPolyline(rounding_dist, closed);
    }
 
    QString pathStr;
    QString value;
    for (size_t j = 0; j < route.size(); ++j)
    {
        const Point& p1 = route.at(j);
        if (j == 0)
        {
            pathStr += value.sprintf("M %g,%g ", p1.x, p1.y);
        }
        else if (perform_rounding && (route.ts[j] == 'C'))
        {
            const Point& p2 = route.at(j + 1);
            const Point& p3 = route.at(j + 2);
            pathStr += value.sprintf("C %g,%g %g,%g %g,%g ", p1.x, p1.y,
                    p2.x, p2.y, p3.x, p3.y);
            j += 2;
        }
        else if (perform_rounding && (route.ts[j] == 'Z'))
        {
            pathStr += value.append("Z");
        }
        else
        {
            pathStr += value.sprintf("L %g,%g ", p1.x, p1.y);
        }
    }
    if (!pathStr.isEmpty())
    {
        newProp(node, "d", pathStr);
    }
}


QDomElement Cluster::to_QDomElement(const unsigned int subset, 
        QDomDocument& doc)
{
    QString value, str;

    QDomElement node = doc.createElement("path");

    if (subset & XMLSS_IOTHER)
    {
        node.setAttribute("dunnart:type", x_cluster);

        newProp(node, "id", getIdString());

        int count = 0;
        for (ShapeList::iterator curr = members.begin(); curr != members.end();
                ++curr)
        {
            value += (*curr)->getIdString();
            if (count > 0)
            {
                value += " ";
            }
            count++;
        }
        newNsProp(node, x_dunnartNs, x_contains, value);

        if (m_fill_colour != clusterFillCol)
        {
            value = value.sprintf("%02x%02x%02x%02x;", m_fill_colour.red(),
                    m_fill_colour.green(), m_fill_colour.blue(),
                    m_fill_colour.alpha());
            newNsProp(node, x_dunnartNs, x_fillCol, value);
        }
        
        newNsProp(node, x_dunnartNs, x_rectangular, rectangular);
    }

    if (subset & XMLSS_ISVG)
    {
        newProp(node, "class", "cluster");

        write_svg_path(node);

        if (m_fill_colour != clusterFillCol)
        {
            value = value.sprintf("fill:#%02X%02X%02X;fill-opacity:%g;",
                    m_fill_colour.red(), m_fill_colour.green(), m_fill_colour.blue(),
                    m_fill_colour.alphaF());
            newProp(node, "style", value);
        }
    }
    return node;
}


Polygon *Cluster::poly(const double b, Polygon *p)
{
    Q_UNUSED (b)

    if (p)
    {
        delete p;
    }
    p = new Avoid::Polygon(psn);
    
    if (!p)
    {
        qFatal("Couldn't calloc memory in Cluster::poly()");
    }
    
    p->_id = (int) m_internal_id;

    for (int i = 0; i < psn; ++i)
    {
        p->ps[i] = boundary[i];
    }

    return p;
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

