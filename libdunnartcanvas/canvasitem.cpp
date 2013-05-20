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

#include <QApplication>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>
#include <QSvgGenerator>
#include <QStyleOptionGraphicsItem>

#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/freehand.h"
#include "libdunnartcanvas/textshape.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/cluster.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/polygon.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/visibility.h"
#include "libdunnartcanvas/svgshape.h"
#include "libdunnartcanvas/pluginshapefactory.h"
#include "libdunnartcanvas/connector.h"

#include "libdunnartcanvas/canvasitem.h"

#include "libavoid/router.h"


namespace dunnart {


typedef QMap<CanvasItem *, QDomElement> CanvasItemXmlList;
static CanvasItemXmlList inactiveObjXml;
static CanvasItemList inactiveObjList;


// SVG:
const char *x_id =               "id";
// Namespace:
const QString x_dunnartNs =        "dunnart";
// Dunnarts:
const char *x_type =             "type";
// Dunnart types:
const char *x_svgNode =          "svgNode";
const char *x_connector =        "connector";
const char *x_guideline =        "guideline";
const char *x_cluster =          "cluster";
const char *x_shPolygon =        "shPolygon";
const char *x_shTextShape =      "shTextShape";
const char *x_shFreehand =       "shFreehand";
// Other attributes;
const char *x_xPos =             "xPos";
const char *x_yPos =             "yPos";
const char *x_centreX =          "cx";
const char *x_centreY =          "cy";
const char *x_width =            "width";
const char *x_height =           "height";
const char *x_transparent =      "transparent";
const char *x_fillCol =          "fillColour";
const char *x_lineCol =          "lineColour";
const char *x_label =            "label";
const char *x_idealLength =      "idealLength";
const char *x_srcX =             "srcX";
const char *x_srcY =             "srcY";
const char *x_srcID =            "srcID";
const char *x_srcFlags =         "srcFlags";
const char *x_srcPinID =         "srcPinID";
const char *x_dstX =             "dstX";
const char *x_dstY =             "dstY";
const char *x_dstID =            "dstID";
const char *x_dstFlags =         "dstFlags";
const char *x_dstPinID =         "dstPinID";
const char *x_directed =         "directed";
const char *x_reversed =         "reversed";
const char *x_rectangular =      "rectangular";
const char *x_obeysDirEdgeConstraints = "obeysDirEdgeConstraints";
const char *x_orthogonalConstraint = "orthogonalConstraint";
const char *x_contains =         "contains";
const char *x_constraint =       "constraint";
const char *x_position =         "position";
const char *x_direction =        "direction";
const char *x_objOneID =         "objOneID";
const char *x_objTwoID =         "objTwoID";
const char *x_constraintID =     "constraintID";
const char *x_alignmentPos =     "alignmentPos";
const char *x_node =             "node";
const char *x_relType =          "relType";
const char *x_alignment =        "alignment";
const char *x_distribution =     "distribution";
const char *x_separation =       "separation";
const char *x_isMultiway =       "isMultiway";
const char *x_sepDistance =      "sepDistance";
const char *x_fontSize =         "fontSize";
const char *x_geometry =         "geometry";
const char *x_lockedPosition =   "lockedPos";
const char *x_libavoidPath =     "libavoidPath";

const char *x_dunnartURI = "http://www.dunnart.org/ns/dunnart";


CanvasItem::CanvasItem(QGraphicsItem *parent, QString id, unsigned int lev)
        : QGraphicsSvgItem(),
          m_string_id(id),
          m_internal_id(0),
          m_is_collapsed(false),
          m_is_inactive(false),
          m_connection_distance(-1),
          m_connection_cascade_glow(false),
          m_size(QSizeF(10, 10)),
          m_constraint_conflict(false)
{
    Q_UNUSED (parent)

    // QGraphicsSvgItem sets a cache type that causes problems for printing,
    // so disable this for our normal CanvasItems.
    setCacheMode(QGraphicsItem::NoCache);

    setZValue(lev);
    m_connected_objects[0] = m_connected_objects[1] = NULL;

    setAcceptHoverEvents(true);
    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemIsMovable, true);
#if QT_VERSION >= 0x040600
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
#endif
    setCursor(Qt::OpenHandCursor);
    setCanvasItemFlag(CanvasItem::ItemIsMovable, true);

