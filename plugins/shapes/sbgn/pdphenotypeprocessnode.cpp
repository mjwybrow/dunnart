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


#include "pdphenotypeprocessnode.h"

using namespace dunnart;

PhenotypeProcessNode::PhenotypeProcessNode(QString l, bool cb) : ProcessNode(Qt::Horizontal, PHENOTYPE), cloned(cb)
{
    ShapeObj::setLabel(l);
}

QPainterPath PhenotypeProcessNode::buildPainterPath(void)
{
    QPainterPath p;
    int indent = 0.2*width();  if (indent > 30) indent = 30;  // make the indent for perturbing agent be 20% of the glyph width, or = 30 (for visual niceness) (Could be a constant declared globally?)
    return (singleShape(width(), height(), indent));
}

// Draws the phenotype glyph in an anti-clockwise direction, starting from the top left outline point.
QPainterPath PhenotypeProcessNode::singleShape(double w, double h, int indent) const{
    QPainterPath p;
    p.moveTo( -w/2+indent, -h/2 );
    p.lineTo( -w/2, 0 );
    p.lineTo( -w/2+indent, h/2 );
    p.lineTo( w/2-indent, h/2 );
    p.lineTo( w/2, 0 );
    p.lineTo( w/2-indent, -h/2 );
    p.closeSubpath();
    return p;
}

QPainterPath PhenotypeProcessNode::clone_marker() const{
    QPainterPath p_clone, p_front, p_back;
    int indent = 0.2*width();  if (indent > 30) indent = 30;  // make the indent for perturbing agent be 20% of the glyph width, or = 30 (for visual niceness) (Could be a constant declared globally?)

    QRectF cloneRect = QRectF( -width()/2, -height()/2, width(), height()*0.7 );
    p_front = PhenotypeProcessNode::singleShape(width(), height(), indent);
    p_back.addRect(cloneRect);
    p_clone = p_front.subtracted(p_back);
    return p_clone;
}

// Phenotype needs its own paint function because it is very different to all the other process nodes.
void PhenotypeProcessNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Call the parent paint method, to draw the node and label
    ShapeObj::paint(painter, option, widget);

    // Then draw the clone region if necessary
    if (this->cloned == true) {
        painter->setOpacity(0.95);
        painter->fillPath(this->clone_marker(), Qt::gray);
        painter->strokePath(painterPath(), painter->pen());
    }

//    painter->setRenderHint(QPainter::Antialiasing, true);
//    painter->setBackgroundMode( Qt::TransparentMode );
//    painter->setOpacity(1.0);
//    painter->setPen( QPen( Qt::black, 2.5) );
//    painter->drawPath(this->shape());
}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
