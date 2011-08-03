/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2010-2011  Monash University
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
 * Author(s): Sarah Boyd  <Sarah.Boyd@monash.edu>
*/


#ifndef PDPHENOTYPEPROCESSNODE_H
#define PDPHENOTYPEPROCESSNODE_H

#include "pdprocessnode.h"

using namespace dunnart;

// This process node is completely different to the other process nodes.  It currently only accepts horizontal alignment.
// It uses Q_OBJECT for testing in the drawing function ...
// This glyph can have a simple clone marker without a label.
//  Cannot be multimeric.
class PhenotypeProcessNode: public ProcessNode
{
public:
    PhenotypeProcessNode(QString l, bool cb);
    QPainterPath buildPainterPath();
    QPainterPath singleShape(double width, double height, int indent) const;
    QPainterPath clone_marker() const;
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

protected:
    int orientation;
    bool cloned;
};

#endif // PDPHENOTYPEPROCESSNODE_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
