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

#ifndef SEPARATION_H
#define SEPARATION_H

#include "libdunnartcanvas/indicator.h"

namespace dunnart {


static const double SEPARATION_DEFAULT_XPOS = 7;
static const double SEPARATION_DEFAULT_YPOS = 4;

enum sgtype { SGT_NONE = 0, SGT_LEFT = 1, SGT_RIGHT = 2, SGT_OTHER = 3 };

class Separation: public Indicator
{
    public:
        Separation(GuidelineList *guides, double gap,
                double xp = SEPARATION_DEFAULT_XPOS,
                double yp = SEPARATION_DEFAULT_YPOS);
        Separation(const QDomElement& node, const QString& ns);
        QDomElement to_QDomElement(const unsigned int subset, 
                QDomDocument& doc);
        virtual void cascade_distance(int dist, unsigned int dir,
                CanvasItem **path);
        void updateFromLayout(double newsep);
        static bool guideLessThan(Guideline *g1, Guideline *g2);
        sgtype is_guide(Guideline *g);
        void moveToDefaultPos(const bool store_undo);
        void register_new_pos_diff(int dx, int dy);
        Guideline *left_guide(void);
        Guideline *right_guide(void);
        bool is_reversed(void);
        virtual void deactivateAll(CanvasItemSet& selSet);
        void RemoveGuideline(Guideline *g);
        void RemoveEntire(void);
        void Resize(Guideline *guide, double oldSep, const QPointF posDiff,
                bool store_undo);
        void shiftLockedGuideline(void);
        Guideline *lockedGuideline(void);
        double getSeparation(void);
        virtual QPainterPath buildPainterPath(void);
        virtual void paint(QPainter *painter,
                const QStyleOptionGraphicsItem *option, QWidget *widget);
        virtual QPainterPath shape() const;
        virtual QRectF boundingRect() const;

        dirctn type;
        sgtype lock_state;
        double handle_x, handle_y;
        double gap;

    protected:
        void recreate(void);
        double space;
        double newpos;
    private:
        void userMoveBy(qreal dx, qreal dy);
        void initialiser(void);
};


extern double separation_constraint_gap;
typedef std::list<Separation *> SeparationList;

extern QPair<Separation *, QString> createSeparation(const dtype type,
        CanvasItemList& objList, double separationDist, bool sortGuidelines = true);


}
#endif // SEPARATION_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

