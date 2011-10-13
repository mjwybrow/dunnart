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

#include <QObject>
#include <QByteArray>
#include <QPainter>
#include <QPointF>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QPainterPathStroker>
#include <QApplication>

#include <cstdlib>
#include <cfloat>
#include <cmath>

#include "libavoid/libavoid.h"
using Avoid::VertID;
using Avoid::Point;

#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/handle.h"
#include "libdunnartcanvas/connectorhandles.h"

#include "libdunnartcanvas/nearestpoint.h"

#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/utility.h"
#include "libdunnartcanvas/visibility.h"

namespace dunnart {


bool createDirectedConns = true;

// For crossing counting to work as expected, 
// we require that EDGEPORTSPACE == routingBuffer.
#define EDGEPORTSPACE (routingBuffer)

const QColor defaultConnLineCol = QColor(0, 0, 0);


Connector::Connector()
    : CanvasItem(NULL, QString(), ZORD_Conn),
      avoidRef(NULL),
      multiEdge(false),
      multiEdgeSize(1),
      multiEdgeInd(0),
      orthogonalConstraint(NONE),
      m_ideal_length(0),
      m_colour(defaultConnLineCol),
      m_saved_colour(defaultConnLineCol),
      m_directed(false),
      m_has_downward_constraint(false),
      m_obeys_directed_edge_constraints(true),
      m_arrow_head_type(normal),
      dotted(false),
      m_is_lone_selected(false)
{
    qRegisterMetaType<dunnart::Connector::RoutingType>("RoutingType");
    qRegisterMetaTypeStreamOperators<int>("RoutingType");
    qRegisterMetaType<dunnart::Connector::ArrowHeadType>("ArrowHeadType");
    qRegisterMetaTypeStreamOperators<int>("ArrowHeadType");

    setItemType("connector");

    m_routing_type = polyline;

    num_points = 0;
    offset_route.clear();
    offset_obs_route.clear();

    setFlag(QGraphicsItem::ItemIsMovable, false);

    setHoverMessage("Connector \"%1\" - Select to modify path, drag to move "
            "connector (disconnecting it from attached shapes).");
}


void Connector::initWithConnection(ShapeObj *sh1, ShapeObj *sh2)
{
    if (sh1)
    {
        srcpt.shape = sh1;
        srcpt.pinClassID = CENTRE_CONNECTION_PIN;
    }
    else
    {
        qFatal("Conn::Conn(ShapeObj *s1, ShapeObj *s2) needs valid shapes");
    }

    if (sh2)
    {
        dstpt.shape = sh2;
        dstpt.pinClassID = CENTRE_CONNECTION_PIN;
    }
    else
    {
        qFatal("Conn::Conn(ShapeObj *s1, ShapeObj *s2) needs valid shapes");
    }
}


Connector::~Connector()
{
}


void Connector::initWithXMLProperties(Canvas *canvas,
        const QDomElement& node, const QString& ns)
{
    // Call equivalent superclass method.
    CanvasItem::initWithXMLProperties(canvas, node, ns);

    if (canvas->forceOrthogonalConnectors())
    {
        setProperty("routingType", "orthogonal");
    }
    else
    {
        QString value = nodeAttribute(node, ns, "routingType");
        if (!value.isNull())
        {
            setProperty("routingType", value);
        }
    }

    // get arrow type
    QString value = nodeAttribute(node, ns, "arrowHeadType");
    if (!value.isNull())
    {
        setProperty("arrowHeadType", value);
        // Arrowhead implies connector is directed.
        m_directed = true;
    }

    optionalProp(node, x_directed, m_directed, ns);

    srcpt.shape = NULL;
    srcpt.pinClassID = 0;
    srcpt.x = srcpt.y = 0;
    QString sshape, dshape;
    if (optionalProp(node, x_srcID, sshape, ns) && !(sshape.isEmpty()))
    {
        ShapeObj *sh = dynamic_cast<ShapeObj *> 
                (canvas->getItemByID(sshape));
        if (sh)
        {
            int pinID = CENTRE_CONNECTION_PIN;
            int flags = 0;
            if (optionalProp(node, x_srcFlags, flags, ns))
            {
                qWarning("Deprecated %s attribute specified", x_srcFlags);
                pinID = sh->connectionPinForConnectionFlags(flags);
            }
            optionalProp(node, x_srcPinID, pinID, ns);
            assert(pinID != 0);

            srcpt.shape = sh;
            srcpt.pinClassID = pinID;
        }
        if (!sh)
        {
            qWarning("could not find shape for `src' connection");
        }
    }
    else
    {
        srcpt.pinClassID = 0;

        srcpt.x = essentialProp<double>(node, x_srcX, ns);
        srcpt.y = essentialProp<double>(node, x_srcY, ns);
    }

    dstpt.shape = NULL;
    dstpt.pinClassID = 0;
    dstpt.x = dstpt.y = 0;
    if (optionalProp(node,x_dstID,dshape,ns) && !(dshape.isEmpty()))
    {
        ShapeObj *sh = dynamic_cast<ShapeObj *> 
            (canvas->getItemByID(dshape));
        if (sh)
        {
            int pinID = CENTRE_CONNECTION_PIN;
            int flags = 0;
            if (optionalProp(node, x_dstFlags, flags, ns))
            {
                qWarning("Deprecated %s attribute specified", x_dstFlags);
                pinID = sh->connectionPinForConnectionFlags(flags);
            }
            optionalProp(node, x_dstPinID, pinID, ns);
            assert(pinID != 0);

            dstpt.shape = sh;
            dstpt.pinClassID = pinID;
        }
        if (!sh)
        {
            qWarning("could not find shape for `dst' connection");
        }
    }
    else
    {
        dstpt.pinClassID = 0;
        dstpt.x = essentialProp<double>(node, x_dstX, ns);
        dstpt.y = essentialProp<double>(node, x_dstY, ns);
    }
    
    optionalProp(node, x_idealLength, m_ideal_length, ns);
    optionalProp(node, x_obeysDirEdgeConstraints,
            m_obeys_directed_edge_constraints, ns);
    int oc=NONE;
    if (optionalProp(node, x_orthogonalConstraint, oc, ns)) 
    {
        orthogonalConstraint=(OrthogonalConstraint)oc;
        printf("orthogonal constraint=%d\n",oc);
    }
    value = nodeAttribute(node, ns, x_lineCol);
    if (!value.isNull())
    {
        m_colour = QColorFromRRGGBBAA(value.toLatin1().data());
        m_saved_colour = m_colour;
        //printf("read id=%d colour=%x\n",get_ID(),colour);
    }
    
    // get dotted
    value = nodeAttribute(node, ns, "LineStyle");
    if (!value.isNull())
    {
        setDotted(value == "dashed");
    }

    // From old PolyConn.
    value = nodeAttribute(node, ns, x_libavoidPath);
    if (canvas->optAutomaticGraphLayout() &&
            canvas->optPreserveTopology() && !value.isNull())
    {
        QStringList strings =
                value.split(QRegExp("[, ]"), QString::SkipEmptyParts);
        int stringIndex = 0;

        // Read the number of points.
        int totalPoints = strings.at(stringIndex++).toInt();

        Avoid::PolyLine initialPath(totalPoints);

        // Read a space separated list of coordinates.
        for (int ptNum = 0; ptNum < totalPoints; ++ptNum)
        {
            initialPath.ps[ptNum].x  = strings.at(stringIndex++).toDouble();
            initialPath.ps[ptNum].y  = strings.at(stringIndex++).toDouble();
            initialPath.ps[ptNum].id = strings.at(stringIndex++).toInt();
            initialPath.ps[ptNum].vn = strings.at(stringIndex++).toInt();
        }
    }
}


void Connector::routerAdd(void)
{
    // Create libavoid ConnRef for the connector.
    avoidRef = new Avoid::ConnRef(canvas()->router(), internalId());

    // Update endpoints.
    setNewLibavoidEndpoint(VertID::src);
    setNewLibavoidEndpoint(VertID::tar);

    if (m_routing_type == orthogonal)
    {
        avoidRef->setRoutingType(Avoid::ConnType_Orthogonal);
    }
    else
    {
        avoidRef->setRoutingType(Avoid::ConnType_PolyLine);
    }

    if (m_ideal_length == 0)
    {
        // If unset, read default.
        m_ideal_length = canvas()->idealConnectorLength();
    }
}


void Connector::routerRemove(void)
{
    canvas()->router()->deleteConnector(avoidRef);
    avoidRef = NULL;
}


void Connector::setNewEndpoint(const int endptType, QPointF pos,
    ShapeObj *shape, uint pinClassID)
{
    if (endptType == SRCPT)
    {
        srcpt.shape = shape;
        srcpt.pinClassID = pinClassID;

        srcpt.x = pos.x();
        srcpt.y = pos.y();
    }
    else
    {
        dstpt.shape = shape;
        dstpt.pinClassID = pinClassID;

        dstpt.x = pos.x();
        dstpt.y = pos.y();
    }

    if (canvas())
    {
        // If this has been added to the canvas, then reroute.
        if (avoidRef->router()->SimpleRouting)
        {
            applySimpleRoute();
            return;
        }

        setNewLibavoidEndpoint((endptType == SRCPT) ? VertID::src : VertID::tar);
        triggerReroute();
    }
}

void Connector::loneSelectedChange(const bool value)
{
    if (!value)
    {
        for (int i = 0; i < m_handles.size(); ++i)
        {
            delete m_handles.at(i);
        }
        m_handles.clear();
    }
    else // (value)
    {
        std::vector<Avoid::Point> checkpoints = avoidRef->routingCheckpoints();
        m_handles.resize(checkpoints.size() + 2);
        m_handles[0] = new ConnectorEndpointHandle(this, SRCPT);
        canvas()->addItem(m_handles[0]);
        m_handles[1] = new ConnectorEndpointHandle(this, DSTPT);
        canvas()->addItem(m_handles[1]);

        bool visible = (canvas()->optStructuralEditingDisabled() == false);
        for (int i = 0; i < 2; ++i)
        {
            m_handles[i]->setVisible(visible);
        }

        for (uint i = 0; i < checkpoints.size(); ++i)
        {
            m_handles[2 + i] = new ConnectorCheckpointHandle(this,
                (int) i, checkpoints[i].x, checkpoints[i].y);
            m_handles.at(2 + i)->setVisible(true);
        }
    }
    m_is_lone_selected = value;
}


QVariant Connector::itemChange(QGraphicsItem::GraphicsItemChange change,
        const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        // If there are handles because this is selected, reposition them.
        for (int i = 0; i < m_handles.size(); ++i)
        {
            m_handles[i]->reposition();
        }
    }
    return CanvasItem::itemChange(change, value);
}


