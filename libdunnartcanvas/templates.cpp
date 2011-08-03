/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2009-2010  Monash University
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
 * Author(s): Sarah Boyd
*/

#include <QRectF>
#include <QGraphicsSceneMouseEvent>

#include <cfloat>
#include <cassert>

#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/templates.h"
#include "libdunnartcanvas/undo.h"

namespace dunnart {


#if 0
static void add_left_branch(GuiObj **c) { 
  BranchedTemplate *bt = (BranchedTemplate*) (*c)->get_parent()->get_ident();
  bt->add_new_left_branch((int) bt->guidelist.front()->getPos(), 50,
                          bt->centreguide->get_dir());
  interrupt_graph_layout(NULL);
  repaint_canvas();
}

static void add_right_branch(GuiObj **c) { 
  BranchedTemplate *bt = (BranchedTemplate*) (*c)->get_parent()->get_ident();
  bt->add_new_right_branch((int) bt->guidelist.back()->getPos(), 50,
                          bt->centreguide->get_dir());
  interrupt_graph_layout(NULL);
  repaint_canvas();  
}
#endif

class TemplateHandle : public Handle{
public:
    TemplateHandle(Template *templ, int hflags, double xr,
            double xo, double yr, double yo) :
        Handle(templ, hflags, xr, xo, yr, yo)
    {
        if (hflags & (HAN_LEFT | HAN_RIGHT))
        {
            setCursor(Qt::SizeHorCursor);
        }
        else
        {
            setCursor(Qt::SizeVerCursor);
        }
        setHoverMessage("Template Handle - Click and drag to resize template.");
    }
protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        Q_UNUSED (event)

        Template *templ = dynamic_cast<Template *> (parentItem());
        templ->startResize();
    }

    virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        Template *templ = dynamic_cast<Template *> (parentItem());
        QPointF newPos = event->scenePos();
        QPointF lastPos = event->lastScenePos();
        double diffx = newPos.x() - lastPos.x();
        double diffy = newPos.y() - lastPos.y();
        templ->resize(diffx, diffy);
//        templ->resize();
    }
    virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event)
    {
        Q_UNUSED (event)
    }
    virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
    {
        Q_UNUSED (event)
    }
};

Template::Template()
    : Indicator(ZORD_Template)
{
    initialise();
}


void Template::initialise(void)
{
    // Hightlight dark red pen for active template outline
    active_template_pen_.setColor(QColor(255, 150, 150, 65));
    active_template_pen_.setWidth(5);
    active_template_pen_.setCosmetic(true);

    // Lighter red transparent pen for filling the template region;
    template_pen_.setColor(QColor(255, 150, 150, 135));
    template_pen_.setWidth(1);
    template_pen_.setCosmetic(true);

    // Fill colour for the template:
    template_fill_ = QBrush(QColor(255, 150, 150, 135));
}

void Template::cascade_distance(int dist, unsigned int dir, CanvasItem **path)
{
    Q_UNUSED (dist)
    Q_UNUSED (dir)
    Q_UNUSED (path)

    // Do nothing.
    return;
}


void Template::deactivateAll(CanvasItemSet&)
{
    return;
}


void Template::move_diff(int diff_x, int diff_y, bool store_undo)
{
    Q_UNUSED (diff_x)
    Q_UNUSED (diff_y)
    Q_UNUSED (store_undo)
    abort();
    // QT move_to(xpos + diff_x, ypos + diff_y, store_undo);
}


/** QT void Template::handler(GuiObj **object_addr, int action)
{
    GuiObj *object = *object_addr;
    Template *templtPtr = dynamic_cast<Template *> (object);

    //Template *templt = dynamic_cast<Template *> (object);
    
    switch (action)
    {
    case MOUSE_OVER: 
        SDL_SetWindowsCursor(SDL_CUR_Move);
        statusBar->setTempMessage("Template");
        break;
    case MOUSE_LEAVE: 
        SDL_SetWindowsCursor(SDL_CUR_NormalSelect);
        statusBar->unsetTempMessage();
        break;
#if 0
#if defined(__APPLE__)
    case MOUSE_MCLICK:
        // Apple counts Left click and Alt as a Middle click.
        if (!(OurGetModState() & KMOD_ALT))
        {
            break;
        }
        // Fallthrough.
#endif
#endif
    case MOUSE_LCLICK:
        selection_object_lclick(object);
        break;
    case MOUSE_RCLICK:
        repaint_canvas();
        templtPtr->displayContextMenu();
        break;
    default:
        break;
    }
}
**/

// calculate whether a Shape object overlaps with the template
bool Template::testShapeOverlap(ShapeObj *shape)
{
    return boundingRect().intersects(shape->boundingRect());
}

void Template::snapShapeToNearestGuideline(ShapeObj *shape) {
    Q_UNUSED (shape)
}

