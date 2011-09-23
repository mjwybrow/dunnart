/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2010  Monash University
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



#ifndef CONNECTORHANDLES_H
#define CONNECTORHANDLES_H

#include "libdunnartcanvas/handle.h"

namespace Avoid {
class ShapeConnectionPin;
}

namespace dunnart {

class ShapeObj;

class ConnectorEndpointHandle : public Handle
{
    Q_OBJECT

    public:
        ConnectorEndpointHandle(Connector *conn, unsigned int endpointType);
        virtual void reposition(void);
    protected:
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void paint(QPainter *painter,
                const QStyleOptionGraphicsItem *option, QWidget *widget);

        Connector *m_conn;
};


class ConnectorCheckpointHandle : public Handle
{
    Q_OBJECT

    public:
        ConnectorCheckpointHandle(Connector *conn, int index, double xpos,
                double ypos);
        virtual void reposition(void);
    protected:
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);

        Connector *m_conn;
        QPointF m_pos;
};


class ConnectionPinHandle : public Handle
{
    Q_OBJECT

    public:
        ConnectionPinHandle(ShapeObj *shape, const uint pinClassId,
                const Avoid::ShapeConnectionPin *pin);
        virtual void reposition(void);
        virtual QRectF boundingRect() const;
        virtual void paint(QPainter *painter,
                const QStyleOptionGraphicsItem *option, QWidget *widget);
    protected:
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual bool sceneEvent(QEvent *event);
        ShapeObj *m_shape;
        const uint m_pin_class_id;
        const Avoid::ShapeConnectionPin *m_pin;
        bool m_clicked;
        Connector *m_new_conn;
};


}
#endif // CONNECTORHANDLES_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

