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


#include <algorithm>
#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>

#include "libavoid/libavoid.h"

#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/handle.h"

#include "libdunnartcanvas/distribution.h"



namespace dunnart {


struct RelationshipLessThan
{
    bool operator()(const Relationship *r1, const Relationship *r2)
    {
        return *(r1->guide) < *(r2->guide);
    }
};


class DistributionHandle : public Handle {
    public:
        DistributionHandle(Distribution *distro, int hflags, double xr,
                double xo, double yr, double yo) :
            Handle(distro, hflags, xr, xo, yr, yo)
        {
            if (hflags & (HAN_LEFT | HAN_RIGHT))
            {
                setCursor(Qt::SizeHorCursor);
            }
            else
            {
                setCursor(Qt::SizeVerCursor);
            }
            setHoverMessage("Distribution Handle - Click "
                    "and drag to resize distribution around marked "
                    "guideline. Hit TAB to change guideline.");
        }
    protected:
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
        {
            Distribution *distro = dynamic_cast<Distribution *> (parentItem());
            Guideline *active_guide = distro->draggedGuideline();
            QPointF newPos = event->scenePos();

            distro->Resize(active_guide, newPos.x(), newPos.y(), false);

            // The handle is moved through resizing of the distribution,
            // so no need to call Handle::mouseMoveEvent(event);
        }
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event)
        {
            Q_UNUSED (event)

            Distribution *distro = dynamic_cast<Distribution *> (parentItem());
            int hflags = this->handleFlags();
            hflags &= (HAN_TOP | HAN_BOT | HAN_RIGHT | HAN_LEFT);

            int guidesN = distro->relationships.size() + 1;

            int low  = distro->isReversed() ? guidesN : 1;
            int high = distro->isReversed() ? 1 : guidesN;

            switch (hflags)
            {
                case HAN_TOP:
                case HAN_LEFT:
                    distro->draggedGuidelineN = low;
                    distro->lockedGuidelineN  = high;
                    break;
                case HAN_BOT:
                case HAN_RIGHT:
                    distro->draggedGuidelineN = high;
                    distro->lockedGuidelineN  = low;
                    break;
                default:
                    break;
            }

            Guideline *guide = distro->lockedGuideline();
            guide->lock();

            if (canvas())
            {
                canvas()->pushStatusMessage("Distribution Handle - Click "
                        "and drag to resize distribution around marked "
                        "guideline. Hit TAB to change guideline.");
            }
        }
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
        {
            Q_UNUSED (event)

            Distribution *distro = dynamic_cast<Distribution *> (parentItem());

            // Do this first before we overwrite the variables that
            // control the locked guide.
            Guideline *guide = distro->lockedGuideline();
            guide->unlock();

            distro->draggedGuidelineN = 0;
            distro->lockedGuidelineN = 0;

            if (canvas())
            {
                canvas()->popStatusMessage();
            }
        }
};


Distribution::Distribution(GuidelineList *guides, int xp, int yp)
        : Indicator(ZORD_Distribution),
          draggedGuidelineN(0),
          lockedGuidelineN(0)
{
    setPos(xp, yp);
    
    setItemType(x_distribution);

    // Order guides:
    guides->sort(guideCompare);
    // Remove duplicates
    guides->unique();

    // Setup distribution representation.
    GuidelineList::iterator prev = guides->begin();
    GuidelineList::iterator curr = prev;
    curr++;
    for ( ; curr != guides->end(); curr++, prev++)
    {
        if (prev == guides->begin())
        {
            // UNDO add_undo_record(DELTA_MOVE, *prev);
        }
        // UNDO add_undo_record(DELTA_MOVE, *curr);
        new Relationship(this, *prev, *curr);
    }
    Guideline *first = guides->front(), *last = guides->back();
    first->move_to(first->x(), first->y(), false, false);
    last->move_to(last->x(), last->y(), false, false);

    type = guides->front()->get_dir();
    space = (last->position() - first->position()) / (double) relationships.size();
    recreate();
    initialiser();

    // There is a chance we have just recorded a move event for a 
    // guideline that is about to be deleted (if multiple guidelines from
    // the distribution were selected and deleted).  Thus, we clear the
    // list of actions.
    // XXX: This is all a bit dodgy.
    // QT Actions& actions = canvas()->getActions();
    // QT actions.clear();
}