/** QT void Template::resize_handler(GuiObj **object_addr, int action)
{
    GuiObj *object = *object_addr;
    Handle *handle = dynamic_cast<Handle *> (object);
    Template *templtPtr = 
            dynamic_cast<Template *> (object->get_parent());
    
    int hflags = handle->get_ident();
    hflags &= (HAN_TOP | HAN_BOT | HAN_RIGHT | HAN_LEFT);
    
    switch (action)
    {
    case MOUSE_OVER: 
        switch (hflags)
        {
            case (HAN_TOP | HAN_LEFT):
            case (HAN_BOT | HAN_RIGHT):
                SDL_SetWindowsCursor(SDL_CUR_DiagonalResize1);
                break;
            case (HAN_TOP | HAN_RIGHT):
            case (HAN_BOT | HAN_LEFT):
                SDL_SetWindowsCursor(SDL_CUR_DiagonalResize2);
                break;
            case HAN_TOP:
            case HAN_BOT:
                SDL_SetWindowsCursor(SDL_CUR_VerticalResize);
                break;
            case HAN_RIGHT:
            case HAN_LEFT:
                SDL_SetWindowsCursor(SDL_CUR_HorizontalResize);
                break;
            default:
                break;
        }
        statusBar->setTempMessage("Template Handle - Click "
                                  "and drag to resize template.");
        break;
    case MOUSE_LEAVE: 
        statusBar->setTempMessage("Editing Template ...");
        break;
    case MOUSE_RCLICK:
    case MOUSE_MCLICK:
        break;
    case MOUSE_LCLICK:
    {
        // Center mouse on handle
        SDL_WarpMouse(object->get_absxpos() + HANDLE_PADDING,
                      object->get_absypos() + HANDLE_PADDING);
       
        ForceEventHandling();
        int ox = templtPtr->get_absxpos(), oy = templtPtr->get_absypos();
        int ow = templtPtr->get_width(), oh = templtPtr->get_height();
        int nx = ox, nw = ow, ny = oy, nh = oh;
        int lx = ox, lw = ow, ly = oy, lh = oh;
        int oldmouse_x, oldmouse_y;
      
        double effects[4] = {0};
        switch (hflags)
        {
        case (HAN_TOP | HAN_LEFT):
            effects[HAND_EFF_X] =  1;
            effects[HAND_EFF_W] = -1;
            effects[HAND_EFF_Y] =  1;
            effects[HAND_EFF_H] = -2;
            break;
        case (HAN_BOT | HAN_RIGHT):
            effects[0] =  0;
            effects[1] =  1;
            effects[2] =  0;
            effects[3] =  2;
            break;
        case (HAN_TOP | HAN_RIGHT):
            effects[0] =  0;
            effects[1] =  1;
            effects[2] =  1;
            effects[3] = -2;
            break;
        case (HAN_BOT | HAN_LEFT):
            effects[0] =  1;
            effects[1] = -1;
            effects[2] =  0;
            effects[3] =  2;
            break;
        case HAN_TOP:
            effects[0] =  0;
            effects[1] =  0;
            effects[2] =  1;
            effects[3] = -1;
            break;
        case HAN_BOT:
            effects[0] =  0;
            effects[1] =  0;
            effects[2] =  0;
            effects[3] =  1;
            break;
        case HAN_RIGHT:
            effects[0] =  0;
            effects[1] =  1;
            effects[2] =  0;
            effects[3] =  0;
            break;
        case HAN_LEFT:
            effects[0] =  1;
            effects[1] = -1;
            effects[2] =  0;
            effects[3] =  0;
            break;
        default:
            break;
        }

        int mouse_strt_x = mouse.x;
        int mouse_strt_y = mouse.y;
        int offset_x = 0, old_offset_x;
        int offset_y = 0, old_offset_y;

        int min_x = 5, max_x = screen->w - 6;
        int min_y = 5, max_y = screen->h - 6;
        
        // Window limits.
        min_x = 5;
        min_y = 5;
        max_x = screen->w - 6;
        max_y = screen->h - 6;
        
        int min_width, min_height;
        templtPtr->recalc_min_width_and_height(min_width, min_height);

        // Prevent shape from ever getting too small.
        if (hflags & HAN_TOP)
        {
            // Top handle active.
            max_y = oy + min_height - 5;
        }
        else if (hflags & HAN_BOT)
        {
            // Bottom handle active.
            min_y = oy + min_height - 5;
        }
        
        if (hflags & HAN_LEFT)
        {
            // Left handle active.
            max_x = ox + ow - min_width + 4;
        }
        else if (hflags & HAN_RIGHT)
        {
            // Right handle active.
            min_x = ox + min_width - 5;
        }
        
        // Make these obscure values to trigger the redraw first time.
        old_offset_y = old_offset_x = INT_MIN;
            
        templtPtr->startResize();
        dragInProgress = true;
        dragAborted = false;
        while ((mouse.b == SDL_BUTTON_LEFT) && !dragAborted)
        {
            // So we can't resize the shape to have a negative
            // width or height, or to have a border outside the
            // canvas.
            if (mouse.y < min_y)
            {
                offset_y = min_y - mouse_strt_y;
            }
            else if (mouse.y > max_y)
            {
                offset_y = max_y - mouse_strt_y;
            }
            else
            {
                offset_y = mouse.y - mouse_strt_y;
            }

            if (mouse.x < min_x)
            {
                offset_x = min_x - mouse_strt_x;
            }
            else if (mouse.x > max_x)
            {
                offset_x = max_x - mouse_strt_x;
            }
            else
            {
                offset_x = mouse.x - mouse_strt_x;
            }

            oldmouse_x = mouse.x;
            oldmouse_y = mouse.y;
            if ((offset_y != old_offset_y) ||
                (offset_x != old_offset_x))
            {
                old_offset_x = offset_x;
                old_offset_y = offset_y;

                // Last shape values.
                lx = nx;
                lw = nw;
                ly = ny;
                lh = nh;
                
                // Prevent jitter:
                offset_x -= (offset_x % 2);
                offset_y -= (offset_y % 2);
                
                // New dimension/location values.
                nx = ox + (int) (offset_x * effects[HAND_EFF_X]);
                ny = oy + (int) (offset_y * effects[HAND_EFF_Y]);
                nw = ow + (int) (offset_x * effects[HAND_EFF_W]);
                nh = oh + (int) (offset_y * effects[HAND_EFF_H]);

                int relx = nx - canvas->get_offabsxpos();
                int rely = ny - canvas->get_offabsypos();
                
                templtPtr->resize(relx, rely, nw, nh);

                stop_graph_layout();
                do_response_tasks();
            }

            while ((mouse.b == SDL_BUTTON_LEFT) &&
                   (oldmouse_x == mouse.x) &&
                   (oldmouse_y == mouse.y) && !dragAborted)
            {
                wait_mouse();
            }
            old_offset_y = offset_y;
            old_offset_x = offset_x;
        }
        dragInProgress = false;
      
        if (dragAborted)
        {
            dragAbortActions();
            break;
        }
        break;
    }
    default:
        break;
    }
}
**/

