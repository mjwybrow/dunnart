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


#include <cfloat>

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QPainter>

#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/undo.h"

#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/cluster.h"
#include "libdunnartcanvas/utility.h"

namespace dunnart {


Guideline::Guideline(dirctn t, double position)
    : Indicator(ZORD_Guideline)
{
    type = t;

    setItemType(x_guideline);

    initialiser(position);
}


Guideline::Guideline(Canvas *canvas, const QDomElement& node, const QString& ns):
    Indicator(node, ns, ZORD_Guideline)
{
    Q_UNUSED (canvas)

    type = (dirctn) essentialProp<int>(node, x_direction, ns);
    double position = essentialProp<double>(node, x_position, ns);

    initialiser(position);
}



void Guideline::initialiser(double position)
{
    QGraphicsItem::setPos(position, position);
    setSize(10, 10);

    setHoverMessage("Guideline \"%1\" - Drag to move guideline and attached "
                    "objects. Hold ALT to drag free from separations or "
                    "distributions.");
}


void Guideline::resizedCanvas(void)
{
// Adjust the length of guidelines due to resizing of the canvas.
if (type == GUIDE_TYPE_VERT)
{
    //QT height = canvas->get_height() - 7;
}
else // if (type == GUIDE_TYPE_HORI)
{
    //QT width = canvas->get_width() - 7;
}
//QT set_noimages(width, height);
}


void Guideline::rangeOfAttachedShapes(double& min, double& max, int& nodes)
{
    min = DBL_MAX;
    max = -DBL_MAX;

    nodes = 0;

    for (RelsList::iterator curr = rels.begin(); curr != rels.end(); ++curr)
    {
        if ((*curr)->shape)
        {
            ++nodes;
            QPointF shPos = (*curr)->shape->centrePos();
            if (get_dir() == GUIDE_TYPE_HORI)
            {
                min = std::min(min, shPos.x());
                max = std::max(max, shPos.x());
            }
            else
            {
                min = std::min(min, shPos.y());
                max = std::max(max, shPos.y());
            }
        }
    }
}


void Guideline::rangeOfAttachedObjects(double& min, double& max) const
{
    min = DBL_MAX;
    max = -DBL_MAX;

    for (RelsList::const_iterator curr = rels.begin(); curr != rels.end(); ++curr)
    {
        QRectF itemRect;
        if ((*curr)->shape)
        {
            itemRect = (*curr)->shape->boundingRect();
            itemRect.moveCenter((*curr)->shape->pos());
        }
        else if ((*curr)->distro)
        {
            itemRect = (*curr)->distro->boundingRect();
            itemRect.moveCenter((*curr)->distro->pos());
        }
        else if ((*curr)->separation)
        {
            itemRect = (*curr)->separation->boundingRect();
            itemRect.moveCenter((*curr)->separation->pos());
        }

        if (itemRect.isValid())
        {
            if (get_dir() == GUIDE_TYPE_HORI)
            {
                min = std::min(min, itemRect.left());
                max = std::max(max, itemRect.right());
            }
            else
            {
                min = std::min(min, itemRect.top());
                max = std::max(max, itemRect.bottom());
            }
        }
    }

    // Cope with the case where there are no attached objects.
    if ((min == DBL_MAX) && (max = -DBL_MAX) && canvas())
    {
        QList<QGraphicsView *> views = canvas()->views();
        QRectF sceneBounds;

        // Determine bounds by taking the union of all view bounds.
        for (int v = 0; v < views.size(); ++v)
        {
            QGraphicsView *view = views.at(v);

            sceneBounds = sceneBounds.united(QRectF(view->mapToScene(0,0),
                    view->mapToScene(view->width(), view->height())));
        }

        if (get_dir() == GUIDE_TYPE_HORI)
        {
            min = sceneBounds.left();
            max = sceneBounds.right();
        }
        else
        {
            min = sceneBounds.top();
            max = sceneBounds.bottom();
        }
    }
}


QAction *Guideline::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event,
        QMenu& menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }

    QAction* breakDistrosAction =
            menu.addAction(tr("Break from Distributions"));

    // Hide the menu item if the guideline is not attached to any
    // distribution relationships.
    bool attachedToDistributions = false;
    for (RelsList::iterator r = rels.begin(); r != rels.end(); r++)
    {
        if (dynamic_cast<Distribution *> ((*r)->distro))
        {
            attachedToDistributions = true;
            break;
        }
    }
    if ( ! attachedToDistributions )
    {
        breakDistrosAction->setVisible(false);
    }

    QAction *action = CanvasItem::buildAndExecContextMenu(event, menu);

    if (action == breakDistrosAction)
    {
        canvas()->beginUndoMacro(tr("Remove From Distribution"));
        bool force = true;
        removeFromDistributions(force);
        canvas()->interrupt_graph_layout();
    }

    return action;
}