void Connector::addXmlProps(const unsigned int subset, QDomElement& node,
        QDomDocument& doc)
{
    CanvasItem::addXmlProps(subset, node, doc);

    if (subset & XMLSS_IOTHER)
    {
        if (m_ideal_length != canvas()->idealConnectorLength())
        {
            newProp(node, x_idealLength, m_ideal_length);
        }

        if ( ! m_obeys_directed_edge_constraints)
        {
            newProp(node, x_obeysDirEdgeConstraints,
                    m_obeys_directed_edge_constraints);
        }

        if (orthogonalConstraint != NONE)
        {
            newProp(node, x_orthogonalConstraint,
                    orthogonalConstraint);
        }

        if (m_routing_type != polyline)
        {
            newProp(node, "routingType",
                      valueStringForEnum("RoutingType", m_routing_type));
        }

        if (m_arrow_head_type != normal)
        {
            newProp(node, "arrowHeadType",
                      valueStringForEnum("ArrowHeadType", m_arrow_head_type));
        }

        if (m_directed)
        {
            newProp(node, x_directed, m_directed);
        }

        if (m_colour != defaultConnLineCol)
        {
            QString value;
            value = value.sprintf("%02x%02x%02x%02x;", m_colour.red(),
                    m_colour.green(), m_colour.blue(), m_colour.alpha());
            newProp(node, x_lineCol, value);
        }
        
        write_libavoid_path(node, doc);

        char value[40];
        // also add line style
        if (dotted)
        {
            strcpy(value, "dashed");
            newProp(node, "LineStyle", value);
        }
    }

    if (subset & XMLSS_ICONNS)
    {
        if (srcpt.shape)
        {
            newProp(node, x_srcID, srcpt.shape->getIdString());
            if (srcpt.pinClassID != CENTRE_CONNECTION_PIN)
            {
                newProp(node, x_srcPinID, srcpt.pinClassID);
            }
        }
        else
        {
            newProp(node, x_srcID, 0);
        }

        newProp(node, x_srcX, srcpt.x);
        newProp(node, x_srcY, srcpt.y);

        if (dstpt.shape)
        {
            newProp(node, x_dstID, dstpt.shape->getIdString());
            if (dstpt.pinClassID != CENTRE_CONNECTION_PIN)
            {
                newProp(node, x_dstPinID, dstpt.pinClassID);
            }
        }
        else
        {
            newProp(node, x_dstID, 0);
        }
        newProp(node, x_dstX, dstpt.x);
        newProp(node, x_dstY, dstpt.y);
    }

    if (subset & XMLSS_XMOVE)
    {
        newProp(node, x_xPos, x());
        newProp(node, x_yPos, y());
    }
}


