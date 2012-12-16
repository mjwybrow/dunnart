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

#ifndef DISTRO_H
#define DISTRO_H

#include "libdunnartcanvas/indicator.h"
#include "libdunnartcanvas/guideline.h"

class QRect;

namespace dunnart {


class Handle;

static const int DISTRO_DEFAULT_XPOS = 7;
static const int DISTRO_DEFAULT_YPOS = 4;

static const int DISTRO_TYPE_HORI = 250;
static const int DISTRO_TYPE_VERT = 251;

enum dgtype { DGT_NONE = 0, DGT_LEFT = 1, DGT_RIGHT = 2, DGT_OTHER = 3 };

class Distribution: public Indicator
{
    public:
        Distribution(GuidelineList *guides, int xp = DISTRO_DEFAULT_XPOS,
                int yp = DISTRO_DEFAULT_YPOS);
        Distribution(const QDomElement& node, const QString& ns);
        QDomElement to_QDomElement(const unsigned int subset, 
                QDomDocument& doc);
        virtual void cascade_distance(int dist, unsigned int dir,
                CanvasItem **path);
        void updateFromLayout(double separ);
        static bool guideLessThan(Guideline *g1, Guideline *g2);
        dgtype is_guide(Guideline *g);
        void moveToDefaultPos(const bool store_undo);
        void register_new_pos_diff(int dx, int dy);
        Guideline *leftGuide(void) const;
        Guideline *rightGuide(void) const;
        bool isReversed(void) const;
        virtual void deactivateAll(CanvasItemSet& selSet);
        void RemoveGuideline(Guideline *g);
        void RemoveEntire(void);
        void Resize(Guideline *guide, const double newx, const double newy,
                bool store_undo, double newogp = 0, double newsepr = 0,
                Guideline *newother = NULL);
        void shiftLockedGuideline(void);
        double getSeparation(void);
        void setSeparation(double new_space);
        virtual QPainterPath buildPainterPath(void);
        virtual void paint(QPainter *painter, 
                const QStyleOptionGraphicsItem *option, QWidget *widget);
        virtual QPainterPath shape() const;
        virtual QRectF boundingRect() const;

        dirctn type;
        dgtype lock_state;

        int draggedGuidelineN;
        int lockedGuidelineN;
        double space;
    protected:
//        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
//                const QVariant &value);
        void recreate(void);
        double newpos;

    private:
        Guideline *lockedGuideline(void) const;
        Guideline *draggedGuideline(void) const;
        Guideline *guidelineAtPosition(int guideN) const;
        void initialiser(void);
        virtual void userMoveBy(qreal dx, qreal dy);

        friend class DistributionHandle;
};


typedef std::list<Distribution *> DistributionList;

extern Distribution *createDistribution(QWidget *window, const dtype type,
        CanvasItemList& objList);


}
#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

