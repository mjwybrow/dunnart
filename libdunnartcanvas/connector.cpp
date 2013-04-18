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


const QColor defaultConnLineCol = QColor(0, 0, 0);


Connector::Connector()
    : CanvasItem(NULL, QString(), ZORD_Conn),
      avoidRef(NULL),
      m_is_multiedge(false),
      m_multiedge_size(1),
      m_multiedge_index(0),
      m_orthogonal_constraint(NONE),
      m_ideal_length(0),
      m_colour(defaultConnLineCol),
      m_saved_colour(defaultConnLineCol),
      m_is_directed(false),
      m_has_downward_constraint(false),
      m_obeys_directed_edge_constraints(true),
      m_arrow_head_type(normal),
      m_is_dotted(false),
      m_is_lone_selected(false)
{
    qRegisterMetaType<dunnart::Connector::RoutingType>("RoutingType");
    qRegisterMetaTypeStreamOperators<int>("RoutingType");
    qRegisterMetaType<dunnart::Connector::ArrowHeadType>("ArrowHeadType");
    qRegisterMetaTypeStreamOperators<int>("ArrowHeadType");

    setCanvasItemFlag(CanvasItem::ItemIsMovable, false);

    setItemType("connector");

    m_routing_type = polyline;

    m_offset_route.clear();

    setHoverMessage("Connector \"%1\" - Select to modify path, drag to move "
            "connector (disconnecting it from attached shapes).");
}


void Connector::initWithConnection(ShapeObj *sh1, ShapeObj *sh2)
{
    if (sh1)
    {
        m_src_pt.shape = sh1;
        m_src_pt.pinClassID = CENTRE_CONNECTION_PIN;
    }
    else
    {
        qFatal("Conn::Conn(ShapeObj *s1, ShapeObj *s2) needs valid shapes");
    }

    if (sh2)
    {
        m_dst_pt.shape = sh2;
        m_dst_pt.pinClassID = CENTRE_CONNECTION_PIN;
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
        m_is_directed = true;
    }

    optionalProp(node, x_directed, m_is_directed, ns);

    m_src_pt.shape = NULL;
    m_src_pt.pinClassID = 0;
    m_src_pt.x = m_src_pt.y = 0;
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

            m_src_pt.shape = sh;
            m_src_pt.pinClassID = pinID;
        }
        if (!sh)
        {
            qWarning("could not find shape for `src' connection: \"%s\"",
                     qPrintable(sshape));
        }
    }
    else
    {
        m_src_pt.pinClassID = 0;

        m_src_pt.x = essentialProp<double>(node, x_srcX, ns);
        m_src_pt.y = essentialProp<double>(node, x_srcY, ns);
    }

    m_dst_pt.shape = NULL;
    m_dst_pt.pinClassID = 0;
    m_dst_pt.x = m_dst_pt.y = 0;
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

            m_dst_pt.shape = sh;
            m_dst_pt.pinClassID = pinID;
        }
        if (!sh)
        {
            qWarning("could not find shape for `dst' connection: \"%s\"",
                    qPrintable(sshape));
        }
    }
    else
    {
        m_dst_pt.pinClassID = 0;
        m_dst_pt.x = essentialProp<double>(node, x_dstX, ns);
        m_dst_pt.y = essentialProp<double>(node, x_dstY, ns);
    }
    
    optionalProp(node, x_idealLength, m_ideal_length, ns);
    optionalProp(node, x_obeysDirEdgeConstraints,
            m_obeys_directed_edge_constraints, ns);
    int oc=NONE;
    if (optionalProp(node, x_orthogonalConstraint, oc, ns)) 
    {
        m_orthogonal_constraint=(OrthogonalConstraint)oc;
        qDebug("orthogonal constraint=%d",oc);
    }
    value = nodeAttribute(node, ns, x_lineCol);
    if (!value.isNull())
    {
        m_colour = QColorFromRRGGBBAA(value.toLatin1().data());
        m_saved_colour = m_colour;
        //qDebug("read id=%s colour=%s", qPrintable(this->idString()),
        //       qPrintable(m_colour.name()));
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
        m_src_pt.shape = shape;
        m_src_pt.pinClassID = pinClassID;

        m_src_pt.x = pos.x();
        m_src_pt.y = pos.y();
    }
    else
    {
        m_dst_pt.shape = shape;
        m_dst_pt.pinClassID = pinClassID;

        m_dst_pt.x = pos.x();
        m_dst_pt.y = pos.y();
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
        std::vector<Avoid::Checkpoint> checkpoints =
                avoidRef->routingCheckpoints();
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
                (int) i, checkpoints[i].point.x, checkpoints[i].point.y);
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

        if (m_orthogonal_constraint != NONE)
        {
            newProp(node, x_orthogonalConstraint,
                    m_orthogonal_constraint);
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

        if (m_is_directed)
        {
            newProp(node, x_directed, m_is_directed);
        }

        if (m_colour != defaultConnLineCol)
        {
            QString value;
            value = value.sprintf("%02x%02x%02x%02x", m_colour.red(),
                    m_colour.green(), m_colour.blue(), m_colour.alpha());
            newProp(node, x_lineCol, value);
        }
        
        write_libavoid_path(node, doc);

        char value[40];
        // also add line style
        if (m_is_dotted)
        {
            strcpy(value, "dashed");
            newProp(node, "LineStyle", value);
        }
    }

    if (subset & XMLSS_ICONNS)
    {
        if (m_src_pt.shape)
        {
            newProp(node, x_srcID, m_src_pt.shape->idString());
            if (m_src_pt.pinClassID != CENTRE_CONNECTION_PIN)
            {
                newProp(node, x_srcPinID, m_src_pt.pinClassID);
            }
        }
        else
        {
            newProp(node, x_srcID, 0);
        }

        newProp(node, x_srcX, m_src_pt.x);
        newProp(node, x_srcY, m_src_pt.y);

        if (m_dst_pt.shape)
        {
            newProp(node, x_dstID, m_dst_pt.shape->idString());
            if (m_dst_pt.pinClassID != CENTRE_CONNECTION_PIN)
            {
                newProp(node, x_dstPinID, m_dst_pt.pinClassID);
            }
        }
        else
        {
            newProp(node, x_dstID, 0);
        }
        newProp(node, x_dstX, m_dst_pt.x);
        newProp(node, x_dstY, m_dst_pt.y);
    }

    if (subset & XMLSS_XMOVE)
    {
        newProp(node, x_xPos, x());
        newProp(node, x_yPos, y());
    }
}


