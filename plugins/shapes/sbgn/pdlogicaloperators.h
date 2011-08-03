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


#ifndef PDLOGICALOPERATORS_H
#define PDLOGICALOPERATORS_H

#include "pdepn.h"

using namespace dunnart;

/***************************     Process Nodes     ***************************/
// Should process nodes just have a fixed size?  This is what I have done here ...
class LogicalOperator: public ShapeObj
{
    Q_OBJECT
    Q_ENUMS (PDLogicalType)

public:
    enum PDLogicalType { AND, OR, NOT };

//    LogicalOperator() : LabelledGlyph("AND", 28, 18), orientation(Qt::Horizontal), stem(5), radius(36), logicalType(lt)  {}
    LogicalOperator(PDLogicalType lt, int o);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);
    QPainterPath buildPainterPath();

    PDLogicalType pdLogicalType(void) const;
    void setPDLogicalType(const PDLogicalType pdLogicalType);

protected:
//    LogicalOperator(QString t, int o);
//    LogicalOperator(QString t, int w, int h, int o) : LabelledGlyph(t, w, h), orientation(o) {}

    int orientation;
    int stem;
    int radius;
    PDLogicalType logicalType;
};

Q_DECLARE_METATYPE (LogicalOperator::PDLogicalType)

#endif // PDLOGICALOPERATORS_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