Distribution::Distribution(const QDomElement& node, const QString& ns)
    : Indicator(node, ns, ZORD_Distribution),
      draggedGuidelineN(0),
      lockedGuidelineN(0)
{
    type = (dirctn) essentialProp<int>(node, x_direction, ns);

    double xpos = 50;
    double ypos = 50;
    if (type == GUIDE_TYPE_VERT)
    {
        ypos = essentialProp<double>(node, x_position, ns);
    }
    else
    {
        xpos = essentialProp<double>(node, x_position, ns);
    }

    space = 0;
    optionalProp(node, x_sepDistance, space, ns);

    setPos(xpos, ypos);

    recreate();
    initialiser();
}


void Distribution::initialiser(void)
{
    if (type == GUIDE_TYPE_VERT)
    {
        handles.push_back(new DistributionHandle(this, HAN_HANDLE | HAN_LEFT,
                HPLACE_MIN, 4, HPLACE_ZERO, 0));
        handles.push_back(new DistributionHandle(this, HAN_HANDLE | HAN_RIGHT,
                HPLACE_MAX, -4, HPLACE_ZERO, 0));
    }
    else // if (type == GUIDE_TYPE_HORI)
    {
        handles.push_back(new DistributionHandle(this, HAN_HANDLE | HAN_TOP,
                HPLACE_ZERO, 0, HPLACE_MIN, 4));
        handles.push_back(new DistributionHandle(this, HAN_HANDLE | HAN_BOT,
                HPLACE_ZERO, 0, HPLACE_MAX, -4));
    }
    handles.front()->setVisible(false);
    handles.back()->setVisible(false);
}

void Distribution::cascade_distance(int dist, unsigned int dir, CanvasItem **path)
{
    int nextval = 0;
    if (cascade_logic(nextval, dist, dir, path) == false)
    {
        return;
    }

    RelsList::iterator start = relationships.begin();
    RelsList::iterator finish = relationships.end();
    if (start != finish)
    {
        if ((*start)->guide)
        {
            (*start)->guide->cascade_distance(nextval, dir, path);
        }
    }

    for (RelsList::iterator r = start; r != finish; ++r)
    {
        if ((*r)->guide2)
        {
            (*r)->guide2->cascade_distance(nextval, dir, path);
        }
    }
}


bool Distribution::guideLessThan(Guideline *g1, Guideline *g2)
{
    if (g1->get_dir() == GUIDE_TYPE_VERT)
    {
        return g1->x() <= g2->x();
    }
    return g1->y() <= g2->y();
}


void Distribution::recreate(void)
{
    RelsList trels = relationships;
    qSort(trels.begin(), trels.end(), RelationshipLessThan());
    
    double bx, by, bw, bh, first, last, size = 16;

    if (relationships.empty())
    {
        bx = x();
        by = y();
        bw = 5;
        bh = 5;
        newpos = (type == GUIDE_TYPE_VERT) ? y() : x();
    }
    else if (type == GUIDE_TYPE_VERT)
    {
        first = qMin(trels.front()->guide->x(),
                trels.front()->guide2->x());
        last  = qMax(trels.back()->guide->x(),
                trels.back()->guide2->x());
        
        bw = last - first + 8;
        bh = size;

        bx = first - 4 + (bw / 2);
        by = y();
    }
    else // if (type == GUIDE_TYPE_HORI)
    {
        first = qMin(trels.front()->guide->y(),
                trels.front()->guide2->y());
        last  = qMax(trels.back()->guide->y(),
                trels.back()->guide2->y());
        
        bw = size;
        bh = last - first + 8;
        
        bx = x();
        by = first - 4 + (bh / 2);
    }

    setPos(bx, by);
    setSizeAndUpdatePainterPath(QSizeF(bw, bh));

    // Should be in initialiser
    setHoverMessage("Distribution Indicator \"%1\" - Drag to move distribution "
                    "and attached objects.");
}


