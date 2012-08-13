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

#include <QByteArray>
#include <climits>
#include <cassert>
#include <utility>


#include "libavoid/libavoid.h"
using Avoid::Point;

#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/visibility.h"
#include "libdunnartcanvas/polygon.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/utility.h"
#include "libdunnartcanvas/handle.h"

#include "libcola/convex_hull.h"

namespace dunnart {

//===========================================================================
//  Polygon object code:


void PolygonShape::initWithXMLProperties(Canvas *canvas,
        const QDomElement& node, const QString& ns)
{
    // Call equivalent superclass method.
    ShapeObj::initWithXMLProperties(canvas, node, ns);

    QString value = nodeAttribute(node, ns, x_geometry);
    if (!value.isNull())
    {
        QStringList strings =
                value.split(QRegExp("[, ]"), QString::SkipEmptyParts);
        int stringIndex = 0;

        // Read the number of points.
        int totalPoints = strings.at(stringIndex++).toInt();
        
        int geometry[2][totalPoints];

        int xp = essentialProp<int>(node, x_xPos, ns);
        int yp = essentialProp<int>(node, x_yPos, ns);

        // Read a space separated list of coordinates. 
        for (int ptNum = 0; ptNum < totalPoints; ++ptNum)
        {
            geometry[0][ptNum] = strings.at(stringIndex++).toInt() + xp;
            geometry[1][ptNum] = strings.at(stringIndex++).toInt() + yp;
        }
        setBoundaryInternal(geometry[0], geometry[1], totalPoints);
    }
    else
    {
        qFatal("No geometry in Polygon constructor");
    }

    //QT setPos(x - 3, y - 3);
    //width = w + 7;
    //height = h + 7;

    new Handle(this, HAN_CENTER | HAN_CONNPT,
            HPLACE_ZERO, 0, HPLACE_ZERO, 0);

    routerAdd();

    setPainterPath(buildPainterPath());
}


PolygonShape::PolygonShape(int *xps, int *yps, int psn, QString id)
    : ShapeObj(x_shPolygon),
      _xps(NULL),
      _yps(NULL)
{
    m_string_id = id;
    setZValue(ZORD_Shape);

    setBoundaryInternal(xps, yps, psn);

    ///QT setPos(x - 3, y - 3);
    //QT set_active_image(SHAPE_STATE_UNSELECTED);
    //QT set_noimages(w + 7, h + 7);
    
    routerAdd();

    setPainterPath(buildPainterPath());
}


bool PolygonShape::getPointPos(int n, double *x, double *y)
{
    if (n >= _psn)
    {
        return false;
    }

    if (x)
    {
        *x = (double) _xps[n];
    }
    if (y)
    {
        *y = (double) _yps[n];
    }

    return true;
}


void PolygonShape::setBoundaryInternal(int *xps, int *yps, int psn)
{
    _psn = psn;
    assert(_psn != 0);
    
    if (_xps != NULL) std::free(_xps);
    _xps = (int *) calloc(_psn, sizeof(int));
    if (_yps != NULL) std::free(_yps);
    _yps = (int *) calloc(_psn, sizeof(int));

    // XXX: Check the winding direction.
    for (int i = 0; i < _psn; ++i)
    {
        _xps[i] = xps[i];
        _yps[i] = yps[i];
    }

    computeBoundingBoxAndNormalise();
}


void PolygonShape::setNewBoundary(int *xps, int *yps, int psn)
{
    //QT restore_behind();

    setBoundaryInternal(xps, yps, psn);

    //QT set_pos(x - 3, y - 3);
    //QT set_noimages(w + 7, h + 7);
    //QT set_active_image(get_active_image_n(), true);
}

void PolygonShape::computeBoundingBoxAndNormalise(void)
{
    int xmin = INT_MAX;
    int xmax = INT_MIN;
    int ymin = INT_MAX;
    int ymax = INT_MIN;

    // Find the bounding box.
    for (int i = 0; i < _psn; ++i)
    {
        xmin = std::min(xmin, (int)_xps[i]);
        xmax = std::max(xmax, (int)_xps[i]);
        ymin = std::min(ymin, (int)_yps[i]);
        ymax = std::max(ymax, (int)_yps[i]);
    }

    x_ = xmin;
    y_ = ymin;
    w_ = xmax - xmin + 1;
    h_ = ymax - ymin + 1;

    // Normalise boundary coordinates:
    for (int i = 0; i < _psn; ++i)
    {
        _xps[i] -= x_; 
        _yps[i] -= y_;
    }
}


void PolygonShape::setLabel(const QString& label)
{
    Q_UNUSED (label)

    // Do nothing
    return;
}


Avoid::Polygon PolygonShape::polygon(void) const
{
    Avoid::Polygon poly(_psn);
    poly._id = (int) m_internal_id;

    double sx = x() + HANDLE_PADDING;
    double sy = y() + HANDLE_PADDING;

    for (int i = 0; i < _psn; ++i)
    {
        poly.ps[i].x = _xps[i] + sx;
        poly.ps[i].y = _yps[i] + sy;
    }

    return poly;
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

