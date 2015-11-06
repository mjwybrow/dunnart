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
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QMessageBox>

#include <algorithm>
#include <climits>

#include "libavoid/libavoid.h"

#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/handle.h"

#include "libdunnartcanvas/separation.h"

namespace dunnart {


static bool relationshipLessThan(Relationship *r1, Relationship *r2);


class SeparationHandle : public Handle {
    public:
        SeparationHandle(Separation *separ, int hflags, int index) :
            Handle(separ, hflags, index)
        {
            if (hflags & (HAN_LEFT | HAN_RIGHT))
            {
                setCursor(Qt::SizeHorCursor);
            }
            else
            {
                setCursor(Qt::SizeVerCursor);
            }
            setHoverMessage("Separation Handle - Click and drag to alter the "
                            "minimum separation distance.");
        }
    protected:
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event)
        {
            Q_UNUSED (event)

            Separation *separation = dynamic_cast<Separation *> (parentItem());
            dragStartPosition =   this->scenePos(); // Initial handle position.
            dragStartSeparation = separation->gap;
        }
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
        {
            Separation *separation = dynamic_cast<Separation *> (parentItem());
            QPointF newPos = event->scenePos() - dragStartPosition;

            separation->Resize(separation->left_guide(),  dragStartSeparation,
                       newPos, false);

            // The handle is moved through resizing of the distribution,
            // so no need to call Handle::mouseMoveEvent(event);
        }
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event)
        {
            Q_UNUSED (event)

            if (canvas())
            {
                canvas()->pushStatusMessage("Separation Handle - Click "
                    "and drag to alter the minimum separation value.");
            }
        }
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
        {
            Q_UNUSED (event)

            if (canvas())
            {
                canvas()->popStatusMessage();
            }
        }
        QPointF dragStartPosition;
        double  dragStartSeparation;
};


Separation::Separation(GuidelineList *guides, double gap, double xp, double yp)
        : Indicator(ZORD_Distribution),
          gap(gap)
{
    setPos(xp, yp);

    setItemType(x_separation);

    // Setup separation representation.
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
    recreate();
    initialiser();

    // There is a chance we have just recorded a move event for a
    // guideline that is about to be deleted (if multiple guidelines from
    // the distribution were selected and deleted).  Thus, we clear the
    // list of actions.
    // XXX: This is all a bit dodgy.
    // QT Actions& actions = getActions();
    // QT actions.clear();
}


Separation::Separation(const QDomElement& node, const QString& ns):
        Indicator(node, ns, ZORD_Distribution)
{
    int typeVal = essentialProp<int>(node, x_direction, ns);
    if ((typeVal != GUIDE_TYPE_VERT) && (typeVal != GUIDE_TYPE_HORI))
    {
        // An old buggy version of Inkscape saved incorrect types for 
        // separations, so correct it here.
        type = (typeVal == 0) ? GUIDE_TYPE_VERT : GUIDE_TYPE_HORI;
    }
    else
    {
        type = (dirctn) typeVal;
    }

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
    setPos(xpos, ypos);

    optionalProp(node, x_sepDistance, gap, ns);

    recreate();
    initialiser();
}


void Separation::initialiser(void)
{
    m_indicator_pen.setColor(QColor(0, 0, 255, 70));

    if (type == GUIDE_TYPE_VERT)
    {
        handle_x = 3;
        handle_y = 0;
        handles.push_back(new SeparationHandle(
                this, HAN_HANDLE | HAN_LEFT | HAN_SEPARA, 0));
    }
    else // if (type == GUIDE_TYPE_HORI)
    {
        handle_x = 0;
        handle_y = 3;
        handles.push_back(new SeparationHandle(
                this, HAN_HANDLE | HAN_TOP | HAN_SEPARA, 0));
    }
    handles.front()->setVisible(false);

    // Force redraw after constraint solving to make sure object
    // is displayed properly:
    update();

    setHoverMessage("Separation Indicator \"%1\" - Drag to move separation and "
                    "attached objects.");
}


void Separation::cascade_distance(int dist, unsigned int dir, CanvasItem **path)
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


