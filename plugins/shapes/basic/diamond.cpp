/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2011  Monash University
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

#include <QString>

#include "libdunnartcanvas/canvasitem.h"
using namespace dunnart;

#include "diamond.h"

//===========================================================================
//  "Diamond" shape code:


QPainterPath DiamondShape::buildPainterPath(void)
{
    QPainterPath painter_path;

    QPolygonF polygon;
    polygon << QPointF(0, -height() / 2)
            << QPointF(-width() / 2, 0)
            << QPointF(0, height() / 2)
            << QPointF(width() / 2, 0);

    painter_path.addPolygon(polygon);
    painter_path.closeSubpath();

    return painter_path;
}

QDomElement DiamondShape::to_QDomElement(const unsigned int subset,
        QDomDocument& doc)
{
    QDomElement node = doc.createElement("dunnart:node");

    if (subset & XMLSS_IOTHER)
    {
        newNsProp(node, x_dunnartNs, x_type, "diamond");
    }

    addXmlProps(subset, node, doc);

    return node;
}

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