//============================================================================
// LinearTemplate


LinearTemplate::LinearTemplate(const double xpos, const double ypos, int orientation, Canvas *canvas)
    : Template(),
      _explicitNewBranchSpan(0)
{
    //    setPos(xpos, ypos);

    setHoverMessage("Linear Template \"%1\" - Click and drag to move template.");

    if (orientation == TEMPLATE_LINEAR_VERT) {
        // default size if linear template is vertical
//        setSize(20, 150);
        
        guide = new Guideline(GUIDE_TYPE_VERT, xpos);
        canvas->addItem(guide);
        Indicator::setPos(150, guide->position());

#if 0
        //QT
        // create handles for template:
        left_handle = new TemplateHandle(this, HAN_HANDLE | HAN_LEFT,
                                         HPLACE_MIN, 4, HPLACE_ZERO, 0);
        left_handle->set_handler(Template::resize_handler);
        right_handle = new Handle(this, HAN_HANDLE | HAN_BOT,
                                  HPLACE_50, 6, HPLACE_100, -5);
        right_handle->set_handler(Template::resize_handler);
#endif
    }
    else {     // orientation == TEMPLATE_LINEAR_HORI
        guide = new Guideline(GUIDE_TYPE_HORI, ypos);
        canvas->addItem(guide);
        Indicator::setSize(TEMPLATE_LENGTH, TEMPLATE_THICKNESS);
        Indicator::setPos(150, guide->position());

        // QT
        // create handles for template:
        left_handle = new TemplateHandle(this, HAN_HANDLE | HAN_LEFT,
                                         HPLACE_MIN, 0, HPLACE_ZERO, 5);
        left_handle->setVisible(true);
        right_handle = new TemplateHandle(this, HAN_HANDLE | HAN_RIGHT,
                                  HPLACE_MAX, 0, HPLACE_ZERO, 5);
        right_handle->setVisible(true);
    }
}


LinearTemplate::~LinearTemplate()
{
}