static bool relationshipLessThan(Relationship *r1, Relationship *r2)
{
    if (r1->guide->get_dir() == GUIDE_TYPE_VERT)
    {
        return r1->guide->x() < r2->guide->x();
    }
    return r1->guide->y() < r2->guide->y();
}


bool Separation::guideLessThan(Guideline *g1, Guideline *g2)
{
    if (g1->get_dir() == GUIDE_TYPE_VERT)
    {
        return g1->x() <= g2->x();
    }
    return g1->y() <= g2->y();
}


double Separation::getSeparation(void)
{
    return gap;
}


void Separation::recreate(void)
{
    RelsList trels = relationships;
    qSort(trels.begin(), trels.end(), relationshipLessThan);

    double bx, by, bw, bh, first = 0, last = 0, size = 12;
    
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

    space = (last - first) / trels.size();
}


void Separation::updateFromLayout(double newsep)
{
    double s = newsep;

    setAlpha(255);
    setGlow();

    gap = s;
    // Recreate the object:
    recreate();
    for (int i = 0; i < handles.size(); ++i)
    {
        handles[i]->reposition();
    }
}

void Separation::userMoveBy(qreal dx, qreal dy)
{
    // Move tha attached guidelines
    Actions& actions = canvas()->getActions();
    std::set<Guideline *> guides;
    for (RelsList::iterator g = relationships.begin(); g != relationships.end(); ++g)
    {
        guides.insert((*g)->guide);
        guides.insert((*g)->guide2);
    }
    for (std::set<Guideline *>::iterator g = guides.begin();
            g != guides.end(); ++g)
    {
        if (!(*g)->isSelected())
        {
            // Move the guideline, if it is unselected.
            (*g)->QGraphicsItem::moveBy(dx, dy);
            actions.moveList.push_back(*g);
        }
    }
    moveBy(dx, dy);
}


QPainterPath Separation::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

QRectF Separation::boundingRect() const
{
    return QRectF(-width() / 2, -height() / 2, width(), height());
}



