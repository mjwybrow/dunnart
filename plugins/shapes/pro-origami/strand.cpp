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
#include "libdunnartcanvas/canvas.h"

#include "strand.h"
using namespace dunnart;

//===========================================================================
//  "Protein Strand" shape code:

BioStrand::BioStrand()
    : ShapeObj(),
      m_direction_reversed(false)
{
    //set_handler(biostrand_handler);
}


void BioStrand::initWithXMLProperties(Canvas *canvas,
        const QDomElement& node, const QString& ns)
{
    // Call equivalent superclass method.
    ShapeObj::initWithXMLProperties(canvas, node, ns);

    optionalProp(node, x_reversed, m_direction_reversed, ns);
}


QPainterPath BioStrand::buildPainterPath(void)
{
    QPainterPath painter_path;

    QPolygonF polygon;

    double qw = width() / 4;
    double qh = height() / 4;
    double hw = width() / 2;
    double hh = height() / 2;

    bool rotated = (width() > height());

    if (!rotated)
    {
        polygon << QPointF(0, -hh)
                << QPointF(hw, -hh + hw)
                << QPointF(qw, -hh + hw)
                << QPointF(qw, hh)
                << QPointF(-qw, hh)
                << QPointF(-qw, -hh + hw)
                << QPointF(-hw, -hh + hw);
    }
    else if (rotated)
    {
        polygon << QPointF(-hw, 0)
                << QPointF(-hw + hh, hh)
                << QPointF(-hw + hh, qh)
                << QPointF(hw, qh)
                << QPointF(hw, -qh)
                << QPointF(-hw + hh, -qh)
                << QPointF(-hw + hh, -hh);
    }
    
    if (m_direction_reversed)
    {
        for (int i = 0; i < polygon.size(); ++i)
        {
            polygon[i] *= -1;
        }
    }

    painter_path.addPolygon(polygon);
    painter_path.closeSubpath();

    return painter_path;
}


QDomElement BioStrand::to_QDomElement(const unsigned int subset, 
        QDomDocument& doc)
{
    QDomElement node = doc.createElement("path");
   
    if (subset & XMLSS_IOTHER)
    {
        newNsProp(node, x_dunnartNs, x_reversed, m_direction_reversed);

        newNsProp(node, x_dunnartNs, x_type, x_shBioStrand);
    }
    
    addXmlProps(subset, node, doc);

    if (subset & XMLSS_ISVG)
    {
        QRectF rect = shapeRect();

        double x = rect.x();
        double y = rect.y();
        double ex = rect.right();
        double ey = rect.bottom();

        double hw = rect.width() / 2;
        double qw = rect.width() / 4;
        double midx = rect.x() + hw;
        
        double hh = rect.height() / 2;
        double qh = rect.height() / 4;
        double midy = rect.y() + hh;

        bool rotated = (rect.width() > rect.height());

        float xs[7];
        float ys[7];

        if (!rotated && !m_direction_reversed)
        {
            xs[0] = midx;
            ys[0] = y;
            xs[1] = ex;
            ys[1] = y + hw;
            xs[2] = midx + qw;
            ys[2] = y + hw;
            xs[3] = midx + qw;
            ys[3] = ey;
            xs[4] = midx - qw;
            ys[4] = ey;
            xs[5] = midx - qw;
            ys[5] = y + hw;
            xs[6] = x;
            ys[6] = y + hw;
        }
        else if (!rotated && m_direction_reversed)
        {
            xs[0] = midx;
            ys[0] = ey;
            xs[1] = x;
            ys[1] = ey - hw;
            xs[2] = midx - qw;
            ys[2] = ey - hw;
            xs[3] = midx - qw;
            ys[3] = y;
            xs[4] = midx + qw;
            ys[4] = y;
            xs[5] = midx + qw;
            ys[5] = ey - hw;
            xs[6] = ex;
            ys[6] = ey - hw;
        }
        else if (rotated && m_direction_reversed)
        {
            xs[0] = ex;
            ys[0] = midy;
            xs[1] = ex - hh;
            ys[1] = y;
            xs[2] = ex - hh;
            ys[2] = midy - qh;
            xs[3] = x;
            ys[3] = midy - qh;
            xs[4] = x;
            ys[4] = midy + qh;
            xs[5] = ex - hh;
            ys[5] = midy + qh;
            xs[6] = ex - hh;
            ys[6] = ey;
        }
        else if (rotated && !m_direction_reversed)
        {
            xs[0] = x;
            ys[0] = midy;
            xs[1] = x + hh;
            ys[1] = ey;
            xs[2] = x + hh;
            ys[2] = midy + qh;
            xs[3] = ex;
            ys[3] = midy + qh;
            xs[4] = ex;
            ys[4] = midy - qh;
            xs[5] = x + hh;
            ys[5] = midy - qh;
            xs[6] = x + hh;
            ys[6] = y;
        }

        QString value;
        value = value.sprintf("M %.10g,%.10g L %.10g,%.10g "
                "L %.10g,%.10g L %.10g,%.10g L %.10g,%.10g L %.10g,%.10g "
                "L %.10g,%.10g L %.10g,%.10g z",
                xs[0], ys[0], xs[1], ys[1], xs[2], ys[2], xs[3], ys[3], 
                xs[4], ys[4], xs[5], ys[5], xs[6], ys[6], xs[0], ys[0]);
        newProp(node, "d", value);
    }

    return node;
}


bool BioStrand::reversed(void) const
{
    return m_direction_reversed;
}

void BioStrand::setReversed(const bool reversed)
{
    m_direction_reversed = reversed;
    setPainterPath(buildPainterPath());
    update();
}


QAction *BioStrand::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event,
        QMenu& menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QAction* reverseAction = menu.addAction(
            tr("Reverse direction"));

    QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);

    if (action == reverseAction)
    {
        setReversed(!reversed());
    }

    return action;
}


#if 0
void biostrand_handler(QWidget **object_addr, int action)
{
    QWidget *object = *object_addr;
    BioStrand *strand = dynamic_cast<BioStrand *> (object);

    switch (action)
    {
        case LABEL_CHANGED:
        {
            //check to see if a resize is needed
            int new_width, new_height;
            bool store_undo = true;
            int buffer = 15;
            strand->determine_good_text_dimensions(&new_width, &new_height);
            strand->setPosAndSize(strand->centrePos(),
                    QSizeF(new_width + buffer, new_height, store_undo);
        }
        default:
            // In all other cases, just call the superclass' handler.
            shape_handler(object_addr, action);
    }
}
#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