    // Initial painter path.
    m_painter_path = buildPainterPath();
}


// This method reads object properties from an xml node.
void CanvasItem::initWithXMLProperties(Canvas *canvas,
        const QDomElement& node, const QString& ns)
{
    Q_UNUSED (canvas)
    Q_UNUSED (ns)
    // Set the object id.
    this->m_string_id = essentialProp<QString>(node, x_id);

    // Set dynamic properties for any attributes not recognised and handled
    // by Dunnart.
    QDomNamedNodeMap attrs = node.attributes();
    for (uint i = 0; i < attrs.length(); ++i)
    {
        QDomNode prop = attrs.item(i);
        QString name = prop.localName();
        if ( ! prop.prefix().isEmpty() && prop.prefix() != x_dunnartNs)
        {
            QString propName = prop.prefix() + ":" + prop.localName();
            setProperty(propName.toLatin1().constData(), prop.nodeValue());
        }
    }
}


CanvasItem::~CanvasItem()
{
    resetQueryModeIllumination(false);
}


void CanvasItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (canvas() == NULL)
    {
        return;
    }

    if (event->button() == Qt::LeftButton)
    {
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        // Drop through to parent handler.
    }
    else if (event->button() == Qt::RightButton)
    {
        QMenu menu;
        QAction *action = buildAndExecContextMenu(event, menu);

        if (action)
        {
            event->accept();
        }
    }

    QGraphicsItem::mousePressEvent(event);
}


void CanvasItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragReleaseEvent(event);
    }

    QGraphicsItem::mouseReleaseEvent(event);
}

bool CanvasItem::constraintConflict(void) const
{
    return m_constraint_conflict;
}

void CanvasItem::setConstraintConflict(const bool conflict)
{
    m_constraint_conflict = conflict;
    this->update();
}

QSizeF CanvasItem::size(void) const
{
    return m_size;
}

void CanvasItem::dragReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED (event)

    canvas()->setDraggedItem(NULL);
}


void CanvasItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    Actions& actions = canvas()->getActions();
    QPointF diff = event->pos() - event->lastPos();

    canvas()->setDraggedItem(this);

    foreach (CanvasItem *item, canvas()->selectedItems())
    {
        if (item->canvasItemFlags() & ItemIsMovable)
        {
            // Only move movable items.
            item->userMoveBy(diff.x(), diff.y());
            actions.moveList.push_back(item);
            canvas()->highlightIndicatorsForItemMove(item);
        }
    }
    canvas()->processResponseTasks();

    // Still do routing even when the layout thread is suspended.
    // XXX We need to only reroute connectors attached to moved shape
    //     during dragging if topology layout is turned on.
    if (canvas()->isLayoutSuspended())
    {
        reroute_connectors(canvas());
    }

    // We handle the moving of the shape, so no need to
    // call QGraphicsItem::mouseMoveEvent().
}


void CanvasItem::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED (event)

    if (canvas())
    {
        canvas()->pushStatusMessage(m_hover_message.arg(m_internal_id));
    }
}

void CanvasItem::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED (event)

    if (canvas())
    {
        canvas()->popStatusMessage();
    }
}

void CanvasItem::setHoverMessage(const QString& message)
{
    m_hover_message = message;
}


void CanvasItem::userMoveBy(qreal dx, qreal dy)
{
    // The standard case is to just move the shape.
    // Subclasses may provide their own version to do other work.
    moveBy(dx, dy);
}

Canvas *CanvasItem::canvas(void) const
{
    return dynamic_cast<Canvas *> (scene());
}


