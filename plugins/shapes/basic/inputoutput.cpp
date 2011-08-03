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

#include "inputoutput.h"

//===========================================================================
//  "Input/Output" shape code:


QPainterPath InputOutputShape::buildPainterPath(void)
{
    QPainterPath painter_path;

    QPolygonF polygon;

    polygon << QPointF((-width() / 2) + 13, -height() / 2)
            << QPointF(width() / 2, -height() / 2)
            << QPointF((width() / 2) - 13, height() / 2)
            << QPointF((-width() / 2), height() / 2);

    painter_path.addPolygon(polygon);
    painter_path.closeSubpath();

    return painter_path;
}

QDomElement InputOutputShape::to_QDomElement(const unsigned int subset,
        QDomDocument& doc)
{
    QDomElement node = doc.createElement("path");

    if (subset & XMLSS_IOTHER)
    {
        newNsProp(node, x_dunnartNs, x_type, x_shInOutput);
    }

    addXmlProps(subset, node, doc);

    if (subset & XMLSS_ISVG)
    {
        QRectF rect = shapeRect();

        float x1 = rect.left() + 12;
        float y1 = rect.top();
        float x2 = rect.right();
        float y2 = rect.top();
        float x3 = rect.right() - 12;
        float y3 = rect.bottom();
        float x4 = rect.left();
        float y4 = rect.bottom();

        QString value;
        value = value.sprintf("M %.10g,%.10g L %.10g,%.10g L %.10g,%.10g "
                "L %.10g,%.10g L %.10g,%.10g z",
                x1, y1, x2, y2, x3, y3, x4, y4, x1, y1);
        newProp(node, "d", value);
    }

    return node;
}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