void Connector::setRoutingCheckPoints(const QList<QPointF>& checkpoints)
{
    std::vector<Avoid::Point> avoid_checkpoints(checkpoints.size());
    for (int i = 0; i < checkpoints.size(); ++i)
    {
        QPointF point = checkpoints.at(i);
        avoid_checkpoints[i] = Avoid::Point(point.x(), point.y());
    }
    avoidRef->setRoutingCheckpoints(avoid_checkpoints);
}


void Connector::deactivateAll(CanvasItemSet& selSet)
{
    // Is a connector.
    QPair<CPoint, CPoint> connpts = get_connpts();
    if (connpts.first.shape)
    {
        if (selSet.find(connpts.first.shape) == selSet.end())
        {
            // The connected shape is outside the selection, so:
            disconnect_from(connpts.first.shape);
        }
    }
    if (connpts.second.shape)
    {
        if (selSet.find(connpts.second.shape) == selSet.end())
        {
            // The connected shape is outside the selection, so:
            disconnect_from(connpts.second.shape);
        }
    }
    if (avoidRef)
    {
        avoidRef->makeInactive();
    }
}


void Connector::cascade_distance(int dist, unsigned int dir, CanvasItem **path)
{
    Q_UNUSED (dist)
    Q_UNUSED (dir)
    Q_UNUSED (path)

    // Does nothing for connectors.
    return;
}


QPair<CPoint, CPoint> Connector::get_connpts(void) const
{
    return qMakePair(srcpt, dstpt);
}


    // Returns pointers to the shapes that this connector is attached to.