void Connector::setRoutingCheckPoints(const QList<QPointF>& checkpoints)
{
    std::vector<Avoid::Checkpoint> avoid_checkpoints(checkpoints.size());
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
    return qMakePair(m_src_pt, m_dst_pt);
}


    // Returns pointers to the shapes that this connector is attached to.
QPair<ShapeObj *, ShapeObj *> Connector::getAttachedShapes(void)
{
    return qMakePair(m_src_pt.shape, m_dst_pt.shape);
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
    double xdiff = fabs(m_src_pt.x - m_dst_pt.x);
    double ydiff = fabs(m_src_pt.y - m_dst_pt.y);
    int maxDiff = 7;

    if ((xdiff <= maxDiff) && (ydiff <= maxDiff))
    {
        return true;
    }
    return false;
}


void Connector::setDirected(const bool directed)
{
    if (directed == m_is_directed)
    {
        return;
    }

    // UNDO add_undo_record(DELTA_CONNDIR, this);

    m_is_directed = directed;
    update();

    if (canvas())
    {
        canvas()->interrupt_graph_layout();
    }
}


QString Connector::getLabel(void) const
{
    return m_label;
}


void Connector::setLabel(const QString& label)
{
    m_label = label;
    update();
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
    CPoint tmp = m_dst_pt;
    m_dst_pt = m_src_pt;
    m_src_pt = tmp;
    
    forceReroute();

    if (canvas())
    {
        canvas()->interrupt_graph_layout();
    }
}


bool Connector::isDirected(void) const
{
    return m_is_directed;
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
    if ((m_src_pt.shape == shape) &&
            ((pinClassID == 0) || (m_src_pt.pinClassID == pinClassID)))
    {
        m_src_pt.shape = NULL;
        m_src_pt.pinClassID = 0;
    }

    if ((m_dst_pt.shape == shape) &&
            ((pinClassID == 0) || (m_dst_pt.pinClassID == pinClassID)))
    {
        m_dst_pt.shape = NULL;
        m_dst_pt.pinClassID = 0;
    }
}