void LinearTemplate::resize(const double newx, const double newy)
{
    // QT
    printf("LinearTemplate::resize %f ... ???\n", newx);
    fflush(stdout);

    double new_width = width() - newx;
    double new_height = height() - newy;
    Indicator::setSize(new_width, new_height);

//    double diff_x, diff_y;
//    if (guide->get_dir() == TEMPLATE_LINEAR_VERT) {
//        diff_x = 0;
//        diff_y = (y() - newy)/2;
//    }
//    else if (guide->get_dir() == TEMPLATE_LINEAR_HORI) {
//        diff_x = (x() - newx)/2;
//        diff_y = 0;
//    }

    left_handle->reposition();
    right_handle->reposition();

    Actions& actions = canvas()->getActions();
    actions.moveList.push_back(this);

    CanvasItemSet attachedShapes;
    guide->addAttachedShapesToSet(attachedShapes);
    if (!attachedShapes.empty())
    {
        ShapeObj *shape = (ShapeObj *) *(attachedShapes.begin());
        shape->moveBy(newx/2, newy/2);
        actions.moveList.push_back(shape);
    }
    else
    {
        actions.moveList.push_back(guide);
    }

    // Move the template, though don't alert the solver -- it will likely
    // re-move the border slightly based on the component shape positions
    // after solving.
    //Indicator::moveBy(newx/2, newy/2);

    // Set new branch span.  The code in graphdata.cpp sees this and uses 
    // it instead of the span computed from the current branch shape positions.
    double minAttached, maxAttached;
    int nodes;
    guide->rangeOfAttachedShapes(minAttached, maxAttached, nodes);
    // We use a value computed as the difference from the width when resizing
    // started.  It is not safe to use the current span/width, incase the 
    // solver is lagging behind the resize dragging.
//    _explicitNewBranchSpan =
//            ((beginResizeRange + (newx - beginResizeW)) / (nodes-1));
    canvas()->stop_graph_layout();
    _explicitNewBranchSpan =
            (new_width / (nodes-1));
//    _explicitNewBranchSpan =
//            ((beginResizeRange + ((diff_y*2) - beginResizeH)) / (nodes-1));
    canvas()->restart_graph_layout();
}

void LinearTemplate::startResize(void)
{
    printf("LinearTemplate::startResize ... ???\n");
    fflush(stdout);

    beginResizeW = width();
    beginResizeH = height();
    double minAttached, maxAttached;
    int nodes;
    guide->rangeOfAttachedShapes(minAttached, maxAttached, nodes);
    beginResizeRange = maxAttached - minAttached;
}

// Need this code because moving a shape or the guidline will handle
// most of the constraints 
void LinearTemplate::userMoveBy(qreal diff_x, qreal diff_y)
{
    // XXX Rework this to give the template a position variable constrained
    //     to the guideline position and have the template position get
    //     updated by the move action.
    moveBy(diff_x, diff_y);

    // Move the contained shapes.
    Actions& actions = canvas()->getActions();

    CanvasItemSet attachedShapes;
    guide->addAttachedShapesToSet(attachedShapes);

    if (!attachedShapes.empty())
    {
        // Moving one of the attached shapes, will move the whole set on the
        // current guide, since they are constrained to be spaced apart.
        // Move first shape on each guide in this list
        CanvasItem *shape = *(attachedShapes.begin());
        actions.moveList.push_back(shape);
        shape->moveBy(diff_x, diff_y);
    }
    else
    {
        // Move the guide since there are no attached shapes.
        actions.moveList.push_back(guide);
        guide->moveBy(diff_x, diff_y);
    }

}

template <typename T>
QRectF getAttachedObjectsBounds(Indicator *ind, double buffer,
        size_t *objectCount = NULL)
{
    size_t count = 0;
    QRectF bounds;

    for (RelsList::iterator curr = ind->rels.begin();
            curr != ind->rels.end(); ++curr)
    {
        CanvasItem *other = NULL;
        if ((*curr)->shape) other = (*curr)->shape;
        if ((*curr)->distro) other =  (*curr)->distro;
        if ((*curr)->separation) other = (*curr)->separation;

        CanvasItem *correctType = dynamic_cast<T> (other);

        if (correctType && (other != ind))
        {
            ++count;
            QRectF rect = correctType->boundingRect();
            rect.moveCenter(correctType->pos());

            bounds = bounds.united(rect);
        }
    }

    bounds.adjust(-buffer, -buffer, buffer, buffer);

    if (objectCount)
    {
        *objectCount = count;
    }

    return bounds;
}