QRectF CanvasItem::boundingRect(void) const
{
    // We expect the painter path to have a valid bounding rect if this
    // is called.  If it is invalid, it may be being called incorrectly
    // from within buildPainterPath().
    assert(!m_painter_path.boundingRect().isNull());

    // Return the boundingRect, with padding for drawing selection cue.
    return m_painter_path.boundingRect().adjusted(-BOUNDINGRECTPADDING,
            -BOUNDINGRECTPADDING, +BOUNDINGRECTPADDING,
            +BOUNDINGRECTPADDING);
}


QPainterPath CanvasItem::shape() const
{
    return m_painter_path;
}


void CanvasItem::paint(QPainter *painter, 
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED (option)
    Q_UNUSED (widget)
    assert(painter->isActive());

    //painter->drawRect(-3, -3, 6, 6);
}


void CanvasItem::setIdString(const QString& id)
{
    if (canvas())
    {
        m_string_id = canvas()->assignStringId(id);
    }
    else
    {
        m_string_id = id;
    }
}

QString CanvasItem::idString(void) const
{
    return m_string_id;
}

uint CanvasItem::internalId(void) const
{
    return m_internal_id;
}

QString CanvasItem::itemType(void) const
{
    return m_type;
}

void CanvasItem::setItemType(const QString& type)
{
    m_type = type;
}

void CanvasItem::bringToFront(void)
{
    QList<QGraphicsItem *> overlapItems = collidingItems();

    foreach (QGraphicsItem *item, overlapItems)
    {
        if (item->zValue() == zValue())
        {
            item->stackBefore(this);
            item->update();
        }
    }
}


void CanvasItem::sendToBack(void)
{
    QList<QGraphicsItem *> overlapItems = collidingItems();

    foreach (QGraphicsItem *item, overlapItems)
    {
        if (item->zValue() == zValue())
        {
            this->stackBefore(item);
            this->update();
        }
    }
}


QColor QColorFromRRGGBBAA(char *str)
{
    unsigned int rrggbbaa = strtoul(str, NULL, 16);
    
    int alpha = rrggbbaa & 0xFF;
    unsigned int rrggbb = rrggbbaa >> 8;

    QColor colour((QRgb) rrggbb);
    colour.setAlpha(alpha);

    return colour;
}


QString nodeAttribute(const QDomElement& node, const QString& ns,
        const QString& prop)
{
    QDomNamedNodeMap attribs = node.attributes();

    for (int i = 0; i < attribs.count(); ++i)
    {
        QDomNode node = attribs.item(i);

        if (qualify(ns, prop) == node.nodeName())
        {
            return node.nodeValue();
        }
    }
    return QString();
}

bool nodeHasAttribute(const QDomElement& node, const QString& ns,
        const QString& prop)
{
    return !nodeAttribute(node, ns, prop).isNull();
}