QPair<ShapeObj *, ShapeObj *> Connector::getAttachedShapes(void)
{
    return qMakePair(srcpt.shape, dstpt.shape);
}


void Connector::rerouteAvoidingIntersections(void)
{
    avoidRef->setHateCrossings(true);

    bool rerouteImmediately = true;
    triggerReroute(rerouteImmediately);

    avoidRef->setHateCrossings(false);
}


bool Connector::hasSameEndpoints(void)
{
    double xdiff = fabs(srcpt.x - dstpt.x);
    double ydiff = fabs(srcpt.y - dstpt.y);
    int maxDiff = 7;

    if ((xdiff <= maxDiff) && (ydiff <= maxDiff))
    {
        return true;
    }
    return false;
}


void Connector::setDirected(const bool directed)
{
    if (directed == m_directed)
    {
        return;
    }

    // UNDO add_undo_record(DELTA_CONNDIR, this);

    m_directed = directed;
    update();

    canvas()->interrupt_graph_layout();
}

double Connector::idealLength(void) const
{
    return m_ideal_length;
}

void Connector::setIdealLength(double length)
{
    m_ideal_length = length;
}

Connector::RoutingType Connector::routingType(void) const
{
    return m_routing_type;
}

void Connector::setRoutingType(const Connector::RoutingType routingTypeVal)
{
    m_routing_type = routingTypeVal;
    if (avoidRef)
    {
        avoidRef->setRoutingType((m_routing_type == polyline) ?
                Avoid::ConnType_PolyLine : Avoid::ConnType_Orthogonal);
        reroute_connectors(canvas());
    }
}

Connector::ArrowHeadType Connector::arrowHeadType(void) const
{
    return m_arrow_head_type;
}

void Connector::setArrowHeadType(const Connector::ArrowHeadType arrowHeadTypeVal)
{
    m_arrow_head_type = arrowHeadTypeVal;
    buildArrowHeadPath();
    update();
}

void Connector::swapDirection(void)
{
    // Swap endpoints.
    CPoint tmp = dstpt;
    dstpt = srcpt;
    srcpt = tmp;
    
    forceReroute();

    canvas()->interrupt_graph_layout();
}


bool Connector::isDirected(void) const
{
    return m_directed;
}

bool Connector::hasDownwardConstraint(void) const
{
    return m_has_downward_constraint;
}

void Connector::setHasDownwardConstraint(const bool value)
{
    m_has_downward_constraint = value;
}

bool Connector::obeysDirectedEdgeConstraints(void) const
{
    return m_obeys_directed_edge_constraints;
}

void Connector::setObeysDirectedEdgeConstraints(const bool value)
{
    m_obeys_directed_edge_constraints = value;
}

QColor Connector::colour(void) const
{
    return m_colour;
}

void Connector::setColour(const QColor colour)
{
    m_colour = colour;
    update();
}

// set the colour to the supplied value, saving old value to enable reset
// to old value with reset_colour()
void Connector::overrideColour(QColor col)
{
    m_saved_colour = m_colour;
    setColour(col);
}

// reset the colour to the saved value.
void Connector::restoreColour()
{
    setColour(m_saved_colour);
}


void Connector::disconnect_from(ShapeObj *shape, uint pinClassID)
{
    if ((srcpt.shape == shape) &&
            ((pinClassID == 0) || (srcpt.pinClassID == pinClassID)))
    {
        srcpt.shape = NULL;
        srcpt.pinClassID = 0;
    }

    if ((dstpt.shape == shape) &&
            ((pinClassID == 0) || (dstpt.pinClassID == pinClassID)))
    {
        dstpt.shape = NULL;
        dstpt.pinClassID = 0;
    }
}


void Connector::applySimpleRoute(void)
{
    if (srcpt.shape)
    {
        // Use shape centre position for endpoint.
        QPointF srcPt = srcpt.shape->centrePos();
        srcpt.x = srcPt.x();
        srcpt.y = srcPt.y();
    }
    if (dstpt.shape)
    {
        // Use shape centre position for endpoint.
        QPointF dstPt = dstpt.shape->centrePos();
        dstpt.x = dstPt.x();
        dstpt.y = dstPt.y();
    }

    Avoid::PolyLine route(2);

    route.ps[0].x = srcpt.x;
    route.ps[0].y = srcpt.y;
    route.ps[0].vn = Avoid::kUnassignedVertexNumber;

    route.ps[1].x = dstpt.x;
    route.ps[1].y = dstpt.y;
    route.ps[1].vn = Avoid::kUnassignedVertexNumber;

    bool updateLibavoid = true;
    this->applyNewRoute(route, updateLibavoid);
}

void Connector::forceReroute(void)
{
    if (avoidRef->router()->SimpleRouting)
    {
        applySimpleRoute();
        return;
    }

    setNewLibavoidEndpoint(VertID::src);
    setNewLibavoidEndpoint(VertID::tar);

    triggerReroute();
}


