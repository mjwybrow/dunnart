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


#ifndef PDLOGICALOPERATORS_CPP
#define PDLOGICALOPERATORS_CPP

#include "pdlogicaloperators.h"

using namespace dunnart;

LogicalOperator::LogicalOperator(PDLogicalType lt, int o) : ShapeObj(),
    orientation(o), stem(5), radius(36), logicalType(AND)
{
    Q_UNUSED(lt)

}

void LogicalOperator::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->fillPath(this->shape(), Qt::white );
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen( QPen( Qt::black, 2.5) );
    painter->drawPath(this->shape());
}

QPainterPath LogicalOperator::buildPainterPath()
{
    QPainterPath p;

    // ******* IN THIS CASE, STEMS ARE DRAWN IN THE HORIZONTAL DIRECTION:
    if (this->orientation == Qt::Horizontal) {
        p.addEllipse(this->stem, 0, this->radius, this->radius);

        int mid = 0.5*this->height();
        p.moveTo( 0, mid );
        p.lineTo( this->stem, mid );
        p.moveTo( this->stem+this->radius, mid);
        p.lineTo( this->width(), mid);
    }
    // ******* IN THIS CASE, STEMS ARE DRAWN IN THE VERTICAL DIRECTION:
    else {
        p.addEllipse( 0, this->stem, this->radius, this->radius );

        int mid = 0.5*this->width();
        p.moveTo( mid, 0 );
        p.lineTo( mid, this->stem );
        p.moveTo( mid, this->stem+this->radius);
        p.lineTo( mid, this->height());
    }

    return p;
}

LogicalOperator::PDLogicalType LogicalOperator::pdLogicalType(void) const { return logicalType; }

void LogicalOperator::setPDLogicalType(const LogicalOperator::PDLogicalType lt) { logicalType = lt; }


#endif // PDLOGICALOPERATORS_CPP
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