void Distribution::updateFromLayout(double separ)
{
    d_printf("\t\t[%4u] - sepr: %.2f  <- solver\n", m_internal_id, separ);

    setAlpha(255);
    setGlow();

    // Recreate the object:

    recreate();
    for (int i = 0; i < handles.size(); ++i)
    {
        handles[i]->reposition();
    }
}


double Distribution::getSeparation(void)
{
    return space;
}

void Distribution::setSeparation(double new_space)
{
    space = new_space;
    
    Actions& actions = canvas()->getActions();
    actions.moveList.push_back(this);
}


void Distribution::userMoveBy(qreal dx, qreal dy)
{
    // Move the attached guidelines
    Actions& actions = canvas()->getActions();
    QSet<Guideline *> guides;
    for (RelsList::iterator g = relationships.begin(); g != relationships.end(); ++g)
    {
        guides.insert((*g)->guide);
        guides.insert((*g)->guide2);
    }
    foreach (Guideline *guideline, guides)
    {
        if (!guideline->isSelected())
        {
            // Move the guideline, if it is unselected.
            guideline->QGraphicsItem::moveBy(dx, dy);
            actions.moveList.push_back(guideline);
        }
    }
    moveBy(dx, dy);
}


QPainterPath Distribution::shape() const
{
    QRectF area(-width() / 2, -height() / 2, width(), height());
    double onePixel = m_curr_path_one_pixel;
    if (type == GUIDE_TYPE_VERT)
    {
        area.setTop(+14 * onePixel);
        area.setBottom(-10 * onePixel);
    }
    else
    {
        area.setLeft(-14 * onePixel);
        area.setRight(10 * onePixel);
    }
    QPainterPath path;
    path.addRect(area);
    return path;
}

QRectF Distribution::boundingRect() const
{
    const double padding = BOUNDINGRECTPADDING;
    QRectF bb(-width() / 2, -height() / 2, width(), height());
    bb.adjust(-padding, -padding, padding, padding);
    return bb;
}



QPainterPath Distribution::buildPainterPath(void)
{
    QPainterPath painter_path;
    
    double halfSep = fabs(space / 2);
    int sections = (int) relationships.size();

    const qreal one =  m_curr_path_one_pixel;
    const qreal gap = one * 2;
    double bridgeMin = DBL_MAX;
    double bridgeMax = -DBL_MAX;
    double bridgePos = -5 * one;
    double markerPos = 0;
    double padding = 4 * one;


    //int size = 2;
    //int skip = 3;

    if ((space < 8.0) && (space > -8.0))
    {
        // Too small to draw.
        return painter_path;
    }

    if (type == GUIDE_TYPE_VERT)
    {
        RelsList::iterator finish = relationships.end();
        for (RelsList::iterator r = relationships.begin(); r != finish; ++r)
        {
            double gp1 = (*r)->guide->x();
            double gp2 = (*r)->guide2->x();

            double pos1 = qMin(gp1, gp2) - x() + padding;
            double pos2 = qMax(gp1, gp2) - x() - padding;
            
            if (sections > 1)
            {
                double position = pos1 - 2 + halfSep - (padding / 2);
                bridgeMin = qMin(bridgeMin, position);
                bridgeMax = qMax(bridgeMax, position);
                painter_path.moveTo(position, bridgePos);
                painter_path.lineTo(position, markerPos);
            }

            {
                // Need extra space for handles.

                if (*r == relationships.front())
                {
                    pos1 += 4 * one;
                }
                
                if (*r == relationships.back())
                {
                    pos2 -= 4 * one;
                }
            }

            painter_path.moveTo(pos1, markerPos);
            painter_path.lineTo(pos2, markerPos);
            
            painter_path.moveTo(pos1, markerPos - gap);
            painter_path.lineTo(pos1, markerPos + gap);
            
            painter_path.moveTo(pos2, markerPos - gap);
            painter_path.lineTo(pos2, markerPos + gap);
        }

        if (sections > 1)
        {
            painter_path.moveTo(bridgeMin, bridgePos);
            painter_path.lineTo(bridgeMax, bridgePos);
        }
    }
    else // if (type == GUIDE_TYPE_HORI)
    {
        RelsList::iterator finish = relationships.end();
        for (RelsList::iterator r = relationships.begin(); r != finish; ++r)
        {
            double gp1 = (*r)->guide->y();
            double gp2 = (*r)->guide2->y();

            double pos1 = qMin(gp1, gp2) - y() + padding;
            double pos2 = qMax(gp1, gp2) - y() - padding;
          
            if (sections > 1)
            {
                double position = pos1 - 2 + halfSep - (padding / 2);
                bridgeMin = qMin(bridgeMin, position);
                bridgeMax = qMax(bridgeMax, position);
                painter_path.moveTo(bridgePos, position);
                painter_path.lineTo(markerPos, position);
            }

            {
                // Need extra space for handles.

                if (*r == relationships.front())
                {
                    pos1 += 4 * one;
                }

                if (*r == relationships.back())
                {
                    pos2 -= 4 * one;
                }
            }

            painter_path.moveTo(markerPos, pos1);
            painter_path.lineTo(markerPos, pos2);
            
            painter_path.moveTo(markerPos - gap, pos1);
            painter_path.lineTo(markerPos + gap, pos1);
            
            painter_path.moveTo(markerPos - gap, pos2);
            painter_path.lineTo(markerPos + gap, pos2);
        }

        if (sections > 1)
        {
            painter_path.moveTo(bridgePos, bridgeMin);
            painter_path.lineTo(bridgePos, bridgeMax);
        }
    }
    return painter_path;
}