// This is called by the layout engine (constraint solver) and updates the
// position of the template when the component shapes or guideline are moved.
void LinearTemplate::updatePositionFromSolver(const double pos, const bool store_undo)
{
    Q_UNUSED (store_undo)

    double tWidth, tHeight;
    size_t count;
    QRectF bounds = getAttachedObjectsBounds<ShapeObj *>(guide, TEMPLATE_PADDING, &count);

    // Existing position is the default.
    double xpos = x(), ypos = y();

    if (guide->get_dir() == GUIDE_TYPE_VERT)
    {
        // If shapes are attached, use them to set y-pos and height (*vertical* template):
        if (count > 0)
        {
            tHeight = bounds.height();
            tWidth = bounds.width();
            ypos = bounds.center().y();
        }
        else  // No shapes attached, use default length/thickness for height/width
        {
            tHeight = TEMPLATE_LENGTH;
            tWidth = TEMPLATE_THICKNESS;
        }
        xpos = pos;
    }
    else // Case = Horizontal template ...
    {
        // If shapes are attached, use them to set x-pos and width (*horizontal* template):
        if (count > 0)
        {
            tHeight = bounds.height();
            tWidth = std::max(bounds.width(), TEMPLATE_LENGTH);
            xpos = bounds.center().x();
        }
        else // No shapes attached, use default length/thickness for width/height
        {
            tWidth = TEMPLATE_LENGTH;
            tHeight = TEMPLATE_THICKNESS;
        }
        ypos = pos;
    }

    Indicator::setSize(tWidth, tHeight);
    Indicator::setPos(xpos, ypos);
    
    // QT left_handle->reposition();
    // QT right_handle->reposition();
}


double LinearTemplate::getPos(void) const
{
    return guide->position();
}


QPainterPath LinearTemplate::shape() const
{
    return painterPath();
}

QPainterPath LinearTemplate::buildPainterPath(void)
{
    QPainterPath painter_path;
    if (guide->get_dir() == GUIDE_TYPE_VERT)
    {
//        painter_path.moveTo(0, -(guide->height()) / 2);
//        painter_path.lineTo(0, (guide->height()) / 2);
    }
    else
    {
        QRectF guideBounds(-width() / 2, -height() / 2, width(), +height());
        painter_path.addRect(guideBounds);
    }
    return painter_path;
}

void LinearTemplate::paint(QPainter *painter,
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED (option)
    Q_UNUSED (widget)
    assert(painter->isActive());

    // First fill the glyph
//    printf("painting ... ???\n");
//    fflush(stdout);

    setPainterPath(this->buildPainterPath());
    left_handle->reposition();
    if (isSelected())
    {
        // Draw selection cue.
        painter->setPen(active_template_pen_);
        painter->drawPath(painterPath());
    }
    painter->setPen(template_pen_);
    painter->setBrush(template_fill_);
    painter->drawPath(painterPath());
}


#if 0
// QT
void LinearTemplate::handler(GuiObj **object_addr, int action)
{
    switch (action)
    {

    case MOUSE_LCLICK:

        GuiObj *object = *object_addr;
        LinearTemplate *templt = dynamic_cast<LinearTemplate *> (object);
        templt->flipGuideOrientation();
        repaint_canvas();

        break;
    }
}

// change the orientation of the guideline from either horizontal->vertical,
// or vertical->horizontal
void LinearTemplate::flipGuideOrientation(void) {
    double minAttached, maxAttached;
    int nodes;
    guide->attachedRange(minAttached, maxAttached, nodes);
    double separation = (maxAttached - minAttached) / (nodes - 1);
    
    CanvasItemSet objSet;
    guide->findAttachedSet(objSet);
    guide->deactivateAll(objSet);
    dirctn old_direction = guide->get_dir();
    
    Guideline *old_guide = guide;
    
    // use these to nudge the nodes appropriately in the x or y direction so
    // that non-overlapping of nodes can be satisfied (otherwise the nodes are
    // directly on top of each other, and cannot be separated).
    int nudge_x = 0;
    int nudge_y = 0;

    // new guide is horizontal: nodes need to be nudged in the x-direction
    if (old_direction == GUIDE_TYPE_VERT) {
        nudge_x += (int)(separation); 
        nudge_y = (int)(-separation);
    }
    // new guide is horizontal: nodes need to be nudged in the y-direction
    else {
        nudge_y += (int)(separation);
        nudge_x = (int)(-separation);
    }

    int shift_x = 0;
    int shift_y = 0;
 
    // reassign the relationships for all the old shapes accordingly
    for (CanvasItemSet::iterator curr = objSet.begin(); curr != objSet.end();
         ++curr) {
        if (dynamic_cast<ShapeObj *> (*curr)) {
      
            // now nudge the shape to help solve overlapping nodes
            ((ShapeObj*)*curr)->move_diff(shift_x, shift_y, true);
            shift_x += nudge_x;
            shift_y += nudge_y;
        }
    }
  
    delete(old_guide);

    // create a list of the objects
    CanvasItemList objList;
    for (CanvasItemSet::iterator curr = objSet.begin(); curr != objSet.end(); ++curr) {
        if (dynamic_cast<ShapeObj *> (*curr)) {
            objList.push_back((ShapeObj*)*curr);
        }
    }
    canvas_deselect_shapes();

    // new guide will be horizontally aligned if old guide was vertical
    if (old_direction == GUIDE_TYPE_VERT) {
        guide = createAlignment(ALIGN_MIDDLE, objList, true);
    } else if(old_direction == GUIDE_TYPE_HORI) {
        guide = createAlignment(ALIGN_CENTER, objList, true);
    }

    // Get the graph layout solver to notice the change in constraints.
    interrupt_graph_layout(NULL);
}
#endif

