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


#include "libdunnartcanvas/gmlgraph.h"

#include <QString>
#include <QByteArray>
#include <map>
#include <list>
#include <algorithm>
//#include <tr1/functional>
#include <functional>
#include <cfloat>


#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/freehand.h"
#include "libdunnartcanvas/textshape.h"
#include "libdunnartcanvas/cluster.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/polygon.h"
#include "libdunnartcanvas/templates.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/utility.h"
#include "libdunnartcanvas/canvas.h"

#ifndef NOGRAPHVIZ
#include "libdunnartcanvas/align3.h"
#endif

#include <libavoid/libavoid.h>
#include <libavoid/viscluster.h>


namespace dunnart {

QXmlNamespaceSupport namespaces;

bool queryMode = false;
CanvasItem *queryObj = NULL;


#if 0
// When a new node is added to the canvas, this function is called
// to check for any constraints that need to be added to the new node
// and/or any other objects already on the canvas.  For example, 
// connecting the new node to an existing node via an edge, or adding
// the new node to a template.
static void addNewNodeConstraints(ShapeObj* toShape)
static void createConnector(ShapeObj* toShape)
{
//printf("*** inside addNewNodeConstraints ***\n");
    Avoid::Point p(mouse.x, mouse.y);
    offsetPoint(p);
    for(QWidget *co = canvas->get_children_head(); co; co=co->get_next())
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (co);
        Template *templatPtr = dynamic_cast<Template *> (co);
        if (shape && shape!=toShape && inPoly(*(shape->shapePoly), p))
        {
            if (!(shape->selectHandle(HAN_CONNPT | HAN_CENTER) &&
                    toShape->selectHandle(HAN_CONNPT | HAN_CENTER)))
            {
                // One of the shapes is missing a center connection point,
                // so do nothing.
                return;
            }

            printf("Over shape\n");
            Conn* newConn = new AvoidingConn(shape,toShape);
            add_undo_record(DELTA_ADD, newConn);
            //add_undo_record(DELTA_CONNS, newConn);

        }
	// if the new Shape overlaps a template, snap to template
        else if (templatPtr) {
		  if(templatPtr->testShapeOverlap(toShape)) {
			printf("###### testShapeOverlap returned true\n");
			templatPtr->snapShapeToNearestGuideline(toShape);
		  }
        }
    }
}

//============================================================================
// Selection code:

void shape_change_highlight_state(QWidget *shape, int highlight_type);
#endif


#if 0
void dragAbortActions(void)
{
    mouse.b = 0;
    
    undo_action(NULL);
    clear_redo_stack();
}


// removeFromCanvas(CanvasItemList& list):
//     Input:   A list of Shape pointers.
//     Result:  Removes those shapes and all resultant dangling connectors
//              from the canvas, and puts a representation for them in the
//              inactiveObjs xml document.
//
void removeFromCanvas(CanvasItemList& list)
{
    assert(&list != &selection);

    // Add to the list, all the attached shapes.
    CanvasItemSet fullSet;
    for (CanvasItemList::iterator curr = list.begin(); curr != list.end(); 
            ++curr)
    {
        (*curr)->findAttachedSet(fullSet);
    }

    // Determine connectors attached to these shapes.
    CanvasItemSet connSet;
    for (CanvasItemSet::iterator curr = fullSet.begin(); curr != fullSet.end(); 
            ++curr)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (*curr);
        if (shape)
        {
            // Add attached connectors.
            ConnMultiset connMultiSet = shape->getConnMultiset();
            connSet.insert(connMultiSet.begin(), connMultiSet.end());
        }
    }
    // Remove the connectors that would be dangling after removing shapes
    // in the list.  Also remove guidelines and relationships.
    for (CanvasItemSet::iterator obj = connSet.begin(); 
            obj != connSet.end(); ++obj)
    {
        (*obj)->setAsInactive(true);
    }
    // Now, remove the shapes and attached objects in the list.
    for (CanvasItemSet::iterator curr = fullSet.begin(); curr != fullSet.end(); 
            ++curr)
    {
        (*curr)->setAsInactive(true, fullSet);
    }
}
#endif

