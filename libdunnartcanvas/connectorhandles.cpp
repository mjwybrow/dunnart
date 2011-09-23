// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2011  Monash University
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

#include <QGraphicsItem>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QBitmap>
#include <QPainter>

#include "libdunnartcanvas/connectorhandles.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/visibility.h"

#include "libavoid/connector.h"
#include "libavoid/connectionpin.h"

namespace dunnart {


static const char *connectorPointerXPM[] = {
  // width height num_colors chars_per_pixel
    "32 32 3 1",
  // colors
    "X c #000000",
    ". c #ffffff",
    "  c None",
  // pixels
    ".                               ",
    "..                              ",
    ".X.                             ",
    ".XX.                            ",
    ".XXX.                           ",
    ".XXXX.                          ",
    ".XXXXX.                         ",
    ".XXXXXX.                        ",
    ".XXXXXXX.                       ",
    ".XXXXXXXX.                      ",
    ".XXXXX.....                     ",
    ".XX.XX.                         ",
    ".X. .XX.                        ",
    "..  .XX.                        ",
    ".    .XX.                       ",
    "     .XX.         ...           ",
    "      ..          .X.           ",
    "                  .X.           ",
    "                  .X.           ",
    "        ...........X.           ",
    "        .XXXXXXXXXXX.           ",
    "        .X...........           ",
    "        .X.                     ",
    "        .X.                     ",
    "      ...X...                   ",
    "      .XXXXX.                   ",
    "       .XXX.                    ",
    "        .X.                     ",
    "         .                      ",
    "                                ",
    "                                ",
    "                                "
};


ConnectorEndpointHandle::ConnectorEndpointHandle(Connector *conn, unsigned int endpointType)
    : Handle(NULL, endpointType, 0),
      m_conn(conn)
{
    // Position it in front of other objects.
    this->setZValue(1000000);

    // Set connector cursor.
    setCursor(QCursor(QPixmap(connectorPointerXPM), 1, 2));

    setHoverMessage("Connector Endpoint Handle - Click and drag "
            "to reroute the connector or attach it to different "
            "objects.");
    reposition();
}

void ConnectorEndpointHandle::reposition(void)
{
    QPair<CPoint, CPoint> points = m_conn->get_connpts();
    int index = this->handleFlags();
    QPointF endpointPos;
    if (index == SRCPT)
    {
        endpointPos = QPointF(points.first.x, points.first.y);
    }
    else
    {
        endpointPos = QPointF(points.second.x, points.second.y);
    }
    setPos(endpointPos);
}


void ConnectorEndpointHandle::paint(QPainter *painter,
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED (option)
    Q_UNUSED (widget)
    assert(painter->isActive());

    if (canvas()->isRenderingForPrinting())
    {
        // Don't display handles if rendering for printing.
        return;
    }

    QColor highlight = QColor(0, 255, 255);
    painter->setPen(QPen(QBrush(Qt::black), 1));
    painter->setBrush(QBrush(QColor(highlight)));
    painter->drawEllipse(boundingRect());
}

void ConnectorEndpointHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    int index = this->handleFlags();

    ShapeObj *attShape = NULL;
    QList<CanvasItem *> canvas_items = m_conn->canvas()->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *>
                (canvas_items.at(i));
        if (shape && shape->contains(event->scenePos() - shape->scenePos()))
        {
            attShape = shape;
            break;
        }
    }

    if (attShape)
    {
        m_conn->setNewEndpoint(index, attShape->centrePos(), attShape,
                CENTRE_CONNECTION_PIN);
    }
    else
    {
        m_conn->setNewEndpoint(index, event->scenePos(), NULL);
    }

    Handle::mouseMoveEvent(event);
}


void ConnectorEndpointHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    reposition();

    // Cause layout to notice change in network structure.
    m_conn->canvas()->interrupt_graph_layout();

    Handle::mouseReleaseEvent(event);
}


ConnectorCheckpointHandle::ConnectorCheckpointHandle(Connector *conn,
        int index, double xpos, double ypos)
    : Handle(conn, index, 0),
      m_conn(conn),
      m_pos(xpos, ypos)
{
    // Set connector cursor.
    setCursor(QCursor(QPixmap(connectorPointerXPM), 1, 2));

    setHoverMessage("Connector Checkpoint Handle - Click "
            "and drag to move the position of this routing checkpoint.");

    reposition();
}

void ConnectorCheckpointHandle::reposition(void)
{
    //int index = this->handleFlags();
    QPointF itemPos = m_pos - m_conn->scenePos();
    setPos(itemPos);
}

void ConnectorCheckpointHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    int index = this->handleFlags();
    QPointF scenePosition = event->scenePos();
    Avoid::Point newPosition(scenePosition.x(), scenePosition.y());
    std::vector<Avoid::Point> checkpoints =
            m_conn->avoidRef->routingCheckpoints();
    checkpoints[index] = newPosition;
    m_conn->avoidRef->setRoutingCheckpoints(checkpoints);
    // XXX Horribly inefficient.
    reroute_connectors(m_conn->canvas(), true);

    Handle::mouseMoveEvent(event);
}

void ConnectorCheckpointHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (canvas() == NULL)
    {
        return;
    }

    if (event->button() == Qt::RightButton)
    {
        QMenu menu;
        QAction* removeCheckpoint = menu.addAction(
                QObject::tr("Remove this routing checkpoint"));

        QAction *action = menu.exec(event->screenPos(), removeCheckpoint);
        if (action == removeCheckpoint)
        {
            size_t index = this->handleFlags();
            std::vector<Avoid::Point> checkpoints =
                    m_conn->avoidRef->routingCheckpoints();
            std::vector<Avoid::Point> newCheckpoints;
            for (size_t i = 0; i < checkpoints.size(); ++i)
            {
                if (i != index)
                {
                    newCheckpoints.push_back(checkpoints[i]);
                }
            }

            m_conn->avoidRef->setRoutingCheckpoints(newCheckpoints);
            // XXX Horribly inefficient.
            reroute_connectors(m_conn->canvas(), true);
            this->setVisible(false);
            event->accept();
            return;
        }
        event->accept();
    }
    Handle::mousePressEvent(event);
}


ConnectionPinHandle::ConnectionPinHandle(ShapeObj *shape,
        const uint pinClassId, const Avoid::ShapeConnectionPin *pin)
    : Handle(shape, 0, 0),
      m_shape(shape),
      m_pin_class_id(pinClassId),
      m_pin(pin),
      m_clicked(false),
      m_new_conn(NULL)
{
    // Position it in front of other objects.
    setZValue(1000000);

    // Place center pin handles (which will cover the whole shape) below
    // other user-defined pin handles.
    if (m_pin_class_id == CENTRE_CONNECTION_PIN)
    {
        setZValue(zValue() - 1);
    }

    // Set connector cursor.
    setCursor(QCursor(QPixmap(connectorPointerXPM), 1, 2));

    setHoverMessage("Connection Point Handle - Click and drag to start "
            "drawing a new connector.");

    reposition();
}

QRectF ConnectionPinHandle::boundingRect(void) const
{
    // Have centre connection pin handles cover the whole shape, so the
    // user can drag from anywhere on the shape (that is not a specific
    // connection pin) to create a new connector that attaches to the
    // centre of the shape.
    if (m_pin_class_id == CENTRE_CONNECTION_PIN)
    {
        return m_shape->boundingRect();
    }
    else
    {
        return Handle::boundingRect();
    }
}

void ConnectionPinHandle::reposition(void)
{
    Avoid::Point position = m_pin->position();
    setPos(QPointF(position.x, position.y) - m_shape->pos());
}

void ConnectionPinHandle::paint(QPainter *painter,
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED (option)
    Q_UNUSED (widget)
    assert(painter->isActive());

    if (canvas()->isRenderingForPrinting())
    {
        // Don't display handles if rendering for printing.
        return;
    }

    QColor highlight = QColor(0, 255, 255);
    painter->setPen(QPen(QBrush(Qt::black), 1));
    painter->setBrush(QBrush(QColor(highlight)));
    painter->drawEllipse(QRectF(-3.5, -3.5, 7, 7));
}

void ConnectionPinHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    m_clicked = true;

    Handle::mousePressEvent(event);
}

void ConnectionPinHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_clicked)
    {
        return;
    }

    Canvas *canvas = m_shape->canvas();
    if (m_new_conn)
    {
        ShapeObj *attShape = NULL;
        QList<CanvasItem *> canvas_items = canvas->items();
        for (int i = 0; i < canvas_items.size(); ++i)
        {
            ShapeObj *shape = dynamic_cast<ShapeObj *> (canvas_items.at(i));
            if (shape == m_shape)
            {
                continue;
            }

            if (shape && shape->contains(event->scenePos() - shape->scenePos()))
            {
                attShape = shape;
                break;
            }
        }

        if (attShape)
        {
            m_new_conn->setNewEndpoint(DSTPT, attShape->centrePos(), attShape,
                    CENTRE_CONNECTION_PIN);
        }
        else
        {
            m_new_conn->setNewEndpoint(DSTPT, event->scenePos(), NULL);
        }
    }
    else
    {
        m_new_conn = new Connector();

        // Temporarily position in front of shapes.
        m_new_conn->setZValue(ZORD_Shape + 1);

        m_new_conn->setNewEndpoint(SRCPT,
                QPointF(m_pin->position().x, m_pin->position().y),
                m_shape, m_pin_class_id);
        m_new_conn->setNewEndpoint(DSTPT, event->scenePos(), NULL);

        UndoMacro *macro = canvas->beginUndoMacro(tr("Create Connector"));
        QUndoCommand *cmd = new CmdCanvasSceneAddItem(canvas, m_new_conn);
        macro->addCommand(cmd);
    }
}

void ConnectionPinHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_new_conn)
    {
        // Restore the normal z-ordering for the connector.
        m_new_conn->setZValue(ZORD_Conn);

        // Have graph layout notice the connection changes.
        Canvas *canvas = m_shape->canvas();
        canvas->interrupt_graph_layout();

        // Make the new shape the only selected one.
        canvas->deselectAll();
        m_new_conn->setSelected(true);
        m_new_conn = NULL;
    }

    Handle::mouseReleaseEvent(event);
}

bool ConnectionPinHandle::sceneEvent(QEvent *event)
{
    if (event->type() == QEvent::UngrabMouse)
    {
        // Ungrab action.
        // XXX We don't get this if the cursor is dragged off the canvas.
        //     Qt bug with dragging items with a parent perhaps?
    }
    Handle::sceneEvent(event);
}

}