void LinearTemplate::snapShapeToNearestGuideline(ShapeObj *shape)
{
    // attach the middle handles of the shape to the nearest guide
    if (guide->get_dir() == GUIDE_TYPE_VERT) {
        new Relationship(guide, shape, ALIGN_CENTER);
    }
    else {
        new Relationship(guide, shape, ALIGN_MIDDLE);
    }
    canvas()->interrupt_graph_layout();
}


//============================================================================
// Branched Template

BranchedTemplate::BranchedTemplate(const double xpos, const double ypos)
    : Template(),
      _explicitNewBranchSpan(0)
{
    Indicator::setPos(xpos, ypos);
     
//    set_handler(BranchedTemplate::handler);
    // QT set_handler(Template::handler);

//    guide = new Guide(GUIDE_TYPE_VERT, xpos + (width / 2) - HANDLE_PADDING);

    // Branched template has a minimum of three guides: centre, left and right
    double centreguidepos = ypos + (height() / 2);
    centreguide = new Guideline(GUIDE_TYPE_HORI, centreguidepos - HANDLE_PADDING);
    guidelist.push_back(centreguide);

    add_new_left_branch(centreguidepos, 50, GUIDE_TYPE_HORI);
    add_new_right_branch(centreguidepos, 50, GUIDE_TYPE_HORI);

    // default size if branched template is horizontal
    double newW, newH;
    recalc_min_width_and_height(newW, newH);
    setSize(newW, newH);
//     width = width;
//     height = 
//         (int)((Guide*) guidelist.back())->getPos() -
//         (int)((Guide*) guidelist.front())->getPos() +
//         30;

#if 0
    // QT
    // create handles for template:
    left_handle = new Handle(this, HAN_HANDLE | HAN_TOP | HAN_LEFT,
                              HPLACE_0, 0, HPLACE_0, 0);
    left_handle->set_handler(Template::resize_handler);
    right_handle = new Handle(this, HAN_HANDLE | HAN_TOP | HAN_RIGHT,
                             HPLACE_100, -5, HPLACE_0, 0);
    right_handle->set_handler(Template::resize_handler);
#endif
}


BranchedTemplate::~BranchedTemplate()
{
}

// create a new left branch guideline
void BranchedTemplate::add_new_left_branch(double ypos, double offset, dirctn guide_dir) {
    GuidelineList gl;  // for the distro between new guide and current left-most guide
    // left branch offest by 50
    Guideline* g = new Guideline(guide_dir, ypos - offset - HANDLE_PADDING);
    gl.push_front(guidelist.front());
    guidelist.push_front(g);
    gl.push_front(g);
    distro_list.push_front(new Distribution(&gl));
}

// create a new left branch guideline
void BranchedTemplate::add_new_right_branch(double ypos, double offset, dirctn guide_dir) {
    GuidelineList gl;  // for the distro between new guide and current right-most guide
    // right guide offest by 50
    Guideline* g = new Guideline(guide_dir, ypos + offset - HANDLE_PADDING);
    gl.push_front(guidelist.back());
    guidelist.push_back(g);
    gl.push_back(g);
    distro_list.push_front(new Distribution(&gl));
}

QPainterPath BranchedTemplate::buildPainterPath(void)
{
    QPainterPath painter_path;
    painter_path.addRect(-width() / 2, -height() / 2, width(), height());
    return painter_path;
}

void BranchedTemplate::move_diff(int diff_x, int diff_y, bool store_undo)
{
    Q_UNUSED (store_undo)

    // first move the template:
    Indicator::setPos(x() + diff_x, y() + diff_y);

    // For each guide, get the list of attached shapes; if shapes are attached,
    // move the first shape, which will move the rest, if there are no shapes,
    // move the first guide, which will move the rest
    bool found_shapes = false;
    for (GuidelineList::iterator curr = guidelist.begin(); curr != guidelist.end();
         ++curr) {
        CanvasItemSet attachedShapes;
        if (dynamic_cast<Guideline *> (*curr)) {
            (*curr)->addAttachedShapesToSet(attachedShapes);
        }
    
        // Moving one of the attached shapes, will move the whole set on the
        // current guide, since they are constrained to be spaced apart.
        // Move first shape on each guide in this list
        if (!attachedShapes.empty()) {
            ShapeObj *shape = (ShapeObj *) *(attachedShapes.begin());
            shape->setCentrePos(pos() + QPointF(diff_x, diff_y));
            found_shapes = true;
        }
    }
  
    // no shapes found, move the first guide, which should move the rest
    if (!found_shapes) {
        if (Guideline* g = guidelist.front()) {
            g->moveBy(diff_x, diff_y);
        }
    }
}