// void returnToCanvas(CanvasItemList& list):
//     Input:   A list of Dunnart shape pointers.
//     Result:  Inactive shapes and their attached connectors are returned
//              to the canvas.
void returnToCanvas(CanvasItemList& list)
{
    Q_UNUSED (list)
#if 0
    // Add to the list, all the attached shapes.
    CanvasItemSet fullSet;
    for (CanvasItemList::iterator curr = list.begin(); curr != list.end(); 
            ++curr)
    {
        (*curr)->findAttachedSet(fullSet);
    }

    // Return the shapes in the list.
    for (CanvasItemSet::iterator curr = fullSet.begin(); curr != fullSet.end(); 
            ++curr)
    {
        if (dynamic_cast<ShapeObj *> (*curr))
        {
            (*curr)->setAsInactive(false);
        }
    }
    for (CanvasItemSet::iterator curr = fullSet.begin(); curr != fullSet.end(); 
            ++curr)
    {
        if (dynamic_cast<Guideline *> (*curr))
        {
            (*curr)->setAsInactive(false);
        }
    }
    // Now, bring back appropriate connectors.
    returnAppropriateConnectors();
#endif
}


#if 0
void selection_makeInactive(QWidget **c)
{
    CanvasItemList selCopy = selection;
    GraphLayout::getInstance()->setInterruptFromDunnart();
    removeFromCanvas(selCopy);
    restart_graph_layout(c);
    repaint_canvas();
}


void selection_returnInactive(QWidget **c)
{
    GraphLayout::getInstance()->setInterruptFromDunnart();
    returnAllInactive();
    restart_graph_layout(c);
}
#endif


#if 0
//============================================================================
void toggle_query_mode(QWidget **c)
{
    if (queryMode == false)
    {
        queryMode = true;
        changeControlState(BUT_QUERY, SDLGui::WIDGET_true);
        if (selection.size() == 1)
        {
            queryObj = dynamic_cast<CanvasItem *> (selection.front());
            if (queryObj)
            {
                // If it's a shape, render the highlight.
                queryObj->update();
            }
        }
        // We might need to highlight straight away.
        if (dynamic_cast<CanvasItem *> (active_obj))
        {
            // Query the shape under the mouse if there is one.
            pair_query(active_obj);
        }
        else if (active_obj)
        {
            // This case copes with us being over a handle.
            // Query the parent of the handle, i.e, the shape.
            if (dynamic_cast<CanvasItem *> (active_obj->get_parent()))
            {
                pair_query(active_obj->get_parent());
            }
        }
    }
    else  // if (queryMode == true)
    {
        changeControlState(BUT_QUERY, SDLGui::WIDGET_false);
        
        bool resetQueryObj = true;
        resetQueryModeIllumination(resetQueryObj);
        queryMode = false;
    }
}


void set_query_mode(const bool setting)
{
    if (queryMode != setting)
    {
        toggle_query_mode(NULL);
    }
}
#endif



#if 0
void relayout_selection(QWidget **c) {
    GraphLayout* gl = GraphLayout::getInstance();
    //gl->runLevel=1;
    gl->setInterruptFromDunnart();
    printf(".Relayout selection!\n");
    if (!selection.empty()) {
        printf("Relayout selection!\n");
        gl->lockUnselectedShapes(c);
        gl->apply(false);
        gl->unlockAll(c);
    } else {
        gl->apply(false);
    }
}


static void fade_settings_callback(QWidget **objectPtr)
{
    int level = (int) placement_aid_base_alpha;

    if (fade_aid_amount == 0)
    {
        // Never faded.
        fade_out_aids = false;
        always_faded_aids = false;
        level = 255;
    }
    else if (fade_aid_amount == 60)
    {
        // Always faded.
        fade_out_aids = false;
        always_faded_aids = true;
    }
    else
    {
        // Variable fade time:
        fade_out_aids = true;
        always_faded_aids = false;
        level = 255;
    }

    for (QWidget *go = canvas->get_children_head(); go; go = go->get_next())
    {
        Indicator *indicator = dynamic_cast<Indicator *> (go);

        if (indicator)
        {
            indicator->setAlpha(level);
            indicator->update();
        }
    }
}