void Connector::updateFromLibavoid(void)
{
    if (avoidRef->router()->SimpleRouting)
    {
        applySimpleRoute();
        return;
    }

    // Add end segments to connect to shape centres for border conn points.
    const Avoid::PolyLine &newroute = avoidRef->displayRoute();

    Avoid::PolyLine fixedroute;
    fixedroute._id = newroute._id;
    fixedroute.ps = newroute.ps;

    //Set points for endpoint handles.
    srcpt.x = fixedroute.at(0).x;
    srcpt.y = fixedroute.at(0).y;
    dstpt.x = fixedroute.at(fixedroute.size() - 1).x;
    dstpt.y = fixedroute.at(fixedroute.size() - 1).y;

    if (srcpt.shape && (srcpt.pinClassID != CENTRE_CONNECTION_PIN))
    {
        QPointF shapeCentre = srcpt.shape->centrePos();
        Point centrePoint = Point(shapeCentre.x(), shapeCentre.y());
        fixedroute.ps.insert(fixedroute.ps.begin(), centrePoint);
    }

    if (dstpt.shape && (dstpt.pinClassID != CENTRE_CONNECTION_PIN))
    {
        QPointF shapeCentre = dstpt.shape->centrePos();
        Point centrePoint = Point(shapeCentre.x(), shapeCentre.y());
        fixedroute.ps.push_back(centrePoint);
    }

    bool updateLibavoid = true;
    applyNewRoute(fixedroute, updateLibavoid);

    // Redraw the connector.
    update();
}


QString Connector::valueStringForEnum(const char *enumName, int enumValue)
{
    int index = staticMetaObject.indexOfEnumerator(enumName);
    QMetaEnum metaEnum = staticMetaObject.enumerator(index);
    QString keyString = metaEnum.valueToKey(enumValue);

    return keyString;
}



static void arrowPoints(const Point& l1, const Point& l2, Point *a1,
        Point *a2, Point *a3, const double arrowLen = 6, 
        const double arrowSpread = 3)
{
    double rise = l2.y - l1.y;
    double run = l2.x - l1.x;
    // Intervals
    double segLength = euclideanDist(l1, l2);
    double riseInt = rise / segLength;
    double runInt = run / segLength;
    
    a1->x = a3->x = l2.x - runInt  * arrowLen;
    a1->y = a3->y = l2.y - riseInt * arrowLen;
    a2->x = l2.x - runInt  * (arrowLen - 2);
    a2->y = l2.y - riseInt * (arrowLen - 2);
    
    a1->y += runInt  * arrowSpread;
    a1->x -= riseInt * arrowSpread;
   
    a3->y -= runInt  * arrowSpread;
    a3->x += riseInt * arrowSpread;
}

bool Connector::drawArrow(QPainterPath& painter_path, double srcx, double srcy,
        double dstx, double dsty, Connector::ArrowHeadType arrow_type)
{
    // Create a String List for the arrow_type value.  For example, if it
    // contains the value "circle_filled_line" then the string list will
    // have the strings "circle", "filled" and "line".
    QString keyString = valueStringForEnum("ArrowHeadType", (int) arrow_type);
    QStringList typeStrings = keyString.split("_");

    Point l1, l2, a1, a2, a3;
    l1.x = srcx;
    l1.y = srcy;
    
    l2.x = dstx;
    l2.y = dsty;
    
    double crossDistance = 6;

    arrowPoints(l1, l2, &a1, &a2, &a3);
    
    if (arrow_type == Connector::normal)
    {
        painter_path.moveTo(l2.x, l2.y);
        painter_path.lineTo(a1.x, a1.y);
        painter_path.lineTo((l2.x + 0.1 * (l2.x-a1.x)), 
                (l2.y + 0.1 * (l2.y-a2.y)));
        painter_path.lineTo(a3.x, a3.y);
        painter_path.closeSubpath();
    }
    else if (typeStrings.contains("diamond"))
    {
        painter_path.moveTo(l2.x, l2.y);
        painter_path.lineTo(a1.x, a1.y);
        painter_path.lineTo((a1.x + (a3.x - l2.x)), (a1.y + (a3.y - l2.y)));
        painter_path.lineTo(a3.x, a3.y);
        painter_path.closeSubpath();
    }
    else if (typeStrings.contains("circle"))
    {
        // Slightly away from shape.
        arrowPoints(l1, l2, &a1, &a2, &a3, 7.5);
        
        painter_path.addEllipse(QPointF(a2.x, a2.y), 4, 4);
    }
    else if (typeStrings.contains("triangle"))
    {
        arrowPoints(l1, l2, &a1, &a2, &a3, 7, 4);
 
        painter_path.moveTo(l2.x, l2.y);
        painter_path.lineTo(a1.x, a1.y);
        painter_path.lineTo(a3.x, a3.y);
        painter_path.closeSubpath();

        crossDistance = 11;
    }

    if (typeStrings.contains("cross"))
    {
        arrowPoints(l1, l2, &a1, &a2, &a3, crossDistance, 4);

        painter_path.moveTo(a1.x, a1.y);
        painter_path.lineTo(a3.x, a3.y);
        painter_path.closeSubpath();
    }

    return typeStrings.contains("outline");
}


