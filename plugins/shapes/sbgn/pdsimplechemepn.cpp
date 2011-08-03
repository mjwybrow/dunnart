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


#include "pdsimplechemepn.h"

using namespace dunnart;

QPainterPath SimpleChemEPN::buildPainterPath(void)
{
    QPainterPath p_front;

    // make sure this glyph stays cirular
    qreal draw_width = width();
    if ( height() < width()) draw_width = height();

    if (this->multimer == true) {
      QPainterPath p_back;
      p_front.addEllipse( -draw_width/2, -draw_width/2, draw_width-GLYPH_PADDING, draw_width-GLYPH_PADDING );
      p_back.addEllipse( -draw_width/2+GLYPH_PADDING, -draw_width/2+GLYPH_PADDING, draw_width-GLYPH_PADDING, draw_width-GLYPH_PADDING );

      p_back = p_back.subtracted(p_front);
      p_front.addPath(p_back);
    }

    else {
      p_front.addEllipse( -draw_width/2, -draw_width/2, draw_width, draw_width );
    }

    return p_front;
}

// The draw function for unspecified is so easy, just calculate the clone marker without a singleShape() drawing function
QPainterPath SimpleChemEPN::clone_marker() const
{
    QPainterPath p_front, p_back, p_clone;

    // this glyph has to be cirular:
    qreal draw_width = width();
    if ( height() < width()) draw_width = height();
    QRectF cloneRect = QRectF( -draw_width/2, -draw_width/2, draw_width, draw_width*0.7 );

    if (this->multimer == true) {
      QPainterPath p_front2, p_back2;

      p_clone.setFillRule(Qt::WindingFill);

      p_front.addEllipse( -draw_width/2, -draw_width/2, draw_width-GLYPH_PADDING, draw_width-GLYPH_PADDING );
      p_back.addRect(cloneRect);
      p_clone.addPath(p_front.subtracted(p_back));

      cloneRect.moveTop( -draw_width/2+GLYPH_PADDING );
      p_front2.addEllipse( -draw_width/2+GLYPH_PADDING, -draw_width/2+GLYPH_PADDING, draw_width-GLYPH_PADDING, draw_width-GLYPH_PADDING );
      p_back2.addRect(cloneRect);
      p_clone.addPath(p_front2.subtracted(p_back2));
    }
    else {
      p_front.addEllipse( -draw_width/2, -draw_width/2, draw_width, draw_width );
      p_back.addRect(cloneRect);
      p_clone = p_front.subtracted(p_back);
    }

    return p_clone;
}

QAction *SimpleChemEPN::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu)
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
