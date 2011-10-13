/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2008  Monash University
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

#ifndef POLYGON_H
#define POLYGON_H


#include "libdunnartcanvas/shape.h"


namespace Avoid {
class Polygon;
}

namespace dunnart {


class PolygonShape: public ShapeObj
{
    public:
        PolygonShape()
            : ShapeObj(x_shPolygon),
              _xps(NULL), 
              _yps(NULL),
              _psn(0)
        {
        }
        PolygonShape(int *xps, int *yps, int psn, QString id);

        virtual void initWithXMLProperties(Canvas *canvas,
                const QDomElement& node, const QString& ns);

        Avoid::Polygon *poly(const double buffer, Avoid::Polygon *poly = NULL);
        virtual void setLabel(const QString& label);
        void setNewBoundary(int *xps, int *yps, int psn);
        void computeBoundingBoxAndNormalise(void);
        bool getPointPos(int n, double *x, double *y);

    private:
        int x_, y_, w_, h_;
        int *_xps;
        int *_yps;
        int _psn;
        
        void setBoundaryInternal(int *xps, int *yps, int psn);
};


}
#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