void Distribution::paint(QPainter *painter, 
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Indicator::paint(painter, option, widget);
}


QDomElement Distribution::to_QDomElement(const unsigned int subset, 
        QDomDocument& doc)
{
    QDomElement node = doc.createElement("dunnart:node");

    if (subset & XMLSS_IOTHER)
    {
        newProp(node, x_type, x_distribution);

        int position = (type == GUIDE_TYPE_VERT) ?  y(): x();
        newProp(node, x_position, position);

        newProp(node, x_direction, (int) type);

        newProp(node, x_sepDistance, space);

        newProp(node, "id", idString());
    }

    if (subset & XMLSS_XMOVE)
    {
        newProp(node, x_xPos, x());
        newProp(node, x_yPos, y());
    }

    for (RelsList::iterator r = relationships.begin(); r != relationships.end(); ++r)
    {
        QDomElement rxn = (*r)->to_QDomElement(subset, doc);
        node.appendChild(rxn);
    }

    return node;
}


dgtype Distribution::is_guide(Guideline *g)
{
    Guideline *tmp1, *tmp2, *left, *right;

    tmp1 = relationships.front()->guide;
    tmp2 = relationships.front()->guide2;
 
    left = (guideLessThan(tmp1, tmp2)) ? tmp1 : tmp2;

    tmp1 = relationships.back()->guide;
    tmp2 = relationships.back()->guide2;
 
    right = (guideLessThan(tmp1, tmp2)) ? tmp2 : tmp1;

    if (g == left)
    {
        return DGT_LEFT;
    }
    else if (g == right)
    {
        return DGT_RIGHT;
    }
    return DGT_OTHER;
}


void Distribution::moveToDefaultPos(const bool store_undo)
{
    int diff_x = 0;
    int diff_y = 0;
    int nxpos = x();
    int nypos = y();
 
    if (type == GUIDE_TYPE_VERT)
    {
        nypos = DISTRO_DEFAULT_YPOS;
        diff_y = nypos - newpos;
    }
    else
    {
        nxpos = DISTRO_DEFAULT_XPOS;
        diff_x = nxpos - newpos;
    }

    if (diff_x || diff_y)
    {
        register_new_pos_diff(diff_x, diff_y);
        move_to(nxpos, nypos, store_undo);
    }
}


void Distribution::register_new_pos_diff(int dx, int dy)
{
    if (type == GUIDE_TYPE_VERT)
    {
        newpos = newpos + dy;
    }
    else // if (type == GUIDE_TYPE_HORI)
    {
        newpos = newpos + dx;
    }
}


