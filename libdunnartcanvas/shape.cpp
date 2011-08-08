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


#include <QFontDatabase>
#include <QFontMetrics>
#include <QImage>

#include <algorithm>
#include <climits>
#include <cstring>
#include <cstdarg>

#include "libavoid/libavoid.h"

#include "libdunnartcanvas/gmlgraph.h"
#include "libdunnartcanvas/utility.h"
#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/freehand.h"
#include "libdunnartcanvas/polygon.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/textshape.h"
#include "libdunnartcanvas/cluster.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/visibility.h"
#include "libdunnartcanvas/handle.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/instrument.h"
#include "libdunnartcanvas/placement.h"

namespace dunnart {


using Avoid::Point;


const QColor shLineCol = QColor(0,   0,   0);
const QColor shFillCol = QColor(240, 240, 210);


double avoidBuffer = AVOID_BUFFER_SMALL;
double routingBuffer = avoidBuffer;

static QPixmap *infoIcon = NULL;

unsigned int shapeFontSize = defaultShapeFontSize;

static QFont *shapeFont = NULL;

const char *x_connectionPins = "connectionPins";


//===========================================================================
// Generic shape and shape-handle handlers.

#if 0
void shape_handler(QWidget **object_addr, int action)
{
    QWidget *object = *object_addr;
    ShapeObj *shape = dynamic_cast<ShapeObj *> (object);

    switch (action)
    {
        case MOUSE_OVER:
        {
            if (queryMode) 
                pair_query(object);
            
            SDL_SetWindowsCursor(SDL_CUR_Move);

            statusBar->setTempMessage(shapeStatusStr);
            bool altMode = 
                    GraphLayout::getInstance()->isFreeShiftFromDunnart();
            if (altMode)
            {
                shape->showConnectionPoints();
                repaint_canvas();
            }
            break;
        }
        case MOUSE_LEAVE: 
            resetQueryModeIllumination(false);
            SDL_SetWindowsCursor(SDL_CUR_NormalSelect);
            statusBar->unsetTempMessage();
            shape->hideConnectionPoints();
            repaint_canvas();
            break;
        case MOUSE_WHEELUP:
            if (! shape->isBeingResized())
            {
                SDLGui::postUserEvent(USEREVENT_SHAPE_RESIZE,
                        (void *) shape, (void *) true);
                shape->setBeingResized(true);
            }
            break;
        case MOUSE_WHEELDOWN:
            if (! shape->isBeingResized())
            {
                SDLGui::postUserEvent(USEREVENT_SHAPE_RESIZE,
                        (void *) shape, (void *) false);
                shape->setBeingResized(true);
            }
            break;
        case MOUSE_LCLICKUP:
            if (!(OurGetModState() & KMOD_CTRL))
            {
                break;
            }
            // Fall through, treat CTRL + left click as middle click
        case MOUSE_MCLICKUP:
            if( gmlGraph ) {
                gmlGraph->expandNeighbours(shape);
            } else if (!(OurGetModState() & KMOD_ALT)) {
                canvas_deselect_shapes();
                shape_select(shape);
                repaint_canvas();
                shape->change_label();
            }
            break;
#if defined(__APPLE__)
        case MOUSE_MCLICK:
            // Apple counts Left click and Alt as a Middle click.
            if (!(OurGetModState() & KMOD_ALT))
            {
                break;
            }
            // Fallthrough.
#endif
        case MOUSE_LCLICK:
            if (!(OurGetModState() & KMOD_CTRL))
            {
                // CTRL and left click simulate middle click,
                // so handle on up event.
                resetQueryModeIllumination(false);
                selection_object_lclick(object);
            }
            break;
        case MOUSE_RCLICK:
            // Highlight the shape.
            if (shape->get_active_image_n() == SHAPE_STATE_UNSELECTED)
            {
                canvas_deselect_shapes();
                shape_select(shape);
            }
            repaint_canvas();

            shape->displayContextMenu();
            break;
        default:
            break;
    }
}
#endif


void ShapeObj::setBeingResized(bool isResizing)
{
    beingResized = isResizing;
}


bool ShapeObj::isBeingResized(void)
{
    return beingResized;
}

class ConnectionPinInfo
{
public:
    ConnectionPinInfo(const unsigned int classId,
            const double xPortionOffset, const double yPortionOffset,
            const double insideOffset = 0.0,
            const Avoid::ConnDirFlags visDirs = Avoid::ConnDirAll,
            const bool exclusive = false);
    ConnectionPinInfo(QString strRep);
    void createPin(Avoid::ShapeRef *avoidRef);
    QString writeToString(void) const;
    bool operator==(const ConnectionPinInfo& rhs);

