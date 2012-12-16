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

#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H

namespace dunnart {


enum atypes { ALIGN_TOP = 0, ALIGN_MIDDLE = 1, ALIGN_BOTTOM = 2,
        ALIGN_LEFT = 3, ALIGN_CENTER = 4, ALIGN_RIGHT = 5 };

enum side { BOTH_SIDE = 3, EVERYTHING = 7};

class ShapeObj;
class Guideline;
class Distribution;
class Separation;

class Relationship
{
    public:
        int relType;
        ShapeObj *shape;
        Guideline *guide, *guide2, *deadguide;
        Distribution *distro;
        Separation *separation;
        atypes type;

        Relationship(Guideline *g, ShapeObj *sh, atypes t, bool no_undo = false);
        Relationship(Distribution *d, Guideline *g, Guideline *g2, bool no_undo = false);
        Relationship(Guideline *g, Guideline *g2, bool no_undo = false);
        Relationship(Separation *s, Guideline *g, Guideline *g2, bool no_undo = false);
        Relationship(Canvas *canvas, const QDomElement& node, 
                const QString& dunNs, bool no_undo = false);
        QDomElement to_QDomElement(const unsigned int subset, 
                QDomDocument& doc);
        void Activate(side s, bool by_undo = false);
        void Deactivate(side s, bool by_undo = false);
   private:
        void commonInit(void);

};


static const int REL_Align = 1;
static const int REL_Distr = 2;
static const int REL_Unify = 3;
static const int REL_Separ = 4;


}
#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

