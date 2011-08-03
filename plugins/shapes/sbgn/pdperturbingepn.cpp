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


#include "pdperturbingepn.h"

using namespace dunnart;

QPainterPath PerturbingEPN::buildPainterPath(void)
{
    QPainterPath p;
    int indent = 0.2*width();  if (indent > 30) indent = 30;  // make the indent for perturbing agent be 20% of the glyph width, or = 30 (for visual niceness) (Could be a constant declared globally?)
    return (singleShape( width(), height(), indent ));
}

// w=width, h=height, indent=degree of indentation of pertubring glyph sides
QPainterPath PerturbingEPN::singleShape(int w, int h, int indent) const
{
    QPainterPath p;
    p.moveTo( -w/2, -h/2 );
    p.lineTo( -w/2+indent, 0);
    p.lineTo( -w/2, h/2);
    p.lineTo( w/2, h/2 );
    p.lineTo( w/2-indent, 0 );
    p.lineTo( w/2, -h/2 );
    p.closeSubpath();
    return p;
}

QPainterPath PerturbingEPN::clone_marker() const
{
    QPainterPath p_clone, p_front, p_back;
    QRectF cloneRect = QRectF( -width()/2, -height()/2, width(), height()*0.7 );
    int indent = 0.2*width();  if (indent > 30) indent = 30;  // make the indent for perturbing agent be 20% of the glyph width, or = 30 (for visual niceness) (Could be a constant declared globally?)

    p_front = PerturbingEPN::singleShape( width(), height(), indent );
    p_back.addRect(cloneRect);
    p_clone = p_front.subtracted(p_back);
    return p_clone;
}

QAction *PerturbingEPN::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }

    QAction* switchCloning = menu.addAction(tr("Switch cloning"));

    QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);

    if (action == switchCloning) {
        cloned = (!cloned);
        setPainterPath(buildPainterPath());
        update();
    }

    return action;
}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
