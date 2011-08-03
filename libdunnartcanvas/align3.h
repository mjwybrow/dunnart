/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2007  Michael Woodward
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
 * Author(s): Michael Woodward
*/

/*
** auto_align3 is an experimental re-layout algorithm that differs to that of
** auto_align or auto_align2 because it makes more use of the existing layout.
** The idea is to take a layout and orthogonalize it (try to straighten out as
** many connectors as possible) without introducing extra connector crossings.
**
** It starts by approximating the original positions of classes with grid
** positions in a coarse grid. Each space in the grid is as large as the
** largest shape. If two smaller shapes are very close in the original diagram
** then one will be pushed to one side in the grid. This method was quick to
** implement and leads to many classes becoming aligned simply because they
** must now fit to a grid, however a fine grid and more code to perform small
** aligments and eliminate overlaps would produce a better result and handle
** varying shape sizes better.
**
** Similarly to the auto_align and auto_align2, shapes are considered one at a
** time, but instead of just being placed next to the shape being considered, 
** its neighbours will instead just be aligned with the shape being considered,
** so that they will only move in one dimension. Connector crossings before and
** after are counted to avoid crossings being introduced (they could
** theoretically be introduced in the fitting to grid step though I think).
** The first shape to be considered is that with highest degree or that which
** is selected, after that shapes are considered in order of highest degree
** instead of breadth-first order. 
**
** The question of what to do with a shape's neighbours when the shape itself
** is moved during an alignment needs more experimentation. They could be
** left where they are (and perhaps aligned in a later step), or they could be
** moved the same distance as the shape being aligned, if possible. Currently
** a shape's neighbours are moved if after its alignment those neighbours are
** on a different side of the shape.
**
** Note that the shape being considered is never moved, only its neighbours.
** This is probably not ideal - in some cases moving the shape being
** considered creates a better alignment, especially if it has fewer
** connected shapes than the neighbour that is to be aligned. Considering
** higher degree shapes first helps avoid this problem but does not eliminate
** it.
**
** Scenarios where overlaps could possibly be created remain, and the layouts
** themselves aren't always good. A more sophisticated approach to
** orthogonalization would probably be best.
** 
** auto_align and auto_align2, described in align.h and align2.h, are also
** experimental re-layout algorithms. 
*/

#ifndef ALIGN_3_H
#define ALIGN_3_H

namespace dunnart {

class CanvasItem;
class Canvas;

extern void auto_align_3(CanvasItem **c);
extern void autoAlignSelection(Canvas *canvas);

}

#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