//===========================================================================
// Code for object-avoiding connectors.


void Connector::setNewLibavoidEndpoint(const int type)
{
    if (avoidRef == NULL)
    {
        return;
    }

    CPoint& ep = (type == VertID::src) ? srcpt : dstpt;
    if (ep.shape)
    {
        Avoid::ConnEnd connEndRef(ep.shape->avoidRef, ep.pinClassID);
        avoidRef->setEndpoint(type, connEndRef);
    }
    else
    {
        Point tmppt(ep.x, ep.y);
        tmppt.id = m_internal_id;
        tmppt.vn = (type == VertID::src) ? 1 : 2;

        avoidRef->setEndpoint(type, tmppt);
    }
}


void Connector::triggerReroute(bool now)
{
    // Mark the old path as invalid.
    avoidRef->makePathInvalid();

    if (now)
    {
        // Do the rerouting right now.
        avoidRef->router()->processTransaction();
    }
    else
    {
        // Let the canvas know it needs to call libavoid's
        // processTransaction() and reroute connectors.
        canvas()->postRoutingRequiredEvent();
    }

    // Either apply the existing route, or a simple (straight-line) route.
    const Avoid::PolyLine &newroute = avoidRef->displayRoute();
    if (newroute.empty())
    {
        applySimpleRoute();
    }
    else
    {
        applyNewRoute(newroute, false);
    }
}


static QPainterPath cutPainterPathEnd(const QPainterPath& path, 
        const QPointF pathPos, const QPolygonF& shape) 
{
    QPainterPath result;
    QPointF cutPoint;
    const QRectF boundingRect = shape.boundingRect();

    bool found_intersection = false;
    int index = path.elementCount() - 1;
    while (!found_intersection && (index > 0))
    {
        QPainterPath::Element c1Elem = path.elementAt(index - 1);
        QPainterPath::Element c2Elem = path.elementAt(index);
        QLineF connLine(c1Elem.x, c1Elem.y, c2Elem.x, c2Elem.y);
        connLine.translate(pathPos);
        QLineF::IntersectType result = QLineF::NoIntersection;
        
        // Cheap test:
        if (boundingRect.contains(connLine.p1()) ||
                boundingRect.contains(connLine.p2()))
        {
            // More expensive test:
            for (int i = 1; i < shape.size(); ++i)
            {
                QLineF shapeLine(shape.at(i - 1), shape.at(i));

                result = connLine.intersect(shapeLine, &cutPoint);
                if (result == QLineF::BoundedIntersection)
                {
                    // Put back into item coordinates.
                    cutPoint -= pathPos;
                    break;
                }
            }
        }
      
        if (result == QLineF::BoundedIntersection)
        {
            // Found an intersection
           
            if (connLine.p1() == cutPoint)
            {
                // Intersection point is the same as the first point of
                // segment so disregard that point.
                index--;
            }
            break;
        }
        // Try the previous segment.
        index--;
    }
    if (index > 0)
    {
        // Found a cut point, now construct new path.
        for (int j = 0; j < path.elementCount(); ++j)
        {
            const QPainterPath::Element& element = path.elementAt(j);
            if (element.isMoveTo())
            {
                result.moveTo(element.x, element.y);
            }
            else if (element.isLineTo())
            {
                if (index == j)
                {
                    result.lineTo(cutPoint);
                    break;
                }
                else 
                {
                    result.lineTo(element.x, element.y);
                }
            }
            else if (element.isCurveTo())
            {
                assert((j + 2) < path.elementCount());
                const QPainterPath::Element& elem2 = path.elementAt(j + 1);
                const QPainterPath::Element& elem3 = path.elementAt(j + 2);
                assert(elem2.type == QPainterPath::CurveToDataElement);
                assert(elem3.type == QPainterPath::CurveToDataElement);
                if ((index >= j) && (index <= j + 2))
                {
                    if (index == j)
                    {
                        result.lineTo(cutPoint);
                    }
                    else if (index == j + 1)
                    {
                        result.cubicTo(element.x, element.y, element.x, 
                                element.y, cutPoint.x(), cutPoint.y());
                    }
                    else if (index == j + 2)
                    {
                        result.cubicTo(element.x, element.y, elem2.x, elem2.y,
                                cutPoint.x(), cutPoint.y());
                    }
                    break;
                }
                else
                {
                    result.cubicTo(element.x, element.y, elem2.x, elem2.y,
                            elem3.x, elem3.y);
                }
                j += 2;
            }
        }
        return result;
    }
    return path;
}


