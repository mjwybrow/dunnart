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


#include <QPainter>
#include <cfloat>

#include "libavoid/libavoid.h"

#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/indicator.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/handle.h"
#include "libdunnartcanvas/canvas.h"

// For handle
#include "libdunnartcanvas/shape.h"

namespace dunnart {

double placement_aid_base_alpha = 128;
bool glow_active_aids = false;
double fade_aid_amount = 0;
bool always_faded_aids = false;
bool fade_out_aids = false;


Indicator::Indicator(const int order)
    : CanvasItem(NULL, QString(), order),
      alpha((always_faded_aids) ? (int) placement_aid_base_alpha : 255),
      glow(0),
      m_locked(false),
      m_highlighted(false)
{
    initialiser();
}


Indicator::Indicator(QString id, const int order)
    : CanvasItem(NULL, id, order),
      alpha((always_faded_aids) ? (int) placement_aid_base_alpha : 255),
      glow(0),
      m_locked(false),
      m_highlighted(false)
{
    initialiser();
}


Indicator::Indicator(const QDomElement& node, const QString& ns, 
        const int order)
    : CanvasItem(NULL, essentialProp<QString>(node, x_id), order),
      alpha((always_faded_aids) ? (int) placement_aid_base_alpha : 255),
      glow(0),
      m_locked(false),
      m_highlighted(false)
{
    Q_UNUSED (ns)

    initialiser();
}


void Indicator::initialiser(void)
{
    // Set standard one pixel width to be one.
    m_curr_path_one_pixel = 1;

    // Dark blue, transparent
    m_indicator_pen.setColor(QColor(0, 0, 255, 135));
    m_indicator_pen.setWidth(1);
    m_indicator_pen.setCosmetic(true);
#if 0
    QVector<qreal> dashes;
    dashes << 1 << 1 << 1;
    indicator_pen_.setDashPattern(dashes);
#endif

    // Selection colour is bright blue;
    m_selection_pen.setColor(QColor(0, 255, 255, 65));
    m_selection_pen.setWidth(5);
    m_selection_pen.setCosmetic(true);

    // Hightlight colour is bright orange;
    m_highlight_pen.setColor(QColor(255, 0, 220, 65));
    m_highlight_pen.setWidth(5);
    m_highlight_pen.setCosmetic(true);
}


bool Indicator::reduceAlpha(void)
{
    unsigned int origAlpha = alpha;
    
    if (fade_out_aids)
    {
        unsigned int lowestAlpha = (unsigned) placement_aid_base_alpha;
        
        unsigned int reduction = (unsigned) fade_aid_amount;
        if (alpha >= reduction)
        {
            alpha -= reduction;
        }
        else
        {
            alpha = 0;
        }
        
        if (alpha < lowestAlpha)
        {
            alpha = lowestAlpha;
        }
    }
    bool changes = (alpha != origAlpha) ? true : false;
    
    changes |= reduceGlow();
    
    return changes;
}


bool Indicator::reduceGlow(void)
{
    if (glow > 0)
    {
        glow -= 1;
        return (glow_active_aids && (glow == 0));
    }
    return false;
}


void Indicator::setAlpha(const int a)
{
    if ((a == 255) && always_faded_aids)
    {
        alpha = (unsigned int) placement_aid_base_alpha;
    }
    else
    {
        alpha = a;
    }
}


void Indicator::setGlow(void)
{
    glow = 2;
}


bool Indicator::isGlow(void)
{
    return (glow_active_aids && (glow > 0));
}


void Indicator::lock(void)
{
    m_locked = true;
}


void Indicator::unlock(void)
{
    m_locked = false;
}


bool Indicator::isLocked(void)
{
    return m_locked;
}


void Indicator::setHighlighted(bool highlight)
{
    m_highlighted = highlight;
}

bool Indicator::isHighlighted(void) const
{
    return m_highlighted;
}


void Indicator::paint(QPainter *painter,
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED (option)
    Q_UNUSED (widget)
    assert(painter->isActive());

    if (canvas()->isRenderingForPrinting())
    {
        // Don't display indicators if rendering for printing.
        return;
    }

    qreal onePixel = painter->transform().inverted().m11();
    if (m_curr_path_one_pixel != onePixel)
    {
        m_curr_path_one_pixel = onePixel;
        //qDebug("Indicator::paint(): Zoom change %g", onePixel);
        setPainterPath(buildPainterPath());
    }
    //painter->drawRect(boundingRect());
    if (isSelected() && canvas()->inSelectionMode())
    {
        // Draw selection cue.
        painter->setPen(m_selection_pen);
        painter->drawPath(painterPath());
    }
    else if (isHighlighted())
    {
        // Draw highlight cue.
        painter->setPen(m_highlight_pen);
        painter->drawPath(painterPath());
    }
    QPen pen = constraintConflict() ? QPen(HAZARD_COLOUR) : m_indicator_pen;
    painter->setPen(pen);
    painter->drawPath(painterPath());
}


void Indicator::loneSelectedChange(const bool value)
{
    for (int i = 0; i < handles.size(); ++i)
    {
        handles[i]->setVisible(value);
    }
}

QVariant Indicator::itemChange(QGraphicsItem::GraphicsItemChange change,
        const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
    }
    return CanvasItem::itemChange(change, value);
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

