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


#include "pdnucleicepn.h"

using namespace dunnart;

QPainterPath NucleicAcidEPN::buildPainterPath(void)
{
    QPainterPath p_front;
    int radius = 10;

    if (this->multimer == true) {
        p_front = NucleicAcidEPN::frontMultimerShape( width(), height(), radius );
        QPainterPath p_back = NucleicAcidEPN::backMultimerShape( width(), height(), radius );

        p_back = p_back.subtracted(p_front);
        p_front.addPath(p_back);
    }
    else {
        p_front = NucleicAcidEPN::singleShape( width(), height(), radius );
    }

    return p_front;
}


QPainterPath NucleicAcidEPN::frontMultimerShape(int w, int h, int r) const
{
    QPainterPath p_front;
    p_front.moveTo( -w/2, -h/2 );
    p_front.lineTo( -w/2, h/2-r-GLYPH_PADDING);
    p_front.arcTo( -w/2, h/2-2*r, 2*r-GLYPH_PADDING, 2*r-GLYPH_PADDING, 180, 90 );
    p_front.lineTo( w/2-r-GLYPH_PADDING, h/2-GLYPH_PADDING );
    p_front.arcTo( w/2-2*r-GLYPH_PADDING, h/2-2*r-GLYPH_PADDING, 2*r, 2*r, 270, 90 );
    p_front.lineTo( w/2-GLYPH_PADDING, -h/2 );
    p_front.closeSubpath();
    return p_front;
}

QPainterPath NucleicAcidEPN::backMultimerShape(int w, int h, int r) const
{
    QPainterPath p_back;
    p_back.moveTo( -w/2+GLYPH_PADDING, -h/2+GLYPH_PADDING );
    p_back.lineTo( -w/2+GLYPH_PADDING, h/2-r);
    p_back.arcTo( -w/2+GLYPH_PADDING, h/2-2*r, 2*r, 2*r, 180, 90 );
    p_back.lineTo( w/2-r, h/2 );
    p_back.arcTo( w/2-2*r, h/2-2*r, 2*r, 2*r, 270, 90 );
    p_back.lineTo( w/2, -h/2+GLYPH_PADDING );
    p_back.closeSubpath();
    return p_back;
}


QPainterPath NucleicAcidEPN::singleShape(int w, int h, int r) const
{
    QPainterPath p_front;
    p_front.moveTo( -w/2, -h/2 );
    p_front.lineTo( -w/2, h/2-r);
    p_front.arcTo( -w/2, h/2-2*r, 2*r, 2*r, 180, 90 );
    p_front.lineTo( w/2-r, h/2 );
    p_front.arcTo( w/2-2*r, h/2-2*r, 2*r, 2*r, 270, 90 );
    p_front.lineTo( w/2, -h/2 );
    p_front.closeSubpath();
    return p_front;
}

QPainterPath NucleicAcidEPN::clone_marker() const
{
    QPainterPath p_clone, p_front, p_back;
    int radius = 10;
    QRectF cloneRect = QRectF( -width()/2, -height()/2, width(), height()*0.7 );

    if (this->multimer == true) {
        p_front = NucleicAcidEPN::frontMultimerShape( width(), height(), radius );
        p_back.addRect(cloneRect);
        p_clone = p_front.subtracted(p_back);

        QPainterPath p_front2, p_back2;
        p_front2 = NucleicAcidEPN::backMultimerShape( width(), height(), radius );
        cloneRect.moveTop( -height()/2+GLYPH_PADDING );
        p_back2.addRect(cloneRect);
        p_clone.setFillRule(Qt::WindingFill);
        p_clone.addPath(p_front2.subtracted(p_back2));
    }
    else {
        p_front = NucleicAcidEPN::singleShape( width(), height(), radius );
        p_back.addRect(cloneRect);
        p_clone = p_front.subtracted(p_back);
    }

    return p_clone;
}

QAction *NucleicAcidEPN::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu)
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
