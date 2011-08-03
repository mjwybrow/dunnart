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


#ifndef PDCOMPLEXEPN_H
#define PDCOMPLEXEPN_H

#include "pdepn.h"

using namespace dunnart;

// Complex EPN glyph
// Sets a label, initializes dimensions, and defines the glyph shape in templates.cpp
// Can have a label, can be multimeric.
// This glyph can have a labelled clone marker !
class ComplexEPN: public PDEPN
{
    Q_OBJECT
    Q_PROPERTY (bool multimeric READ isMultimeric WRITE setMultimeric)

public:
//    ComplexEPN(Label *t, bool cb, Label *cl, bool m) : PDEPN(t, cb, cl, m) {}
    ComplexEPN(QString t, bool cb, QString cl, bool m) : PDEPN(t, cb, cl, m) {}
    QPainterPath buildPainterPath();
    bool isMultimeric() { return multimer; } // only defined for EPNs that can be multimeric
    void setMultimeric(const bool multimer);
    QPainterPath singleShape(int w, int h, int r) const;
    QPainterPath frontMultimerShape(int w, int h, int i) const;
    QPainterPath backMultimerShape(int w, int h, int r) const;
    QPainterPath clone_marker() const;
    QAction* buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu);
};

#endif // PDCOMPLEXEPN_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