    unsigned int classId;
    double xPortionOffset;
    double yPortionOffset;
    double insideOffset;
    Avoid::ConnDirFlags visDirs;
    bool exclusive;
    Avoid::ShapeConnectionPin *pin;
};

ConnectionPinInfo::ConnectionPinInfo(
        const unsigned int classId, const double xPortionOffset,
        const double yPortionOffset, const double insideOffset,
        const Avoid::ConnDirFlags visDirs, const bool exclusive)
    : classId(classId),
      xPortionOffset(xPortionOffset),
      yPortionOffset(yPortionOffset),
      insideOffset(insideOffset),
      visDirs(visDirs),
      exclusive(exclusive),
      pin(NULL)
{
}

ConnectionPinInfo::ConnectionPinInfo(QString strRep)
    : classId(0),
      xPortionOffset(Avoid::ATTACH_POS_CENTRE),
      yPortionOffset(Avoid::ATTACH_POS_CENTRE),
      insideOffset(0.0),
      visDirs(Avoid::ConnDirNone),
      exclusive(false),
      pin(NULL)
{
    QStringList list = strRep.split(" ");
    bool okay;

    classId = list[0].toUInt(&okay);
    if (!okay)
    {
        qWarning("Could not read classId from \"%s\"",
                qPrintable(strRep));
    }

    xPortionOffset = list[1].toDouble(&okay);
    if (!okay)
    {
        qWarning("Could not read xPortionOffset from \"%s\"",
                qPrintable(strRep));
    }

    yPortionOffset = list[2].toDouble(&okay);
    if (!okay)
    {
        qWarning("Could not read yPortionOffset from \"%s\"",
                qPrintable(strRep));
    }

    for (int i = 3; i < list.size(); ++i)
    {
        double valueD = list[i].toDouble(&okay);
        if (okay)
        {
            insideOffset = valueD;
        }
        else if (list[i] == "exclusive")
        {
            exclusive = true;
        }

        if (list[i] == "up")
        {
            visDirs = Avoid::ConnDirUp;
        }

        if (list[i] == "down")
        {
            visDirs = Avoid::ConnDirDown;
        }

        if (list[i] == "left")
        {
            visDirs = Avoid::ConnDirLeft;
        }

        if (list[i] == "right")
        {
            visDirs = Avoid::ConnDirRight;
        }
    }

    if (visDirs == Avoid::ConnDirNone)
    {
        visDirs = Avoid::ConnDirAll;
    }
}


void ConnectionPinInfo::createPin(Avoid::ShapeRef *avoidRef)
{
    pin = new Avoid::ShapeConnectionPin(avoidRef, classId,
            xPortionOffset, yPortionOffset, insideOffset, visDirs);
    pin->setExclusive(exclusive);
}


QString ConnectionPinInfo::writeToString(void) const
{
    QString strRep("%1 %2 %3");
    strRep = strRep.arg(classId);
    strRep = strRep.arg(xPortionOffset);
    strRep = strRep.arg(yPortionOffset);
    if (insideOffset)
    {
        strRep += QString(" %1").arg(insideOffset);
    }
    if (visDirs != Avoid::ConnDirAll)
    {
        if (visDirs == Avoid::ConnDirUp)
        {
            strRep += " up";
        }
        if (visDirs == Avoid::ConnDirDown)
        {
            strRep += " down";
        }
        if (visDirs == Avoid::ConnDirLeft)
        {
            strRep += " left";
        }
        if (visDirs == Avoid::ConnDirRight)
        {
            strRep += " right";
        }
    }
    if (exclusive)
    {
        strRep += " exclusive";
    }
    return strRep;
}


bool ConnectionPinInfo::operator==(const ConnectionPinInfo& rhs)
{
    return ((classId == rhs.classId) &&
            (xPortionOffset == rhs.xPortionOffset) &&
            (yPortionOffset == rhs.yPortionOffset) &&
            (insideOffset == rhs.insideOffset) &&
            (visDirs == rhs.visDirs) &&
            (exclusive == rhs.exclusive));
}


void ShapeObj::addConnectionPin(ConnectionPinInfo pinInfo)
{
    if (m_connection_pins.contains(pinInfo) == false)
    {
        m_connection_pins.push_back(pinInfo);
    }
    if (avoidRef)
    {
        m_connection_pins.back().createPin(avoidRef);
    }
}

void ShapeObj::routerAdd(void)
{
    Avoid::Router *router = canvas()->router();

    // Create ShapeRef
    Avoid::Polygon *polygon = poly(routingBuffer);
    int pid = polygon->_id;
    assert(avoidRef == NULL);
    avoidRef = new Avoid::ShapeRef(router, *polygon, pid);
    delete polygon;

    for (int i = 0; i < m_connection_pins.size(); ++i)
    {
        m_connection_pins[i].createPin(avoidRef);
    }
}


// This creates pins for what used to be the default pins on shape edge
// handles in Dunnart version 1.
uint ShapeObj::connectionPinForConnectionFlags(uint flags)
{
    uint pinID = 0;
    double insideOffset = 10;

    if (flags == 544)
    {
        pinID = CENTRE_CONNECTION_PIN;
    }
    else if (flags == 49)
    {
        pinID = DEPRECATED_CONNECTION_PIN_TOP;
        addConnectionPin(ConnectionPinInfo(pinID,
                Avoid::ATTACH_POS_CENTRE, Avoid::ATTACH_POS_TOP,
                insideOffset, Avoid::ConnDirUp));
    }
    else if (flags == 50)
    {
        pinID = DEPRECATED_CONNECTION_PIN_BOTTOM;
        addConnectionPin(ConnectionPinInfo(pinID,
                Avoid::ATTACH_POS_CENTRE, Avoid::ATTACH_POS_BOTTOM,
                insideOffset, Avoid::ConnDirDown));
    }
    else if ((flags == 52) || (flags == 36))
    {
        pinID = DEPRECATED_CONNECTION_PIN_LEFT;
        addConnectionPin(ConnectionPinInfo(pinID,
                Avoid::ATTACH_POS_LEFT, Avoid::ATTACH_POS_CENTRE,
                insideOffset, Avoid::ConnDirLeft));
    }
    else if ((flags == 56) || (flags == 40))
    {
        pinID = DEPRECATED_CONNECTION_PIN_RIGHT;
        addConnectionPin(ConnectionPinInfo(pinID,
                Avoid::ATTACH_POS_RIGHT, Avoid::ATTACH_POS_CENTRE,
                insideOffset, Avoid::ConnDirRight));
    }
    return pinID;
}


void ShapeObj::routerRemove(void)
{
    if (avoidRef == NULL)
    {
        return;
    }
    Avoid::Router *router = canvas()->router();

    // Delete shape
    router->deleteShape(avoidRef);
    avoidRef = NULL;
}


void ShapeObj::routerMove(void)
{
    if (avoidRef == NULL)
    {
        return;
    }
    Avoid::Router *router = canvas()->router();

    // Move shape
    Avoid::Polygon *polygon = poly(routingBuffer);
    router->moveShape(avoidRef, *polygon);
    delete polygon;
}


void ShapeObj::routerResize(void)
{
    // Treat as a move.
    routerMove();
}


void ShapeObj::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    canvas()->processSelectionDropEvent(event);
    CanvasItem::mouseReleaseEvent(event);
}

void ShapeObj::updateContainment(void)
{
    if (m_child_shapes.empty())
    {
        setZValue(ZORD_Shape);
    }
    else
    {
        setZValue(ZORD_Cluster);
    }
    update();
}

void ShapeObj::addContainedShape(ShapeObj *shape)
{
    m_child_shapes.insert(shape);
    updateContainment();
}
void ShapeObj::addContainedShapes(QList<ShapeObj *>& shapes)
{
    m_child_shapes.unite(QSet<ShapeObj *>::fromList(shapes));
    updateContainment();
}
void ShapeObj::removeContainedShape(ShapeObj *shape)
{
    m_child_shapes.remove(shape);
    updateContainment();
}
void ShapeObj::removeContainedShapes(QList<ShapeObj *>& shapes)
{
    m_child_shapes.subtract(QSet<ShapeObj *>::fromList(shapes));
    updateContainment();
}


void ShapeObj::userMoveBy(qreal dx, qreal dy)
{
    // Move the contained shapes.
    Actions& actions = canvas()->getActions();

    // If ALT-dragging, disconnect shape from guidelines.
    bool freeshift = canvas()->isLayoutSuspended();
    if (freeshift && (dx || dy))
    {
        for (int i = 0; i < 6; i++)
        {
            if (rels[i])
            {
                rels[i]->Deactivate(BOTH_SIDE);
            }
        }
    }

    for (QSet<ShapeObj *>::iterator curr = m_child_shapes.begin();
            curr != m_child_shapes.end(); ++curr)
    {
        if (!(*curr)->isSelected())
        {
            // Move the shape, if it is unselected.
            (*curr)->userMoveBy(dx, dy);
            actions.moveList.push_back(*curr);
        }
    }

    cmd_setCentrePos(centrePos() + QPointF(dx, dy));
}