CanvasItem *CanvasItem::create(Canvas *canvas, const QDomElement& node,
        const QString& dunnartURI, int pass)
{
    QString type = nodeAttribute(node, dunnartURI, x_type);
    assert(!type.isEmpty());

    QStringList oldConnTypes;
    oldConnTypes << "connStraight" << "connAvoidCurved" << "connAvoidPoly" <<
                    "connOrthogonal" << "connAvoidOrtho";

    CanvasItem *newObj = NULL;

    if (pass == PASS_SHAPES)
    {
        ShapeObj *shape = NULL;

        // Support legacy shapes from Dunnart v1.
        if (type == "flowEndOProc")
        {
            type = "roundedRect";
        }
        else if (type == "flowDiamond")
        {
            type = "diamond";
        }
        else if (type == "indGuide")
        {
            type = x_guideline;
        }

        if (type == x_guideline)
        {
            newObj = new Guideline(canvas, node, dunnartURI);
        }
        else if (type == x_distribution)
        {
            newObj = new Distribution(node, dunnartURI);
        }
        else if (type == x_separation)
        {
            newObj = new Separation(node, dunnartURI);
        }
        else if (!type.isEmpty() && (type != x_cluster) &&
                (type != x_constraint) && (type != x_connector) &&
                ! oldConnTypes.contains(type))
        {
            if (type == x_svgNode)
            {
                shape = new SvgShape();
            }
            else if (type == x_shTextShape)
            {
                shape = new TextShape();
            }
            else if (type == x_shPolygon)
            {
                shape = new PolygonShape();
            }
            else if (type == x_shFreehand)
            {
                shape = new FreehandShape();
            }
            else
            {
                // Load this shape from a plugin if the factory supports it,
                // or else create a generic RectangleShape().
                PluginShapeFactory *factory = sharedPluginShapeFactory();
                shape = factory->createShape(type);
            }
        }

        if (shape)
        {
            shape->initWithXMLProperties(canvas, node, dunnartURI);
            newObj = shape;
        }
    }
    else if (pass == PASS_CLUSTERS)
    {
        if (type == x_cluster)
        {
            newObj = new Cluster(canvas, node, dunnartURI);
        }
    }
    else if (pass == PASS_CONNECTORS)
    {
        if (type == x_connector)
        {
            Connector *conn = new Connector();
            conn->initWithXMLProperties(canvas, node, dunnartURI);
            newObj = conn;
        }
        else if (oldConnTypes.contains(type))
        {
            QString id = essentialProp<QString>(node, x_id);
            qWarning("Conn [%s] specified with old syntax: %s.",
                    qPrintable(id), qPrintable(type));

            Connector *conn = new Connector();
            conn->initWithXMLProperties(canvas, node, dunnartURI);
            newObj = conn;
        }
    }
    else if (pass == PASS_RELATIONSHIPS)
    {
        if (type == x_constraint)
        {
            new Relationship(canvas, node, dunnartURI);
        }
    }

    if (newObj)
    {
        QUndoCommand *cmd = new CmdCanvasSceneAddItem(canvas, newObj);
        canvas->currentUndoMacro()->addCommand(cmd);
    }

    return newObj;
}


void CanvasItem::addXmlProps(const unsigned int subset, QDomElement& node,
        QDomDocument& doc)
{
    Q_UNUSED (doc)

    if (subset & XMLSS_IOTHER)
    {
        newProp(node, "id", idString());

        newProp(node, x_type, itemType());

        // Add saved properties for props not used by Dunnart
        QList<QByteArray> propertyList = this->dynamicPropertyNames();
        for (int i = 0; i < propertyList.size(); ++i)
        {
            const char *propName = propertyList.at(i).constData();
            QStringList nameParts = QString(propName).split(":");
            assert(nameParts.length() <= 2);
            QString localName = nameParts.at(nameParts.length() - 1);
            QString prefix = "";
            if (nameParts.length() > 1)
            {
                prefix = nameParts.at(nameParts.length() - 2);
            }

            if ((prefix == node.prefix()) || (prefix == "svg"))
            {
                // Don't put this attribute in a namespace.
                newProp(node, localName, this->property(propName).toString());
            }
            else
            {
                // Add attribute in a NS prefix.
                newProp(node, qualify(prefix, localName),
                          this->property(propName).toString());
            }
        }
    }
}

QDomElement CanvasItem::to_QDomElement(const unsigned int subset, 
        QDomDocument& doc)
{
    QDomElement node = doc.createElement("dunnart:node");
    addXmlProps(subset, node, doc);

    return node;
}


QVariant CanvasItem::itemChange(GraphicsItemChange change,
        const QVariant &value)
{
    if (change == QGraphicsItem::ItemPositionHasChanged)
    {
        routerMove();
    }
    else if (change == QGraphicsItem::ItemSceneChange)
    {
        if (canvas())
        {
            // Being removed from the canvas
            routerRemove();
        }
    }
    else if (change == QGraphicsItem::ItemSceneHasChanged)
    {
        if (canvas())
        {
            // Give this item an id if it doesn't have one.
            m_string_id = canvas()->assignStringId(m_string_id);
            m_internal_id = canvas()->assignInternalId();
            // Being added to canvas
            routerAdd();
        }
    }
    return value;
}


