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

#ifndef INDICATOR_H
#define INDICATOR_H

#include <QList>
#include <QPen>
#include <list>
#include <cfloat>

#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/relationship.h"

namespace dunnart {

class Handle;

typedef std::list<Relationship *> RelsList;


class Indicator: public CanvasItem
{
    public:
        Indicator(const int order);
        Indicator(QString id, const int order);
        Indicator(const QDomElement& node, const QString& ns, const int order);
        bool reduceAlpha(void);
        bool reduceGlow(void);
        void setAlpha(const int a);
        void setGlow(void);
        bool isGlow(void);
        void lock(void);
        void unlock(void);
        bool isLocked(void);

        void setHighlighted(bool highlight);
        bool isHighlighted(void) const;

        virtual void paint(QPainter *painter,
                const QStyleOptionGraphicsItem *option, QWidget *widget);
        virtual void loneSelectedChange(const bool value);

        RelsList rels;
    protected:
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
                const QVariant &value);
        virtual QPainterPath buildPainterPath(void) = 0;

        unsigned int alpha;
        QList<Handle *> handles;
        QPen m_indicator_pen;
        QPen m_highlight_pen;
        QPen m_selection_pen;
        qreal m_curr_path_one_pixel;

    private:
        void initialiser(void);

        unsigned int glow;
        bool m_locked;
        bool m_highlighted;
};


extern double placement_aid_base_alpha;
extern bool glow_active_aids;
extern double fade_aid_amount;
extern bool fade_out_aids;
extern bool always_faded_aids;


}
#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

