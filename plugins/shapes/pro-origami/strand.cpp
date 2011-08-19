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
    QDomElement node = doc.createElement("dunnart:node");

    if (subset & XMLSS_IOTHER)
    {
        newNsProp(node, x_dunnartNs, x_reversed, m_direction_reversed);

        newNsProp(node, x_dunnartNs, x_type, x_shBioStrand);
    }
    
    addXmlProps(subset, node, doc);

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