QList<ShapeObj *> ShapeObj::containedShapes(void) const
{
    return QList<ShapeObj *>::fromSet(m_child_shapes);
}

QPainterPath ShapeObj::buildPainterPath(void)
{
    QPainterPath painter_path;

    painter_path.addRect(-width() / 2, -height() / 2, width(), height());

    return painter_path;
}


void ShapeObj::paint(QPainter *painter, 
        const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED (option)
    Q_UNUSED (widget)
    assert(painter->isActive());
    bool showDecorations = canvas() && ! canvas()->isRenderingForPrinting();

    if ( isSelected() && showDecorations )
    {
        QColor colour(0, 255, 255, 100);
        QPen highlight;
        highlight.setColor(colour);
        highlight.setWidth(7);
        highlight.setCosmetic(true);
        // Draw selection cue.
        painter->setPen(highlight);
        painter->drawPath(painterPath());
    }

#if 0
    // Shadow.
    if (canvas()->isLayoutSuspended(); && ((type == SHAPE_DRAW_HIGHLIGHTED) ||
                (type == SHAPE_DRAW_LEAD_HIGHLIGHTED)))
    {
        boxColor(surface, x, y + 6, x + w - 8, y + h - 1,
                QColor(66, 60, 60, 100));
    }
#endif

    painter->setPen(strokeColour());
    painter->setBrush(QBrush(fillColour()));
    
    painter->drawPath(painterPath());

    paintShapeDecorations(painter);
}


static const int ALBUT_L = 200 + ALIGN_LEFT;
static const int ALBUT_C = 200 + ALIGN_CENTER;
static const int ALBUT_R = 200 + ALIGN_RIGHT;
static const int ALBUT_T = 200 + ALIGN_TOP;
static const int ALBUT_M = 200 + ALIGN_MIDDLE;
static const int ALBUT_B = 200 + ALIGN_BOTTOM;

#if 0
void ShapeObj::determine_good_text_dimensions(int *w, int *h)
{
    int iw, ih;
    TTF_SizeUNICODE(shapeFont, unicodeLabel, &iw, &ih);

    int lnsh = 0;
    while (lnsh != -1)
    {
        lnsh = TextArea::writeWrappedUNICODE(unicodeLabel, iw, shapeFont);
        iw -= 5;
    }
    iw += 10;
    lnsh = TextArea::writeWrappedUNICODE(unicodeLabel, iw, shapeFont);

    *w = iw + 10;
    *h = lnsh + 10;

    *w = std::max(*w, 24);
    *h = std::max(*h, 24);
}


void ShapeObj::determine_best_dimensions(int *w, int *h)
{
    if (!gmlGraph || (label == NULL) || (strlen(label) == 0))
    {
        *w = (int)fWidth;
        *h = (int)fHeight;
        return;
    }

    determine_good_text_dimensions(w, h);

    if (decorativeImage)
    {
        // Add room for a preview:
        *w = std::max(*w, decorativeImage->w + 13);
        *h += decorativeImage->h + 6;
    }
}


void ShapeObj::determine_small_dimensions(int *w, int *h)
{
    if (!gmlGraph || (label == NULL) || (strlen(label) == 0))
    {
        *w = (int)fWidth;
        *h = (int)fHeight;
        return;
    }

    determine_good_text_dimensions(w, h);

    if (decorativeImage)
    {
        // Add room for a preview:
        *h += 19;
    }
}


static const double levelsOfDetail = 9;


//adjusts level of detail shown
bool ShapeObj::change_detail_level(bool expand)
{
    if (expand)
    {
        if (detailLevel == 100)
        {
            return true;
        }
        detailLevel += (int) (100 / levelsOfDetail);
        detailLevel = std::min(detailLevel, 100.0);
    }
    else
    {
        if (detailLevel == 0)
        {
            return true;
        }
        detailLevel -= (int) (100 / levelsOfDetail);
        detailLevel = std::max(detailLevel, 0.0);
    }
    
    int low_width, low_height;
    int high_width, high_height;
    determine_small_dimensions(&low_width, &low_height);
    determine_best_dimensions(&high_width, &high_height);
    double portion = (detailLevel / (double) 100);
    int new_width = (int) (low_width + (high_width - low_width) * portion);
    int new_height = (int) (low_height + (high_height - low_height) * portion);
    
    new_width += HANDLE_PADDING * 2;
    new_height += HANDLE_PADDING * 2;
    int dwidth = new_width - width;
    int dheight = new_height - height;
    if (dwidth || dheight)
    {
        setPosAndSize(QPointF(xpos - dwidth/2, ypos-dheight/2),
                QSizeF(new_width, new_height));
    }
    bool setDetailLevel = true;
    on_resize(setDetailLevel);

    return false;
}
#endif


QAction *ShapeObj::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event,
        QMenu& menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QAction* frontAction = menu.addAction(tr("Bring to Front"));
    QAction* backAction = menu.addAction(tr("Send to Back"));

    QAction *action = CanvasItem::buildAndExecContextMenu(event, menu);

    if (action == frontAction)
    {
        bringToFront();
    }
    else if (action == backAction)
    {
        sendToBack();
    }

    return action;
}

#if 0
void ShapeObj::addContextMenuItems(MenuItems& items)
{
    const char *breakStr = "Alt+Drag";
    
    items.push_back(
            MenuItem(BUT_TYP_Button, ALBUT_L, "Break Left Alignment", 
                    breakStr, NULL, rfl_action));
    items.push_back(
            MenuItem(BUT_TYP_Button, ALBUT_C, "Break Center Alignment",
                    breakStr, NULL, rfc_action));
    items.push_back(
            MenuItem(BUT_TYP_Button, ALBUT_R, "Break Right Alignment",
                    breakStr, NULL, rfr_action));
    items.push_back(MenuSeparator());
    items.push_back(
            MenuItem(BUT_TYP_Button, ALBUT_T, "Break Top Alignment",
                    breakStr, NULL, rft_action));
    items.push_back(
            MenuItem(BUT_TYP_Button, ALBUT_M, "Break Middle Alignment",
                    breakStr, NULL, rfm_action));
    items.push_back(
            MenuItem(BUT_TYP_Button, ALBUT_B, "Break Bottom Alignment",
                    breakStr, NULL, rfb_action));
    items.push_back(MenuSeparator());

    CanvasItem::addContextMenuItems(items);

    items.push_back(MenuSeparator());
    items.push_back(
            MenuItem(BUT_TYP_Button, BUT_CUTCOPY, "Bring to Front", "",
                    "buttons/bringtop.png", btf_action));
    items.push_back(
            MenuItem(BUT_TYP_Button, BUT_CUTCOPY, "Send to Back", "",
                    "buttons/bringbot.png", stb_action));
}


