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

#ifndef OLDCANVAS_H
#define OLDCANVAS_H

#include <QList>
#include <QDomNode>
#include <QString>
#include <QXmlNamespaceSupport>


namespace dunnart {


namespace gml {
    class Graph;
}
class Canvas;
class ShapeObj;
class CanvasItem;
class Handle;


extern void redraw_connectors(QWidget **slider);

extern QXmlNamespaceSupport namespaces;


extern void direct_manip_checkbox_handler(QWidget **object_addr,
        int action);
//QT extern void selection_outlines(SDL_Surface *surface, int *sel_x, int *sel_y,
//        int *sel_w, int *sel_h);
//QT extern void copy_selection_to(CanvasItemList *target);
extern void setup_toolbar();
extern void setup_sliders();
extern void load_diagram(Canvas *canvas, const QString& filename);
extern void toggle_direct_manip(void);
extern void toggle_partial_feedback(QWidget **c);
extern void dragAbortActions(void);
extern bool do_response_tasks(void);
extern bool checkUserInterrupt(void);
extern void relayout_selection(QWidget **gobj);
//QT extern void idle_loop_handler(SDL_Event *event);
extern void removeFromCanvas(QList<CanvasItem *>& list);
extern void returnToCanvas(QList<CanvasItem *>& list);

extern bool queryMode;
extern CanvasItem *queryObj;


struct LayoutDoneCallback {
    virtual void notify() = 0;
    virtual ~LayoutDoneCallback() {}
};
extern LayoutDoneCallback* layoutDoneCallback;


}
#endif  // OLDCANVAS_H

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

