/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2008-2009  Monash University
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
#include <cassert>

#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/cluster.h"
#include "libdunnartcanvas/relationship.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/canvas.h"

namespace dunnart {


Relationship::Relationship(Canvas *canvas, const QDomElement& node, 
        const QString& dunNs, bool no_undo)
    : shape(NULL),
      guide(NULL),
      guide2(NULL),
      distro(NULL),
      separation(NULL)
{
    Q_UNUSED (dunNs)

    QString relTypeStr = node.attribute(x_relType);
    assert(!relTypeStr.isNull());

    if (relTypeStr == x_alignment)
    {
        relType = REL_Align;
    
        type = (atypes) essentialProp<int>(node, x_alignmentPos);

        guide = dynamic_cast<Guideline *> (canvas->getItemByID(
                    essentialProp<QString>(node, x_constraintID)));
        shape = dynamic_cast<ShapeObj *> (canvas->getItemByID(
                    essentialProp<QString>(node, x_objOneID)));
        assert(shape);
        assert(guide);

        if (atypes_to_dirctn(type) != guide->get_dir())
        {
            qWarning("Constraint relationship with wrong "
                    "direction, adjusting...");
            type = (atypes) (((int)type + 3) % 6);
        }
   
        if (dynamic_cast<Cluster *> (shape))
        {
            qWarning("Ignoring cluster %d attached to guideline %d.",
                    shape->internalId(), guide->internalId());
        }
        else
        {
            Activate(BOTH_SIDE, no_undo);
        }
    }
    else if (relTypeStr == x_distribution)
    {
        relType = REL_Distr;
        
        distro = dynamic_cast<Distribution *> (canvas->getItemByID(
                    essentialProp<QString>(node, x_constraintID)));
        guide = dynamic_cast<Guideline *> (canvas->getItemByID(
                    essentialProp<QString>(node, x_objOneID)));
        guide2 = dynamic_cast<Guideline *> (canvas->getItemByID(
                    essentialProp<QString>(node, x_objTwoID)));
        assert(distro);
        assert(guide);
        assert(guide2);
        
        Activate(BOTH_SIDE, no_undo);
    }
    else if (relTypeStr == x_separation)
    {
        relType = REL_Separ;
        
        separation = dynamic_cast<Separation *> (canvas->getItemByID(
                    essentialProp<QString>(node, x_constraintID)));
        guide = dynamic_cast<Guideline *> (canvas->getItemByID(
                    essentialProp<QString>(node, x_objOneID)));
        guide2 = dynamic_cast<Guideline *> (canvas->getItemByID(
                    essentialProp<QString>(node, x_objTwoID)));
        assert(separation);
        assert(guide);
        assert(guide2);
        
        Activate(BOTH_SIDE, no_undo);
    }
}


    // We're using multi-way constraints to simulate the behaviour of
    // one-way constraints.
    //
Relationship::Relationship(Guideline *g, ShapeObj *sh, atypes t, bool no_undo)
{
    commonInit();

    relType = REL_Align;
    shape = sh;
    type = t;
    guide = g;
    
    Activate(BOTH_SIDE, no_undo);
}


Relationship::Relationship(Distribution *d, Guideline *g1, Guideline *g2, bool no_undo)
{
    commonInit();

    relType = REL_Distr;
    assert(g1);
    assert(g2);
    distro = d;
    guide = g1;
    guide2 = g2;
    
    Activate(BOTH_SIDE, no_undo);
}


Relationship::Relationship(Separation *s, Guideline *g1, Guideline *g2, bool no_undo)
{
    commonInit();

    relType = REL_Separ;
    assert(g1);
    assert(g2);
    separation = s;
    guide = g1;
    guide2 = g2;
    
    Activate(BOTH_SIDE, no_undo);
}