QPainterPath Separation::buildPainterPath(void)
{
    QPainterPath painter_path;

    // PainterPath of bounding rectange.
    //painter_path.addRect(-width() / 2, -height() / 2, width(), height());

    int sections = (int) relationships.size();

    const qreal one =  m_curr_path_one_pixel;
    const qreal gap = one * 2;
    double bridgeMin = DBL_MAX;
    double bridgeMax = -DBL_MAX;
    double bridgePos = -5 * one;
    double markerPos = 0;
    double padding = 4 * one;

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
            
#if 0
            if (sections > 1)
            {
                double position = pos1 + halfSep - (padding / 2);
                bridgeMin = qMin(bridgeMin, position);
                bridgeMax = qMax(bridgeMax, position);
                painter_path.moveTo(position, bridgePos);
                painter_path.lineTo(position, markerPos);
            }

#endif
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
          
#if 0
            if (sections > 1)
            {
                double position = pos1 + halfSep - (padding / 2);
                bridgeMin = qMin(bridgeMin, position);
                bridgeMax = qMax(bridgeMax, position);
                painter_path.moveTo(bridgePos, position);
                painter_path.lineTo(markerPos, position);
            }
#endif
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


void Separation::paint(QPainter *painter, 
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Indicator::paint(painter, option, widget);


    QPainterPath painter_path;

    // PainterPath of bounding rectange.
    //painter_path.addRect(-width() / 2, -height() / 2, width(), height());

    const qreal one =  m_curr_path_one_pixel;
    const qreal two = one * 2;
    double markerPos = 0;
    double padding = 4 * one;

    if (type == GUIDE_TYPE_VERT)
    {
        RelsList::iterator finish = relationships.end();
        for (RelsList::iterator r = relationships.begin(); r != finish; ++r)
        {
            double gp1 = (*r)->guide->x();
            double gp2 = (*r)->guide2->x();
            double pos1 = qMin(gp1, gp2) - x() + padding;
            double pos2 = qMax(gp1, gp2) - x() - padding;

            double pos = pos1 + (pos2 - pos1) / 2;
            double half = gap / 2 - padding;

            if (r == relationships.begin())
            {
                handle_x = pos - half;
                handles.front()->reposition();
            }

            painter_path.moveTo(pos - half, markerPos);
            painter_path.lineTo(pos + half, markerPos);

            painter_path.moveTo(pos - half, markerPos - two);
            painter_path.lineTo(pos - half, markerPos + two);

            painter_path.moveTo(pos + half, markerPos - two);
            painter_path.lineTo(pos + half, markerPos + two);
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

            double pos = pos1 + (pos2 - pos1) / 2;
            double half = gap / 2 - padding;

            if (r == relationships.begin())
            {
                handle_y = pos - half;
                handles.front()->reposition();
            }

            painter_path.moveTo(markerPos, pos - half);
            painter_path.lineTo(markerPos, pos + half);

            painter_path.moveTo(markerPos - two, pos - half);
            painter_path.lineTo(markerPos + two, pos - half);

            painter_path.moveTo(markerPos - two, pos + half);
            painter_path.lineTo(markerPos + two, pos + half);
        }
    }
    painter->setPen(m_indicator_pen);
    painter->drawPath(painter_path);
}


QDomElement Separation::to_QDomElement(const unsigned int subset, 
        QDomDocument& doc)
{
    QDomElement node = doc.createElement("dunnart:node");
   
    if (subset & XMLSS_IOTHER)
    {
        newProp(node, x_type, x_separation);

        double position = (type == GUIDE_TYPE_VERT) ? y(): x();
        
        newProp(node, x_position, position);
        newProp(node, x_direction, (int) type);
        newProp(node, x_sepDistance, gap);

        newProp(node, "id", idString());
    }

    for (RelsList::iterator r = relationships.begin(); r != relationships.end(); ++r)
    {
        QDomElement rxn = (*r)->to_QDomElement(subset, doc);
        node.appendChild(rxn);
    }

    return node;
}


sgtype Separation::is_guide(Guideline *g)
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
        return SGT_LEFT;
    }
    else if (g == right)
    {
        return SGT_RIGHT;
    }
    return SGT_OTHER;
}


void Separation::moveToDefaultPos(const bool store_undo)
{
    int diff_x = 0;
    int diff_y = 0;
    int nxpos = x();
    int nypos = y();
 
    if (type == GUIDE_TYPE_VERT)
    {
        nypos = SEPARATION_DEFAULT_YPOS;
        diff_y = nypos - newpos;
    }
    else
    {
        nxpos = SEPARATION_DEFAULT_XPOS;
        diff_x = nxpos - newpos;
    }

    if (diff_x || diff_y)
    {
        register_new_pos_diff(diff_x, diff_y);
        move_to(nxpos, nypos, store_undo);
    }
}


void Separation::register_new_pos_diff(int dx, int dy)
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


Guideline *Separation::left_guide(void)
{
    if (relationships.size() < 1)
    {
        return NULL;
    }
    Guideline *l = (is_reversed()) ? relationships.back()->guide2 : relationships.front()->guide;
         
    return l;
}


Guideline *Separation::right_guide(void)
{
    if (relationships.size() < 1)
    {
        return NULL;
    }
    Guideline *r = (is_reversed()) ? relationships.front()->guide : relationships.back()->guide2;
         
    return r;
}


bool Separation::is_reversed(void)
{
    Guideline *tmp1, *tmp2;

    if (relationships.size() == 0)
    {
        return false;
    }
    
    tmp1 = relationships.front()->guide;
    tmp2 = relationships.front()->guide2;
         
    if (guideLessThan(tmp1, tmp2))
    {
        return false;
    }
    return true;
}


void Separation::deactivateAll(CanvasItemSet& selSet)
{
    Q_UNUSED (selSet)

    RemoveEntire();
}