void Connector::applyNewRoute(const Avoid::PolyLine& route,
        bool updateLibavoid)
{
    if (route.size() < 2)
    {
        // XXX Fix this case properly.
        return;
        qFatal("Conn with too few points in Connector::applyNewRoute()");
    }
   
    if (updateLibavoid)
    {
        avoidRef->set_route(route);

        // Update Connpt positions, so that handles can be updated during
        // topology preserving layout when routes are returned won't always
        // match our internal endpoint positions.
        srcpt.x = route.ps[0].x;
        srcpt.y = route.ps[0].y;
        dstpt.x = route.ps[route.size() - 1].x;
        dstpt.y = route.ps[route.size() - 1].y;
    }

    applyNewRoute(Avoid::Polygon(route));
}

void Connector::reapplyRoute(void)
{
    applyNewRoute(avoidRef->displayRoute());
}

void Connector::applyNewRoute(const Avoid::Polygon& oroute)
{
    avoidRef->calcRouteDist();

    double roundingDist = (double) canvas()->optConnectorRoundingDistance();
    if (roundingDist > 0)
    {
        offset_route = oroute.curvedPolyline(roundingDist);
    }
    else
    {
        offset_route = oroute;
    }

    // Connectors position will be the first point.
    setPos(offset_route.ps[0].x, offset_route.ps[0].y);
    // Reset the painter path for this connector.
    QPainterPath painter_path = QPainterPath();
    int route_pn = offset_route.size();
    for (int j = 0; j < route_pn; ++j)
    {
        // Switch to local coordinates.
        offset_route.ps[j].x -= x();
        offset_route.ps[j].y -= y();
        if (j > 0)
        {
            applyMultiEdgeOffset(offset_route.ps[j - 1], offset_route.ps[j],
                    (j > 1));
        }
    }
    for (int j = 0; j < route_pn; ++j)
    {
        QPointF point(offset_route.ps[j].x, offset_route.ps[j].y);
        if (j == 0)
        {
            // First point.
            painter_path.moveTo(point);
        }
        else if ((roundingDist > 0) && (offset_route.ts[j] == 'C'))
        {
            // This is a bezier curve.
            QPointF c2(offset_route.ps[j + 1].x, offset_route.ps[j + 1].y);
            QPointF c3(offset_route.ps[j + 2].x, offset_route.ps[j + 2].y);
            painter_path.cubicTo(point, c2, c3);
            j += 2;
        }
        else
        {
            // This is a line.
            painter_path.lineTo(point);
        }
    }

    prepareGeometryChange();
    setPainterPath(painter_path);
    buildArrowHeadPath();
}

void Connector::buildArrowHeadPath(void)
{
    m_arrow_path = QPainterPath();
    if (m_directed)
    {
        if (dstpt.shape)
        {
            ShapeObj *shape = dstpt.shape;
            QPolygonF polygon = shape->shape().toFillPolygon();
            polygon.translate(shape->pos());
            
            QPainterPath cutPath = 
                    cutPainterPathEnd(m_conn_path, pos(), polygon);

            size_t path_size = cutPath.elementCount();
            if (path_size >= 2)
            {
                // Overlapping shapes can result in a connector with the two
                // endpoints at the same position.  This in turn results in
                // a QPainterPath with only a single element.  So, we only
                // add the arrow if there are at least two elements.
                QPainterPath::Element last = cutPath.elementAt(path_size - 1);
                QPainterPath::Element prev = cutPath.elementAt(path_size - 2);
                m_arrow_head_outline = drawArrow(m_arrow_path, prev.x,
                        prev.y, last.x, last.y, arrowHeadType());
            }
        }
        else
        {
            int line_size = offset_route.size();
            // The destination end is not attached to any shape.
            m_arrow_head_outline = drawArrow(m_arrow_path,
                    offset_route.ps[line_size - 2].x,
                    offset_route.ps[line_size - 2].y,
                    offset_route.ps[line_size - 1].x,
                    offset_route.ps[line_size - 1].y, arrowHeadType());
        }
    }
}

void Connector::setPainterPath(QPainterPath path)
{
    m_conn_path = path;

    // Wider path for cursor hit detection.
    QPainterPathStroker stroker;
    stroker.setWidth(8);
    m_shape_path = stroker.createStroke(m_conn_path);

    // Don't want this to be an open shape with fill, so add the reverse
    // path to join it back up to the beginning of the connector.
    path.connectPath(path.toReversed());
    path.closeSubpath();
    CanvasItem::setPainterPath(path);
}



#define mid(a, b) ((a < b) ? a + ((b - a) / 2) : b + ((a - b) / 2))
void Connector::applyMultiEdgeOffset(Point& p1, Point& p2, bool justSecond)
{
    double nx = p1.y - p2.y;
    double ny = p2.x - p1.x;
    double nl = sqrt(nx*nx+ny*ny);
    if (nl >= 1)
    {
        nx/=nl;
        ny/=nl;
    }
    double pos = (double)multiEdgeInd - ((double)multiEdgeSize-1.)/2.;
    Point offset(nx*pos*3., ny*pos*3.);

    if (!justSecond)
    {
        p1.x += offset.x;
        p1.y += offset.y;
    }
    p2.x += offset.x;
    p2.y += offset.y;
}