void Connector::applySimpleRoute(void)
{
    QVariant srcPaddingVar = QVariant();
    QVariant dstPaddingVar = QVariant();
    double connGroupPadding = 0.0;
    if (m_src_pt.shape)
    {
        // Use shape centre position for endpoint.
        QPointF srcPt = m_src_pt.shape->centrePos();
        m_src_pt.x = srcPt.x();
        m_src_pt.y = srcPt.y();
        srcPaddingVar = m_src_pt.shape->property("layeredEndChannelPadding");
    }
    if (m_dst_pt.shape)
    {
        // Use shape centre position for endpoint.
        QPointF dstPt = m_dst_pt.shape->centrePos();
        m_dst_pt.x = dstPt.x();
        m_dst_pt.y = dstPt.y();
        dstPaddingVar = m_dst_pt.shape->property("layeredStartChannelPadding");

        // We're going to space the intermediate segment of orthogonal
        // connectors apart, based on their source in the tree.  We should
        // probably change this spacing based on the available space between
        // levels, but for now we just make it a fixed amount of 10.0.
        connGroupPadding = m_dst_pt.shape->property("layeredEndConnectorGroup").toDouble() * 10.0;
    }

    // Bend placement is halfway between endpoints.
    double bendPlacement = 0.50;

    if (m_routing_type == orthogonal)
    {
        // Draw a simple orthogonal line.
        Avoid::PolyLine route(4);

        // Source endpoint.
        route.ps[0] = Point(m_src_pt.x, m_src_pt.y);

        double xDiff = fabs(m_src_pt.x - m_dst_pt.x);
        double yDiff = fabs(m_src_pt.y - m_dst_pt.y);
        int xDir = (m_src_pt.x <= m_dst_pt.x) ? 1 : -1;
        int yDir = (m_src_pt.y <= m_dst_pt.y) ? 1 : -1;

        bool leftToRight = (xDiff > yDiff);
        if (m_has_downward_constraint &&
                srcPaddingVar.isValid() && dstPaddingVar.isValid() &&
                (canvas()->optLayoutMode() == Canvas::LayeredLayout))
        {
            // We are doing Layered layout and we have information about the
            // positions of the layer channel, so we can align the bends for
            // all connectors bridging the same two levels.
            double srcPadding = srcPaddingVar.toDouble();
            double dstPadding = dstPaddingVar.toDouble();

            // Use uniform connector direction during flow layout.
            Canvas::FlowDirection dir = canvas()->optFlowDirection();
            // Enter and exit shapes in the direction of flow.
            leftToRight = (dir == Canvas::FlowLeft) ||
                    (dir == Canvas::FlowRight);
            // At the beginning of channel.
            bendPlacement = 0.0;

            // Midpoints.
            if (leftToRight)
            {
                // o---+
                //     |
                //     +--->o

                xDiff -= (srcPadding + dstPadding);

                double xSegPos = m_dst_pt.x - xDir *
                        (dstPadding + connGroupPadding + (bendPlacement * xDiff));

                route.ps[1] = Point(xSegPos, m_src_pt.y);
                route.ps[2] = Point(xSegPos, m_dst_pt.y);
            }
            else
            {
                // o
                // |
                // |
                // +---+
                //     |
                //     v
                //     o

                yDiff -= (srcPadding + dstPadding);

                double ySegPos = m_dst_pt.y - yDir *
                        (srcPadding + connGroupPadding + (bendPlacement * yDiff));

                route.ps[1] = Point(m_src_pt.x, ySegPos);
                route.ps[2] = Point(m_dst_pt.x, ySegPos);
            }
        }
        else if (m_has_downward_constraint &&
                ((canvas()->optLayoutMode() == Canvas::FlowLayout) ||
                 (canvas()->optLayoutMode() == Canvas::LayeredLayout)))
        {
            // Use uniform connector direction during flow layout.
            Canvas::FlowDirection dir = canvas()->optFlowDirection();
            // Enter and exit shapes in the direction of flow.
            leftToRight = (dir == Canvas::FlowLeft) ||
                    (dir == Canvas::FlowRight);

            // Midpoints.
            if (leftToRight)
            {
                // o---+
                //     |
                //     +--->o

                double xSegPos = m_src_pt.x + xDir * (bendPlacement * xDiff);

                route.ps[1] = Point(xSegPos, m_src_pt.y);
                route.ps[2] = Point(xSegPos, m_dst_pt.y);
            }
            else
            {
                // o
                // |
                // |
                // +---+
                //     |
                //     v
                //     o

                double ySegPos = m_src_pt.y + yDir * (bendPlacement * yDiff);

                route.ps[1] = Point(m_src_pt.x, ySegPos);
                route.ps[2] = Point(m_dst_pt.x, ySegPos);
            }
        }
        else
        {
            // Standard case.
            // These use three segment paths rather than more optimal two
            // segment paths since these show the edges representing the
            // forces a little more clearly, and are also easier to perform
            // orthogonal improvement on.

            if (yDiff < xDiff)
            {
                route.ps[1] = Point(m_src_pt.x + (xDiff / 2.0) * xDir,
                        m_src_pt.y);
                route.ps[2] = Point(m_src_pt.x + (xDiff / 2.0) * xDir,
                        m_dst_pt.y);
            }
            else
            {
                route.ps[1] = Point(m_src_pt.x,
                        m_src_pt.y + (yDiff / 2.0) * yDir);
                route.ps[2] = Point(m_dst_pt.x,
                        m_src_pt.y + (yDiff / 2.0) * yDir);
            }
        }
        // Destination endpoint.
        route.ps[3] = Point(m_dst_pt.x, m_dst_pt.y);

        bool updateLibavoid = true;
        this->applyNewRoute(route, updateLibavoid);
    }
    else if (m_routing_type == polyline)
    {
        // Draw a simple straight line.
        Avoid::PolyLine route(2);

        route.ps[0] = Point(m_src_pt.x, m_src_pt.y);

        route.ps[1] = Point(m_dst_pt.x, m_dst_pt.y);

        bool updateLibavoid = true;
        this->applyNewRoute(route, updateLibavoid);
    }
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
    m_src_pt.x = fixedroute.at(0).x;
    m_src_pt.y = fixedroute.at(0).y;
    m_dst_pt.x = fixedroute.at(fixedroute.size() - 1).x;
    m_dst_pt.y = fixedroute.at(fixedroute.size() - 1).y;

    if (m_src_pt.shape && (m_src_pt.pinClassID != CENTRE_CONNECTION_PIN))
    {
        QPointF shapeCentre = m_src_pt.shape->centrePos();
        Point centrePoint = Point(shapeCentre.x(), shapeCentre.y());
        fixedroute.ps.insert(fixedroute.ps.begin(), centrePoint);
    }

    if (m_dst_pt.shape && (m_dst_pt.pinClassID != CENTRE_CONNECTION_PIN))
    {
        QPointF shapeCentre = m_dst_pt.shape->centrePos();
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

    CPoint& ep = (type == VertID::src) ? m_src_pt : m_dst_pt;
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
    QPainterPath cutPainterPath;
    QPointF cutPoint;
    const QRectF boundingRect = shape.boundingRect();

    bool hasfoundIntersection = false;
    int index = path.elementCount() - 1;
    while (!hasfoundIntersection && (index > 0))
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
                cutPainterPath.moveTo(element.x, element.y);
            }
            else if (element.isLineTo())
            {
                if (index == j)
                {
                    cutPainterPath.lineTo(cutPoint);
                    break;
                }
                else 
                {
                    cutPainterPath.lineTo(element.x, element.y);
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
                        cutPainterPath.lineTo(cutPoint);
                    }
                    else if (index == j + 1)
                    {
                        cutPainterPath.cubicTo(element.x, element.y, element.x,
                                element.y, cutPoint.x(), cutPoint.y());
                    }
                    else if (index == j + 2)
                    {
                        cutPainterPath.cubicTo(element.x, element.y, elem2.x, elem2.y,
                                cutPoint.x(), cutPoint.y());
                    }
                    break;
                }
                else
                {
                    cutPainterPath.cubicTo(element.x, element.y, elem2.x, elem2.y,
                            elem3.x, elem3.y);
                }
                j += 2;
            }
        }
        return cutPainterPath;
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
        m_src_pt.x = route.ps[0].x;
        m_src_pt.y = route.ps[0].y;
        m_dst_pt.x = route.ps[route.size() - 1].x;
        m_dst_pt.y = route.ps[route.size() - 1].y;
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
        m_offset_route = oroute.curvedPolyline(roundingDist);
    }
    else
    {
        m_offset_route = oroute;
    }

    // Connectors position will be the first point.
    setPos(m_offset_route.ps[0].x, m_offset_route.ps[0].y);
    // Reset the painter path for this connector.
    QPainterPath painter_path = QPainterPath();
    int route_pn = m_offset_route.size();
    for (int j = 0; j < route_pn; ++j)
    {
        // Switch to local coordinates.
        m_offset_route.ps[j].x -= x();
        m_offset_route.ps[j].y -= y();
        if (j > 0)
        {
            applyMultiEdgeOffset(m_offset_route.ps[j - 1], m_offset_route.ps[j],
                    (j > 1));
        }
    }
    for (int j = 0; j < route_pn; ++j)
    {
        QPointF point(m_offset_route.ps[j].x, m_offset_route.ps[j].y);
        if (j == 0)
        {
            // First point.
            painter_path.moveTo(point);
        }
        else if ((roundingDist > 0) && (m_offset_route.ts[j] == 'C'))
        {
            // This is a bezier curve.
            QPointF c2(m_offset_route.ps[j + 1].x, m_offset_route.ps[j + 1].y);
            QPointF c3(m_offset_route.ps[j + 2].x, m_offset_route.ps[j + 2].y);
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
    if (m_is_directed)
    {
        if (m_dst_pt.shape)
        {
            ShapeObj *shape = m_dst_pt.shape;
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
            int line_size = m_offset_route.size();
            // The destination end is not attached to any shape.
            m_arrow_head_outline = drawArrow(m_arrow_path,
                    m_offset_route.ps[line_size - 2].x,
                    m_offset_route.ps[line_size - 2].y,
                    m_offset_route.ps[line_size - 1].x,
                    m_offset_route.ps[line_size - 1].y, arrowHeadType());
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
    double pos = (double)m_multiedge_index - ((double)m_multiedge_size-1.)/2.;
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
         ( (canvas()->optLayoutMode() == Canvas::FlowLayout) ||
           (canvas()->optLayoutMode() == Canvas::LayeredLayout) ) )
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
        std::vector<Avoid::Checkpoint> checkpoints =
                avoidRef->routingCheckpoints();
        for (size_t i = 0; i < checkpoints.size(); ++i)
        {
            QPointF checkpointPos(checkpoints[i].point.x,
                    checkpoints[i].point.y);
            checkpointPos -= this->pos();
            painter->setPen(Qt::magenta);
            painter->setBrush(QBrush(Qt::magenta));
            painter->drawEllipse(checkpointPos, 4, 4);
        }
    }

    QPen pen(m_colour);
    if (m_is_dotted)
    {
        QVector<qreal> dashes;
        dashes << 5 << 3;
        pen.setDashPattern(dashes); 
    }
    painter->setPen(pen);
    painter->drawPath(painterPath());

    // Draw the connector's label.
    // XXX We need to work on positioning labels.
    painter->setPen(Qt::black);
    painter->setFont(canvas()->canvasFont());
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->drawText(painterPath().pointAtPercent(0.25), m_label);
    
    // Add the Arrowhead.
    if (m_is_directed)
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
    QAction *changeDirected = menu.addAction((m_is_directed) ?
            tr("Change to undirected") : tr("Change to directed"));
    QAction *swapDirection = menu.addAction(
            tr("Swap connector direction"));
    menu.addSeparator();
    QAction *addCheckpoint = menu.addAction(
            tr("Add routing checkpoint at this point"));

    if (!m_is_directed)
    {
        swapDirection->setVisible(false);
    }

    QAction *action = CanvasItem::buildAndExecContextMenu(event, menu);

    if (action == addCheckpoint)
    {
        // Checkpoint will only be added to connector under the cursor.
        Avoid::Point new_checkpoint(event->scenePos().x(),
                event->scenePos().y());
        std::vector<Avoid::Checkpoint> checkpoints =
                avoidRef->routingCheckpoints();
        checkpoints.push_back(new_checkpoint);
        avoidRef->makePathInvalid();
        avoidRef->setRoutingCheckpoints(checkpoints);
        reroute_connectors(canvas());
        bool wasVisible = isSelected();
        setSelected(false);
        setSelected(wasVisible);
    }
    else
    {
        // These actions affect all connectors in the current selection.
        QList<CanvasItem *> selected_items = canvas()->selectedItems();

        // Or the unselected connector under the cursor if there is no
        // selection at present.
        if (selected_items.empty())
        {
            selected_items.append(this);
        }

        RoutingType newRoutingType = (m_routing_type == orthogonal) ?
                polyline : orthogonal;

        // For each connector.
        for (int i = 0; i < selected_items.size(); ++i)
        {
            Connector *connector =
                    dynamic_cast<Connector *> (selected_items.at(i));

            if (connector)
            {
                if (action == changeType)
                {
                    connector->setRoutingType(newRoutingType);
                }
                else if (action == swapDirection)
                {
                    connector->swapDirection();
                }
                else if (action == changeDirected)
                {
                    connector->setDirected(!connector->isDirected());
                }
            }
        }
        canvas()->setSelection(selected_items);
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

    const Avoid::PolyLine& displayRoute = avoidRef->displayRoute();
    pathStr.clear();
    for (size_t i = 0; i < displayRoute.size(); ++i)
    {
        pathStr += str.sprintf("%g,%g ", displayRoute.ps[i].x,
                displayRoute.ps[i].y);
    }
    if (!pathStr.isEmpty())
    {
        newProp(node, "path", pathStr);
    }
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

