/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2010  Monash University
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

#include "helix.h"
using namespace dunnart;

//===========================================================================
//  "Protein Helix" shape code:

BioHelix::BioHelix()
    : ShapeObj("bioHelix")
{
}


QPainterPath BioHelix::buildPainterPath(void)
{
    QPainterPath painter_path;
    
    painter_path.setFillRule(Qt::WindingFill);

    double yround = (height() / 2);
    double xround = (width() / 2);

    double start_angle = 180;
    if (xround <= yround)
    {
        yround = xround / 2;
    }
    else
    {
        xround = yround / 2;
        start_angle = 270;
    }

    painter_path.addRoundedRect((-width() / 2), (-height() / 2),
            width(), height(), xround, yround);
    painter_path.addRoundedRect((-width() / 2), (-height() / 2),
            xround * 2, yround * 2, xround, yround);

    return painter_path;
}


QRectF BioHelix::labelBoundingRect(void) const
{
    QRectF label_rect = boundingRect();

    double yround = (label_rect.height() / 2);
    double xround = (label_rect.width() / 2);
    double xoffset = 0;
    double yoffset = 0;

    if (xround <= yround)
    {
        yoffset = (xround / 2) + 4;
    }
    else
    {
        xoffset = (yround / 2) + 4;
    }

    // Put the label in the cylinder's body, not overlapping its end.
    label_rect.adjust(xoffset, yoffset, 0, 0);

    return label_rect;
}


// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

