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


// See pdepn.h for comments on this class.


#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/shape.h"

#include "pdepn.h"
#include <QPainter>
#include <QFontDatabase>

using namespace dunnart;

#if 0
// Creates a label with the given text (handles multiline)
Label::Label(QString t) : text(t), textFlags(Qt::AlignCenter) { } //|Qt::TextWordWrap

// Draws the label in a given drawing style (color, thickness, etc.)
void Label::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setPen( QPen( Qt::black, 2.5) );
    painter->drawText(this->boundingRect(), this->textFlags, this->text);
}

// The bounding rectangle of the text of the label
QRectF Label::boundingRect() const
{
    return QFontMetrics( this->font ).boundingRect(QRect(), this->textFlags, this->text);
}

// The shape of the label ( == bounding rectangle of the label )
QPainterPath Label::shape() const
{
    QPainterPath p;
    p.addRect(this->boundingRect());
    return p;
}
#endif

// Draws the glyph in a given style, based on the shape of the glyph (which is defined in subclasses)
void PDEPN::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Call the parent paint method, to draw the node and label
    ShapeObj::paint(painter, option, widget);

    // Then draw the clone region if necessary
    if (this->cloned == true) {
        painter->setOpacity(0.95);
        painter->fillPath(this->clone_marker(), Qt::gray);

        painter->setPen(Qt::black);
        if (canvas())
        {
            painter->setFont(canvas()->canvasFont());
        }
        painter->setRenderHint(QPainter::TextAntialiasing, true);
//        painter->drawText(labelBoundingRect(), Qt::AlignLeft, cloneLabel);

//        QRectF r(-width()/2, -height()/2*0.7, width(), height());
//        painter->drawText(r, cloneLabel, QTextOption::NoWrap);
        painter->drawText(-width()/2+8, height()*0.38, cloneLabel);
        // need this to redraw the node outlines, because the second clone marker paints over top node in multimers:
        painter->strokePath(painterPath(), painter->pen());
    }
}

// Extension of the labelled glyph, creates the centred label, and sets a boolean to keep track of whether the glyph is cloned.
PDEPN::PDEPN(QString l, bool cb, QString cl, bool m) : ShapeObj("sbgn.NucleicAcidEPN"), cloned(cb), cloneLabel(cl), multimer(m)
{
    ShapeObj::setLabel(l);
}

// This function overrides the function defined in the Glyph class, to return true, because by definition this class is cloneable :-)
bool PDEPN::isCloned() { return cloned; }









#if 0
// QT

PDEPN::PDEPN()
{
    PDEPN::set_class_members();
}

PDEPN::PDEPN(xmlNode *node, xmlNs *ns)
    :Shape(node, ns)
{
    PDEPN::set_class_members();
}


PDEPN::PDEPN(const double x, const double y, const int w, const int h)
    :Shape(++maxid, x, y, w, h)
{
    PDEPN::set_class_members();
}


PDEPN::~PDEPN() {}

SDL_Rect PDEPN::labelBoundingRect(void) const
{
    SDL_Rect text_rect;

    int yround = ((height - 8) / 2);
    int xround = ((width - 8) / 2);
    int xoffset = 0;
    int yoffset = 0;

    if (xround <= yround)
    {
        yoffset = (xround / 2) + 4;
    }
    else
    {
        xoffset = (yround / 2) + 4;
    }

    text_rect.x = 4 + xoffset;
    text_rect.y = 4 + yoffset;
    text_rect.w = width - 8 - xoffset;
    text_rect.h = height - 8 - yoffset;

    return text_rect;
}


QAction *PDEPN::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event,
        QMenu& menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }

    QAction* switchCloneMarkerAction = menu.addAction(tr("Switch clone marker"));

    QAction *action = CanvasItem::buildAndExecContextMenu(event, menu);

    if (action == switchCloneMarkerAction)
    {
        switchCloning();
    }

    return action;
}
#endif


// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

