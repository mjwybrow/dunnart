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

#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/placement.h"

namespace dunnart {


#if 0
static void clear_change_indicators(void)
{
    for (QWidget *go = canvas->get_children_head(); go; go = go->get_next())
    {
        CanvasObj *co = dynamic_cast<CanvasObj *> (go);
        co->update();
    }
}


#endif


    // Reset illumination associated with cascade-type queries.
void resetQueryModeIllumination(const bool clearQueryObj)
{
    Q_UNUSED (clearQueryObj)

#if 0
    if (!queryMode)
    {
        return;
    }
    bool changes = false;

    // Reset distances on all the canvas obj;
    for (QWidget *go = canvas->get_children_head(); go;
                go = go->get_next())
    {
        CanvasObj *cobj = dynamic_cast<CanvasObj *> (go);

        if (clearQueryObj && queryObj)
        {
            // Causes the querObj to be acted upon.
            queryObj->cascade_glow = true;
            queryObj = NULL;
        }
            
        if (cobj->cascade_glow)
        {
            cobj->distance = -1;
            cobj->cascade_glow = false;
            cobj->connectedObjs[0] = NULL;
            cobj->connectedObjs[1] = NULL;

            cobj->update();
            changes = true;
        }
        else if (cobj->distance != -1)
        {
            cobj->distance = -1;
            cobj->cascade_glow = false;
            cobj->connectedObjs[0] = NULL;
            cobj->connectedObjs[1] = NULL;
        }
    }
    if (changes)
    {
        repaint_canvas();
        SDL_FastFlip(screen);
    }
#endif
}


#if 0
void pair_query(QWidget *curr)
{
    // XXX: Have better support for querying the lead object if it in not
    //      the first canvasObj in the selection.
    if ((selection.size() != 1) || !queryObj)
    {
        return;
    }
    CanvasObj *cobj = dynamic_cast<CanvasObj *> (selection.front());
    if (!cobj)
    {
        // Only query CanvasObjs
    }
    // NOTE: May want to limit this to certain kinds of objects.
    //
    // if (!(dynamic_cast<ShapeObj *> (cobj) && !(dynamic_cast<Guide *> (cobj))))

    CanvasObj *ccurr = dynamic_cast<CanvasObj *> (curr);
    
    CanvasObj *cobjs[50] = { NULL };
    cobjs[0] = ccurr;
    cobj->cascade_distance(0, CASCADE_ALL, cobjs);

    repaint_canvas();
    SDL_FastFlip(screen);
}
#endif


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