static void redraw_indicators(QWidget **slider)
{
    for (QWidget *go = canvas->get_children_head(); go; go = go->get_next())
    {
        Indicator *indicator = dynamic_cast<Indicator *> (go);

        if (indicator)
        {
            if (always_faded_aids)
            {
                indicator->setAlpha((int) placement_aid_base_alpha);
            }
            indicator->update();
        }
    }
}


static bool repaint_all_indicators(void)
{
    bool changes = false;

    for (QWidget *go = canvas->get_children_head(); go; go = go->get_next())
    {
        Indicator *indicator = dynamic_cast<Indicator *> (go);

        if (indicator)
        {
            if (fade_out_aids) {
                changes |= indicator->reduceAlpha();
            }
            else {
                changes |= indicator->reduceGlow();
            }
            
            if (changes)
            {
                indicator->repaint();
            }
        }
    }
    return changes;
}


int fadeLevelExtraDraw(QPixmap *sur, bool disabled)
{
    int texth = TTF_FontHeight(winStndrd);

    if (!sur)
    {
        return 8;
    }

    int lower = 8;
    int upper = sur->w - 11;
    int pixels = upper - lower + 1;
    for (int x = lower; x <= upper; x++)
    {
        SDL_Rect crect = { x, texth + 2, 1, 10 };
        SDL_SetClipRect(sur, &crect);
        int alpha = (int) (((x - lower) / (double) pixels) * 245);
        boxRGBA(sur, lower, texth + 5, upper, texth + 9,
                0, 0, 255, alpha);
        SDL_SetClipRect(sur, NULL);
    }
    return 0;
}
    

int fadeSpeedExtraDraw(QPixmap *image, bool disabled)
{
    int texth = TTF_FontHeight(winStndrd);

    if (!image)
    {
        return 8;
    }

    int lower = 8;
    int upper = image->w - 11;
    filledTrigonRGBA(image, lower, texth + 9, upper, texth + 3, upper,
            texth + 9, 128, 128, 128, 255);
    aatrigonRGBA(image, lower, texth + 9, upper, texth + 3, upper,
            texth + 9, 128, 128, 128, 255);
    return 0;
}
    

static Widget *indicatorPropertiesDialog = NULL;

static void createIndicatorPropertiesDialog(QWidget **c)
{
    if (indicatorPropertiesDialog)
    {
        indicatorPropertiesDialog->bring_to_front();
        return;
    }

    int sectionOffset = 10;
    int itemOffset = 0;
    int itemBuffer = 4;
    QWidget *item = NULL;

    Window *win = new Window(0, 1, 55 + shboxh, 150, 215,
            "Indicator Visibility");
    win->setGlobalPointer(&indicatorPropertiesDialog);
    
    int winYStart = 35;
    int awidth = win->get_width() - 20;

    itemOffset = winYStart;
    item = new Checkbox(CHK_GLOW, 10, itemOffset, "Glow when active",
            &glow_active_aids, redraw_indicators, win);
    itemOffset += item->get_height() + itemBuffer + sectionOffset;

    item = new TextArea(10, itemOffset, awidth, "Inactive fade speed:", win);
    itemOffset += item->get_height() + itemBuffer;
    Slider *slider = new Slider(10, itemOffset, awidth,
            &fade_aid_amount, 0, 60, fade_settings_callback,
            fadeSpeedExtraDraw, win);
    slider->setLabels("Never", "Instantly");
    slider->setPositions(5);
    itemOffset += slider->get_height() + itemBuffer + sectionOffset;
    
    item = new TextArea(5, itemOffset, awidth, "Minimum Fade Level:", win);
    itemOffset += item->get_height() + itemBuffer;
    slider = new Slider(5, itemOffset, awidth,
            &placement_aid_base_alpha, 0, 255, redraw_indicators,
            fadeLevelExtraDraw, win);
    slider->setLabels("Low", "High");
    slider->setPositions(7);
    itemOffset += slider->get_height() + itemBuffer;
    
}
#endif


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

