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


#ifndef PDSIMPLECHEMEPN_H
#define PDSIMPLECHEMEPN_H

#include "pdepn.h"

using namespace dunnart;

// Simple Chemical EPN glyph
// Sets a label, initializes dimensions, and defines the glyph shape in templates.cpp
// This glyph can have a simple clone marker without a label.
// Can be multimeric.
// Only one dimension specified, because this should have a square bounding box
class SimpleChemEPN: public PDEPN
{
public:
//    SimpleChemEPN(Label *t, bool cb, bool m) : PDEPN(t, cb, new Label(""), m) {}
    SimpleChemEPN(QString t, bool cb, bool m) : PDEPN(t, cb, "", m) {}
    QPainterPath clone_marker() const;
    QPainterPath buildPainterPath();
    QAction* buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu);
};

#endif // PDSIMPLECHEMEPN_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
