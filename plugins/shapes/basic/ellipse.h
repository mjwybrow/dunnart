/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2012  Monash University
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



#ifndef ELLIPSE_H
#define ELLIPSE_H

#include "libdunnartcanvas/shape.h"
using dunnart::ShapeObj;

class EllipseShape: public ShapeObj
{
    public:
        EllipseShape();
        virtual ~EllipseShape() { }

        virtual QPainterPath buildPainterPath(void);
};


#endif // ELLIPSE_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
