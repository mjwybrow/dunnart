/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2009-2010  Monash University
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
 * Author(s): Sarah Boyd
*/

#ifndef TEMPLATE_H_
#define TEMPLATE_H_

#include <QWidget>
#include <QPainter>
#include <QPen>

#include "libcola/commondefs.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/handle.h"
#include "libdunnartcanvas/indicator.h"
#include "libdunnartcanvas/shape.h"
#include "libavoid/geomtypes.h"

namespace dunnart {


class Guideline;
class Handle;


typedef std::list<Guideline *> GuidelineList;
typedef std::list<Distribution *> DistributionList;

const int TEMPLATE_LINEAR_VERT = 0;
const int TEMPLATE_LINEAR_HORI = 1;
const int TEMPLATE_BRANCHED    = 3;

const double TEMPLATE_PADDING = 15;   // padding between the edge of nodes and the template outline
const double TEMPLATE_LENGTH = 200;   // default width/height for a template
const double TEMPLATE_THICKNESS = 50; // default thickness for a template


class Template : public Indicator
{
public:
    Template();
    void cascade_distance(int dist, unsigned int dir, CanvasItem **path);
    void deactivateAll(CanvasItemSet&);
    virtual void updatePositionFromSolver(const double pos, const bool store_undo) = 0;
    virtual double getPos(void) const = 0;
    virtual void move_diff(int diff_x, int diff_y, bool store_undo);
    virtual void snapShapeToNearestGuideline(ShapeObj *shape);
    bool testShapeOverlap(ShapeObj *shape);
    virtual void resize(const double newx, const double newy) = 0;
    virtual void startResize(void) {};
    static void resize_handler(QWidget **object_addr, int action);
    virtual void recalc_min_width_and_height(double &min_width, double &min_height) = 0;
    //virtual void addContextMenuItems(MenuItems& items){};
    
    static void handler(QWidget **object_addr, int action);
    
private:
    void initialise(void);


protected:
    double beginResizeW;
    double beginResizeH;
    double beginResizeRange;

    QPen template_pen_;
    QPen active_template_pen_;
    QBrush template_fill_;
};


class LinearTemplate : public Template
{
public:
    LinearTemplate(const double xpos, const double ypos, int orientation, Canvas *canvas);
    ~LinearTemplate();
    void updatePositionFromSolver(const double pos, const bool store_undo);
    double getPos(void) const;
    void flipGuidelineOrientation(void);
    // QT static void handler(QWidget **object_addr, int action);
    // QT static void resize_handler(QWidget **object_addr, int action){};
    void snapShapeToNearestGuideline(ShapeObj *shape);
    void recalc_min_width_and_height(double &min_width, double &min_height)
    {
        Q_UNUSED (min_width)
        Q_UNUSED (min_height)
    }
    void resize(const double newx, const double newy);
    //virtual void addContextMenuItems(MenuItems& items){};
    virtual void startResize(void);

    virtual QPainterPath buildPainterPath(void);
    virtual void paint(QPainter *painter,
            const QStyleOptionGraphicsItem *option, QWidget *widget);
    virtual QPainterPath shape() const;

    
    Guideline *guide;
    Handle* left_handle;
    Handle* right_handle;
    double _explicitNewBranchSpan;

private:
    virtual void userMoveBy(qreal diff_x, qreal diff_y);
};


class BranchedTemplate : public Template
{
public:
    BranchedTemplate(const double xpos, const double ypos);
    ~BranchedTemplate();
    void add_new_left_branch(double ypos,  double offset, dirctn guide_dir);
    void add_new_right_branch(double ypos, double offset, dirctn guide_dir);
    virtual void move_diff(int diff_x, int diff_y, bool store_undo);
    // QT static void handler(QWidget **object_addr, int action);
    // QT static void resize_handler(QWidget **object_addr, int action);
    void updatePositionFromSolver(const double pos, const bool store_undo);
    double getPos(void) const;
    virtual QPainterPath buildPainterPath(void);
    void snapShapeToNearestGuideline(ShapeObj *shape);
    void resize(const double newx, const double newy);
    void recalc_min_width_and_height(double &min_width, double &min_height);
    double currentBranchSpan(void);
    //virtual void addContextMenuItems(MenuItems& items);
    virtual void startResize(void);

    GuidelineList guidelist;
    DistributionList distro_list;
    Guideline* centreguide;
    Handle* left_handle;
    Handle* right_handle;

    double _explicitNewBranchSpan;
};

}
#endif // TEMPLATE_H_
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
