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


#ifndef PDNUCLEICEPN_H
#define PDNUCLEICEPN_H

#include "pdepn.h"

using namespace dunnart;

// Nucleic Acid EPN glyph
// Sets a label, initializes dimensions, and defines the glyph shape in templates.cpp
// This glyph can have a labelled clone marker.
//  Can be multimeric.
class NucleicAcidEPN: public PDEPN
{
public:
//    NucleicAcidEPN(Label *t, bool cb, Label *cl, bool m) : PDEPN(t, cb, cl, m) {}
    NucleicAcidEPN(QString t, bool cb, QString cl, bool m) : PDEPN(t, cb, cl, m) {}
    QPainterPath buildPainterPath();
    QPainterPath clone_marker() const;
    bool isMultimeric() { return multimer; } // only defined for EPNs that can be multimeric
    QPainterPath singleShape(int w, int h, int r) const;
    QPainterPath frontMultimerShape(int w, int h, int r) const;
    QPainterPath backMultimerShape(int w, int h, int r) const;
    QAction* buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu);
};

#endif // PDNUCLEICEPN_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