void BranchedTemplate::startResize(void)
{
    beginResizeW = width();
    beginResizeH = height();
    beginResizeRange = currentBranchSpan();
}

#if 0
// QT
void BranchedTemplate::handler(GuiObj **object_addr, int action)
{
}

void BranchedTemplate::addContextMenuItems(MenuItems& items)
{
    items.push_back(MenuSeparator());
    items.push_back(
            MenuItem(BUT_TYP_Button, BUT_GENERIC, "Add top branch", "",
                     NULL, add_left_branch));
    items.push_back(
            MenuItem(BUT_TYP_Button, BUT_GENERIC, "Add bottom branch", "",
                    NULL, add_right_branch));
}
#endif


double BranchedTemplate::currentBranchSpan(void)
{
    double sep = 0;
    for (GuidelineList::iterator curr = guidelist.begin();
            curr != guidelist.end(); ++curr) 
    {
        Guideline* guide = (*curr);
        double minAttached, maxAttached;
        int nodes;
        guide->rangeOfAttachedShapes(minAttached, maxAttached, nodes);

        if (guide != centreguide) 
        {
            if ((maxAttached - minAttached) > sep) 
            {
                sep = (maxAttached - minAttached);
            }
        }
    }
    return sep;
}

void BranchedTemplate::resize(const double newx, const double newy)
{
    Q_UNUSED (newx)
    Q_UNUSED (newy)

#if 0
    // QT
    int diff_x = nx - (int)xpos;

    width = nw;
    height = nh;

    left_handle->reposition();
    right_handle->reposition();

    // reset the distance between the guides:
    double new_sep = (height - 30)/(distro_list.size());
    for (DistroList::iterator curr_distro = distro_list.begin(); 
         curr_distro != distro_list.end(); curr_distro++) {
        (*curr_distro)->setSeparation(new_sep);
    }

    // Lock the branch-point node in place.  It makes most sense to resize
    // around this one.  It is the first guide on the centre guideline.
    // If it doesn't exist, then lock the centre guideline.  This will exist.
    Actions& actions = getActions();
    actions.flags |= ACTION_MOVES;
    CanvasItemSet attachedShapes;
    centreguide->addAttachedShapesToSet(attachedShapes);
    if (!attachedShapes.empty())
    {
        ShapeObj *shape = (ShapeObj *) *(attachedShapes.begin());
        shape->move_diff(diff_x, 0, true);
    }
    else
    {
        actions.movList.push_back(centreguide);
    }

    // Move the template, though don't alert the solver -- it will likely
    // re-move the border slightly based on the component shape positions
    // after solving.
    bool layout_notice_move = false;

    move_to(nx, nw, layout_notice_move);

    // Set new branch span.  The code in graphdata.cpp sees this and uses 
    // it instead of the span computed from the current branch shape positions.
    // We use a value computed as the difference from the width when resizing
    // started.  It is not safe to use the current span/width, incase the 
    // solver is lagging behind the resize dragging.
    _explicitNewBranchSpan = beginResizeRange + (nw - beginResizeW);
#endif
}


// Take two ints by reference as input, calculate the minimum width and
// height, values will be available from calling location
void BranchedTemplate::recalc_min_width_and_height(double &min_width,
                                                   double &min_height) {
    // recalculate min width and height:
    size_t num_vert_nodes = 0;
    size_t num_horiz_nodes = 0;
    size_t max_horiz_nodes = 0;
    for (GuidelineList::iterator curr = guidelist.begin();
         curr != guidelist.end(); ++curr) 
    {
        size_t gcount = 0; // number of shapes on the guide
        
        // calculate the bounds based on all the shapes attached to this guide:
        double buffer = 15;
        QRectF bounds = getAttachedObjectsBounds<ShapeObj *>(*curr, buffer, &gcount);
        if (gcount > 0) 
        {
            num_vert_nodes++;
            if (*curr == centreguide) { num_horiz_nodes += std::min(2, (int)gcount); }
            else if (gcount > max_horiz_nodes) { max_horiz_nodes = gcount; }
        }
    }
    num_horiz_nodes += max_horiz_nodes;

    min_height = (guidelist.size()-1)*50+2*HANDLE_PADDING;
    if (num_horiz_nodes > 0) { min_width = num_horiz_nodes*80; }
    else { min_width = 50; }
//     if (num_horiz_nodes > 0) { min_height = num_horiz_nodes*80; }
//     else { min_height = 150; }
}

