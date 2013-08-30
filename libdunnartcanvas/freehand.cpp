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

#include <climits>
#include <QByteArray>
#include <QTimer>

#include "libavoid/libavoid.h"
using Avoid::Polygon;
using Avoid::Point;

#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/visibility.h"
#include "libdunnartcanvas/freehand.h"

#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/utility.h"


namespace dunnart {


static FreehandShape *lastFreehand = NULL;

void freehand_time_expired(void)
{
    if (lastFreehand == NULL)
    {
        return;
    }

    lastFreehand->finalise();
    lastFreehand = NULL;
}


#if 0
QColor freehand_timer_callback(QColor interval, void *param)
{
    SDLGui::postUserEvent(USEREVENT_FREEHAND_FINISHED, &lastFreehand);
    return 0;
}


bool handle_freehand_shape_click(void)
{
    bool clickStolen = false;

    // User has clicked on a shape.
    if (lastFreehand)
    {
        //QT SDL_SetWindowsCursor(SDL_CUR_NormalSelect);
        //QT statusBar->unsetTempMessage();

        // Timeout hasn't expired, so allow more drawing:
        start_freehand_drawing();
        clickStolen = true;
    }
    return clickStolen;
}


void start_freehand_drawing(void)
{
    FreehandGeometry geometry;

    int oldmouse_x, oldmouse_y;
    int offsetx = cxoff + canvas->get_absxpos();
    int offsety = cyoff + canvas->get_absypos();
    //SDL_Surface *sur = canvas->get_image(canvas->get_active_image_n());

    SDL_Surface *behind = SDLGui::createSurface(screen->w, screen->h);
    blit(screen, behind, 0, 0, 0, 0, screen->w, screen->h);
                
    SDL_Event event;
    oldmouse_x = mouse.x;
    oldmouse_y = mouse.y;
    
    unsigned processed = 0;
    StlStroke& stroke = *(geometry.currStroke);


    while(mouse.b == SDL_BUTTON_LEFT)
    {
        // Update the screen with the last round of geometry read from the 
        // event queue.
        SDL_PumpEvents();
        for (unsigned i = processed; i < stroke.size(); i++)
        {
            if (i == 0)
            {
                //putpixel(screen, stroke[i].x + offsetx,
                //        stroke[i].y + offsety, BLACK);
                continue;
            }

            aalineRGBA(screen, (int) stroke[i - 1].x + offsetx,
                    (int) stroke[i - 1].y + offsety, 
                    (int) stroke[i].x + offsetx,
                    (int) stroke[i].y + offsety, 0, 0, 0, 128);
            processed++;
        }
        SDL_PumpEvents();
        SDL_Flip(screen);

        // Wait for a new event.
        SDL_WaitEvent(&event);
        // Process all events in the queue.
        do
        {
            switch (event.type)
            {
                case SDL_MOUSEMOTION:
                    mouse.x = event.motion.x;
                    mouse.y = event.motion.y;
                    if ((oldmouse_x != mouse.x) || (oldmouse_y != mouse.y))
                    {
                        geometry.addPoint(mouse.x - offsetx, mouse.y - offsety);
                        oldmouse_x = mouse.x;
                        oldmouse_y = mouse.y;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    mouse.b = 0;
                    break;
                case SDL_QUIT:
                    mouse.kill_sig = 1;
                    break;
                default:
                    break;
            }
        }
        while ((mouse.b != 0) && SDL_PollEvent(&event));
    }

    //qDebug("Points: %d", (int) points.size());
    SDL_FastBlit(behind, screen, 0, 0, 0, 0, screen->w, screen->h);
    SDL_FreeSurface(behind);
   
    if (lastFreehand == NULL)
    {
        strokeTimerID = SDL_AddTimer(strokeTimeout, freehand_timer_callback, 
                NULL);
        
        lastFreehand = new Freehand(geometry);
        // Visibility graph stuff:
        visalgo->obj_add(lastFreehand);

        //QT canvas_deselect_shapes();
    }
    else
    {
        SDL_RemoveTimer(strokeTimerID);
        strokeTimerID = SDL_AddTimer(strokeTimeout, freehand_timer_callback, 
                NULL);
        lastFreehand->addStroke(geometry);
        visalgo->obj_resize(lastFreehand);
    }
#if defined(__APPLE__)
    // After using carbon for mouse input, we need to resync the sdl mouse.
    GUI_DummyMouseEvent();
    while(SDL_PollEvent(&event));
        // noop;
    reset_mouse();
    mouse.bstate = mouse.b = 0;
#endif
    repaint_canvas();
}
#endif


//===========================================================================
//  Freehand object code:


FreehandShape::FreehandShape()
    : ShapeObj(x_shFreehand)
{
    _colour = QColor(0,0,0);
}


void FreehandShape::initWithXMLProperties(Canvas *canvas,
        const QDomElement& node, const QString& ns)
{
    // Call equivalent superclass method.
    ShapeObj::initWithXMLProperties(canvas, node, ns);

    // Read in geometry;
    QString value = nodeAttribute(node, ns, x_geometry);
    if (!value.isNull())
    {
        QStringList strings =
                value.split(QRegExp("[, ]"), QString::SkipEmptyParts);
        int stringIndex = 0;

        // Read the number of points.
        int strokes = strings.at(stringIndex++).toInt();

        bool first = true;
        for (int c = 0; c < strokes; ++c)
        {
            int points = strings.at(stringIndex++).toInt();

            if (first == false)
            {
                // XXX: hint vector size?
                _geometry.startNewStroke();
            }

            for (int p = 0; p < points; ++p)
            {
                double x = strings.at(stringIndex++).toDouble();
                double y = strings.at(stringIndex++).toDouble();
                unsigned int t = strings.at(stringIndex++).toUInt();
                _geometry.addPoint(x, y, t);
            }
            first = false;
        }

        double initOffX = strings.at(stringIndex++).toDouble();
        double initOffY = strings.at(stringIndex++).toDouble();
        _geometry.setInitialOffset(initOffX, initOffY);
    }
    setPainterPath(buildPainterPath());
}


// Function is called when a complete gesture is formed.
void FreehandShape::finalise(void)
{

}


void FreehandShape::addStroke(FreehandGeometry& geometry)
{
    _geometry.addStroke(geometry);

    QGraphicsItem::setPos(_geometry.x, _geometry.y);
    //QT set_noimages(_geometry.w, _geometry.h);
    //QT set_active_image(get_active_image_n(), true);
}

QPainterPath FreehandShape::buildPainterPath(void)
{
    QPainterPath path;

    StlStrokeList::iterator sEnd = _geometry.strokes.end();
    for (StlStrokeList::iterator s = _geometry.strokes.begin(); s != sEnd; ++s)
    {
        StlStroke::iterator pBegin = (*s).begin();
        StlStroke::iterator pEnd = (*s).end();
        for (StlStroke::iterator p = pBegin; p != pEnd; ++p)
        {
            if (p == pBegin)
            {
                // First point.
                path.moveTo(QPointF((*p).x, (*p).y));
            }
            else
            {
                path.lineTo(QPointF((*p).x, (*p).y));
            }
        }
    }
    return path;
}


void FreehandShape::setPosition(int x, int y, bool from_cider)
{
    bool store_undo = true;
    bool from_solver = false;
    move_to(x, y, store_undo, from_solver, from_cider);
}


FreehandGeometry& FreehandShape::getGeometry(void)
{
    return _geometry;
}


// Function returns the class ID and associated probability of this
// freehand gesture.
void FreehandShape::cache_mostlikely_class()
{
	StlClassProbabilities::const_iterator iter = class_probs.begin();
	// only search through the probability map if it has been setup by 
	// classification
	if(class_probs.size() > 0)
	{
		// assume the first class has the highest probability
		mostlikely_class_id = iter->first;
		mostlikely_class_prob = iter->second;
		// linearly search through the other classes
		for(iter++; iter != class_probs.end(); iter++)
		{
			// adjust result if a new highest probability is found
			if(iter->second > mostlikely_class_prob)
			{
				mostlikely_class_id = iter->first;
				mostlikely_class_prob = iter->second;
			}
		}
	}
	// the probability map has not been created so set some default values
	else
	{
		mostlikely_class_id = 0;
		mostlikely_class_prob = 0.0;
	}
}

// Function to get the class ID of the most likely prediction.
int FreehandShape::get_mostlikely_id() const
{
	return mostlikely_class_id;
}

// Function to get the probability of the most likely prediction.
StlClassProbabilityT FreehandShape::get_mostlikely_prob() const
{
	return mostlikely_class_prob;
}

// Function to get the classification probability map.
StlClassProbabilities FreehandShape::get_probability_map() const
{
	return class_probs;
}

Avoid::Polygon FreehandShape::polygon(void) const
{
#if 0
    // XXX: Just use a rectangle for the moment.

    double sx = x();
    double sy = y();
    double sw = width_ - 1, sh = height_ - 1;
    
    Polygn *p = poly;
    
    if (p)
    {
        // Reusing poly memory, but clear points.
        std::free(p->ps);
        p->ps = NULL;
    }
    else
    {
        // Not reusing, so alloc memory.
        p = new Polygn();
    }

    if (p)
    {
        p->ps = (Point *) calloc(4, sizeof(Point));
    }
    
    if (!p || !(p->ps))
    {
        qFatal("Couldn't calloc memory in Freehand::poly()");
    }
    
    p->pn = 4;
    p->_id = (int) ID;

    p->ps[0].x = sx + sw + b;
    p->ps[0].y = sy - b;

    p->ps[1].x = sx + sw + b;
    p->ps[1].y = sy + sh + b;

    p->ps[2].x = sx - b;
    p->ps[2].y = sy + sh + b; 

    p->ps[3].x = sx - b;
    p->ps[3].y = sy - b;

    return p;
#endif
    return NULL;
}


void FreehandShape::setColour(unsigned int r, unsigned int g, unsigned int b)
{
    _colour = QColor(qMin(r, (unsigned)255), qMin(g, (unsigned)255), qMin(b, (unsigned)255));

    //bool regen_cache = true;
    //QT set_active_image(get_active_image_n(), regen_cache);
    update();
}


//===========================================================================
// Stroke geometry code;


FreehandGeometry::FreehandGeometry()
    :currStroke(NULL)
    ,x(0)
    ,y(0)
    ,w(0)
    ,h(0)
    ,startTime(0)
    ,initialOffsetX(0.0)
    ,initialOffsetY(0.0)
{
    startNewStroke();
}


void FreehandGeometry::setInitialOffset(double x, double y)
{
    initialOffsetX = x;
    initialOffsetY = y;
}


void FreehandGeometry::addPoint(double x, double y)
{
    unsigned int thisTime = 0; //QT SDL_GetTicks();
	addPoint(x, y, thisTime);
}


void FreehandGeometry::addPoint(double x, double y, unsigned int t)
{
    if (startTime == 0)
    {
        startTime = t;
    }
    float pressure = 0.0;
    currStroke->push_back(StlStrokePoint(x, y, pressure, t - startTime));
}


void FreehandGeometry::startNewStroke(void)
{
    strokes.push_back(StlStroke());
    currStroke = &(strokes.back());
}


void FreehandGeometry::removeOffset(void)
{
    calcBBox();

    // Adjust all points to be relative to the bounding box.
    StlStrokeList::iterator sEnd = strokes.end();
    for (StlStrokeList::iterator s = strokes.begin(); s != sEnd; ++s)
    {
        StlStroke::iterator pEnd = (*s).end();
        for (StlStroke::iterator p = (*s).begin(); p != pEnd; ++p)
        {
            (*p).x -= x;
            (*p).y -= y;
        }
    }
}


void FreehandGeometry::calcBBox(void)
{
    int xmin = INT_MAX;
    int xmax = INT_MIN;
    int ymin = INT_MAX;
    int ymax = INT_MIN;

    // Find the bounding box.
    StlStrokeList::iterator sEnd = strokes.end();
    for (StlStrokeList::iterator s = strokes.begin(); s != sEnd; ++s)
    {
        StlStroke::iterator pEnd = (*s).end();
        for (StlStroke::iterator p = (*s).begin(); p != pEnd; ++p)
        {
            xmin = qMin(xmin, (int) (*p).x);
            xmax = qMax(xmax, (int) (*p).x);
            ymin = qMin(ymin, (int) (*p).y);
            ymax = qMax(ymax, (int) (*p).y);
        }
    }

    x = xmin;
    y = ymin;
    w = xmax - xmin + 1;
    h = ymax - ymin + 1;
}


void FreehandGeometry::addStroke(const FreehandGeometry& geometry)
{
    // Adjust all points to be absolute.
    StlStrokeList::iterator sEnd = strokes.end();
    for (StlStrokeList::iterator s = strokes.begin(); s != sEnd; ++s)
    {
        StlStroke::iterator pEnd = (*s).end();
        for (StlStroke::iterator p = (*s).begin(); p != pEnd; ++p)
        {
            (*p).x += x;
            (*p).y += y;
        }
    }

    assert(geometry.startTime > startTime);
    // Find the bounding box.
    StlStrokeList::const_iterator scEnd = geometry.strokes.end();
    for (StlStrokeList::const_iterator s = geometry.strokes.begin(); 
            s != scEnd; ++s)
    {
        // Copy the stroke.
        startNewStroke();
        *currStroke = (*s);

        // Adjust times of later strokes to be timed from the start of
        // the first stroke, rather that the start of the current stroke.
        StlStroke::iterator pEnd = currStroke->end();
        for (StlStroke::iterator p = currStroke->begin(); p != pEnd; ++p)
        {
            (*p).milliTime += (geometry.startTime - startTime);
        }
    }

    // Calculate bbox and adjust points to be relative to that.
    removeOffset();
    setInitialOffset(x, y);
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

