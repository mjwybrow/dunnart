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

#ifndef GUIDELINE_H
#define GUIDELINE_H

#include <QObject>
#include <list>

#include "libdunnartcanvas/indicator.h"
#include "libdunnartcanvas/undo.h"

namespace dunnart {


enum dirctn {
    GUIDE_TYPE_VERT = 100,
    GUIDE_TYPE_HORI = 101
};

static const int CONNECT_TYPE_MULTIWAY = 150;

enum dtype {
    DIST_TOP = 0,
    DIST_MIDDLE = 1,
    DIST_BOTTOM = 2,
    DIST_LEFT = 3,
    DIST_CENTER = 4,
    DIST_RIGHT = 5,
    DIST_EDGES_V = 6,
    DIST_EDGES_H = 7,
    SEP_VERTICAL,
    SEP_HORIZONTAL
};

extern dirctn atypes_to_dirctn(atypes atype);


class Guideline: public Indicator
{
    Q_OBJECT
    Q_PROPERTY (double position READ position WRITE cmd_setPosition)

    UNDO_ACTION (Guideline, double, position, setPosition, UNDO_GUIDELINE_POS, "move guideline")

    public:
        Guideline(dirctn t, double position);
        Guideline(Canvas *canvas, const QDomElement& node, const QString& ns);

        double position(void) const;
        void setPosition(const double position);

        QDomElement to_QDomElement(const unsigned int subset, 
                QDomDocument& doc);
        virtual void cascade_distance(int dist, unsigned int dir,
                CanvasItem **path);
        void move_to(int x, int y, bool store_undo, bool human);
        virtual void deactivateAll(CanvasItemSet& selSet);
        void updateFromLayout(const double pos, const bool hasPos);
        void removeFromDistributions(const bool force);
        void resizedCanvas(void);
        QPainterPath buildPainterPath(void);
        virtual void paint(QPainter *painter, 
                const QStyleOptionGraphicsItem *option, QWidget *widget);
        virtual QPainterPath shape() const;
        virtual void findAttachedSet(CanvasItemSet& objSet);
        void addAttachedShapesToSet(CanvasItemSet& objSet);
        dirctn get_dir(void) const
        {
            return type;
        }
        void rangeOfAttachedShapes(double& min, double& max, int& nodes);
        void rangeOfAttachedObjects(double& min, double& max) const;
        bool operator<(const Guideline& rhs) const;
        bool operator==(const Guideline& rhs) const;
    protected:
        virtual QAction *buildAndExecContextMenu(
                QGraphicsSceneMouseEvent *event, QMenu& menu);
        virtual QVariant itemChange(GraphicsItemChange change,
                const QVariant &value);
        friend void createIndicatorHighlightCache(void);
        void initialiser(double position);

        dirctn type;
    private:
        virtual void userMoveBy(qreal dx, qreal dy);
        QRectF calculateBounds(QPointF pos) const;

        QRectF bounding_box_;
};


typedef std::list<Guideline *> GuidelineList;


extern bool guideCompare(Guideline *g1, Guideline *g2);
extern Guideline *createAlignment(const atypes atype, CanvasItemList& objList);


}
#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