void CanvasItem::routerAdd(void)
{
    // Do nothing for CanvasItem.  Subclasses will implement behaviour.
}
void CanvasItem::routerRemove(void)
{
    // Do nothing for CanvasItem.  Subclasses will implement behaviour.
}
void CanvasItem::routerMove(void)
{
    // Do nothing for CanvasItem.  Subclasses will implement behaviour.
}
void CanvasItem::routerResize(void)
{
    // Do nothing for CanvasItem.  Subclasses will implement behaviour.
}

QPainterPath CanvasItem::buildPainterPath(void)
{
    QPainterPath painter_path;
    return painter_path;
}


QPainterPath CanvasItem::painterPath(void) const
{
    return m_painter_path;
}

void CanvasItem::setPainterPath(QPainterPath path)
{
    m_painter_path = path;
}


void CanvasItem::setSizeAndUpdatePainterPath(const QSizeF& newSize)
{
    if (newSize == size())
    {
        // setSize won't update the painter path if the shape size
        // hasn't changed, so force that here.
        m_painter_path = buildPainterPath();
        update();
    }
    else
    {
        // If the size is different, then let setSize do the work.
        setSize(newSize);
    }
}

void CanvasItem::setSize(const QSizeF& newSize)
{
    if (newSize == size() && ! m_painter_path.isEmpty())
    {
        // Don't do anything if the size hasn't changed and
        // shape has a painter path.
        return;
    }
    prepareGeometryChange();
    m_size = newSize;
    // Update the painter path used to draw the shape.
    m_painter_path = buildPainterPath();

    //assert(m_painter_path.boundingRect().isNull() == false);

    // Update visibility graph for connector routing.
    routerResize();
}

void CanvasItem::setSize(const double w, const double h)
{
    CanvasItem::setSize(QSizeF(w, h));
}

double CanvasItem::width(void) const
{
    return m_size.width();
}

double CanvasItem::height(void) const
{
    return m_size.height();
}

QAction *CanvasItem::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event,
        QMenu& menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
#if 0
    QAction *cutAction = menu.addAction(tr("Cut"));
    QAction *copyAction = menu.addAction(tr("Copy"));
    QAction *deleteAction = menu.addAction(tr("Delete"));
#endif

    QAction *action = NULL;
    if (!menu.isEmpty())
    {
        QApplication::restoreOverrideCursor();
        action = menu.exec(event->screenPos());
    }

#if 0
    if (action == cutAction)
    {

    }
    else if (action == copyAction)
    {

    }
    else if (action == deleteAction)
    {

    }
#endif
    return action;
}

void CanvasItem::setCanvasItemFlag(CanvasItemFlag flag, bool enabled)
{
    if (enabled)
    {
        m_flags = m_flags | flag;
    }
    else
    {
        m_flags = m_flags & ~flag;
    }
}

CanvasItem::CanvasItemFlags CanvasItem::canvasItemFlags(void) const
{
    return m_flags;
}

bool CanvasItem::isCollapsed(void)
{
    return m_is_collapsed;
}


void CanvasItem::setAsCollapsed(bool collapsed)
{
    m_is_collapsed = collapsed;
    if (m_is_collapsed)
    {
        setSelected(false);
    }
}


bool CanvasItem::isInactive(void) const
{
    return m_is_inactive;
}


void returnAllInactive(void)
{
    // Return shapes.
    CanvasItemList tmpList = inactiveObjList;
    returnToCanvas(tmpList);
}


void returnAppropriateConnectors(void)
{
    CanvasItemList tmpList = inactiveObjList;
    for (CanvasItemList::iterator curr = tmpList.begin(); curr != tmpList.end(); 
            ++curr)
    {
        Connector *conn = dynamic_cast<Connector *> (*curr);
        if (conn)
        {
            conn->maybeReturn();
        }
    }
}