void Guideline::removeFromDistributions(const bool force)
{
    DistributionList dlist;

    // Don't delete using rels iterator, build a distrolist.
    for (RelsList::iterator r = rels.begin(); r != rels.end(); r++)
    {
        Distribution *distro = dynamic_cast<Distribution *> ((*r)->distro);

        if (distro && force)
        {
            dlist.push_back(distro);
        }
    }
    dlist.sort();
    dlist.unique();
    for (DistributionList::iterator d = dlist.begin(); d != dlist.end(); d++)
    {
        (*d)->RemoveGuideline(this);
    }
}


void Guideline::userMoveBy(qreal dx, qreal dy)
{
    // If ALT-dragging, disconnect shape from guidelines.
    bool freeshift = canvas()->isLayoutSuspended();
    if (freeshift)
    {
        removeFromDistributions(freeshift);
    }

    double pos = (type == GUIDE_TYPE_VERT) ? x() + dx : y() + dy;
    cmd_setPosition(pos);
}


void Guideline::move_to(int x, int y, bool store_undo, bool human)
{
    Q_UNUSED (x)
    Q_UNUSED (y)
    Q_UNUSED (store_undo)
    Q_UNUSED (human)
#if 0
    CanvasItem *cobj = this;
    int newval = 0;
    bool is_leader = false;
    int newx = 4 - cxoff, newy = 4 - cyoff;

    if (type == GUIDE_TYPE_VERT)
    {
        newval = x;
        newx = x;
    }
    else if (type == GUIDE_TYPE_HORI)
    {
        newval = y;
        newy = y;
    }
    fPos = newval;

    if ((newx == xpos) && (newy == ypos))
    {
        // Not moving, so do nothing.
        return;
    }

    bool freeshift = canvas()->isLayoutSuspended();
    if (freeshift)
    {
        d_printf("FREESHIFT\n");
    }
    
    if (human && (!is_leader || freeshift))
    {
        removeFromDistributions(freeshift);
        // Since the previous call will have suggested new values for
        // the guidelines involved in the distribution we need to resolve
        // here to avoid what looks like an unsovable state.
        
    }

    cobj->move_to(newx, newy, store_undo);
#endif
}


double Guideline::position(void) const
{
    return (type == GUIDE_TYPE_VERT) ? x() : y();
}


void Guideline::setPosition(const double position)
{
    if (canvas() && !canvas()->processingLayoutUpdates())
    {
        Actions& actions = canvas()->getActions();
        actions.moveList.push_back(this);
    }

    QPointF posPoint = pos();
    if (type == GUIDE_TYPE_VERT)
    {
        posPoint.setX(position);
    }
    else
    {
        posPoint.setY(position);
    }
    setPos(posPoint);
}


void Guideline::findAttachedSet(CanvasItemSet& objSet)
{
    if (objSet.find(this) != objSet.end())
    {
        // Already seen this node.
        return;
    }
    objSet.insert(this);

    RelsList::iterator finish = rels.end();
    for (RelsList::iterator r = rels.begin(); r != finish; r++)
    {
        if ((*r)->shape)
        {
            (*r)->shape->findAttachedSet(objSet);
        }
    }
}


void Guideline::addAttachedShapesToSet(CanvasItemSet& objSet)
{
    // Add pointers to all shapes attached to this guidline into the objSet.
    RelsList::iterator finish = rels.end();
    for (RelsList::iterator r = rels.begin(); r != finish; r++)
    {
        if ((*r)->shape)
        {
            objSet.insert((*r)->shape);
        }
    }
}