void ShapeObj::changeContextMenuState(Menu *menu)
{
    for (int i = 0; i < 6; i++)
    {
        if (!(rels[i]))
        {
            menu->changeWidgetState(200 + i, SDLGui::WIDGET_disable);
        }
    }

    CanvasItem::changeContextMenuState(menu);
}


void ShapeObj::showConnectionPoints(void)
{
    QWidget *gcurr = children_head;
    while (gcurr)
    {
        Handle *handle = dynamic_cast<Handle *> (gcurr);
        if (handle)
        {
            unsigned int flags = handle->flags();
            if (flags & HAN_CONNPT)
            {
                if (handle->get_active_image_n() == 0)
                {
                    handle->set_active_image(4);
                    handle->display();
                }
            }
        }
        gcurr = gcurr->get_next();
    }
}
#endif

void ShapeObj::findAttachedSet(CanvasItemSet& objSet)
{
    if (objSet.find(this) != objSet.end())
    {
        // Already seen this node.
        return;
    }
    objSet.insert(this);

    for (int r = 0; r < 6; ++r)
    {
        if (!rels[r])
        {
            // There is no connection here.
            continue;
        }

        // Do the recursive cascade call.
        if (rels[r]->guide)
        {
            rels[r]->guide->findAttachedSet(objSet);
        }
    }
}


ConnMultiset ShapeObj::getConnMultiset(void)
{
    ConnMultiset connMs;

    Avoid::ConnRefList attachedConns = avoidRef->attachedConnectors();
    for (Avoid::ConnRefList::iterator conn = attachedConns.begin();
            conn != attachedConns.end(); ++conn)
    {
        Connector *connector = dynamic_cast<Connector *>
                (canvas()->getItemByInternalId((*conn)->id()));
        connMs.insert(connector);
    }
    return connMs;
}


void ShapeObj::hideConnectionPoints(void)
{
    QList<QGraphicsItem *> child_items = childItems();
    for (int i = 0; i < child_items.size(); ++i)
    {
        Handle *handle = dynamic_cast<Handle *> (child_items.at(i));
        if (handle)
        {
            unsigned int flags = handle->handleFlags();
            if (flags & HAN_CONNPT)
            {
#if 0
                int active = handle->get_active_image_n();
                if (active == 4)
                {
                    handle->set_active_image(0);
                    handle->display();
                }
#endif
            }
        }
    }
}


void ShapeObj::deactivateAll(CanvasItemSet& selSet)
{
    assert(avoidRef != NULL);
    Avoid::ConnRefList attachedConns = avoidRef->attachedConnectors();
    // Check to see if there is a connector attached to this shape that
    // isn't in the selection.
    for (Avoid::ConnRefList::iterator conn = attachedConns.begin();
            conn != attachedConns.end(); ++conn)
    {
        Connector *connector = dynamic_cast<Connector *>
                (canvas()->getItemByInternalId((*conn)->id()));
        if (selSet.find(connector) == selSet.end())
        {
            // The connected handle is outside the selection.
            // UNDO add_undo_record(DELTA_CONNS, connector);
            connector->disconnect_from(this);
        }
    }

    // Deactivate constraints:
    for (int i = 0; i < 6; i++)
    {
        if (rels[i])
        {
            rels[i]->Deactivate(BOTH_SIDE);
        }
    }

    // Visibility graph stuff:
    routerRemove();
}


bool ShapeObj::canBe(const unsigned int flags)
{
    return flags & (C_CLUSTERED | C_ALIGNED);
}


#if 0

static void rfl_action(QWidget **c)
{
    ShapeObj *shape = (ShapeObj *) (*c)->get_parent()->get_ident();

    canvas()->beginUndoMacro(tr("Dettach From Guideline"));
    shape->rels[ALIGN_LEFT]->Deactivate(BOTH_SIDE);
}