void CanvasItem::maybeReturn(void)
{
    assert(dynamic_cast<Connector *> (this));
    
    QDomElement node = inactiveObjXml[this];
    assert(!node.isNull());

    QString sshape, dshape;

    // XXX: This could have faster lookup with a set.
    if (optionalProp(node, x_srcID, sshape) && canvas()->getItemByID(sshape) &&
        optionalProp(node, x_dstID, dshape) && canvas()->getItemByID(dshape))
    {
        setAsInactive(false);
    }
}


void CanvasItem::setAsInactive(bool inactive, CanvasItemSet fullSet)
{
    if (m_is_inactive == inactive)
    {
        qFatal("Existing inactive state passed to CanvasItem::setAsInactive");
    }
    
    Connector *conn = dynamic_cast<Connector *> (this);
    Guideline *guide = dynamic_cast<Guideline *> (this);
    ShapeObj *shape = dynamic_cast<ShapeObj *> (this);

    QDomDocument doc("XML");
    m_is_inactive = inactive;
    if (m_is_inactive)
    {
        QDomElement xn = to_QDomElement(XMLSS_ALL, doc);
        assert(!xn.isNull());

        inactiveObjXml[this] = xn;

        setSelected(false);

        routerRemove();
        if (conn)
        {
            deactivateAll(fullSet);
        }

        setVisible(false);
        
        inactiveObjList.push_back(this);
    }
    else
    {
        inactiveObjList.removeAll(this);

        setVisible(true);
        if (shape)
        {
            deactivateAll(fullSet);
        }
        routerAdd();

        if (conn)
        {
            // This is a connector, so reattach.
            // QT conn->updateFromXmlRep(DELTA_REROUTE, inactiveObjXml[this]);
        }
        else if (guide)
        {
            // Reattach shapes to this guideline.
            // QT recursiveReadSVG(NULL, inactiveObjXml[this], NULL,
            //                 PASS_RELATIONSHIPS);
        }
        inactiveObjXml.remove(this);
    }
}


#if 0
void CanvasItem::glowSetClipRect(SDL_Surface *surface)
{
    // Only display guides to edge of things they are connected to.
    const bool two_tier_indicators = true;

    Guideline *guide = dynamic_cast<Guideline *> (this);

    if (connectedObjs[0] && connectedObjs[1])
    {
        // We're doing a pair query, only show relevant section
        // of the connector
        
        int offsetX = cxoff;
        int offsetY = cyoff;
        int x1 = connectedObjs[0]->get_xpos() + offsetX;
        int x2 = connectedObjs[1]->get_xpos() + offsetX;
        int y1 = connectedObjs[0]->get_ypos() + offsetY;
        int y2 = connectedObjs[1]->get_ypos() + offsetY;
        int w1 = connectedObjs[0]->get_width();
        int w2 = connectedObjs[1]->get_width();
        int h1 = connectedObjs[0]->get_height();
        int h2 = connectedObjs[1]->get_height();

        int cx = qMin(x1, x2);
        int cy = qMin(y1, y2);
        int cw = qMax(x1 + w1, x2 + w2) - cx;
        int ch = qMax(y1 + h1, y2 + h2) - cy;
        SDL_Rect crect = { cx, cy, cw, ch };
        SDL_SetClipRect(surface, &crect);
    }
    else if (two_tier_indicators && guide)
    {
        int cx1, cy1, cx2, cy2;

        // Extra pixels past shapes.
        int buffer = 15;
        getAttachedObjectsBounds<ShapeObj *>(guide, &cx1, &cy1, &cx2, &cy2,
                buffer);

        cx1 += cxoff;
        cy1 += cyoff;
        cx2 += cxoff;
        cy2 += cyoff;

        SDL_Rect crect = { cx1, cy1, cx2 - cx1, cy2 - cy1 };
        SDL_SetClipRect(surface, &crect);
    }
}