void Guideline::cascade_distance(int dist, unsigned int dir, CanvasItem **path)
{
    if (type == GUIDE_TYPE_VERT)
    {
        dir = CASCADE_HORI;
    }
    else
    {
        dir = CASCADE_VERT;
    }

    int nextval = 0;
    if (cascade_logic(nextval, dist, dir, path) == false)
    {
        return;
    }

    RelsList::iterator finish = rels.end();
    for (RelsList::iterator r = rels.begin(); r != finish; r++)
    {
        if ((*r)->shape)
        {
            (*r)->shape->cascade_distance(nextval, dir, path);
        }
        else if ((*r)->distro)
        {
            (*r)->distro->cascade_distance(nextval, dir, path);
        }
        else if ((*r)->separation)
        {
            (*r)->separation->cascade_distance(nextval, dir, path);
        }
    }
}


QPainterPath Guideline::shape() const
{
    QRectF area(-width() / 2, -height() / 2, width(), height());
    double onePixel = m_curr_path_one_pixel;
    if (type == GUIDE_TYPE_VERT)
    {
        area.setRight(+3 * onePixel);
        area.setLeft(-3 * onePixel);
    }
    else
    {
        area.setBottom(-3 * onePixel);
        area.setTop(3 * onePixel);
    }
    QPainterPath path;
    path.addRect(area);
    return path;
}


QPainterPath Guideline::buildPainterPath(void)
{
    QPainterPath painter_path;
    if (type == GUIDE_TYPE_VERT)
    {
        painter_path.moveTo(0, -height() / 2);
        painter_path.lineTo(0, height() / 2);
    }
    else
    {
        painter_path.moveTo(-width() / 2, 0);
        painter_path.lineTo(width() / 2, 0);
    }
    //painter_path.addRect(-width() / 2, -height() / 2, width(), height());
    return painter_path;
}


void Guideline::paint(QPainter *painter, 
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Indicator::paint(painter, option, widget);
}


QRectF Guideline::calculateBounds(QPointF pos) const
{
    QRectF bounds;
    // Whenever it is moved, set the guideline to stretch only to
    // any connected objects.
    double min, max;
    rangeOfAttachedObjects(min, max);
    min -= 20;
    max += 20;
    if (type == GUIDE_TYPE_VERT)
    {
        bounds.setHeight(max - min);
        bounds.setWidth(10);
        bounds.moveCenter(QPointF(pos.x(), min + bounds.height() / 2.0));
    }
    else // if (type == GUIDE_TYPE_HORI)
    {
        bounds.setWidth(max - min);
        bounds.setHeight(10);
        bounds.moveCenter(QPointF(min + (bounds.width() / 2.0), pos.y()));
    }
    return bounds;
}

void Guideline::updateFromLayout(const double posVal, const bool updatePos)
{
    QPointF position = pos();
    if (updatePos)
    {
        position = QPointF(posVal, posVal);
    }
    QRectF bounds = calculateBounds(position);
    if (updatePos || (pos() == bounds.center()))
    {
        setSize(bounds.width(), bounds.height());
    }
    setPos(bounds.center());
}


QVariant Guideline::itemChange(GraphicsItemChange change,
        const QVariant &value)
{
    if ((change == QGraphicsItem::ItemPositionChange) && scene() &&
            !scene()->views().empty())
    {
        QPointF newPos = value.toPointF();
        QRectF bounds = calculateBounds(newPos);
        setSize(bounds.width(), bounds.height());
        newPos = bounds.center();
        return newPos;
    }
    return Indicator::itemChange(change, value);
}


QDomElement Guideline::to_QDomElement(const unsigned int subset, 
        QDomDocument& doc)
{
    double position = (type == GUIDE_TYPE_VERT) ? x(): y();

    QDomElement node = doc.createElement("dunnart:node");

    if (subset & XMLSS_IOTHER)
    {
        newProp(node, x_direction, (int) type);

        newProp(node, x_type, x_guideline);

        newProp(node, "id", idString());
    }
    
    if (subset & XMLSS_IMOVE)
    {
        newProp(node, x_position, position);
    }

    for (RelsList::iterator r = rels.begin(); r != rels.end(); ++r)
    {
        if ((*r)->shape)
        {
            QDomElement rxn = (*r)->to_QDomElement(subset, doc);
            node.appendChild(rxn);
        }
    }

    return node;
}


