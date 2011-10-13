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

#ifndef TEXTSHAPE_H
#define TEXTSHAPE_H


#include "libdunnartcanvas/shape.h"

namespace dunnart {


class TextShape: public ShapeObj
{
    public:
        TextShape();
        TextShape(const double x, const double y, const char *txt, int ptSize);
        TextShape(QString id, const double x, const double y, int w, int h,
                const char *txt, int ptSize);

        virtual void initWithXMLProperties(Canvas *canvas,
                const QDomElement& node, const QString& ns);
        virtual QDomElement to_QDomElement(const unsigned int subset,
                QDomDocument& doc);
        Avoid::Polygon *poly(const double buffer, Avoid::Polygon *poly = NULL);
        virtual void setLabel(const QString& label);

        void setText(const QString& t);
        char *getText(void);
        void setFontSize(const unsigned int ptSize);
        unsigned int getFontSize(void);
        void getTextDimensions(int *x, int *y, int *w, int *h, int *b,
                int *yOff = NULL);
        unsigned int setSizeToFit(const int w, const int h);
        void setColour(unsigned int r, unsigned int g, unsigned int b);
        void setTextPosition(int x, int y, bool from_cider);
        void setTextPositionByBaseline(int x, int b, bool from_cider);
        unsigned int makeLine(const int width);

    private:
        char *txtStr;
        QString *unicodeStr;
        unsigned int fontSize;
        int baselineOffset;
        QColor _colour;
};


}
#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