void CanvasItem::glowClearClipRect(SDL_Surface *surface)
{
    if (connectedObjs[0] && connectedObjs[1])
    {
        SDL_SetClipRect(surface, NULL);
    }
}

void CanvasItem::update_after_unhide(void)
{
    int nabsxpos = canvas->get_xpos() + cxoff + xpos;
    int nabsypos = canvas->get_ypos() + cyoff + ypos;
    
    Conn *c = dynamic_cast<Conn *> (this);
    if (c)
    {
        c->move_diff_points(nabsxpos - absxpos, nabsypos - absypos);
    }
    absxpos = nabsxpos;
    absypos = nabsypos;
}

#endif

void CanvasItem::move_to(const int x, const int y, bool store_undo)
{
    Q_UNUSED (store_undo)

    // UNDO undo record.

    // Visibility graph stuff:
    routerMove();
    
    setPos(x, y);
}
       

bool CanvasItem::cascade_logic(int& nextval, int dist, unsigned int dir,
        CanvasItem **path)
{
    Q_UNUSED (dir)

    // Clear stuff ahead:
    if (path)
    {
        int index = dist + 1;
        while (path[index])
        {
            path[index]->m_connection_distance = -1;
            path[index] = NULL;
            ++index;
        }
    }
    
    if (path && (this == path[0]))
    {
        if (m_connection_distance == -1)
        {
            m_connection_distance = dist;
        }
        path[dist + 1] = this;
        // Have reached destination
        for (int i = dist + 1; i > 0; --i)
        {
            // Mark path.
            path[i]->m_connection_distance = -2;
            path[i]->m_connection_cascade_glow = true;

            if ((i > 1) && (i < (dist + 1)))
            {
                path[i]->m_connected_objects[0] = path[i - 1];
                path[i]->m_connected_objects[1] = path[i + 1];
            }

#if 0
            bool regen_cache = true;
            path[i]->set_active_image(path[i]->get_active_image_n(),
                    regen_cache);
            path[i]->update();
#endif
        }
        // No more searching
        return false;
    }

    if (m_connection_distance == -1)
    {
        m_connection_distance = dist;
        path[dist + 1] = this;
    }
    else if (dist >= m_connection_distance)
    {
        return false;
    }

    nextval = dist + 1; 
    return true;
}


void CanvasItem::loneSelectedChange(const bool value)
{
    Q_UNUSED(value)

    // Nothing.
}


// This method returns the SVG tags that can be used to draw the CanvasItem
// in an output SVG file.  It does this by outputing the canvas with just the
// one item and them stripping the header and footer SVG tags.  Doing this
// allows us to know the SVG for individual CanvasItems and transform it by
// injecting object IDs or onHover code if we desire.
// This is required because the QSvgGenerator just uses QPainter instructions
// to generate the SVG and this has now information about the objects that
// were the source of the drawing instructions.
//
QString CanvasItem::svgCodeAsString(const QSize& size, const QRectF& viewBox)
{
    // Write SVG to a QBuffer.
    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);
    QSvgGenerator generator;
    generator.setOutputDevice(&buffer);

    // The the generator size and view the same as the set for the
    // wrapper SVG.
    generator.setSize(size);
    generator.setViewBox(viewBox);

    // Do the actual painting (output of SVG code)
    QPainter painter;
    if (painter.begin(&generator))
    {        
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setMatrix(this->sceneMatrix(), true);
        QStyleOptionGraphicsItem styleItem;
        this->paint(&painter, &styleItem, NULL);
        painter.end();
    }
    buffer.close();

    QString svgStr(buffer.data());

    // Remove SVG tags up to beginning of first group tag, which will be
    // the beginning of the definition of the object.
    svgStr = svgStr.remove(0, svgStr.indexOf("<g "));

    // Remove "</svg>\n" from end of string.
    svgStr.chop(7);

    return svgStr;
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

