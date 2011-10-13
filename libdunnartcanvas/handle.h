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


#ifndef HANDLE_H
#define HANDLE_H

#include <QObject>
#include <QGraphicsItem>
#include <QString>

#include <set>

#include "libavoid/geomtypes.h"

namespace Avoid {
class ShapeConnectionPin;
}

namespace dunnart {

class Connector;
typedef std::set<Connector *> ConnSet;
class Canvas;

static const int HPLACE_MIN  = 0;
static const int HPLACE_ZERO = 1;
static const int HPLACE_MAX  = 2;

enum solVar { solX = 0, solY = 1, solW = 2, solH = 3 };

// Ways that the handle can function:
static const unsigned int AS_NOTHING       = 0;
static const unsigned int AS_SHAPE_HANDLE  = 1;
static const unsigned int AS_CONN_HANDLE   = 2;

static const int  HAND_EFF_X = 0;
static const int  HAND_EFF_W = 1;
static const int  HAND_EFF_Y = 2;
static const int  HAND_EFF_H = 3;


static const unsigned int  HAN_TOP    = 1;
static const unsigned int  HAN_BOT    = 2;
static const unsigned int  HAN_LEFT   = 4;
static const unsigned int  HAN_RIGHT  = 8;
static const unsigned int  HAN_HANDLE = 16;
static const unsigned int  HAN_CONNPT = 32;
static const unsigned int  HAN_CENDPT = 64;    // Free-floating connector endpoint.
static const unsigned int  HAN_SRC    = 128;
static const unsigned int  HAN_DST    = 256;
static const unsigned int  HAN_CENTER = 512;
static const unsigned int  HAN_POINT  = 1024;
static const unsigned int  HAN_SEPARA = 2048;


class Handle: public QObject, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)

    public:
        //friend void ShapeObj::deactivateAll(CanvasItemSet& selSet);
        Handle(QGraphicsItem *shape, int hflags, int index);
        Handle(QGraphicsItem *shape, int hflags, double xr, double xo,
                double yr, double yo);
        ~Handle();
        virtual void reposition(void);
        Connector *lead_selected_conn(void);
        unsigned int get_directions(void);
        Avoid::Point getPointPosition(void);
        virtual QRectF boundingRect() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                        QWidget *widget);
        int handleFlags(void)
        {
            return m_flags;
        }
        void setHandleFlags(int flags)
        {
            m_flags = flags;
        }
        Canvas *canvas(void) const;
        Avoid::ShapeConnectionPin *getConnPin(void) const;
        void setConnPin(Avoid::ShapeConnectionPin *pin);

        ConnSet conns;
    protected:
        void setHoverMessage(const QString& message);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        //virtual bool sceneEvent(QEvent *event);

        bool isActive(void) const;
    private:
        void initialiser(void);
        double xrel, xoff, yrel, yoff;
        int m_flags;
        QString m_hover_message;
        Avoid::ShapeConnectionPin *m_pin;
        bool m_active;
};

}
#endif // HANDLE_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