    // This one is for unifying guidelines.
Relationship::Relationship(Guideline *g1, Guideline *g2, bool no_undo)
{
    commonInit();

    relType = REL_Unify;
    assert(g1);
    assert(g2);
    guide = g1;
    guide2 = g2;
    
    Activate(BOTH_SIDE, no_undo);
}

void Relationship::commonInit(void)
{
    shape = NULL;
    distro = NULL;
    guide = NULL;
    guide2 = NULL;
    deadguide = NULL;
}

QDomElement Relationship::to_QDomElement(const unsigned int subset, 
        QDomDocument& doc)
{
    QDomElement node = doc.createElement("dunnart:node");
    
    if (subset & XMLSS_IOTHER)
    {
        newProp(node, x_type, x_constraint);

        if (distro)
        {
            newProp(node, x_relType, x_distribution);
            
            newProp(node, x_constraintID, distro->idString());
            newProp(node, x_objOneID, guide->idString());
            newProp(node, x_objTwoID, guide2->idString());
        }
        else if (separation)
        {
            newProp(node, x_relType, x_separation);

            newProp(node, x_constraintID, separation->idString());
            newProp(node, x_objOneID, guide->idString());
            newProp(node, x_objTwoID, guide2->idString());
        }
        else
        {
            newProp(node, x_relType, x_alignment);

            newProp(node, x_constraintID, guide->idString());
            newProp(node, x_objOneID, shape->idString());
            
            newProp(node, x_alignmentPos, (int) type);
        }
    }

    return node;
}


void Relationship::Deactivate(side s, bool by_undo)
{
    RelsList *rlist = NULL;
    RelsList::iterator rlistItem;
            
    // If a guideline is deleted, delete the distribution.
    if (s == EVERYTHING)
    {
        if (relType == REL_Distr)
        {
            if (!deadguide)
            {
                distro->RemoveEntire();
                return;
            }
            else
            {
                distro->RemoveGuideline(deadguide);
                return;
            }
        }
        else if (relType == REL_Separ)
        {
            if (!deadguide)
            {
                separation->RemoveEntire();
                return;
            }
            else
            {
                separation->RemoveGuideline(deadguide);
                return;
            }
        }
    }

    if (!by_undo)
    {
        // UNDO bool care_order = true;
        // UNDO add_undo_record(DELTA_DEL_REL, this, s, care_order);
    }
    if (s & PARASITE_SIDE)
    {
        if (relType == REL_Align)
        {
            shape->rels[type] = NULL;
        }
        else if (relType == REL_Distr)
        {
            // Must use a pointer so we modify the original list.
            rlist =  &(guide->rels);
            rlistItem = find(rlist->begin(), rlist->end(), this);
            assert(rlistItem != rlist->end());
            rlist->erase(rlistItem);

            rlist =  &(guide2->rels);
            rlistItem = find(rlist->begin(), rlist->end(), this);
            assert(rlistItem != rlist->end());
            rlist->erase(rlistItem);
        }
        else if (relType == REL_Separ)
        {
            // Must use a pointer so we modify the original list.
            rlist =  &(guide->rels);
            rlistItem = find(rlist->begin(), rlist->end(), this);
            assert(rlistItem != rlist->end());
            rlist->erase(rlistItem);

            rlist =  &(guide2->rels);
            rlistItem = find(rlist->begin(), rlist->end(), this);
            assert(rlistItem != rlist->end());
            rlist->erase(rlistItem);
        }
        else if (relType == REL_Unify)
        {
            assert(s & BOTH_SIDE);
            rlist =  &(guide2->rels);
            rlistItem = find(rlist->begin(), rlist->end(), this);
            assert(rlistItem != rlist->end());
            rlist->erase(rlistItem);
        }
    }
    if (s & HOST_SIDE)
    { 
        // Must use a pointer so we modify the original list.
        if (relType == REL_Distr)
        {
            rlist = &(distro->rels);
        }
        else if (relType == REL_Separ)
        {
            rlist = &(separation->rels);
        }
        else
        {
            rlist = &(guide->rels);
        }
        rlistItem = find(rlist->begin(), rlist->end(), this);
        assert(rlistItem != rlist->end());
        rlist->erase(rlistItem);
    }
}


void Relationship::Activate(side s, bool by_undo)
{
    if (s & PARASITE_SIDE)
    {
        if (relType == REL_Align)
        {
            shape->rels[type] = this;
        }
        else if (relType == REL_Distr)
        {
            guide->rels.push_back(this);
            guide2->rels.push_back(this);
        }
        else if (relType == REL_Separ)
        {
            guide->rels.push_back(this);
            guide2->rels.push_back(this);
        }
        else if (relType == REL_Unify)
        {
            assert(s & BOTH_SIDE);
            guide->rels.push_back(this);
        }
    }
    if (s & HOST_SIDE)
    {
        if (relType == REL_Align)
        {
            guide->rels.push_back(this);
        }
        else if (relType == REL_Distr)
        {
            distro->rels.push_back(this);
        }
        else if (relType == REL_Separ)
        {
            separation->rels.push_back(this);
        }
        else if (relType == REL_Unify)
        {
            assert(s & BOTH_SIDE);
            guide2->rels.push_back(this);
        }
    }
    if (!by_undo)
    {
        // UNDO add_undo_record(DELTA_ADD_REL, this, s);
    }

    // This little check prevents them from being unhidden on creation
    // or if they don't exist.
#if 0
    // QT
    for (int i = 0; i < 2; i++)
    {
        if (indi[i] && indi[i]->is_hidden())
        {
            if (indi[i]->get_parent()->get_active_image_n() !=
                    SHAPE_STATE_UNSELECTED)
            {
                indi[i]->set_active_image(1);
            }
            indi[i]->setVisible(true);
        }
    }
#endif
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