void Separation::RemoveGuideline(Guideline *g)
{
    if (relationships.size() < 2)
    {
        RemoveEntire();
        return;
    }
    // There are obviously at least four guides, and the user has
    // deleted one.  Let's regenerate the separation.
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
        // UNDO add_undo_record(DELTA_DEL_REL, (*r), PARASITE_SIDE);
        
        (*r)->guide->relationships.removeOne(*r);
        (*r)->guide2->relationships.removeOne(*r);
    }

    Separation *newdistro = new Separation(&guides, gap, x(), y());

    UndoMacro *macro = canvas()->currentUndoMacro();
    QUndoCommand *cmd = new CmdCanvasSceneAddItem(canvas(), newdistro);
    macro->addCommand(cmd);

    // Actually delete the indicator:
    cmd = new CmdCanvasSceneRemoveItem(canvas(), this);
    macro->addCommand(cmd);
}


void Separation::RemoveEntire(void)
{
    RelsList::iterator r;
    for (r = relationships.begin(); r != relationships.end(); r++)
    {
        // Add delete of pos, to every separation-rel.
        
        // UNDO add_undo_record(DELTA_DEL_REL, (*r), PARASITE_SIDE);
        
        (*r)->guide->relationships.removeOne(*r);
        (*r)->guide2->relationships.removeOne(*r);
     }

    // Actually delete the indicator:
    UndoMacro *macro = canvas()->currentUndoMacro();
    QUndoCommand *cmd = new CmdCanvasSceneRemoveItem(canvas(), this);
    macro->addCommand(cmd);
}


void Separation::shiftLockedGuideline(void)
{
    Guideline *guide = lockedGuideline();
    guide->unlock();

    guide = lockedGuideline();
    guide->lock();
}


Guideline *Separation::lockedGuideline(void)
{
    int relsN = relationships.size();
    if (relsN < 1)
    {
        return NULL;
    }
    
    RelsList::iterator r = relationships.begin();
    return (*r)->guide;
}


void Separation::Resize(Guideline *guide, double origSep, const QPointF posDiff,
        bool store_undo)
{
    Q_UNUSED (guide)

    double diffpos = (type == GUIDE_TYPE_VERT) ? posDiff.x() : posDiff.y();
    //double oval = other->getPos();

    if (store_undo)
    {
        //add_undo_record(DELTA_DRESIZE, this, guide, other, (int) oval);
    }

    gap = qMax(origSep - (diffpos * 2), 0.);

    canvas()->getActions().moveList.push_back(this);

    // Make the layout engine notice the changes.
    canvas()->restart_graph_layout();
}


QPair<Separation *, QString> createSeparation(const dtype type,
        CanvasItemList& objList, double separationDist, bool sortGuidelines)
{
    Separation *separation = NULL;
    Guideline *guide = NULL;
    Canvas *canvas_copy = NULL;
    atypes atype = type==SEP_VERTICAL?ALIGN_MIDDLE:ALIGN_CENTER;
    
    GuidelineList guideList;

    // Generate missing guides:
    for (CanvasItemList::iterator sh = objList.begin(); sh != objList.end();
            ++sh)
    {
        guide = NULL;
        ShapeObj *shape = dynamic_cast<ShapeObj *> (*sh);
        Guideline *guidesh = dynamic_cast<Guideline *> (*sh);
        if (shape)
        {
            canvas_copy = shape->canvas();
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
            canvas_copy = guidesh->canvas();
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

    if (sortGuidelines)
    {
        // Order guides:
        guideList.sort(guideCompare);
    }
    // Remove duplicates
    guideList.unique();

    QString errorMessage;

    bool must_undo = false;
    if (guideList.size() < 2)
    {
        errorMessage = QString(
                "<p><b>Separation constraints must be applied "
                "to two or more selected shapes or guidelines.</b></p>"
                "<p>The current selection contains only %1 such object."
                "</p>").arg(guideList.size());

        must_undo = true;
    }
    else
    {
        separation = new Separation(&guideList, separationDist);
        canvas_copy->addItem(separation);
        // UNDO add_undo_record(DELTA_ADD, separation);
    }

    if (must_undo)
    {
        // UNDO undo_action(NULL);
        // UNDO clear_redo_stack();
    }

    return QPair<Separation *, QString>(separation, errorMessage);
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

