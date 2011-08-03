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


#include "pdunspecifiedepn.h"

using namespace dunnart;

QPainterPath UnspecifiedEPN::buildPainterPath(void)
{
    QPainterPath p;

    QRectF r(-width()/2, -height()/2, width(), height());
    // NOTE THAT THIS NODE SHOULD ENSURE THAT WIDTH != HEIGHT, BECAUSE THIS IS AN SBGN REQUIREMENT THAT THIS IS NOT A CIRCLE ...
    p.addEllipse(r);

    return p;
}

// The draw function for unspecified is so easy, just calculate the clone marker without a singleShape() drawing function
QPainterPath UnspecifiedEPN::clone_marker() const
{
  QPainterPath p_front, p_back, p_clone;
  QRectF r(-width()/2, -height()/2, width(), height());

  // NOTE THAT THIS NODE SHOULD ENSURE THAT WIDTH != HEIGHT, BECAUSE THIS IS AN SBGN REQUIREMENT THAT THIS IS NOT A CIRCLE ...
  QRectF cloneRect = QRectF( -width()/2, -height()/2, width(), height()*0.7);
  p_front.addEllipse( r );
  p_back.addRect(cloneRect);
  p_clone = p_front.subtracted(p_back);

  return p_clone;
}

QAction *UnspecifiedEPN::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu)
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
