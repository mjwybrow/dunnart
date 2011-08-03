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

#include "libdunnartcanvas/connectorhandles.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/visibility.h"

#include "libavoid/connector.h"

namespace dunnart {

ConnectorEndpointHandle::ConnectorEndpointHandle(Connector *conn, unsigned int endpointType)
    : Handle(NULL, endpointType, 0),
      m_conn(conn)
{
    // Position it in front of other objects.
    this->setZValue(1000000);

    setCursor(Qt::ArrowCursor);
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
        m_conn->setNewEndpointPos(index, attShape->centrePos(), attShape,
                CENTRE_CONNECTION_PIN);
    }
    else
    {
        m_conn->setNewEndpointPos(index, event->scenePos(), NULL);
    }

    Handle::mouseMoveEvent(event);
}


void ConnectorEndpointHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)

    reposition();

    Handle::mouseReleaseEvent(event);
}


ConnectorCheckpointHandle::ConnectorCheckpointHandle(Connector *conn, int index, double xpos,
                double ypos)
            : Handle(conn, index, 0),
              m_conn(conn),
              m_pos(xpos, ypos)
        {
            setCursor(Qt::ArrowCursor);
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




}