// This is called by the layout engine (constraint solver) and updates the
// position of the template when the component shapes or guideline are moved.
void BranchedTemplate::updatePositionFromSolver(const double pos, const bool store_undo)
{
    Q_UNUSED (pos)
    Q_UNUSED (store_undo)

    // use these to calculate the new size and position
    QRectF branchedTemplateBounds;
  
    // Extra pixels past shapes.
    double buffer = 15;
    // check each guide in the branched template for attached shapes 
    // in order to calculate the bounds:
    size_t count = 0;
    for (GuidelineList::iterator curr = guidelist.begin(); curr != guidelist.end();
         ++curr) {
        size_t gcount = 0; // number of shapes on the guide

        // calculate the bounds based on all the shapes attached to this guide:
        QRectF bounds = getAttachedObjectsBounds<ShapeObj *>(*curr, buffer, &gcount);

        branchedTemplateBounds = branchedTemplateBounds.united(bounds);
        count += gcount;
    }
    
    // Existing position is the default.
    double xpos = x(), ypos = y();
    double newW = width(), newH = height();

    // XXX: Currently it starts at the top of the top shape cf the centre.
  
    // The GuiObj position does not reflect the real size
    // of the guideline, due to glow space around the guide.
    if (centreguide->get_dir() == GUIDE_TYPE_VERT) {


        if (count > 0) {
            // find the height range using the y positions of the nodes
//            range = cy2 - cy1;
      
            // There are shapes attached, use the position of the top-most shape
            //ypos = cy1;
        }
        else {
            // The template height (or width) is at least 100 pixels.
//            range = MIN_BRANCHED_WIDTH;
        }

    
        // use the first guide to calculate the x position
        xpos = ((Guideline*) guidelist.front())->position() - buffer;

        // Set height of the template based on the position of the shapes 
        // attached to the guideline.
//        height = range;
    
        // width is the distance from the first to the last guide + padding:
/*
        if (guidelist.front() && guidelist.back()) {
            width = 
                (int)((Guide*) guidelist.back())->getPos() -
                (int)((Guide*) guidelist.back())->getPos() +
                (buffer*2);
        }
        else {
            width = 14; // a default width value for a single guide
        }
*/
    }
    else {  // centre guide is GUIDE_TYPE_HORIZ
#if 0
        // QT
        double min_width, min_height;
        recalc_min_width_and_height(min_width, min_height);

        if (count > 0) {
            // find the width range using the x positions of the nodes
            range = std::max((cx2 - cx1), width());
      
            // There are shapes attached, use the position of the left-most shape
            xpos = cx1;
        }
        else {
            // The template width is at least 100 pixels.
            range = min_width;
        }
    
        // use the first guide to calculate the y position
        ypos = ((Guideline*) guidelist.front())->getPos() - buffer;

        // Set height of the template based on the position of the shapes 
        // attached to the guideline.
        newW = range;

        // height is the distance from the first to the last guide + padding:
        if (guidelist.front() && guidelist.back()) {
             newH =
                 std::max(
                     (((Guideline*) guidelist.back())->getPos() -
                      ((Guideline*) guidelist.front())->getPos() +
                      (buffer*2)),
                     height());
        } 
        else {
            newH = min_height; // default value for the width of a single guide
        }
#endif
    }

    Indicator::setPos(xpos, ypos);
    setSize(newW, newH);


    //QT left_handle->reposition();
    //QT right_handle->reposition();
}


double BranchedTemplate::getPos(void) const
{
    if (centreguide->get_dir() == GUIDE_TYPE_VERT) {
        return x();
    }
    else {
        return y();
    }
}


void BranchedTemplate::snapShapeToNearestGuideline(ShapeObj *shape) {
    // calculate nearest guide to *shape.
    // for a vertical branched template, this is based on the ypos of the shape,
    if (centreguide->get_dir() == GUIDE_TYPE_VERT) {

    }
    // for a horizontal branched template, this is based on the xpos of the shape
    else {
        int shape_mid_pos = shape->y();
        int distance = height(); // shape can't be further away than height of template
        Guideline* nearest_guide = NULL;
        int count = 1;
        int nearest_guide_index = 1;
        int new_distance;
        for (GuidelineList::iterator curr = guidelist.begin(); curr != guidelist.end();
             ++curr) {
            new_distance = abs(shape_mid_pos-(int)((Guideline*)*curr)->position());
            if(new_distance < distance) {
                distance = new_distance;
                nearest_guide = (Guideline*)(*curr);
                nearest_guide_index = count;
            }
            ++count;
        }
        assert(nearest_guide != NULL);

        // now attach the middle handles of the shape to the nearest guide
        new Relationship(nearest_guide, shape, ALIGN_MIDDLE);
    
//        resize(xpos, ypos, width, height);
        canvas()->interrupt_graph_layout();
    }
}

}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