void Guideline::deactivateAll(CanvasItemSet& selSet)
{
    Q_UNUSED (selSet)

    RelsList::iterator r;
    while ( (r = rels.begin()) != rels.end() )
    {
        (*r)->deadguide = this;
        (*r)->Deactivate(EVERYTHING);
    }
}




dirctn atypes_to_dirctn(atypes atype)
{
    if ((atype == ALIGN_LEFT) || (atype == ALIGN_CENTER) ||
            (atype == ALIGN_RIGHT))
    {
        return GUIDE_TYPE_VERT;
    }
    else
    {
        return GUIDE_TYPE_HORI;
    }
}


bool Guideline::operator<(const Guideline& rhs) const
{
    assert(type == rhs.type);
    if (type == GUIDE_TYPE_VERT)
    {
        // It's vertical
        return (x() < rhs.x());
    }
    // It horizontal
    return (y() < rhs.y());
}


bool Guideline::operator==(const Guideline& rhs) const
{
    assert(type == rhs.type);
    if (type == GUIDE_TYPE_VERT)
    {
        // It's vertical
        return (x() == rhs.x());
    }
    // It horizontal
    return (y() == rhs.y());
}


bool guideCompare(Guideline *g1, Guideline *g2)
{
    assert(g1->get_dir() == g2->get_dir());
    return *g1 < *g2;
}



Guideline *createAlignment(const atypes atype, CanvasItemList& objList)
{
    bool first_shape = true;
    Guideline *guide = NULL;

    for (CanvasItemList::iterator sh = objList.begin(); sh != objList.end();
            sh++)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (*sh);
        Guideline *guidesh = dynamic_cast<Guideline *> (*sh);
        if (shape && shape->canBe(C_ALIGNED))
        {
            if ((guide = shape->get_guide(atype)))
            {
                // found a guide of this type.
                break;
            }
        }
        else if (guidesh)
        {
            // Don't use horizontal guides for vertical alignment.
            if (guidesh->get_dir() == atypes_to_dirctn(atype))
            {
                guide = guidesh;
            }
            break;
        }
    }
    for (CanvasItemList::iterator sh = objList.begin(); sh != objList.end();
            ++sh)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (*sh);
        Guideline *guidesh = dynamic_cast<Guideline *> (*sh);
        if (shape && shape->canBe(C_ALIGNED))
        {
            if (first_shape)
            {
                if (!guide)
                {
                    guide = shape->new_guide(atype);
                    // UNDO add_undo_record(DELTA_ADD, guide);
                }
                else
                {
                    // Using a guide attached to a non-lead object,
                    // need to move it.

                    d_printf("\t\tBring guideline to lead shape:\n");
                    
                    double gx = 0, gy = 0;
                    double value = shape->attachedGuidelinePosition(atype);
                    if (atypes_to_dirctn(atype) == GUIDE_TYPE_VERT)
                    {
                        gx = value;
                    }
                    else
                    {
                        gy = value;
                    }
                    guide->move_to(gx, gy, true, false);
                }
                first_shape = false;
            }
            Guideline *guide2 = shape->get_guide(atype);

            if (guide2 == NULL)
            {
                // UNDO if (recUndo) add_undo_record(DELTA_MOVE, shape);
                new Relationship(guide, shape, atype);
            }
            else if (guide2 != guide)
            {
                // UNDO if (recUndo) add_undo_record(DELTA_MOVE, guide2);
                new Relationship(guide, guide2);
            }
            else // if (guide2 == guide)
            {
                // This is already attached, so do nothing.
            }
        }
        else if (guidesh && (guidesh->get_dir() == atypes_to_dirctn(atype)))
        {
            if (first_shape)
            {
                // If this is the lead, do nothing to it.
                first_shape = false;
            }
            else if (guidesh != guide)
            {
                // UNDO if (recUndo) add_undo_record(DELTA_MOVE, guidesh);
                new Relationship(guide, guidesh);
            }
        }
    }
    if (!guide)
    {
        fprintf(stderr, "WARNING: No alignment created in createAlignment().\n");
    }

    return guide;
}




}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

