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

#ifndef STRAND_H
#define STRAND_H

#include "libdunnartcanvas/shape.h"
using dunnart::ShapeObj;


class BioStrand: public ShapeObj
{
    Q_OBJECT
    Q_PROPERTY (bool reversed READ reversed WRITE setReversed)

    public:
        BioStrand();

        virtual void initWithXMLProperties(dunnart::Canvas *canvas,
                const QDomElement& node, const QString& ns);
        virtual QPainterPath buildPainterPath(void);

        bool reversed(void) const;
        void setReversed(const bool reversed);

    protected:
        virtual QAction *buildAndExecContextMenu(
                QGraphicsSceneMouseEvent *event, QMenu& menu);

        bool m_direction_reversed;
};


#endif // STRAND_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