Guideline *Distribution::leftGuide(void) const
{
    return (relationships.empty()) ? NULL : relationships.front()->guide;
}


Guideline *Distribution::rightGuide(void) const
{
    return (relationships.empty()) ? NULL : relationships.back()->guide2;
}


bool Distribution::isReversed(void) const
{
    if (relationships.size() == 0)
    {
        return false;
    }
    
    if (relationships.front()->guide->position() > relationships.front()->guide2->position())
    {
        return true;
    }
    return false;
}


void Distribution::deactivateAll(CanvasItemSet& selSet)
{
    Q_UNUSED (selSet)

    RemoveEntire();
}


void Distribution::RemoveGuideline(Guideline *g)
{
    if (relationships.size() < 2)
    {
        RemoveEntire();
        return;
    }
    // There are obviously at least four guides, and the user has
    // deleted one.  Let's regenerate the distribution.
    GuidelineList guides;
    
    RelsList::iterator r;
    for (r = relationships.begin(); r != relationships.end(); ++r)
    {
        // Add all guides from relationships to list except the deleted one.
        if (g != (*r)->guide)
        {
            guides.push_back((*r)->guide);
        }
        if (g != (*r)->guide2)
        {
            guides.push_back((*r)->guide2);
        }
    }
    // Order guides:
    guides.sort(guideCompare);
    // Remove duplicates
    guides.unique();

    for (r = relationships.begin(); r != relationships.end(); r++)
    {
        //UNDO add_undo_record(DELTA_DEL_REL, (*r), PARASITE_SIDE);
        
        (*r)->guide->relationships.removeOne(*r);
        (*r)->guide2->relationships.removeOne(*r);
    }

    Distribution *newdistro = new Distribution(&guides, x(), y());

    UndoMacro *macro = canvas()->currentUndoMacro();
    QUndoCommand *cmd = new CmdCanvasSceneAddItem(canvas(), newdistro);
    macro->addCommand(cmd);

    // Actually delete the indicator:
    cmd = new CmdCanvasSceneRemoveItem(canvas(), this);
    macro->addCommand(cmd);
}


void Distribution::RemoveEntire(void)
{
    RelsList::iterator r;
    for (r = relationships.begin(); r != relationships.end(); r++)
    {
        // Add delete of pos, to every distro-rel.
        
        // UNDO add_undo_record(DELTA_DEL_REL, (*r), PARASITE_SIDE);
        
        (*r)->guide->relationships.removeOne(*r);
        (*r)->guide2->relationships.removeOne(*r);
    }

    // Actually delete the indicator:
    UndoMacro *macro = canvas()->currentUndoMacro();
    QUndoCommand *cmd = new CmdCanvasSceneRemoveItem(canvas(), this);
    macro->addCommand(cmd);
}


void Distribution::shiftLockedGuideline(void)
{
    if (lockedGuidelineN == 0)
    {
        return;
    }
    Guideline *guide = lockedGuideline();
    guide->unlock();

    int guidesN = relationships.size() + 1;

    lockedGuidelineN = (lockedGuidelineN % guidesN) + 1;
    
    if (lockedGuidelineN == draggedGuidelineN)
    {
        lockedGuidelineN = (lockedGuidelineN % guidesN) + 1;
    }
    guide = lockedGuideline();
    guide->lock();
}


Guideline *Distribution::lockedGuideline(void) const
{
    return guidelineAtPosition(lockedGuidelineN);
}

Guideline *Distribution::draggedGuideline(void) const
{
    return guidelineAtPosition(draggedGuidelineN);
}

Guideline *Distribution::guidelineAtPosition(int position) const
{
    assert(!relationships.empty());
    
    if (position > relationships.size())
    {
        return relationships.back()->guide2;
    }
    
    int relN = 1;
    RelsList::const_iterator r = relationships.begin();
    while (relN < position)
    {
        relN++;
        ++r;
    }
    return (*r)->guide;
}