static void rfc_action(QWidget **c)
{
    ShapeObj *shape = (ShapeObj *) (*c)->get_parent()->get_ident();

    canvas()->beginUndoMacro(tr("Dettach From Guideline");
    shape->rels[ALIGN_CENTER]->Deactivate(BOTH_SIDE);
}


static void rfr_action(QWidget **c)
{
    ShapeObj *shape = (ShapeObj *) (*c)->get_parent()->get_ident();

    canvas()->beginUndoMacro(tr("Dettach From Guideline"));
    shape->rels[ALIGN_RIGHT]->Deactivate(BOTH_SIDE);
}


static void rft_action(QWidget **c)
{
    ShapeObj *shape = (ShapeObj *) (*c)->get_parent()->get_ident();

    canvas()->beginUndoMacro(tr("Dettach From Guideline"));
    shape->rels[ALIGN_TOP]->Deactivate(BOTH_SIDE);
}


static void rfm_action(QWidget **c)
{
    ShapeObj *shape = (ShapeObj *) (*c)->get_parent()->get_ident();

    canvas()->beginUndoMacro(tr("Dettach From Guideline"));
    shape->rels[ALIGN_MIDDLE]->Deactivate(BOTH_SIDE);
}


static void rfb_action(QWidget **c)
{
    ShapeObj *shape = (ShapeObj *) (*c)->get_parent()->get_ident();

    canvas()->beginUndoMacro(tr("Dettach From Guideline"));
    shape->rels[ALIGN_BOTTOM]->Deactivate(BOTH_SIDE);
}
#endif


QColor ShapeObj::strokeColour(void) const
{
    return m_stroke_colour;
}

void ShapeObj::setStrokeColour(const QColor& colour)
{
    m_stroke_colour = colour;
    update();
}


QColor ShapeObj::fillColour(void) const
{
    if ((this == queryObj) || cascade_glow || constraintConflict())
    {
        return HAZARD_COLOUR;
    }
    return m_fill_colour;
}

void ShapeObj::setFillColour(const QColor& colour)
{
    m_fill_colour = colour;
    update();
}

bool ShapeObj::sizeLocked(void) const
{
    return m_size_locked;
}

void ShapeObj::setSizeLocked(const bool locked)
{
    m_size_locked = locked;
}


QRectF ShapeObj::shapeRect(const double buffer) const
{
    QRectF rect;
    rect.setSize(size());
    rect.moveCenter(centrePos());
    if (buffer > 0)
    {
        rect.adjust(-buffer, -buffer, buffer, buffer);
    }
    return rect;
}


void ShapeObj::setPos(const QPointF& pos)
{
    Q_UNUSED (pos)

    qWarning("Don't call ShapeObj::setPos, use ShapeObj::setCentrePos instead");
    abort();
}


void ShapeObj::setCentrePos(const QPointF & pos)
{
    if (canvas() && !canvas()->processingLayoutUpdates())
    {
        Actions& actions = canvas()->getActions();
        actions.moveList.push_back(this);
    }
    CanvasItem::setPos(pos);
}

QPointF ShapeObj::centrePos(void) const
{
    return pos();
}

void ShapeObj::setSize(const QSizeF& newSize)
{
    if (newSize == size())
    {
        return;
    }
    if (canvas() && !canvas()->processingLayoutUpdates())
    {
        Actions& actions = canvas()->getActions();
        actions.resizeList.push_back(this);
    }
    CanvasItem::setSize(newSize);
}

void ShapeObj::paintShapeDecorations(QPainter *painter)
{
#if 0
    // QT
    if (decorativeImage) 
    {
        drawLabelAndImage(surface, x, y);
        return;
    }
#endif

    if ( ! m_is_collapsed )
    {
        paintLabel(painter);
    }

    bool showDecorations = canvas() && ! canvas()->isRenderingForPrinting();

    if (this == queryObj)
    {
        if ( showDecorations && ! infoIcon )
        {
            infoIcon = new QPixmap();
            infoIcon->load("nuvola_icons/info_small.png");
        }
        painter->drawPixmap((width() / 2 - infoIcon->width()),
                -(height() / 2), *infoIcon);
    }

    if ( m_has_locked_position && showDecorations )
    {
        double iconSize = 25;
        if ( ! m_lock_icon )
        {
            m_lock_icon = new QGraphicsSvgItem(
                    ":/resources/images/pushpin.svg", this);

            QSizeF s =  m_lock_icon->sceneBoundingRect().size() / iconSize;
            m_lock_icon->scale( 1 / s.rwidth(), 1 / s.rheight() );
        }
        m_lock_icon->setPos((width() / 2) - iconSize + 6, -4 - (height() / 2));
        m_lock_icon->show();
    }
    else if ( m_lock_icon )
    {
        m_lock_icon->hide();
    }
}


static int wrapQString(const QString& text, int width, 
        const QFontMetrics& metrics, QStringList& lines)
{
    // Indexes:
    int strptr = 0;
    int lineptr = 0;
   
    int tl = 0, loffset = 0, pixels = 0;
    int textw;
    int texth = metrics.height();
    int wrappedExplicitNewLineSpacing = 0;;

    int text_length = text.length();
    const QChar *data = text.data();
    while (strptr < text_length)
    {
        // Find a space
        while ((data[strptr] != ' ') && (data[strptr] != '\0') && 
                (data[strptr] != '\n'))
        {
            strptr++;
        }

        textw = metrics.width(text.mid(lineptr, strptr - lineptr));
        if (textw > width) // if line too long
        {
            strptr--;
            while ((strptr != lineptr) && (data[strptr] != ' '))
            {
                // backtrack to last space
                strptr--;
            }
            if (strptr == lineptr)
            {
                return -1;
            }
            
            lines.push_back(text.mid(lineptr, strptr - lineptr));
            
            strptr++;
            tl++;
            lineptr = strptr;
        }
        else if (data[strptr] == '\n')                 // Forced newline
        {
            lines.push_back(text.mid(lineptr, strptr - lineptr));
            
            tl++;
            loffset += wrappedExplicitNewLineSpacing;
            lineptr = ++strptr;
        }
        else             // else, line might be too short so keep going
        {
            if (data[strptr] != '\0')
            {
                strptr++;
            }
        }
    }
    
    lines.push_back(text.mid(lineptr, strptr - lineptr));
    
    pixels = loffset + (texth * (tl + 1));

    return pixels;
}


void ShapeObj::addXmlProps(const unsigned int subset, QDomElement& node,
        QDomDocument& doc)
{
    CanvasItem::addXmlProps(subset, node, doc);

    if (subset & XMLSS_IMOVE)
    {
        newNsProp(node, x_dunnartNs, x_centreX, x());
        newNsProp(node, x_dunnartNs, x_centreY, y());
    }

    if (subset & XMLSS_IRESIZE)
    {
        newNsProp(node, x_dunnartNs, x_width, width());
        newNsProp(node, x_dunnartNs, x_height, height());
    }
    
    if (subset & XMLSS_ILABEL)
    {
        newNsProp(node, x_dunnartNs, x_label, m_label.toUtf8().data());
    }

    if (subset & XMLSS_IOTHER)
    {
        if (m_fill_colour != shFillCol)
        {
            QString value;
            value = value.sprintf("%02x%02x%02x%02x;", m_fill_colour.red(),
                    m_fill_colour.green(), m_fill_colour.blue(),
                    m_fill_colour.alpha());
            newNsProp(node, x_dunnartNs, x_fillCol, value);
        }
        if (m_stroke_colour != shLineCol)
        {
            QString value;
            value = value.sprintf("%02x%02x%02x%02x;", m_stroke_colour.red(),
                    m_stroke_colour.green(), m_stroke_colour.blue(),
                    m_stroke_colour.alpha());
            newNsProp(node, x_dunnartNs, x_lineCol, value);
        }

        QString value;
        value = value.sprintf("%g", detailLevel);
        newNsProp(node, x_dunnartNs, "detailLevel", value);

        if (m_has_locked_position)
        {
            newNsProp(node, x_dunnartNs, x_lockedPosition, "1");
        }

        // Store info about all connection pins other than the centre one,
        // which will be the first one in the list.
        QString pinRepStr;
        for (int i = 1; i < m_connection_pins.size(); ++i)
        {
            if (!pinRepStr.isEmpty())
            {
                pinRepStr += ",";
            }
            pinRepStr += m_connection_pins[i].writeToString();
        }
        if (!pinRepStr.isEmpty())
        {
            newNsProp(node, x_dunnartNs, x_connectionPins, pinRepStr);
        }
    }

     if (subset & XMLSS_ISVG)
     {
        newProp(node, "class", "shape");

        if (m_fill_colour != shFillCol)
        {
            QString value;
            value = value.sprintf("fill:#%02X%02X%02X;", m_fill_colour.red(),
                    m_fill_colour.green(), m_fill_colour.blue());
            newProp(node, "style", value);
        }

        // Handle shape text;
        if (m_label.length() > 0)
        {
            QRectF text_rect = labelBoundingRect();

            double xp = x();
            double yp = y();
            //double w = text_rect.width();
            //double h = text_rect.height();

            // XXX This memory may be lost, but we need to allocate it so
            // the group element doesn't go out of scope when the function
            // exits.  This needs to be rethought so the group can be easily
            // passed back.
            QDomElement *groupnode = new QDomElement(doc.createElement("g"));
            groupnode->appendChild(node);
#if 1 
            // Output wrapped text as individual lines.
            QDomElement tnode = doc.createElement("text");
            groupnode->appendChild(tnode);
            
            QString idValue = QString("_text_%1").arg(getIdString());
            newProp(tnode, "id", idValue);

            newProp(tnode, "class", "tLabel");

            // Get the wrapped lines for the shape.
            QStringList lines;
            QFontMetrics metrics(*shapeFont);
            int lnsh = wrapQString(m_label, width() - 8, metrics, lines);
            int shapeFontHeight = metrics.height();

            newProp(tnode, "x", xp);
            
            double textY = yp - (lnsh / 2.0) + shapeFontSize;
            
            newProp(tnode, "y", textY);

            int n = 0;
            for (QStringList::iterator curr = lines.begin(); curr != lines.end();
                    ++curr)
            {
                QString& line = *curr;

                QDomElement tsnode = doc.createElement("tspan");
                tnode.appendChild(tsnode);

                QString idValue = QString("_tspan_%1_%2").arg(getIdString()).arg(n);
                newProp(tsnode, "id", idValue);

                newProp(tsnode, "class", "tsLabel");

                newProp(tsnode, "x", xp);
                newProp(tsnode, "y", textY + (n * shapeFontHeight));
                newProp(tsnode, "sodipodi:role", "line");

                QDomText text = doc.createTextNode(line);
                tsnode.appendChild(text);

                n++;
            }
#else
            QDomElement froot = doc.createElement("flowRoot");
            groupnode->appendChild(froot);

            sprintf(value, "flowRoot%d", id);
            newProp(froot, "id", value);

            //newProp(froot, "xml:space", "preserve");
            
            newProp(froot, "class", "frLabel");

            QDomElement fregion = doc.createElement("flowRegion");
            froot.appendChild(fregion);

            sprintf(value, "flowRegion%d", id);
            newProp(fregion, "id", value);
            
            QDomElement use = doc.createElement("use");
            fregion.appendChild(use);
            
            newProp(use, "x", "0");
            newProp(use, "y", "0");
            
            sprintf(value, "#%d", id);
            newProp(use, "xlink:href", value);
            
            sprintf(value, "use%d", id);
            newProp(use, "id", value);
            
            QDomElement fpara = doc.createElement("flowPara");
            froot.appendChild(fpara);
            
            sprintf(value, "flowPara%d", id);
            newProp(fpara, "id", value);

            QDomText text = doc.createTextNode(label_);
            fpara.appendChild(text);
#endif
        }
    }
}

//===========================================================================
//  Rectangle code ("Action or Process"):


QDomElement RectangleShape::to_QDomElement(const unsigned int subset,
        QDomDocument& doc)
{
    QDomElement node = doc.createElement("rect");
   
    if (subset & XMLSS_IOTHER)
    {
        newNsProp(node, x_dunnartNs, x_type, x_shRect);
    }

    addXmlProps(subset, node, doc);

    if (subset & XMLSS_ISVG)
    {
        char value[50];
        QRectF rect = shapeRect();

        sprintf(value, "%.10g", rect.width());
        newProp(node, "width", value);
        sprintf(value, "%.10g", rect.height());
        newProp(node, "height", value);
        sprintf(value, "%.10g", rect.x());
        newProp(node, "x", value);
        sprintf(value, "%.10g", rect.y());
        newProp(node, "y", value);
    }

    return node;
}


#if 0
void ShapeObj::setDecorativeImageFile(const std::string fileName) {
    SDL_Surface *origDecorativeImage = IMG_Load(fileName.c_str());
    if(!origDecorativeImage) {
        fprintf(stderr,"IMG_Load: %s\n",IMG_GetError());
        return;
    }
    // Get rid of the bottom 20 pixels, since it contains a ID code.
    decorativeImage = SDLGui::createSurface(origDecorativeImage->w,
            origDecorativeImage->h - 20);
    SDL_BlitSurface(origDecorativeImage, NULL, decorativeImage, NULL);
    SDL_FreeSurface(origDecorativeImage);
}

bool Rect::trans_outside(QWidget *obj)
{
    int xpos = obj->get_absxpos(), ypos = obj->get_absypos();
    int width = obj->get_width(), height = obj->get_height();

    // Low-cost check to see if mouse is outside the bounding box:
    if ((mouse.x < xpos) || (mouse.x > (xpos + width  - 1)) ||
        (mouse.y < ypos) || (mouse.y > (ypos + height - 1)))
    {
        return true;
    }
    
    // Is cursor inside shape?
    if ((mouse.x > xpos + 7) && (mouse.x < (xpos + width  - 8)) &&
        (mouse.y > ypos + 7) && (mouse.y < (ypos + height - 8)))
    {
        return true;
    }
    
    return false;
}
#endif


Avoid::Polygon *ShapeObj::poly(const double b, Avoid::Polygon *p)
{
    double sw = width();
    double sh = height();
    double sx = centrePos().x() - (sw / 2);
    double sy = centrePos().y() - (sh / 2);
   
    if (p)
    {
        delete p;
    }
    p = new Avoid::Rectangle(Avoid::Point(sx - b, sy - b),
                Avoid::Point(sx + sw + b, sy + sh + b));
    
    if (!p)
    {
        qFatal("Couldn't calloc memory in Rect::poly()");
    }
    
    p->_id = (int) m_internal_id;

    for (int i = 0; i < 4; ++i)
    {
        p->ps[i].id = m_internal_id;
        p->ps[i].vn = i;
    }

    return p;
}













//===========================================================================
// Shape class code


ShapeObj::ShapeObj()
    : CanvasItem(NULL, QString(), ZORD_Shape),
      avoidRef(NULL),
      m_has_locked_position(false),
      m_lock_icon(NULL),
      decorativeImage(NULL),
      smallDecorativeImage(NULL),
      smallDecorativeScale(-1),
      m_fill_colour(shFillCol),
      m_stroke_colour(shLineCol),
      m_size_locked(false)
{
    if (!shapeFont)
    {
        QFontDatabase database;
        database.addApplicationFont(":/resources/DejaVuSans.ttf");

        shapeFont = new QFont("DejaVu Sans", shapeFontSize);
    }

    setHoverMessage("Shape \"%1\" - Drag to move. Hold ALT to drag free "
                    "from guidelines.");

    beingResized = false;

    setLabel("");

    // No initial relationships:
    for(int i = 0; i < 6; i++)
    {
        rels[i] = NULL;
    }

    addConnectionPin(ConnectionPinInfo(CENTRE_CONNECTION_PIN,
            Avoid::ATTACH_POS_CENTRE, Avoid::ATTACH_POS_CENTRE));
}


void ShapeObj::initWithDimensions(QString id, const double x, const double y,
        const double w, const double h)
{
    m_string_id = id;
    CanvasItem::setSize(w, h);
    CanvasItem::setPos(x, y);
}


void ShapeObj::initWithXMLProperties(Canvas *canvas,
        const QDomElement& node, const QString& ns)
{
    // Call equivalent superclass method.
    CanvasItem::initWithXMLProperties(canvas, node, ns);

    double x, y, w = 20, h = 20;

    bool has_size =  optionalProp(node, x_width, w, ns) &&
            optionalProp(node, x_height, h, ns);

    bool centre_pos = optionalProp(node, x_centreX, x, ns) && 
            optionalProp(node, x_centreY, y, ns);
    if (!centre_pos)
    {
        // The previous version of Dunnart stored top-left positions for 
        // shapes, relative to the canvas widget.  It also had 6 pixels of
        // padding around shapes.
        optionalProp(node, x_xPos, x, ns);
        optionalProp(node, x_yPos, y, ns);
        x += (w / 2);
        y += (h / 2);
        w -= 6;
        h -= 6;
    }

    if (has_size)
    {
        CanvasItem::setSize(w, h);
        CanvasItem::setPos(x, y);
    }

    QString value = nodeAttribute(node, ns, x_label);
    if (!value.isNull())
    {
        setLabel(value);
    }
    
    value = nodeAttribute(node, ns, x_fillCol);
    if (!value.isNull())
    {
        m_fill_colour = QColorFromRRGGBBAA(value.toLatin1().data());
    }

    value = nodeAttribute(node, ns, x_lineCol);
    if (!value.isNull())
    {
        m_stroke_colour = QColorFromRRGGBBAA(value.toLatin1().data());
    }
    
    optionalProp(node, "detailLevel", detailLevel, ns);
    
    value = nodeAttribute(node, ns, x_lockedPosition);
    if (!value.isNull())
    {
        m_has_locked_position = (bool) value.toInt();
    }

    value = nodeAttribute(node, ns, x_connectionPins);
    if (!value.isNull())
    {
        QStringList list = value.split(",");
        for (int i = 0; i < list.size(); ++i)
        {
            addConnectionPin(ConnectionPinInfo(list[i]));
        }
    }

    setPainterPath(buildPainterPath());
}


ShapeObj::~ShapeObj()
{
    if (decorativeImage)
    {
        // QT SDL_FreeSurface(decorativeImage);
    }
}


void ShapeObj::setLockedPosition(const bool val)
{
    m_has_locked_position = val;
    update();
}


bool ShapeObj::hasLockedPosition(void)
{
    return m_has_locked_position;
}


void ShapeObj::cascade_distance(int dist, unsigned int dir, CanvasItem **path)
{
    int nextval = 0;
    if (cascade_logic(nextval, dist, dir, path) == false)
    {
        return;
    }

    for (int r = 0; r < 6; ++r)
    {
        if (!rels[r])
        {
            // There is no connection here.
            continue;
        }

        if (!(dir & CASCADE_VERT))
        {
            if ((r >= ALIGN_TOP) && (r <= ALIGN_BOTTOM))
            {
                // We're not cascading in this direction.
                continue;
            }
        }
        if (!(dir & CASCADE_HORI))
        {
            if ((r >= ALIGN_LEFT) && (r <= ALIGN_RIGHT))
            {
                // We're not cascading in this direction.
                continue;
            }
        }

        // Do the recursive cascade call.
        if (rels[r]->guide)
        {
            rels[r]->guide->cascade_distance(nextval, dir, path);
        }
    }
}


QString ShapeObj::getLabel(void) const
{
    return m_label;
}


void ShapeObj::setLabel(const QString label)
{
    m_label = label;
    update();
}


QRectF ShapeObj::labelBoundingRect(void) const
{
    return boundingRect();
}


void ShapeObj::paintLabel(QPainter *painter)
{
    painter->setPen(Qt::black);
    painter->setFont(*shapeFont);
    painter->setRenderHint(QPainter::TextAntialiasing, true);
    painter->drawText(labelBoundingRect(), Qt::AlignCenter | Qt::TextWordWrap,
            m_label);
}


#if 0
void ShapeObj::drawLabelAndImage(SDL_Surface *target, const int x, const int y)
{
    if ((unicodeLabel == NULL) || (SDLGui::UTF16_strlen(unicodeLabel) == 0))
    {
        return;
    }

    int w = width, h = height;
    int textw, texth;

    // Get text size:
    TTF_SizeUNICODE(shapeFont, unicodeLabel, &textw, &texth);
    
    SDL_Color black = { 0x00, 0x00, 0x00, 0 };
    int lnsh = TextArea::writeWrappedUNICODE(unicodeLabel, w - 8, shapeFont);
    if (!in_graph_mode && (lnsh != -1) && (lnsh < (h - 4)))
    {
        TextArea::writeWrappedUNICODE(unicodeLabel, w - 8, shapeFont,
                target, x + 4, y + 6, TEXT_CENTER);
    }
    else
    {
        if (!in_graph_mode)
        {
            TTF_SizeText(shapeFont, "...", &textw, &texth);
            SDL_WriteText(target, x + (w / 2) - (textw / 2),
                    y + 6, "...", &black, shapeFont);
        }
        else
        {
            SDL_WriteUNICODE(target, x + (w / 2) - (textw / 2),
                    y + (h / 2) - (texth / 2), unicodeLabel, &black, 
                    shapeFont);
        }
        lnsh = -1;
    }

    if (lnsh == -1)
    {
        lnsh = TTF_FontHeight(shapeFont);
    }

    double scaleX = (width - 11) / (double) decorativeImage->w;
    double scaleY = (height - lnsh - 13) / (double) decorativeImage->h;
    double scale = std::min(scaleX, scaleY);
    SDL_Surface *small = NULL;
    if (scale < 1.0)
    {
        if (scale != smallDecorativeScale)
        {
            if (smallDecorativeImage)
            {
                SDL_FreeSurface(smallDecorativeImage);
                smallDecorativeImage = NULL;
            }
            smallDecorativeImage = SDL_Resize(decorativeImage, scale);
            smallDecorativeScale = scale;
        }
        small = smallDecorativeImage;
    }
    else
    {
        // No need to resize, use normal image.
        small = decorativeImage;
    }
    SDL_Rect dst_rect = {x+ (int) floor(w / 2.0 - small->w / 2.0), y+ lnsh + 7, 0, 0};
    SDL_BlitSurface(small, NULL, target, &dst_rect);
}
#endif


void ShapeObj::move_to(const int xn, const int yn, bool store_undo,
        bool from_solver, bool from_cider)
{
    // Can't be an action from both the solver and Cider.
    assert(!(from_solver && from_cider));

    bool diff_x = false, diff_y = false;
    int rs = 3, re = 3;
    if (xn != x())
    {
        diff_x = true;
        re = 6;
    }
    if (yn != y())
    {
        diff_y = true;
        rs = 0;
    }

    // Handle the case where the user is holding ALT:
    bool freeshift = canvas()->isLayoutSuspended();
    if (store_undo && freeshift && (diff_x || diff_y))
    {
        rs = 0;
        re = 6;

        bool selected = false;
        CanvasItemList selection = canvas()->selectedItems();
        for (CanvasItemList::iterator sh = selection.begin();
                sh != selection.end(); sh++)
        {
            if ((*sh) == this)
            {
                selected = true;
                break;
            }
        }
       
        if (selected)
        {
            for (int i = rs; i < re; i++)
            {
                if (rels[i])
                {
                    rels[i]->Deactivate(BOTH_SIDE);
                }
            }
        }
    }
    
    CanvasItem *cobj = this;
    cobj->move_to(xn, yn, store_undo);
}


void ShapeObj::setPosAndSize(const QPointF& newCentrePos,
        const QSizeF& newSize)
{
    // Handle the case where the user is holding ALT:
    bool freeshift = canvas()->isLayoutSuspended();
    if (freeshift)
    {
        int rs = 0, re = 6;

        for (int i = rs; i < re; i++)
        {
            if (rels[i])
            {
                rels[i]->Deactivate(BOTH_SIDE);
            }
        }
    }

    cmd_setCentrePos(newCentrePos);
    cmd_setSize(newSize);
}


void ShapeObj::set_label(const char *l)
{
    setLabel(l);
    
    update();
    
    // Generate a label change event
    //QT QWidget *gobj = this;
    //QT gobj->call_handler(&gobj, LABEL_CHANGED);
}

#if 0
static std::vector<std::string> explode(std::string s, std::string e)
{
    std::vector<std::string> ret;
    int iPos = s.find(e, 0);
    int iPit = e.length();
    while(iPos >- 1) 
    {
        //if(iPos!=0) // filter blank strings
        ret.push_back(s.substr(0, iPos));
        s.erase(0,iPos + iPit);
        iPos = s.find(e, 0);
    }
    //if(s != "")
    ret.push_back(s);
    return ret;
}


static std::string implode(std::vector<std::string> vec, std::string e)
{
    // Doesn't filter blank strings.
    std::string ret;

    for (unsigned int i = 0; i < vec.size(); ++i)
    {
        if (i == 0)
        {
            ret += vec[i];
        }
        else
        {
            ret = ret + e + vec[i];
        }
    }
    return ret;
}
#endif


void ShapeObj::change_label(void)
{
#if 0
    int padding = 7;
    int texth = TTF_FontHeight(winStndrd);
    int fwidth = 400 + padding;
    int fheight = (8 * texth) + padding;

    int fxpos = absxpos + (width / 2) - (fwidth / 2);
    int fypos = absypos + 6;

    int screenPadding = 8;
    // Make sure the text field doesn't go off the screen:
    fxpos = std::min(fxpos, screen->w - (fwidth + screenPadding));
    fxpos = std::max(fxpos, screenPadding);
    fypos = std::min(fypos, screen->h - (fheight + screenPadding));
    fypos = std::max(fypos, screenPadding);
    
    std::string labelStr(label);
    std::string nlStr("\n");
    FieldLines lines = explode(labelStr, nlStr);

    Field *field = new Field(NULL, fxpos, fypos, fwidth, fheight);
    field->setValue(lines);
    SDL_FastFlip(screen);
    bool modified = field->editText();

    if (modified)
    {
        //Save undo information:
        canvas()->beginUndoMacro(tr("Change Label"));
        add_undo_record(DELTA_LABEL, this);
        
        FieldLines& lines = field->getLines();
        std::string result = implode(lines, nlStr);
        set_label(result.c_str());
    }
    delete field;
#endif
}


Guideline *ShapeObj::get_guide(atypes type)
{
    if (rels[type])
    {
        return rels[type]->guide;
    }
    return NULL;
}


double ShapeObj::attachedGuidelinePosition(atypes type) const
{
    QRectF shapeRect;
    shapeRect.setSize(QSizeF(width(), height()));
    shapeRect.moveCenter(centrePos());
    return attachedGuidelinePosition(type, shapeRect);
}


double ShapeObj::attachedGuidelinePosition(atypes type, const QRectF& shapeRect)
{
    double pos = 0;

    if (type == ALIGN_LEFT)
    {
        pos = shapeRect.left();
    }
    else if (type == ALIGN_TOP)
    {
        pos = shapeRect.top();
    }
    else if (type == ALIGN_CENTER)
    {
        pos = shapeRect.center().x();
    }
    else if (type == ALIGN_MIDDLE)
    {
        pos = shapeRect.center().y();
    }
    else if (type == ALIGN_RIGHT)
    {
        pos = shapeRect.right();
    }
    else if (type == ALIGN_BOTTOM)
    {
        pos = shapeRect.bottom();
    }
    else
    {
        qFatal("ALIGN_type NYI.");
    }
    return pos;
}


Guideline *ShapeObj::new_guide(atypes type)
{
    Guideline *guide =  new Guideline(atypes_to_dirctn(type), attachedGuidelinePosition(type));
    QUndoCommand *cmd = new CmdCanvasSceneAddItem(canvas(), guide);
    canvas()->currentUndoMacro()->addCommand(cmd);
    return guide;
}

void ShapeObj::on_resize(bool setDetailLevel)
{
    Q_UNUSED (setDetailLevel)
}


ShapeObj *isShapeForLayout(QGraphicsItem *obj)
{
    ShapeObj *shape = dynamic_cast<ShapeObj *> (obj);
    Cluster *cluster = dynamic_cast<Cluster *> (obj);
    if (cluster)
    {
        // Treat as a node in layout only when collapsed.
        if (cluster->isCollapsed())
        {
            return shape;
        }
        else
        {
            return NULL;
        }
    }
    // Obviously treat normal shapes as nodes in layout.
    return shape;
}


}
/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  indent-tabs-mode:nil
  fill-column:79
  End:
*/
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

