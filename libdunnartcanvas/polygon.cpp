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
        char *str_copy = qstrdup(value.toLatin1().data());
        char *sStart = str_copy;
        char *sEnd = NULL;
       
        sEnd = strSetEnd(sStart);
        int psn = atoi(sStart);
        sStart = strSetStart(sStart, sEnd);
        
        int geometry[2][psn];

        int xp = essentialProp<int>(node, x_xPos, ns);
        int yp = essentialProp<int>(node, x_yPos, ns);

        int xy = 0;
        int pt = 0;
        // Read a space separated list of coordinates. 
        while (1)
        {
            sEnd = strSetEnd(sStart);

            geometry[xy][pt] = atoi(sStart) + ((xy == 0) ? xp : yp) + 3;
            
            xy++;
            if (xy == 2)
            {
                pt++;
                xy = 0;
                if (pt == psn)
                {
                    // At end;
                    break;
                }
            }

            sStart = strSetStart(sStart, sEnd);
            if (sEnd == NULL)
            {
                break;
            }
        }
        setBoundaryInternal(geometry[0], geometry[1], psn);
    
        delete[] str_copy;
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
    : ShapeObj(),
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


#if 0
void Polygon::draw(SDL_Surface *surface, const int x, const int y,
        const int type, const int w, const int h)
{
    if (!surface)
    {
        return;
    }
    
    //boxColor(surface, x, y, x + w - 1, y + h - 1, strokeCol);
   
    QColor strokeCol = shLineCol;
    switch (type)
    {
        case SHAPE_DRAW_NORMAL:
            // Nothing to do.
            break;
        case SHAPE_DRAW_HIGHLIGHTED:
        case SHAPE_DRAW_LEAD_HIGHLIGHTED:
            strokeCol = QColor(0, 255, 255);
            break;
        case SHAPE_DRAW_OUTLINE:
            strokeCol = QColor(0, 0, 0, 128);
            break;
        default:
            break;
    }
    QColor fillCol = shFillCol;

    int *xpsOff = (int *) calloc(_psn, sizeof(int));
    int *ypsOff = (int *) calloc(_psn, sizeof(int));
   
    for (int i = 0; i < _psn; ++i)
    {
        xpsOff[i] = _xps[i] + x + 3;
        ypsOff[i] = _yps[i] + y + 3;
    }

    filledPolygonColor(surface, xpsOff, ypsOff, _psn, fillCol);
    aapolygonColor(surface, xpsOff, ypsOff, _psn, strokeCol);

    std::free(xpsOff);
    std::free(ypsOff);
}
#endif

void PolygonShape::set_label(const char *l)
{
    Q_UNUSED (l)

    // Do nothing
    return;
}


QDomElement PolygonShape::to_QDomElement(const unsigned int subset,
        QDomDocument& doc)
{
    QDomElement node = doc.createElement("dunnart:node");

    if (subset & XMLSS_IOTHER)
    {
        newNsProp(node, x_dunnartNs, x_type, x_shPolygon);
    }
    
    addXmlProps(subset, node, doc);

    return node;
}


Avoid::Polygon *PolygonShape::poly(const double b, Avoid::Polygon *p)
{
    Q_UNUSED (b)
    if (p)
    {
        delete p;
    }
    p = new Avoid::Polygon(_psn);
    
    if (!p)
    {
        qFatal("Couldn't calloc memory in Polygon::poly()");
    }
    
    p->_id = (int) m_internal_id;

    double sx = x() + HANDLE_PADDING;
    double sy = y() + HANDLE_PADDING;

    for (int i = 0; i < _psn; ++i)
    {
        p->ps[i].x = _xps[i] + sx;
        p->ps[i].y = _yps[i] + sy;
    }

    return p;
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