void Distribution::Resize(Guideline *guide, const double newx, const double newy,
        bool store_undo, double newogp, double newsepr, Guideline *newother)
{
    Q_UNUSED (newogp)

    Guideline *right = relationships.back()->guide2;
   
    Guideline *other = (newother) ? newother : lockedGuideline();

    double newpos;
    double oldpos = guide->position();
    double oval = other->position();

    if (type == GUIDE_TYPE_VERT)
    {
        newpos = newx;
    }
    else // if (type == GUIDE_TYPE_HORI)
    {
        newpos = newy;
    }
    if (newpos == oldpos)
    {
        // It's in the same place, nothing to do.
        return;
    }

    if (store_undo)
    {
        // UNDO add_undo_record(DELTA_DRESIZE, this, guide, other, (int) oval);
    }

    //double nval = (newogp != 0) ? newogp : oval;

    //layoutLockShape(other);
    Actions& actions = canvas()->getActions();
    actions.clear();
    actions.moveList.push_back(other);
    actions.moveList.push_back(this);

    int gapsBetween = abs(draggedGuidelineN - lockedGuidelineN);
    double val;
    if (guide == right)
    {
        val = (newpos - oval) / (double) gapsBetween;
    }
    else
    {
        val = (oval - newpos) / (double) gapsBetween;
    }
    double mval = (newsepr != 0) ? newsepr : val;
    space = mval;

    d_printf("\t\t\t[%4u] - Distribution::qSepr <- %.2f\n", m_internal_id, mval);

    // Make the layout engine notice the changes.
    canvas()->restart_graph_layout();
}


Distribution *createDistribution(QWidget *window, const dtype type,
        CanvasItemList& objList)
{
    Distribution *distro = NULL;
    Guideline *guide = NULL;
    atypes atype = (atypes) type;
    GuidelineList guideList;

    // Generate missing guides:
    for (CanvasItemList::iterator sh = objList.begin(); sh != objList.end();
            ++sh)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (*sh);
        Guideline *guidesh = dynamic_cast<Guideline *> (*sh);
        if (shape)
        {
            guide = shape->attachedGuidelineOfType(atype);
            if (!guide)
            {
                // No guideline, so create a new one:
                guide = shape->newGuidelineOfType(atype);
                // UNDO add_undo_record(DELTA_MOVE, shape);
                // UNDO add_undo_record(DELTA_ADD, guide);
                new Relationship(guide, shape, atype);
            }
            else
            {
                // UNDO if (recUndo) add_undo_record(DELTA_MOVE, guide);
            }
        }
        else if (guidesh)
        {
            if (guidesh->get_dir() == atypes_to_dirctn(atype))
            {
                guide = guidesh;
                // UNDO if (recUndo) add_undo_record(DELTA_MOVE, guide);
            }
            else
            {
                // Wrong type (direction) of guideline, ignore.
                guide = NULL;
            }
        }

        if (guide)
        {
            guideList.push_back(guide);
        }
    }

    // Order guides:
    guideList.sort(guideCompare);
    // Remove duplicates
    guideList.unique();

    bool must_undo = false;
    if (guideList.size() < 2)
    {
        if (window)
        {
            QString warning = QString(
                    QObject::tr("<p><b>Distribution constraints must be applied "
                    "to two or more selected shapes or guidelines.</b></p>"
                    "<p>The current selection contains only %1 such object."
                    "</p>")).arg(guideList.size());

            QMessageBox message(QMessageBox::Warning, "Invalid Action",
                                warning, QMessageBox::Ok, window);
            message.setWindowModality(Qt::WindowModal);
            message.exec();
        }
        must_undo = true;
    }
    else
    {
        distro = new Distribution(&guideList);
        guide->canvas()->addItem(distro);
        // UNDO if (recUndo) add_undo_record(DELTA_ADD, distro);
    }

    if (must_undo)
    {
        // UNDO undo_action(NULL);
        // UNDO clear_redo_stack();
    }

    return distro;
}


#if 0
QVariant Distribution::itemChange(QGraphicsItem::GraphicsItemChange change,
        const QVariant &value)
{
    if (change == QGraphicsItem::ItemScenePositionHasChanged)
    {
        qDebug("Transform Changed!");
        fflush(stdout);
        abort();
        buildPainterPath();
    }
    return Indicator::itemChange(change, value);
}
#endif




}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