QRectF Connector::boundingRect(void) const
{
    const double padding = BOUNDINGRECTPADDING;
    return (painterPath().boundingRect().adjusted(-padding, -padding,
            +padding, +padding) | m_arrow_path.boundingRect());
}


QPainterPath Connector::shape() const
{
    return m_shape_path;
}


void Connector::paint(QPainter *painter,
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED (option)
    Q_UNUSED (widget)
    assert(painter->isActive());
#if 0
    painter->setPen(QColor(255, 0, 0, 100));
    painter->setBrush(QBrush(QColor(255, 0, 0, 100)));
    
    painter->drawRect(boundingRect());
#endif
    bool showDecorations = canvas() && ! canvas()->isRenderingForPrinting();

    // Draw downward constraint indicator:
    if ( m_has_downward_constraint && showDecorations &&
         (canvas()->optLayoutMode() == LAYOUT_STRUCTURE_FLOW) )
    {
        QColor colour(180, 0, 255, 25);
        QPen highlight(colour);
        highlight.setWidth(7);
        highlight.setCosmetic(true);
        // Draw highlight.
        painter->setPen(highlight);

        painter->drawPath(painterPath());
    }

    // Draw selection cue.
    if ( isSelected() && showDecorations &&
         (canvas()->inSelectionMode() || m_is_lone_selected) )
    {
        QColor colour(0, 255, 255, 70);
        QPen highlight(colour);
        highlight.setWidth(7);
        highlight.setCosmetic(true);
        // Draw highlight.
        painter->setPen(highlight);

        painter->drawPath(painterPath());
    }

    // Debug.
    if ( ! isSelected() && showDecorations )
    {
        std::vector<Avoid::Point> checkpoints = avoidRef->routingCheckpoints();
        for (size_t i = 0; i < checkpoints.size(); ++i)
        {
            QPointF checkpointPos(checkpoints[i].x, checkpoints[i].y);
            checkpointPos -= this->pos();
            painter->setPen(Qt::magenta);
            painter->setBrush(QBrush(Qt::magenta));
            painter->drawEllipse(checkpointPos, 4, 4);
        }
    }

    QPen pen(m_colour);
    if (dotted)
    {
        QVector<qreal> dashes;
        dashes << 5 << 3;
        pen.setDashPattern(dashes); 
    }
    painter->setPen(pen);
    painter->drawPath(painterPath());
    
    // Add the Arrowhead.
    if (m_directed)
    {
        // There is an arrowhead.
        if (m_arrow_head_outline)
        {
            // Use white fill.
            painter->setBrush(QBrush(Qt::white));
        }
        else
        {
            painter->setBrush(QBrush(m_colour));
        }
        painter->drawPath(m_arrow_path);
    }
}


QAction *Connector::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event,
        QMenu& menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QAction *changeType = menu.addAction((m_routing_type == orthogonal) ?
            tr("Change to polyline") : tr("Change to orthogonal"));
    QAction *changeDirected = menu.addAction((m_directed) ?
            tr("Change to undirected") : tr("Change to directed"));
    QAction *swapDirection = menu.addAction(
            tr("Swap connector direction"));
    QAction *addCheckpoint = menu.addAction(
            tr("Add routing checkpoint at this point"));

    if (!m_directed)
    {
        swapDirection->setVisible(false);
    }

    QAction *action = CanvasItem::buildAndExecContextMenu(event, menu);

    if (action == changeType)
    {
        RoutingType newRoutingType = (m_routing_type == orthogonal) ?
                    polyline : orthogonal;
        setRoutingType(newRoutingType);
    }
    else if (action == addCheckpoint)
    {
        Avoid::Point new_checkpoint(event->scenePos().x(), event->scenePos().y());
        std::vector<Avoid::Point> checkpoints = avoidRef->routingCheckpoints();
        checkpoints.push_back(new_checkpoint);
        avoidRef->makePathInvalid();
        avoidRef->setRoutingCheckpoints(checkpoints);
        reroute_connectors(canvas());
        bool wasVisible = isSelected();
        setSelected(false);
        setSelected(wasVisible);
    }
    else if (action == swapDirection)
    {
        this->swapDirection();
        bool wasVisible = isSelected();
        setSelected(false);
        setSelected(wasVisible);
    }
    else if (action == changeDirected)
    {
        setDirected(!isDirected());
        setSelected(true);
    }

    return action;
}


void Connector::write_libavoid_path(QDomElement& node, QDomDocument& doc)
{
    Q_UNUSED (doc)

    QString pathStr;
    QString str;
    const Avoid::PolyLine& route = avoidRef->route();

    if (route.empty())
    {
        // Don't write empty paths.
        return;
    }

    pathStr += str.sprintf("%d ", (int) route.size());
    for (size_t i = 0; i < route.size(); ++i)
    {
        pathStr += str.sprintf("%g %g %d %d ", route.ps[i].x, route.ps[i].y,
                route.ps[i].id, route.ps[i].vn);
    }
    
    if (!pathStr.isEmpty())
    {
        newProp(node, x_libavoidPath, pathStr);
    }
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

