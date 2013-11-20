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

#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>

#include "libavoid/libavoid.h"

#include "libdunnartcanvas/handle.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/guideline.h"

namespace dunnart {


Handle::Handle(QGraphicsItem *shape, int hflags, int index)
    : QGraphicsItem(shape),
      m_pin(NULL),
      m_active(false)
{
    Q_UNUSED (index)

    setHandleFlags(hflags);

    initialiser();
}


Handle::Handle(QGraphicsItem *shape, int hflags, double xr, double xo,
        double yr, double yo)
    : QGraphicsItem(shape),
      m_pin(NULL),
      m_active(false)
{
    xrel = xr;
    yrel = yr;
    xoff = xo;
    yoff = yo;

    setHandleFlags(hflags);

    initialiser();
}


void Handle::initialiser(void)
{
    // Don't scale this handle.
    setFlag(QGraphicsItem::ItemIgnoresTransformations, true);

    setFlag(QGraphicsItem::ItemIsMovable, true);
    setAcceptHoverEvents(true);

    reposition();

    setVisible(false);
}

Handle::~Handle()
{
}

Avoid::ShapeConnectionPin *Handle::getConnPin(void) const
{
    return m_pin;
}

void Handle::setConnPin(Avoid::ShapeConnectionPin *pin)
{
    m_pin = pin;
}


QRectF Handle::boundingRect(void) const
{
    return QRectF(-3.5, -3.5, 7, 7);
}


bool Handle::isActive(void) const
{
    return m_active;
}


void Handle::paint(QPainter *painter,
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
    painter->drawRect(boundingRect());
}


    // Return the first selected connector attached to this handle.
Connector *Handle::lead_selected_conn(void)
{
#if 0
    for (ConnSet::iterator c = conns.begin(); c != conns.end(); c++)
    {
        if ((*c)->get_active_image_n() == SHAPE_STATE_SELECTED)
        {
            return (*c);
        }
    }
#endif
    return NULL;
}


void Handle::reposition(void)
{
    double sw = 0, sh = 0;

    double nxpos = 0, nypos = 0;

    CanvasItem *parent = dynamic_cast<CanvasItem *> (parentItem());
    if (parent == NULL)
    {
        return;
    }

    int hflags = handleFlags();
    if (hflags & HAN_POINT)
    {
#if 0
        int index = get_identn(3);
        Polygon *poly = dynamic_cast<Polygon *> (parentItem());
        assert(poly != NULL);
        poly->getPointPos(index, &nxpos, &nypos);
#endif
    }
    else if (hflags & HAN_SEPARA)
    {
        Separation *separation = dynamic_cast<Separation *> (parentItem());

        nxpos = separation->handle_x;
        nypos = separation->handle_y;
    }
    else
    {
        sw = parent->width();
        sh = parent->height();

        if (xrel == HPLACE_MIN)
        {
            nxpos = - (sw / 2) + xoff;
        }
        else if (xrel == HPLACE_ZERO)
        {
            nxpos = 0 + xoff;
        }
        else // if (xrel == HPLACE_MAX)
        {
            nxpos = + (sw / 2) + xoff;
        }

        if (yrel == HPLACE_MIN)
        {
            nypos = - (sh / 2) + yoff;
        }
        else if (yrel == HPLACE_ZERO)
        {
            nypos = 0 + yoff;
        }
        else // if (yrel == HPLACE_MAX)
        {
            nypos = + (sh / 2) + yoff;
        }
    }

    if ((nxpos != x()) || (nypos != y()))
    {
        // If the handle has been moved.
        QGraphicsItem::setPos(nxpos, nypos);
    }
}


Avoid::Point Handle::getPointPosition(void)
{
    Avoid::Point retVal;

    retVal.x = scenePos().x();
    retVal.y = scenePos().y();

    return retVal;
}


#if 0
bool Handle::sceneEvent(QEvent *event)
{
    qDebug() << "Event\n";
    return false;
}
#endif

void Handle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mousePressEvent(event);
}


void Handle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseReleaseEvent(event);
}


Canvas *Handle::canvas(void) const
{
    return dynamic_cast<Canvas *> (scene());
}


void Handle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsItem::mouseMoveEvent(event);
}


void Handle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED (event)

    if (canvas())
    {
        canvas()->pushStatusMessage(m_hover_message);

        // Note that this is active and trigger redraw.
        m_active = true;
        update();
    }
}

void Handle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED (event)

    if (canvas())
    {
        canvas()->popStatusMessage();

        // Note that this is inactive and trigger redraw.
        m_active = false;
        update();
    }
}

void Handle::setHoverMessage(const QString& message)
{
    m_hover_message = message;
}


unsigned int Handle::get_directions(void)
{
    unsigned int hflags = handleFlags();
    hflags &= (HAN_TOP | HAN_BOT | HAN_RIGHT | HAN_LEFT);

    if (hflags)
    {
        return hflags;
    }
    return HAN_TOP | HAN_BOT | HAN_RIGHT | HAN_LEFT;
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
