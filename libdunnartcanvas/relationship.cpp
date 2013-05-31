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
            activate(no_undo);
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
        
        activate(no_undo);
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
        
        activate(no_undo);
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
    
    activate(no_undo);
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
    
    activate(no_undo);
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
    
    activate(no_undo);
}


void Relationship::commonInit(void)
{
    shape = NULL;
    distro = NULL;
    guide = NULL;
    guide2 = NULL;
    separation = NULL;
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

void Relationship::removeGuideline(Guideline *deadguide)
{
    // If a guideline is deleted, delete the distribution.
    if (relType == REL_Distr)
    {
        distro->RemoveGuideline(deadguide);
    }
    else if (relType == REL_Separ)
    {
        separation->RemoveGuideline(deadguide);
    }
    else
    {
        deactivate();
    }
}

void Relationship::deactivate(bool by_undo)
{
    if (!by_undo)
    {
        // UNDO bool care_order = true;
        // UNDO add_undo_record(DELTA_DEL_REL, this, s, care_order);
    }

    if (relType == REL_Align)
    {
        shape->rels[type] = NULL;
        guide->relationships.removeOne(this);
    }
    else if (relType == REL_Distr)
    {
        distro->relationships.removeOne(this);
        guide->relationships.removeOne(this);
        guide2->relationships.removeOne(this);
    }
    else if (relType == REL_Separ)
    {
        separation->relationships.removeOne(this);
        guide->relationships.removeOne(this);
        guide2->relationships.removeOne(this);
    }
}


void Relationship::activate(bool by_undo)
{
    if (relType == REL_Align)
    {
        shape->rels[type] = this;
        guide->relationships.push_back(this);
    }
    else if (relType == REL_Distr)
    {
        distro->relationships.push_back(this);
        guide->relationships.push_back(this);
        guide2->relationships.push_back(this);
    }
    else if (relType == REL_Separ)
    {
        separation->relationships.push_back(this);
        guide->relationships.push_back(this);
        guide2->relationships.push_back(this);
    }

    if (!by_undo)
    {
        // UNDO add_undo_record(DELTA_ADD_REL, this, s);
    }
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

