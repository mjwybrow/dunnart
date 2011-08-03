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

        // QT How can I get this shapeFont from ShapeObj instead of re-declaring it here??!!
//        QFontDatabase database;
//        database.addApplicationFont("DejaVuSans.ttf");
//        static QFont *shapeFont = new QFont("DejaVu Sans", shapeFontSize);
        painter->setPen(Qt::black);
//        painter->setFont(*shapeFont);
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
PDEPN::PDEPN(QString l, bool cb, QString cl, bool m) : ShapeObj(), cloned(cb), cloneLabel(cl), multimer(m)
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

    set_handler(epn_handler);
}


PDEPN::PDEPN(const double x, const double y, const int w, const int h)
    :Shape(++maxid, x, y, w, h)
{
    PDEPN::set_class_members();

    set_handler(epn_handler);
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

// !!! need to implement this
xmlNodePtr PDEPN::to_xmlNode(const unsigned int subset, xmlNs *dunnartNs)
{
    xmlNodePtr node = xmlNewNode(NULL, BAD_CAST("rect"));

    //     if (subset & XMLSS_IOTHER)
    //     {
    //         xmlNewNsProp(node, dunnartNs, x_type, x_shBioHelix);
    //     }

    //     addXmlProps(subset, node, dunnartNs);

    //     if (subset & XMLSS_ISVG)
    //     {
    //         char value[50];

    //         xmlNodePtr node2 = xmlNewNode(NULL, BAD_CAST("rect"));
    //         xmlAddSibling(node, node2);

    //         double x, y, w, h;
    //         getPosAndSize(x, y, w, h);

    //         double yround = h / 2;
    //         double xround = w / 2;

    //         if (xround <= yround)
    //         {
    //             yround = xround / 2;

    //             sprintf(value, "%.10g", w);
    //             xmlNewProp(node2, BAD_CAST("width"), BAD_CAST(value));
    //             sprintf(value, "%.10g", yround * 2);
    //             xmlNewProp(node2, BAD_CAST("height"), BAD_CAST(value));
    //         }
    //         else
    //         {
    //             xround = yround / 2;

    //             sprintf(value, "%.10g", xround * 2);
    //             xmlNewProp(node2, BAD_CAST("width"), BAD_CAST(value));
    //             sprintf(value, "%.10g", h);
    //             xmlNewProp(node2, BAD_CAST("height"), BAD_CAST(value));
    //         }

    //         //double radius = std::min(w, h) / 2;

    //         sprintf(value, "%.10g", w);
    //         xmlNewProp(node, BAD_CAST("width"), BAD_CAST(value));
    //         sprintf(value, "%.10g", h);
    //         xmlNewProp(node, BAD_CAST("height"), BAD_CAST(value));
    //         sprintf(value, "%.10g", x);
    //         xmlNewProp(node, BAD_CAST("x"), BAD_CAST(value));
    //         xmlNewProp(node2, BAD_CAST("x"), BAD_CAST(value));
    //         sprintf(value, "%.10g", y);
    //         xmlNewProp(node, BAD_CAST("y"), BAD_CAST(value));
    //         xmlNewProp(node2, BAD_CAST("y"), BAD_CAST(value));

    //         sprintf(value, "%.10g", xround);
    //         xmlNewProp(node, BAD_CAST("rx"), BAD_CAST(value));
    //         xmlNewProp(node2, BAD_CAST("rx"), BAD_CAST(value));
    //         sprintf(value, "%.10g", yround);
    //         xmlNewProp(node, BAD_CAST("ry"), BAD_CAST(value));
    //         xmlNewProp(node2, BAD_CAST("ry"), BAD_CAST(value));

    //         newProp(node2, BAD_CAST("class"), "shape");
    //         if (colour != shFillCol)
    //         {
    //             char* s;
    //             asprintf(&s, "fill:#%06X;", colour >> 8);
    //             newProp(node2, BAD_CAST("style"), s);
    //             std::free(s);
    //         }
    //     }

    return node;
}

void PDEPN::epn_handler(GuiObj **object_addr, int action)
{
    GuiObj *object = *object_addr;
    PDEPN *epn = dynamic_cast<PDEPN *> (object);

    switch (action)
    {
    case LABEL_CHANGED:
    {
	//check to see if a resize is needed
	int new_width, new_height;
	bool store_undo = true;
	int buffer = 15;
	epn->determine_good_text_dimensions(&new_width, &new_height);
        epn->setPosAndSize(epn->centrePos(),
                QSizeF(new_width + buffer, new_height, store_undo);
    }
    case MOUSE_RCLICK: {
        //      epn->addContextMenuItems();

        printf("testing ...\n");

    }
    default:
        // In all other cases, just call the superclass' handler.
        shape_handler(object_addr, action);
    }
}

void PDEPN::addContextMenuItems(MenuItems& items) {

    // option to turn on/off multimeric is not included in this class becase
    // cannot be multimeric
    //   items.push_back(
    // 		  MenuItem(BUT_TYP_Button, BUT_GENERIC, "Switch multimeric", "",
    // 			   NULL, switchMultimeric));

    // option to turn on/off clone marker
    items.push_back(
                MenuItem(BUT_TYP_Button, BUT_GENERIC, "Switch clone marker", "",
                         NULL, switchCloning));
    items.push_back(MenuSeparator());
}

#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

