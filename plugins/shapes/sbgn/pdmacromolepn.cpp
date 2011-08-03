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


#include "pdmacromolepn.h"

using namespace dunnart;

QPainterPath MacromolEPN::buildPainterPath(void)
{
    QPainterPath p_front;
    int r = 10;

    if (this->multimer == true) {
        QPainterPath p_back;
        p_front = MacromolEPN::frontMultimerShape( width(), height(), r);
        p_back = MacromolEPN::backMultimerShape( width(), height(), r );

        p_back = p_back.subtracted(p_front);
        p_front.addPath(p_back);
    }

    else {
        p_front = MacromolEPN::singleShape(width(), height(), r);
    }

    return p_front;
}

// w = width, h = height, r = corner radius
QPainterPath MacromolEPN::frontMultimerShape(double w, double h, int r) const
{
    QPainterPath p_front;
    p_front.addRoundedRect( -w/2, -h/2, w-GLYPH_PADDING, h-GLYPH_PADDING, r, r );
    return p_front;
}

// w = width, h = height, r = corner radius
QPainterPath MacromolEPN::backMultimerShape(double w, double h, int r) const
{
    QPainterPath p_back;
    p_back.addRoundedRect( -w/2+GLYPH_PADDING, -h/2+GLYPH_PADDING, w-GLYPH_PADDING, h-GLYPH_PADDING, r, r );
    return p_back;
}


// w = width, h = height, r = corner radius
QPainterPath MacromolEPN::singleShape(double w, double h, int r) const
{
    QPainterPath p_front;
    p_front.addRoundedRect( -w/2, -h/2, w, h, r, r );
    return p_front;
}

QPainterPath MacromolEPN::clone_marker() const
{
    QPainterPath p_clone, p_front, p_back;
    int radius = 10;   // corner radius maybe should be declared as a global variable somewhere?  Can then change look at once ...
    QRectF cloneRect = QRectF( -width()/2, -height()/2, width(), height()*0.7 );

    if (this->multimer == true) {
        p_front = MacromolEPN::frontMultimerShape( width(), height(), radius );
        p_back.addRect(cloneRect);
        p_clone = p_front.subtracted(p_back);

        QPainterPath p_front2, p_back2;
        p_front2 = MacromolEPN::backMultimerShape( width(), height(), radius );
        cloneRect.moveTop( -height()/2+GLYPH_PADDING );
        p_back2.addRect(cloneRect);
        p_clone.setFillRule(Qt::WindingFill);
        p_clone.addPath(p_front2.subtracted(p_back2));
    }
    else {
        p_front = MacromolEPN::singleShape( width(), height(), radius );
        p_back.addRect(cloneRect);
        p_clone = p_front.subtracted(p_back);
    }

    return p_clone;
}

QAction *MacromolEPN::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }

    QAction* switchMultimeric = menu.addAction(tr("Switch multimeric"));
    QAction* switchCloning = menu.addAction(tr("Switch cloning"));

    QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);

    if (action == switchMultimeric) {
        multimer = (!multimer);
        setPainterPath(buildPainterPath());
        update();
    }
    else if (action == switchCloning) {
        cloned = (!cloned);
        setPainterPath(buildPainterPath());
        update();
    }

    return action;
}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
