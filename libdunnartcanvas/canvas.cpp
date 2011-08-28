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

#include <QToolBar>
#include <QMenu>
#include <QTimer>
#include <QPainter>
#include <QStatusBar>
#include <QSvgRenderer>
#include <QGraphicsSceneMouseEvent>
#include <QUndoStack>
#include <QSvgGenerator>

#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/visibility.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/separation.h"
#include "libdunnartcanvas/distribution.h"
#include "libdunnartcanvas/guideline.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/gmlgraph.h"

#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/templates.h"
#include "libdunnartcanvas/ui/createseparation.h"
#include "libdunnartcanvas/ui/createdistribution.h"
#include "libdunnartcanvas/ui/createtemplate.h"

#include "libavoid/libavoid.h"


namespace dunnart {

enum {
    PASTE_UPDATEOBJIDS = 1,
    PASTE_UPDATEIDPROPS,
    PASTE_FINDBADDISTROS,
    PASTE_REMOVEBADDISTROS,
    PASTE_SELECTSHAPES
};

Actions::Actions()
{
    clear();
}


void Actions::clear(void)
{
    flags = ACTION_NONE;
    moveList.clear();
    resizeList.clear();
}

bool Actions::empty(void) const
{
    return moveList.empty() && resizeList.empty() &&
           !(flags & (ACTION_ADDITIONS | ACTION_MODIFICATIONS | ACTION_DELETIONS));
}


// Resize Handle Types
enum {
    HAND_TOP_LEFT = 0,
    HAND_TOP_CENTRE,
    HAND_TOP_RIGHT,
    HAND_RIGHT_CENTRE,
    HAND_BOTTOM_RIGHT,
    HAND_BOTTOM_CENTRE,
    HAND_BOTTOM_LEFT,
    HAND_LEFT_CENTRE
};

class SelectionResizeHandle : public Handle {
    public:
        SelectionResizeHandle(int index, double xpos,
                double ypos) :
            Handle(NULL, index, 0),
            m_conn(NULL),
            m_pos(xpos, ypos)
        {
            // Position it in front of other objects.
            this->setZValue(1000000);

            switch (index)
            {
            case HAND_TOP_LEFT:
            case HAND_BOTTOM_RIGHT:
                setCursor(Qt::SizeFDiagCursor);
                break;
            case HAND_TOP_CENTRE:
            case HAND_BOTTOM_CENTRE:
                setCursor(Qt::SizeVerCursor);
                break;
            case HAND_TOP_RIGHT:
            case HAND_BOTTOM_LEFT:
                setCursor(Qt::SizeBDiagCursor);
                break;
            case HAND_RIGHT_CENTRE:
            case HAND_LEFT_CENTRE:
                setCursor(Qt::SizeHorCursor);
                break;
            default:
                break;
            }

            setHoverMessage("Selection Resize Handle - Click "
                    "and drag to resize the selected shapes. .");
        }
    protected:
        Connector *m_conn;
        QPointF m_pos;
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event)
        {
            int index = this->handleFlags();
            QPointF scenePosition = event->scenePos();

            Canvas *canvas = dynamic_cast<Canvas *> (scene());

            canvas->moveSelectionResizeHandle(index, scenePosition);
        }
        void mousePressEvent(QGraphicsSceneMouseEvent *event)
        {
            Canvas *canvas = dynamic_cast<Canvas *> (scene());
            canvas->beginUndoMacro(tr("Resize"));
            canvas->storeSelectionResizeInfo();
            event->accept();
            Handle::mousePressEvent(event);
        }
};

Canvas::Canvas()
    : QGraphicsScene(),
      m_visual_page_buffer(3.0),
      m_layout_update_timer(NULL),
      m_layout_finish_timer(NULL),
      m_processing_layout_updates(false),
      m_graphlayout(NULL),
      m_router(NULL),
      m_svg_renderer(NULL),
      m_status_bar(NULL),
      m_max_string_id(0),
      m_max_internal_id(0),
      m_gml_graph(NULL),
      m_use_gml_clusters(true),
      m_connector_nudge_distance(0),
      m_opt_ideal_connector_length_modifier(1.0),
      m_dragged_item(NULL),
      m_lone_selected_item(NULL),
      m_undo_stack(NULL),
      m_current_undo_macro(NULL),
      m_overlay_router_obstacles(false),
      m_overlay_router_visgraph(false),
      m_overlay_router_orthogonal_visgraph(false),
      m_rendering_for_printing(false)
{
    m_ideal_connector_length = 100;
    m_directed_edge_height_modifier = 0.5;
    m_sticky_nodes = false;
    m_downward_edges = false;
    m_avoid_connector_crossings = false;
    m_nudge_orthogonal_routes = true;
    m_avoid_cluster_crossings = false;
    m_rectangle_constraint_test = false;
    m_batch_diagram_layout = false;
    m_simple_paths_during_layout = true;
    m_force_orthogonal_connectors = false;

    m_undo_stack = new QUndoStack(this);

    // This is faster for dynamic scenes.
    setItemIndexMethod(QGraphicsScene::NoIndex);

    // Options for controlling behaviour of Constraint-based layout:
    m_opt_automatic_graph_layout = false;
    m_opt_prevent_overlaps       = false;
    m_opt_preserve_topology      = false;
    m_opt_rubber_band_routing    = false;
    m_opt_fit_within_page        = false;
    m_opt_colour_interfering_connectors = false;
    m_opt_connector_rounding_distance = 5;
    m_opt_stuctural_editing_disabled = false;

    // Default list of connector colors. Use only dark colors (and full
    // opacity) since connectors are drawn as thin lines so light
    // colors will be hard to see.
    m_default_connector_colours <<
            QColor(0, 0, 0) <<             // default (black)
            QColor(0, 0, 255) <<           // blue
            QColor(139, 0, 0) <<           // darkred
            QColor(143, 188, 143) <<       // dark sea green
            QColor(85, 26, 139) <<         // purple4
            QColor(139, 0, 139) <<         // magenta4
            QColor(139, 35, 35) <<         // brown4
            QColor(25, 25, 112) <<         // midnight blue
            QColor(148, 0, 211);           // dark violet

    m_graphlayout = new GraphLayout(this);

    // Avoid::PolyLineRouting
    m_router = new Avoid::Router(Avoid::OrthogonalRouting |
            Avoid::PolyLineRouting);
    m_router->setRoutingOption(
            Avoid::nudgeOrthogonalSegmentsConnectedToShapes, true);

    m_router->setRoutingPenalty(Avoid::segmentPenalty, 50);
    m_router->setRoutingPenalty(Avoid::clusterCrossingPenalty, 0);
    //m_router->setRoutingPenalty(Avoid::fixedSharedPathPenalty);

    selectionResizeHandles = QVector<SelectionResizeHandle *>(8);
    for (int i = 0; i < 8; ++i)
    {
        selectionResizeHandles[i] = new SelectionResizeHandle(i, 0, 0);
        addItem(selectionResizeHandles[i]);
        selectionResizeHandles[i]->setVisible(false);
    }

    connect(this, SIGNAL(selectionChanged()), this,
            SLOT(selectionChangeTriggers()));

#ifdef FPSTIMER
    feasibleStartTime = 0;
    feasibleEndTime = 0;
    totalTime = 0;
    updates = 0;
    timerRunning = false;
#endif
}


Canvas::~Canvas()
{
    delete m_graphlayout;
    delete m_router;

    if (m_svg_renderer)
    {
        delete m_svg_renderer;
    }

    // Free selection resize handles if they are not currently displayed
    // and thus owned by the scene.
    for (int i = 0; i < selectionResizeHandles.size(); ++i)
    {
        if (selectionResizeHandles[i]->scene() == NULL)
        {
            // Not part of the scene, so we need to free.
            delete selectionResizeHandles[i];
            selectionResizeHandles[i] = NULL;
        }
    }
}


void Canvas::loadGmlDiagram(const QString& filename)
{
    setOptFitWithinPage(true);
    setOptAutomaticGraphLayout(true);
    avoidBuffer = 10;
    int cxoff, cyoff;
    m_gml_graph = new gml::Graph(this, filename.toStdString(),
            gml::Page(this), gml::COff(cxoff, cyoff));
    //QT changeControlState(BUT_GMLGRAPH, SDLGui::WIDGET_enable);
}


void Canvas::loadSvgDiagram(const QString& filename)
{
    QDomDocument doc(filename);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "WARNING:\tCannot open file '" << filename <<
                "' for reading.\n"
                "\t\tSaving the active diagram will create this file.\n";
        return;
    }
    if (!doc.setContent(&file, true))
    {
        file.close();
        qFatal("Can't set XML content.");
    }
    file.close();
    m_svg_renderer = new QSvgRenderer(filename);

    QDomElement root = doc.documentElement();

    for (int pass = 0; pass < PASS_LAST; ++pass)
    {
        if (pass == PASS_CLUSTERS)
        {
            // Cause shapes to be added before clusters try and reference them.
            router()->processTransaction();
        }
        recursiveReadSVG(root, x_dunnartNs, pass);
    }
    // XXX Quick test
    //canvas->loadConstraintInfoFromString("<!DOCTYPE svg> <svg> <dunnart:options penaliseCrossings=\"0\" layoutMode=\"0\" preventOverlaps=\"1\" defaultIdealConnectorLength=\"1\" segmentPenalty=\"50\" layoutMethod=\"0\" avoidBuffer=\"0\" pageBoundaryConstraints=\"0\"/> <guide position=\"275\" id=\"4\" dunnart:direction=\"100\" dunnart:position=\"272\" dunnart:type=\"indGuide\" orientation=\"vertical\"> <dunnart:node constraintID=\"4\" alignmentPos=\"4\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"3\"/> <dunnart:node constraintID=\"4\" alignmentPos=\"4\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"2\"/> <dunnart:node constraintID=\"4\" alignmentPos=\"4\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"1\"/> </guide> <guide position=\"623\" id=\"8\" dunnart:direction=\"100\" dunnart:position=\"620\" dunnart:type=\"indGuide\" orientation=\"vertical\"> <dunnart:node constraintID=\"8\" alignmentPos=\"4\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"7\"/> </guide> <guide position=\"522\" id=\"9\" dunnart:direction=\"100\" dunnart:position=\"519\" dunnart:type=\"indGuide\" orientation=\"vertical\"> <dunnart:node constraintID=\"9\" alignmentPos=\"4\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"6\"/> </guide> <guide position=\"401\" id=\"10\" dunnart:direction=\"100\" dunnart:position=\"398\" dunnart:type=\"indGuide\" orientation=\"vertical\"> <dunnart:node constraintID=\"10\" alignmentPos=\"4\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"5\"/> </guide> <guide position=\"275.166\" id=\"12\" dunnart:direction=\"101\" dunnart:position=\"272.166\" dunnart:type=\"indGuide\" orientation=\"horizontal\"> <dunnart:node constraintID=\"12\" alignmentPos=\"1\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"3\"/> </guide> <guide position=\"209.666\" id=\"13\" dunnart:direction=\"101\" dunnart:position=\"206.666\" dunnart:type=\"indGuide\" orientation=\"horizontal\"> <dunnart:node constraintID=\"13\" alignmentPos=\"1\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"2\"/> </guide> <guide position=\"144.166\" id=\"14\" dunnart:direction=\"101\" dunnart:position=\"141.166\" dunnart:type=\"indGuide\" orientation=\"horizontal\"> <dunnart:node constraintID=\"14\" alignmentPos=\"1\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"1\"/> </guide> <dunnart:node id=\"11\" dunnart:direction=\"100\" dunnart:position=\"52\" dunnart:type=\"separation\" dunnart:sepDistance=\"50\"> <dunnart:node objTwoID=\"10\" constraintID=\"11\" dunnart:type=\"constraint\" relType=\"separation\" objOneID=\"4\"/> <dunnart:node objTwoID=\"9\" constraintID=\"11\" dunnart:type=\"constraint\" relType=\"separation\" objOneID=\"10\"/> <dunnart:node objTwoID=\"8\" constraintID=\"11\" dunnart:type=\"constraint\" relType=\"separation\" objOneID=\"9\"/> </dunnart:node> <dunnart:node id=\"15\" dunnart:direction=\"101\" dunnart:position=\"7\" dunnart:type=\"distribution\" dunnart:sepDistance=\"65.5\"> <dunnart:node objTwoID=\"13\" constraintID=\"15\" dunnart:type=\"constraint\" relType=\"distribution\" objOneID=\"14\"/> <dunnart:node objTwoID=\"12\" constraintID=\"15\" dunnart:type=\"constraint\" relType=\"distribution\" objOneID=\"13\"/> </dunnart:node> </svg>\" penaliseCrossings=\"0\" layoutMode=\"0\" preventOverlaps=\"1\" defaultIdealConnectorLength=\"1\" segmentPenalty=\"50\" layoutMethod=\"0\" avoidBuffer=\"0\" pageBoundaryConstraints=\"0\"/> <guide position=\"275\" id=\"4\" dunnart:direction=\"100\" dunnart:position=\"272\" dunnart:type=\"indGuide\" orientation=\"vertical\"> <dunnart:node constraintID=\"4\" alignmentPos=\"4\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"3\"/> <dunnart:node constraintID=\"4\" alignmentPos=\"4\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"2\"/> <dunnart:node constraintID=\"4\" alignmentPos=\"4\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"1\"/> </guide> <guide position=\"623\" id=\"8\" dunnart:direction=\"100\" dunnart:position=\"620\" dunnart:type=\"indGuide\" orientation=\"vertical\"> <dunnart:node constraintID=\"8\" alignmentPos=\"4\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"7\"/> </guide> <guide position=\"522\" id=\"9\" dunnart:direction=\"100\" dunnart:position=\"519\" dunnart:type=\"indGuide\" orientation=\"vertical\"> <dunnart:node constraintID=\"9\" alignmentPos=\"4\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"6\"/> </guide> <guide position=\"401\" id=\"10\" dunnart:direction=\"100\" dunnart:position=\"398\" dunnart:type=\"indGuide\" orientation=\"vertical\"> <dunnart:node constraintID=\"10\" alignmentPos=\"4\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"5\"/> </guide> <guide position=\"275.166\" id=\"12\" dunnart:direction=\"101\" dunnart:position=\"272.166\" dunnart:type=\"indGuide\" orientation=\"horizontal\"> <dunnart:node constraintID=\"12\" alignmentPos=\"1\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"3\"/> </guide> <guide position=\"209.666\" id=\"13\" dunnart:direction=\"101\" dunnart:position=\"206.666\" dunnart:type=\"indGuide\" orientation=\"horizontal\"> <dunnart:node constraintID=\"13\" alignmentPos=\"1\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"2\"/> </guide> <guide position=\"144.166\" id=\"14\" dunnart:direction=\"101\" dunnart:position=\"141.166\" dunnart:type=\"indGuide\" orientation=\"horizontal\"> <dunnart:node constraintID=\"14\" alignmentPos=\"1\" dunnart:type=\"constraint\" relType=\"alignment\" objOneID=\"1\"/> </guide> <dunnart:node id=\"11\" dunnart:direction=\"100\" dunnart:position=\"52\" dunnart:type=\"separation\" dunnart:sepDistance=\"50\"> <dunnart:node objTwoID=\"10\" constraintID=\"11\" dunnart:type=\"constraint\" relType=\"separation\" objOneID=\"4\"/> <dunnart:node objTwoID=\"9\" constraintID=\"11\" dunnart:type=\"constraint\" relType=\"separation\" objOneID=\"10\"/> <dunnart:node objTwoID=\"8\" constraintID=\"11\" dunnart:type=\"constraint\" relType=\"separation\" objOneID=\"9\"/> </dunnart:node> <dunnart:node id=\"15\" dunnart:direction=\"101\" dunnart:position=\"7\" dunnart:type=\"distribution\" dunnart:sepDistance=\"65.5\"> <dunnart:node objTwoID=\"13\" constraintID=\"15\" dunnart:type=\"constraint\" relType=\"distribution\" objOneID=\"14\"/> <dunnart:node objTwoID=\"12\" constraintID=\"15\" dunnart:type=\"constraint\" relType=\"distribution\" objOneID=\"13\"/> </dunnart:node> </svg> ");
}


void Canvas::postDiagramLoad(void)
{
    setOptAutomaticGraphLayout(m_opt_automatic_graph_layout);

    // Update on-screen representation of dependant indicators.
    QList<CanvasItem *> citems = items();
    for (int i = 0; i < citems.size(); ++i)
    {
        Guideline *gobj =    dynamic_cast<Guideline *> (citems.at(i));
        Distribution *dobj = dynamic_cast<Distribution *> (citems.at(i));
        Separation *sobj =   dynamic_cast<Separation *> (citems.at(i));

        if (dobj)
        {
            dobj->updateFromLayout(dobj->getSeparation());
        }
        else if (sobj)
        {
            sobj->updateFromLayout(sobj->getSeparation());
        }
        else  if (gobj)
        {
            gobj->updateFromLayout(gobj->position(), true);
        }
    }
    if (!m_opt_preserve_topology)
    {
        bool lastSimpleRouting = m_router->SimpleRouting;
        m_router->SimpleRouting = false;
        if (!m_batch_diagram_layout)
        {
            reroute_connectors(this);
        }
        m_router->SimpleRouting = lastSimpleRouting;
    }

#if 0
    if (gmlGraph)
    {
        createOverviewDialog(NULL);
    }
#endif

    // QT clear_undo_stack();
}


CanvasItem *Canvas::getItemByID(QString ID) const
{
    assert(this != NULL);
    QList<CanvasItem *> canvas_items = items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        CanvasItem *cobj = canvas_items.at(i);
        if (ID == cobj->getIdString())
        {
            return cobj;
        }
    }
    return NULL;
}

CanvasItem *Canvas::getItemByInternalId(uint internalId) const
{
    assert(this != NULL);
    QList<CanvasItem *> canvas_items = items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        CanvasItem *cobj = canvas_items.at(i);
        if (internalId == cobj->internalId())
        {
            return cobj;
        }
    }
    return NULL;
}


QUndoStack *Canvas::undoStack(void) const
{
    return m_undo_stack;
}

UndoMacro *Canvas::currentUndoMacro(void)
{
    if (m_current_undo_macro == NULL)
    {
        m_current_undo_macro = new UndoMacro(this);
    }
    return m_current_undo_macro;
}

UndoMacro *Canvas::beginUndoMacro(const QString& text)
{
    m_current_undo_macro = new UndoMacro(this);
    m_current_undo_macro->setText(text);
    m_undo_stack->push(m_current_undo_macro);

    return m_current_undo_macro;
}

void Canvas::endUndoMacro(void)
{
    m_current_undo_macro = NULL;
}


Actions& Canvas::getActions(void)
{
    return m_actions;
}

uint Canvas::assignInternalId(void)
{
    return ++m_max_internal_id;
}


QString Canvas::assignStringId(QString id)
{
    if (id.isEmpty())
    {
        id.setNum(++m_max_string_id);
    }
    else
    {
        if (idIsUnique(id))
        {
            // If id is a uint then count it in m_max_id, since we use integer
            // ids when we get clashes and reassign ids.
            bool isUInt = false;
            uint idUInt = id.toUInt (&isUInt);
            if (isUInt)
            {
                m_max_string_id = std::max(idUInt, m_max_string_id);
            }
        }
        else
        {
            ++m_max_string_id;
            qWarning("Clashing id \"%s\", reassigned \"%d\"", qPrintable(id),
                     m_max_string_id);
            id.setNum(m_max_string_id);
        }

    }
    return id;
}

bool Canvas::idIsUnique(QString id) const
{
    unsigned int count = 0;
    CanvasItem *item;
    foreach (item, items())
    {
        if (item->getIdString() == id)
        {
            ++count;
        }
    }
    return (count == 1);
}


void Canvas::setExpandedPage(const QRectF newExpandedPage)
{
    if (newExpandedPage != this->m_expanded_page)
    {
        QRectF updateArea = this->m_expanded_page;
        this->m_expanded_page = newExpandedPage;
        updateArea |= newExpandedPage;

        // Update each of the four page-ballon rectangles, triggering redraw.
        // Empty rectanges, if the drawing is contained on that side, do nothing.
        QRectF updateRect;

        updateRect = updateArea;
        updateRect.setTop(this->m_page.bottom());
        update(updateRect);

        updateRect = updateArea;
        updateRect.setBottom(this->m_page.top());
        update(updateRect);

        updateRect = updateArea;
        updateRect.setLeft(this->m_page.right());
        update(updateRect);

        updateRect = updateArea;
        updateRect.setRight(this->m_page.left());
        update(updateRect);
    }
}


void Canvas::drawBackground(QPainter *painter, const QRectF& rect)
{
    if ( m_rendering_for_printing )
    {
        // Don't draw any background at all.
        return;
    }

    if ( m_expanded_page.isNull() )
    {
        // No expanded page: effectively show just the normal page.
        m_expanded_page = m_page;
    }

    painter->fillRect(rect, QColor(189, 189, 223));
    painter->fillRect(m_expanded_page, QColor(200, 200, 200));
    painter->fillRect(m_page, QColor(255, 255, 255));
    painter->setPen(QColor(110, 110, 110));
    painter->drawRect(m_expanded_page);
}


void Canvas::drawForeground(QPainter *painter, const QRectF& rect)
{
    Q_UNUSED(rect);

    if ( m_rendering_for_printing )
    {
        // Don't draw any foreground at all.
        return;
    }

    if (m_overlay_router_obstacles)
    {
        QPen pen(Qt::red);
        pen.setCosmetic(true);
        painter->setPen(pen);
        QList<CanvasItem *> canvas_items = items();
        for (int i = 0; i < canvas_items.size(); ++i)
        {
            ShapeObj *shape = dynamic_cast<ShapeObj *> (canvas_items.at(i));

            if (shape && shape->avoidRef)
            {
                Avoid::Polygon poly = shape->avoidRef->polygon();
                Avoid::Point topLeft = poly.at(3);
                Avoid::Point bottomRight = poly.at(1);

                QRectF rect(QPointF(topLeft.x, topLeft.y),
                        QPointF(bottomRight.x, bottomRight.y));
                painter->drawRect(rect);
            }
        }
    }

    if (m_overlay_router_visgraph)
    {
        Avoid::EdgeList& visList = router()->visGraph;

        Avoid::EdgeInf *finish = visList.end();
        for (Avoid::EdgeInf *t = visList.begin(); t != finish; t = t->lstNext)
        {
            std::pair<Avoid::Point, Avoid::Point> ptpair = t->points();

            QPointF pt1(ptpair.first.x, ptpair.first.y);
            QPointF pt2(ptpair.second.x, ptpair.second.y);

            std::pair<Avoid::VertID, Avoid::VertID> ids = t->ids();

            if (ids.first.isConnPt() || ids.second.isConnPt())
            {
                // Endpt
                QColor colour(Qt::blue);
                colour.setAlpha(50);
                QPen pen(colour);
                pen.setCosmetic(true);
                painter->setPen(pen);
                painter->drawLine(pt1, pt2);
            }
            else
            {
                // Shape
                QColor colour(Qt::red);
                colour.setAlpha(50);
                QPen pen(colour);
                pen.setCosmetic(true);
                painter->setPen(pen);
                painter->drawLine(pt1, pt2);
            }
            QPen pen(Qt::black);
            pen.setCosmetic(true);
            painter->setPen(pen);
            painter->drawPoint(pt1);
            painter->drawPoint(pt2);
        }
    }

    if (m_overlay_router_orthogonal_visgraph)
    {
        Avoid::EdgeList& visList = router()->visOrthogGraph;

        Avoid::EdgeInf *finish = visList.end();
        for (Avoid::EdgeInf *t = visList.begin(); t != finish; t = t->lstNext)
        {
            std::pair<Avoid::Point, Avoid::Point> ptpair = t->points();

            QPointF pt1(ptpair.first.x, ptpair.first.y);
            QPointF pt2(ptpair.second.x, ptpair.second.y);

            std::pair<Avoid::VertID, Avoid::VertID> ids = t->ids();

            if (ids.first.isConnPt() || ids.second.isConnPt())
            {
                // Endpt
                QColor colour(Qt::blue);
                colour.setAlpha(50);
                QPen pen(colour);
                pen.setCosmetic(true);
                painter->setPen(pen);
                painter->drawLine(pt1, pt2);
            }
            else
            {
                // Shape
                QColor colour(Qt::red);
                colour.setAlpha(50);
                QPen pen(colour);
                pen.setCosmetic(true);
                painter->setPen(pen);
                painter->drawLine(pt1, pt2);
            }
            QPen pen(Qt::black);
            pen.setCosmetic(true);
            painter->setPen(pen);
            painter->drawPoint(pt1);
            painter->drawPoint(pt2);
        }
    }

    /*
        if (! router->clusterRefs.empty() )
        {
            // There are clusters so do cluster routing.
            for (Avoid::ClusterRefList::const_iterator cl =
                    router->clusterRefs.begin();
                    cl != router->clusterRefs.end(); ++cl)
            {
                Avoid::ReferencingPolygon& cBoundary = (*cl)->polygon();
                printf("PRINT: Points: %lu\n",  cBoundary.size());
                for (size_t j = 0; j < cBoundary.size(); ++j)
                {
                    Point p1 = cBoundary.at(j);
                    Point p2 = cBoundary.at((j + 1) % cBoundary.size());

                    unoffsetPoint(p1);
                    unoffsetPoint(p2);
                    aalineRGBA(screen, (int) p1.x, (int) p1.y, (int) p2.x,
                            (int) p2.y, 0, 0, 255, 255);
                }
            }
        }
    */
}


void Canvas::processSelectionDropEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED (event)

    if (!isLayoutSuspended())
    {
        // Don't do this processing if the layout is running.
        return;
    }

    QList<CanvasItem *> selected_items = selectedItems();
    QList<CanvasItem *> canvas_items = items();
    for (int i = 0; i < selected_items.size(); ++i)
    {
        ShapeObj *selectedShape = 
                dynamic_cast<ShapeObj *> (selected_items.at(i));

        if (!selectedShape)
        {
            continue;
        }

        QRectF selectedShapeRect = selectedShape->boundingRect().translated(
                selectedShape->scenePos());
        
        for (int j = 0; j < canvas_items.size(); ++j)
        {
            ShapeObj *shape = dynamic_cast<ShapeObj *> (canvas_items.at(j));
            
            if (shape)
            {
                shape->removeContainedShape(selectedShape);
            }
        }

        for (int j = 0; j < canvas_items.size(); ++j)
        {
            ShapeObj *shape = dynamic_cast<ShapeObj *> (canvas_items.at(j));
            if (!shape || (shape == selectedShape))
            {
                continue;
            }

            QRectF shapeRect = 
                    shape->boundingRect().translated(shape->scenePos());

            if (shape && shapeRect.contains(selectedShapeRect))
            {
                shape->addContainedShape(selectedShape);
            }
        }
    }
}

void Canvas::setDraggedItem(CanvasItem *item)
{
    if (item == NULL)
    {
        QApplication::restoreOverrideCursor();
        if (m_dragged_item)
        {
            glueObjectsToIndicators();
            clearIndicatorHighlights(true);
            repositionAndShowSelectionResizeHandles(true);
        }
        m_dragged_item = NULL;
    }

    if ((m_dragged_item == NULL) && item)
    {
        m_dragged_item = item;
        clearIndicatorHighlights(true);
        createIndicatorHighlightCache();
        hideSelectionResizeHandles();
        beginUndoMacro(tr("Move"));
    }
    else
    {
        clearIndicatorHighlights();
        assert(item == m_dragged_item);
    }
}


bool Canvas::processingLayoutUpdates(void) const
{
    return m_processing_layout_updates;
}

void Canvas::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // For some reason if a shape is dragged off the canvas under other 
    // widget and back then it doesn't recieve a mouse release event, so 
    // effectively pass on the event here.
    if (event->button() == Qt::LeftButton)
    {
        if (m_dragged_item)
        {
            m_dragged_item->dragReleaseEvent(event);
        }
    }

    QGraphicsScene::mouseReleaseEvent(event);
}


void Canvas::customEvent(QEvent *event)
{
    if (dynamic_cast<LayoutUpdateEvent *> (event))
    {
        //qDebug() << "Update " << (long long) this;
        this->startLayoutUpdateTimer();
    }
    else if (dynamic_cast<LayoutFinishedEvent *> (event))
    {
        //qDebug() << "Finish " << (long long) this;
        this->startLayoutFinishTimer();
    }
    else
    {
        QGraphicsScene::customEvent(event);
    }
}


void Canvas::lockSelectedShapes(void)
{
    QList<CanvasItem *> selected_items = selectedItems();
    for (int i = 0; i < selected_items.size(); ++i)
    {
        if (ShapeObj *shape = isShapeForLayout(selected_items.at(i)))
        {
            // Toggle position lock setting.
            if (shape->hasLockedPosition())
            {
                shape->setLockedPosition(false);
            }
            else
            {
                shape->setLockedPosition(true);
            }
            shape->update();
        }
    }
}


void Canvas::setStatusBar(QStatusBar *statusBar)
{
    m_status_bar = statusBar;

    if (m_status_bar && !m_status_messages.empty())
    {
        m_status_bar->showMessage(m_status_messages.top());
    }
}


void Canvas::pushStatusMessage(const QString& message)
{
    m_status_messages.push(message);

    if (m_status_bar)
    {
        m_status_bar->showMessage(m_status_messages.top());
    }
}


void Canvas::popStatusMessage(void)
{
    m_status_messages.pop();

    if (m_status_bar)
    {
        if (m_status_messages.empty())
        {
            m_status_bar->clearMessage();
        }
        else
        {
            m_status_bar->showMessage(m_status_messages.top());
        }
    }
}


void Canvas::selectAll(void)
{
    QPainterPath selectionArea;
    selectionArea.addPolygon(sceneRect());
    setSelectionArea(selectionArea);
}


void Canvas::templateFromSelection(int type)
{
    CanvasItemList selected_items = selectedItems();

    Indicator *indicator = NULL;
//    Guideline *guide = NULL;
    switch (type)
    {
        case TEMPLATE_LINEAR_HORI:
        {
            printf("creating horizontal linear template\n");
            fflush(stdout);

            double xpos = 150;
            double ypos = 150;
//            guide = new Guideline(GUIDE_TYPE_HORI, 150);
//            guide = createAlignment(ALIGN_CENTER, selected_items);

            indicator = new LinearTemplate(xpos, ypos, type, this);
//            Canvas::addItem(guide);
            Canvas::addItem(indicator);
            break;
        }
        case TEMPLATE_LINEAR_VERT:
        {
            double xpos = 50;
            double ypos = 50;
            indicator = new LinearTemplate(xpos, ypos, type, this);
            break;
        }
        case TEMPLATE_BRANCHED:
        {
            double xpos = 50;
            double ypos = 150;
            indicator = new BranchedTemplate(xpos, ypos);
            break;
        }
        default:
            break;
    }

    // Delselect shapes so they can be moved by layout solver.
    deselectAll();
    // Clear previous moves.
    Actions& actions = getActions();
    actions.clear();
//    add_undo_record(DELTA_ADD, indicator);
//add_undo_record(DELTA_MOVE, guide);
    // Relayout.
    interrupt_graph_layout();
}

void Canvas::alignSelection(int type)
{
    CanvasItemList selected_items = selectedItems();
    beginUndoMacro(tr("Create Alignment"));
    Guideline *guide = createAlignment((atypes) type, selected_items);

    // Delselect shapes so they can be moved by layout solver.
    deselectAll();
    // Clear previous moves.
    Actions& actions = getActions();
    actions.clear();
    // UNDO add_undo_record(DELTA_MOVE, guide);
    // Relayout.
    interrupt_graph_layout();
}


void Canvas::distributeSelection(int type)
{
    CreateDistributionDialog *sepDialog =
            dynamic_cast<CreateDistributionDialog *> (sender()->parent());
    CanvasItemList selected_items = selectedItems();
    QWidget *window = (sepDialog) ? sepDialog->window() : NULL;
    beginUndoMacro(tr("Create Distribution"));
    createDistribution(window, (dtype) type, selected_items);

    // Delselect shapes so they can be moved by layout solver.
    deselectAll();
    // Clear previous moves.
    Actions& actions = getActions();
    actions.clear();
    // Relayout.
    interrupt_graph_layout();
}

void Canvas::separateSelection(int type)
{
    CreateSeparationDialog *sepDialog =
            dynamic_cast<CreateSeparationDialog *> (sender()->parent());
    double minSeparationDist = (sepDialog) ? sepDialog->separationDistance() : 50.0;
    CanvasItemList selected_items = selectedItems();

    QWidget *window = (sepDialog) ? sepDialog->window() : NULL;
    beginUndoMacro(tr("Create Separation"));
    createSeparation(window, (dtype) type, selected_items, minSeparationDist);

    // Deselect shapes so they can be moved by layout solver.
    deselectAll();
    // Clear previous moves.
    Actions& actions = getActions();
    actions.clear();
    // Relayout.
    fully_restart_graph_layout();
}


QRectF Canvas::pageRect(void) const
{
    return m_page;
}

double Canvas::visualPageBuffer(void) const
{
    return m_visual_page_buffer;
}

GraphLayout *Canvas::layout(void) const
{
    return m_graphlayout;
}


Avoid::Router *Canvas::router(void) const
{
    return m_router;
}


void Canvas::setPageRect(const QRectF &rect)
{
    if (!rect.isEmpty())
    {
        QRectF updateArea = m_expanded_page;
        updateArea |= m_page;

        m_page = rect;
        m_expanded_page = QRectF();

        updateArea |= m_page;

        // Schedule repaint.
        update(updateArea);
    }
}

QString Canvas::saveConstraintInfoToString(void) const
{
    QDomDocument doc("svg");

    QDomElement svg = doc.createElement("svg");
    doc.appendChild(svg);

    QDomElement options = writeLayoutOptionsToDomElement(doc);
    doc.appendChild(options);

    // Put things into a multimap before outputting them,
    // so that they will be sorted in the correct Z-order.
    QList<CanvasItem *> canvas_items = this->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        Indicator *indicator = dynamic_cast<Indicator *> (canvas_items.at(i));

        if (indicator)
        {
            QDomElement node = indicator->to_QDomElement(XMLSS_ALL, doc);
            QDomNode parent = node.parentNode();
            if (!parent.isNull())
            {
                // The constructed node will have a parent if it was placed
                // in a group.  In this case, add the group node to the tree.
                svg.appendChild(node.parentNode());
            }
            else
            {
                svg.appendChild(node);
            }
        }
    }

    qDebug() << doc.toString();
    return doc.toString();
}


void Canvas::loadConstraintInfoFromString(const QString& constraintInfo)
{
    QDomDocument doc("svg");

    bool parseNamespaces = true;
    doc.setContent(constraintInfo, parseNamespaces);

    QDomElement root = doc.documentElement();

    // Actually do the pasting, in correct order.
    for (int pass = 0; pass < PASS_LAST; ++pass)
    {
        this->recursiveReadSVG(root, x_dunnartNs, pass);
    }

    this->fully_restart_graph_layout();
}

void Canvas::setIdealConnectorLength(const double length)
{
    m_ideal_connector_length = length;
}

double Canvas::idealConnectorLength(void) const
{
    return m_ideal_connector_length;
}

bool Canvas::avoidConnectorCrossings(void) const
{
    return m_avoid_connector_crossings;
}

bool Canvas::avoidClusterCrossings(void) const
{
    return m_avoid_cluster_crossings;
}


bool Canvas::optAutomaticGraphLayout(void) const
{
    return m_opt_automatic_graph_layout;
}

bool Canvas::optPreventOverlaps(void) const
{
    return m_opt_prevent_overlaps;
}

bool Canvas::optPreserveTopology(void) const
{
    return m_opt_preserve_topology;
}

bool Canvas::optRubberBandRouting(void) const
{
    return m_opt_rubber_band_routing;
}

bool Canvas::optFitWithinPage(void) const
{
    return m_opt_fit_within_page;
}

bool Canvas::optColourInterferingConnectors(void) const
{
    return m_opt_colour_interfering_connectors;
}

void Canvas::setDebugCOLAOutput(const bool value)
{
    m_graphlayout->setOutputDebugFiles(value);
}

void Canvas::setOptAutomaticGraphLayout(const bool value)
{
    m_opt_automatic_graph_layout = value;
    
    if (m_opt_automatic_graph_layout)
    {
        m_router->SimpleRouting = m_simple_paths_during_layout;
    }
    else // if (!opt_automatic_graph_layout_)
    {
        m_router->SimpleRouting = false;
    }
    emit optChangedAutomaticLayout(m_opt_automatic_graph_layout);
    
    fully_restart_graph_layout();
    
    if (!m_opt_automatic_graph_layout)
    {
        reroute_connectors(this, true);
    }
}

bool Canvas::optStructuralEditingDisabled(void) const
{
    return m_opt_stuctural_editing_disabled;
}


void Canvas::setOptStructuralEditingDisabled(const bool value)
{
    m_opt_stuctural_editing_disabled = value;

    clearSelection();

    emit optChangedStructuralEditingDisabled(m_opt_stuctural_editing_disabled);
}


void Canvas::setOptPreserveTopology(const bool value)
{
    m_opt_preserve_topology = value;
    emit optChangedPreserveTopology(m_opt_preserve_topology);
    fully_restart_graph_layout();
}


void Canvas::setOptRubberBandRouting(const bool value)
{
    m_opt_rubber_band_routing = value;

    m_router->RubberBandRouting = m_opt_rubber_band_routing;
    emit optChangedRubberBandRouting(m_opt_rubber_band_routing);
    restart_graph_layout();
}


void Canvas::setOptFitWithinPage(const bool value)
{
    m_opt_fit_within_page = value;
    if (!m_opt_fit_within_page)
    {
        setExpandedPage(QRectF());
    }
    emit optChangedFitWithinPage(m_opt_fit_within_page);
    interrupt_graph_layout();
}


void Canvas::setOptPreventOverlaps(const bool value)
{
    m_opt_prevent_overlaps = value;
    emit optChangedPreventOverlaps(m_opt_prevent_overlaps);
    fully_restart_graph_layout();
}


void Canvas::setOptConnPenaltySegment(const int value)
{
    m_router->setRoutingPenalty(Avoid::segmentPenalty, (double) value);

    reroute_all_connectors(this);
}


void Canvas::setOptConnRoundingDist(const int value)
{
    m_opt_connector_rounding_distance = value;

    redraw_connectors(this);
}


void Canvas::set_ideal_connector_length_from_slider(int int_modifier)
{
    double double_modifier = int_modifier / 100.0;
    set_ideal_connector_length(double_modifier);
}


void Canvas::set_ideal_connector_length(double modifier)
{
    m_opt_ideal_connector_length_modifier = modifier;
    interrupt_graph_layout();
}

bool Canvas::hasVisibleOverlays(void) const
{
    return m_overlay_router_obstacles || m_overlay_router_visgraph ||
            m_overlay_router_orthogonal_visgraph;
}

void Canvas::setRenderingForPrinting(const bool printingMode)
{
    m_rendering_for_printing = printingMode;
}

bool Canvas::isRenderingForPrinting(void) const
{
    return m_rendering_for_printing;
}
void Canvas::setOverlayRouterObstacles(const bool value)
{
    m_overlay_router_obstacles = value;
    emit debugOverlayEnabled(hasVisibleOverlays());
    this->update();
}


bool Canvas::overlayRouterObstacles(void) const
{
    return m_overlay_router_obstacles;
}

void Canvas::setOverlayRouterVisGraph(const bool value)
{
    m_overlay_router_visgraph = value;
    emit debugOverlayEnabled(hasVisibleOverlays());
    this->update();
}


bool Canvas::overlayRouterVisGraph(void) const
{
    return m_overlay_router_visgraph;
}


void Canvas::setOverlayRouterOrthogonalVisGraph(const bool value)
{
    m_overlay_router_orthogonal_visgraph = value;
    emit debugOverlayEnabled(hasVisibleOverlays());
    this->update();
}


bool Canvas::overlayRouterOrthogonalVisGraph(void) const
{
    return m_overlay_router_orthogonal_visgraph;
}


double Canvas::optIdealConnectorLengthModifier(void) const
{
    return m_opt_ideal_connector_length_modifier;
}


int Canvas::optConnectorRoundingDistance(void) const
{
    return m_opt_connector_rounding_distance;
}


int Canvas::optConnPenaltySegment(void) const
{
    return (int) m_router->routingPenalty(Avoid::segmentPenalty);
}

void Canvas::bringToFront(void)
{
    if (selectedItems().isEmpty())
    {
        return;
    }

    QList<CanvasItem *> selected_items = selectedItems();
    for (int i = 0; i < selected_items.size(); ++i)
    {
        selected_items.at(i)->bringToFront();
    }
}


void Canvas::sendToBack(void)
{
    if (selectedItems().isEmpty())
    {
        return;
    }

    QList<CanvasItem *> selected_items = selectedItems();
    for (int i = 0; i < selected_items.size(); ++i)
    {
        selected_items.at(i)->sendToBack();
    }
}


void Canvas::deselectAll(void)
{
    QList<CanvasItem *> selected_items = selectedItems();
    for (int i = 0; i < selected_items.size(); ++i)
    {
        selected_items.at(i)->setSelected(false);
    }
}


void Canvas::cutSelection(void)
{
    if (selectedItems().empty())
    {
        return;
    }
    copySelection();
    deleteSelection();
}


void Canvas::copySelection(void)
{
    if (selectedItems().empty())
    {
        return;
    }
    m_clipboard = QDomDocument();

    QDomElement svg = m_clipboard.createElement("svg");
    m_clipboard.appendChild(svg);
    newProp(svg, "xmlns", "http://www.w3.org/2000/svg");
    newProp(svg, "xmlns:dunnart", x_dunnartURI);

    //QT selection_box_props(&clip_x, &clip_y, &clip_w, &clip_h);

    QList<CanvasItem *> selected_items = selectedItems();
    for (int i = 0; i < selected_items.size(); ++i)
    {
        QDomElement elem =
                selected_items.at(i)->to_QDomElement(XMLSS_ALL, m_clipboard);

        svg.appendChild(elem);
    }
    emit clipboardContentsChanged();
}


void Canvas::pasteSelection(void)
{
    if (!m_clipboard.hasChildNodes())
    {
        // No children, so clipboard is empty.
        return;
    }

    beginUndoMacro(tr("Paste"));

    this->deselectAll();

    QString dunnartNs = x_dunnartNs;

    m_paste_id_map.clear();
    m_paste_bad_constraint_ids.clear();

    // Assign new clipboard IDs.
    recursiveMapIDs(m_clipboard, dunnartNs, PASTE_UPDATEOBJIDS);

    // Update IDs for connectors and relationships.
    recursiveMapIDs(m_clipboard, dunnartNs, PASTE_UPDATEIDPROPS);

    // Find bad distributions and separations.
    recursiveMapIDs(m_clipboard, dunnartNs, PASTE_FINDBADDISTROS);

    // Remove bad distributions and separations.
    recursiveMapIDs(m_clipboard, dunnartNs, PASTE_REMOVEBADDISTROS);

    QDomElement root = m_clipboard.documentElement();

    // Actually do the pasting, in correct order.
    for (int pass = 0; pass < PASS_LAST; ++pass)
    {
        this->recursiveReadSVG(m_clipboard, dunnartNs, pass);
    }

    // Select all new shapes.
    recursiveMapIDs(m_clipboard, dunnartNs, PASTE_SELECTSHAPES);

#if 0
    int posOffX = 0;
    int posOffY = 0;

    // Center of original clipboard selection (screen co-ordinates)
    int clip_cx = clip_x_ + (clip_w_ / 2);
    int clip_cy = clip_y_ + (clip_h_ / 2);
    // Center of canvas (screen co-ordinates)
    int canvas_cx = canvas->get_absxpos() + (canvas->get_width() / 2);
    int canvas_cy = canvas->get_absypos() + (canvas->get_height() / 2);

    if (!canvas->expensive_outside() && !c)
    {
        // If the cursor is inside the canvas, and the paste operation
        // hasn't been triggered from the menu, then paste the objects
        // under the cursor, else paste to the center of the page.
        posOffX = mouse.x - clip_cx;
        posOffY = mouse.y - clip_cy;
    }
    else
    {
        // Paste to the center of the canvas.
        posOffX = canvas_cx - clip_cx;
        posOffY = canvas_cy - clip_cy;
    }
    
    if ((posOffX != 0) || (posOffY != 0))
    {
        selection_move_diff(posOffX, posOffY, false, true);
    }
#endif
    // Put the distribution indicators at their default positions:
    QList<CanvasItem *> selected_items = selectedItems();
    for (int i = 0; i < selected_items.size(); ++i)
    {
        Distribution *distro = dynamic_cast<Distribution *> (selected_items.at(i));
        Separation *sep = dynamic_cast<Separation *> (selected_items.at(i));
        bool store_undo = false;

        if (distro)
        {
            distro->moveToDefaultPos(store_undo);
        }
        else if (sep)
        {
            sep->moveToDefaultPos(store_undo);
        }
    }
    interrupt_graph_layout();
    restart_graph_layout();
}


void Canvas::deleteSelection(void)
{
    if (selectedItems().empty())
    {
        return;
    }
    else if (m_opt_stuctural_editing_disabled)
    {
        return;
    }

    Actions& actions = getActions();
    actions.clear();

    UndoMacro *undoMacro = beginUndoMacro(tr("Delete"));

    // Disconnect all shape--connector connections where one is inside the
    // selection and one is outside the selection.
    CanvasItemSet selSet;
    // Use a copy since the deselect action will change the selection.
    QList<CanvasItem *> sel_copy = this->selectedItems();
    QList<ShapeObj *> sel_shapes;

    this->deselectAll();

    // Do distro's first, incase they have guidelines and distros selected.
    for (QList<CanvasItem *>::iterator sh = sel_copy.begin();
            sh != sel_copy.end(); ++sh)
    {
        Distribution *distro = dynamic_cast<Distribution *> (*sh);
        Separation *sep = dynamic_cast<Separation *> (*sh);

        if (distro || sep)
        {
            // Deactivate constraints:
            (*sh)->deactivateAll(selSet);
        }
    }
    for (QList<CanvasItem *>::iterator sh = sel_copy.begin();
            sh != sel_copy.end(); ++sh)
    {
        Connector  *conn  = dynamic_cast<Connector *>  (*sh);
        ShapeObj *shape = dynamic_cast<ShapeObj *> (*sh);
        Guideline *guide = dynamic_cast<Guideline *> (*sh);
        
        if (conn || guide || shape)
        {
            (*sh)->deactivateAll(selSet);
        }
        if (shape)
        {
            sel_shapes.push_back(shape); 
        }
    }
    
    // Remove containment relationships.
    QList<CanvasItem *> canvas_items = this->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (canvas_items.at(i));
        if (shape)
        {
            shape->removeContainedShapes(sel_shapes);
        }
    }

    for (QList<CanvasItem *>::iterator sh = sel_copy.begin();
            sh != sel_copy.end(); ++sh)
    {
        QUndoCommand *cmd = new CmdCanvasSceneRemoveItem(this, *sh);
        undoMacro->addCommand(cmd);
    }
    interrupt_graph_layout();
    restart_graph_layout();
}

// Delay, in milliseconds, to give the event loop time to respond to normal
// events like mouse movements.  A delay of zero will flood the event queue.
static const unsigned int updateEventDelay = 25;

void Canvas::startLayoutUpdateTimer(void)
{
    if (!m_layout_update_timer)
    {
        m_layout_update_timer = new QTimer(this);
        connect(m_layout_update_timer, SIGNAL(timeout()), this,
                SLOT(processLayoutUpdateEvent()));
    }
    m_layout_update_timer->start(updateEventDelay);
}


void Canvas::startLayoutFinishTimer(void)
{
    if (!m_layout_finish_timer)
    {
        m_layout_finish_timer = new QTimer(this);
        connect(m_layout_finish_timer, SIGNAL(timeout()), this,
                SLOT(processLayoutFinishedEvent()));
    }
    m_layout_finish_timer->start(updateEventDelay);
}


void Canvas::set_filename(QString filename)
{
    m_filename = filename;
    QFileInfo info(m_filename);
    emit diagramFilenameChanged(info);
}


QString Canvas::filename(void)
{
    return m_filename;
}


QList<CanvasItem *> Canvas::items(void) const
{
    QList<CanvasItem *> result;

    QList<QGraphicsItem *> itemList = QGraphicsScene::items();
    for (int i = 0; i < itemList.size(); ++i)
    {
        CanvasItem *canvasItem = dynamic_cast<CanvasItem *> (itemList.at(i));
        if (canvasItem)
        {
            result.push_back(canvasItem);
        }
    }

    return result;
}


QList<CanvasItem *> Canvas::selectedItems(void) const
{
    QList<CanvasItem *> result;
    
    QList<QGraphicsItem *> selection = QGraphicsScene::selectedItems();
    for (int i = 0; i < selection.size(); ++i)
    {
        CanvasItem *canvasItem = dynamic_cast<CanvasItem *> (selection.at(i));
        if (canvasItem)
        {
            result.push_back(canvasItem);
        }
    }

    return result;
}

bool Canvas::useGmlClusters(void) const
{
    return m_use_gml_clusters;
}

void Canvas::setNudgeDistance(const double dist)
{
    m_connector_nudge_distance = dist;
}

void Canvas::processLayoutUpdateEvent(void)
{
    m_layout_update_timer->stop();

    //printf("LayoutUpdateEvent\n");
    //fflush(stdout);
#ifdef FPSTIMER
    if (!timerRunning)
    {
        startTime = clock();
        timerRunning = true;
    }
    updates++;
#endif
    QList<CanvasItem *> canvas_items = items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        CanvasItem *canvasObj = canvas_items.at(i);

        if (canvasObj && canvasObj->constraintConflict())
        {
            canvasObj->setConstraintConflict(false);
        }
    }
    //int changes = m_graphlayout->processReturnPositions();
    m_graphlayout->processReturnPositions();
    if ((!m_simple_paths_during_layout || !m_opt_automatic_graph_layout ||
            !m_opt_preserve_topology) && !m_gml_graph && !m_batch_diagram_layout)
    {
        // Don't reroute connectors in the case of automatic graph layout.
        reroute_connectors(this);
    }
    //dumpScreenshot();

    //qDebug("processLayoutUpdateEvent %7d", ++layoutUpdates);
}


void Canvas::processLayoutFinishedEvent(void)
{
    m_layout_finish_timer->stop();

    //printf("LayoutFinishedEvent\n");
#if 0
#ifdef FPSTIMER
    if (timerRunning)
    {
        stopTime = clock();
        timerRunning = false;
        totalTime = stopTime - startTime;
        double totalSecs = totalTime / (double) CLOCKS_PER_SEC;
        double convergeSecs = (stopTime - clickUpTime) /
                (double) CLOCKS_PER_SEC;
        double feasibleSecs = (feasibleEndTime - feasibleStartTime) /
                (double) CLOCKS_PER_SEC;

        printf("************** Avg Framerate: %g\n", updates / totalSecs);
        printf("************** Time to converge: %g\n", convergeSecs);
        printf("************** makeFeasible time: %g\n", feasibleSecs);
        updates = 0;
    }
#endif
#endif
    /*
    if (!straighten_bends &&
            automatic_graph_layout && simple_paths_during_layout)
    {
        // Only bother doing this if automatic graph layout is on
        // and we would have been drawing simple paths up until this
        // point.
        bool lastSimpleRouting = router->SimpleRouting;
        router->SimpleRouting = false;
        reroute_connectors(this);
        router->SimpleRouting = lastSimpleRouting;
    }
    repaint_canvas();
    */
    GraphLayout* gl=m_graphlayout;
    if (!m_opt_automatic_graph_layout && !m_batch_diagram_layout)
    {
        // Do connector post-processing.
        reroute_connectors(this, false, true);
    }
    bool changes = !getActions().empty();
    if (gl->runLevel == 0)
    {
        gl->runLevel=1;
        printf("runLevel=1\n");
        interrupt_graph_layout();
        changes = true;
    }

    if (layoutDoneCallback!=NULL)
    {
        layoutDoneCallback->notify();
    }

    if (m_batch_diagram_layout && !changes)
    {
        // Reroute connectors.
        reroute_connectors(this, true, true);
        // Nudge connectors if requested (-w option)
        if (m_connector_nudge_distance > 0)
        {
            nudgeConnectors(this, m_connector_nudge_distance, true);
        }
        // redo the connector interference coloring after nudging
        if (m_opt_colour_interfering_connectors)
        {
            colourInterferingConnectors(this);
        }
        // Fit the page size to the entire diagram.
        //QT getPageSize(NULL, NULL, NULL, NULL, BUT_FITPAGETODIAGRAM);
        // Save the SVG and exit.
        //QT saveDiagramAsSVG(this, filename());
        exit(EXIT_SUCCESS);
    }
}


QSvgRenderer *Canvas::svgRenderer(void) const
{
    return m_svg_renderer;
}


void Canvas::setLayoutSuspended(bool suspend)
{
    if (suspend)
    {
        m_graphlayout->setLayoutSuspended(true);
    }
    else
    {
        m_graphlayout->setLayoutSuspended(false);
        this->interrupt_graph_layout();
        this->restart_graph_layout();
    }
}

bool Canvas::isLayoutSuspended(void) const
{
    return m_graphlayout->isFreeShiftFromDunnart();
}

void Canvas::createIndicatorHighlightCache(void)
{
    if (!isLayoutSuspended())
    {
        // User is not holding ALT, so don't automatically attach to guidelines.
        return;
    }

    QList<CanvasItem *> canvas_items = items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        Guideline *g = dynamic_cast<Guideline *> (canvas_items.at(i));
        if (g && (g->isSelected() == false))
        {
            // We don't want guides that are being moved, because they
            // are attached via multi-way constraints to selected shapes.
            bool invalid = false;
            for (RelsList::iterator r = g->rels.begin(); r != g->rels.end();
                    r++)
            {
                if ((*r)->shape && (*r)->shape->isSelected())
                {
                    invalid = true;
                    break;
                }
            }
            if (invalid)
            {
                continue;
            }

            if (g->get_dir() == GUIDE_TYPE_VERT)
            {
                double pos = g->x();
                m_vguides[pos - 1] = g;
                m_vguides[pos] = g;
                m_vguides[pos + 1] = g;
            }
            else
            {
                double pos = g->y();
                m_hguides[pos - 1] = g;
                m_hguides[pos] = g;
                m_hguides[pos + 1] = g;
            }
        }
    }
}


void Canvas::highlightIndicatorsForItemMove(CanvasItem *item)
{
    bool vfound = false, hfound = false;
    ShapeObj *shape = dynamic_cast<ShapeObj *> (item);
    if (shape && shape->canBe(C_ALIGNED))
    {
        for (int i = 0; i < 6; i++)
        {
            if (!(shape->rels[i]))
            {
                double pos = shape->attachedGuidelinePosition((atypes) i);

                if (!hfound && (i < 3))
                {
                    if (m_hguides.find(pos) != m_hguides.end())
                    {
                        m_hguides[pos]->setHighlighted(true);
                        m_hguides[pos]->update();
                    }
                }
                else if (!vfound && (i >= 3))
                {
                    if (m_vguides.find(pos) != m_vguides.end())
                    {
                        m_vguides[pos]->setHighlighted(true);
                        m_vguides[pos]->update();
                    }
                }
            }
        }
    }
}

/*
void highlightIndicators(Shape *shape, const QRectF& shapeRect)
{
    Canvas *canvas = shape->canvas();
    bool vfound = false, hfound = false;

    // QT nx -= canvas->get_xpos() + cxoff;
    // QT ny -= canvas->get_ypos() + cyoff;
    for (int i = 0; i < 6; i++)
    {
        if (!(shape->rels[i]))
        {
            double pos = ShapeObj::attachedGuidelinePosition((atypes) i, shapeRect);

            if (!hfound && (i < 3))
            {
                if (m_hguides.find(pos) != m_hguides.end())
                {
                    m_hguides[pos]->setHighlighted(true);
                    m_hguides[pos]->update();
                }
            }
            else if (!vfound && (i >= 3))
            {
                if (m_vguides.find(pos) != m_vguides.end())
                {
                    m_vguides[pos]->setHighlighted(true);
                    m_vguides[pos]->update();
                }
            }
        }
    }
}
*/

void Canvas::clearIndicatorHighlights(const bool clearCache)
{
    QList<CanvasItem *> canvas_items = items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {
        Guideline *g = dynamic_cast<Guideline *> (canvas_items.at(i));
        if (g)
        {
            if (g->isHighlighted())
            {
                g->setHighlighted(false);
                g->update();
            }
        }
    }

    if (clearCache)
    {
        m_hguides.clear();
        m_vguides.clear();
    }
}


void Canvas::glueObjectsToIndicators(void)
{
    if (!isLayoutSuspended())
    {
        // User is not holding ALT, so don't automatically attach to guidelines.
        return;
    }

    QList<CanvasItem *> selection = selectedItems();
    for (int s = 0; s < selection.size(); ++s)
    {
        bool vfound = false, hfound = false;
        ShapeObj *shape = dynamic_cast<ShapeObj *> (selection.at(s));
        if (shape && shape->canBe(C_ALIGNED))
        {
            for (int i = 0; i < 6; i++)
            {
                if (!(shape->rels[i]))
                {
                    double pos = shape->attachedGuidelinePosition((atypes) i);

                    if (!hfound && (i < 3))
                    {
                        if (m_hguides.find(pos) != m_hguides.end())
                        {
                            new Relationship(m_hguides[pos], shape, (atypes) i);
                        }
                    }
                    else if (!vfound && (i >= 3))
                    {
                        if (m_vguides.find(pos) != m_vguides.end())
                        {
                            new Relationship(m_vguides[pos], shape, (atypes) i);
                        }
                    }
                }
            }
        }
    }
    m_hguides.clear();
    m_vguides.clear();
}

void Canvas::processResponseTasks(void)
{
    GraphLayout* gl = m_graphlayout;
    gl->setRestartFromDunnart();
    gl->apply(!m_opt_automatic_graph_layout);

    getActions().clear();
}

void Canvas::processUndoResponseTasks(void)
{
    getActions().clear();
    repositionAndShowSelectionResizeHandles(true);
    stop_graph_layout();
    reroute_connectors(this);
}

void Canvas::fully_restart_graph_layout(void)
{
    GraphLayout* gl = m_graphlayout;
    gl->setInterruptFromDunnart();
    gl->setRestartFromDunnart();
    gl->runLevel=0;
    if (gl->mode == GraphLayout::LAYERED) 
    {
        gl->runLevel=1;
    }
    gl->apply(!m_opt_automatic_graph_layout);
}

void Canvas::restart_graph_layout(void)
{
    GraphLayout* gl = m_graphlayout;
    gl->setRestartFromDunnart();
    gl->apply(!m_opt_automatic_graph_layout);
}

void Canvas::stop_graph_layout(void)
{
    GraphLayout* gl = m_graphlayout;
    gl->setInterruptFromDunnart();
}


void Canvas::interrupt_graph_layout(void)
{
    GraphLayout* gl = m_graphlayout;
    gl->setInterruptFromDunnart();
    gl->apply(!m_opt_automatic_graph_layout);
}


void Canvas::selectionChangeTriggers(void)
{
    int shapeCount = 0;
    int indicatorCount = 0;

    this->hideSelectionResizeHandles();

    // Update the area covered by the selection and the resize handles.
    this->update(selectionShapesBoundingRect.adjusted(
            -BOUNDINGRECTPADDING, -BOUNDINGRECTPADDING,
            +BOUNDINGRECTPADDING, BOUNDINGRECTPADDING));
    // Then reset the selection shape's bounding Rectangle.
    selectionShapesBoundingRect = QRectF();

    QList<CanvasItem *> selected_items = selectedItems();
    for (int i = 0; i < selected_items.size(); ++i)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (selected_items.at(i));
        if (shape)
        {
            if (!shape->sizeLocked())
            {
                // Build the bounding rectangle from the union of all
                // shape's boundingRects in the selection.
                selectionShapesBoundingRect =
                        selectionShapesBoundingRect.united(
                        shape->boundingRect().translated(shape->scenePos()));
            }
            shapeCount++;
        }
        else if (dynamic_cast<Indicator *> (selected_items.at(i)))
        {
            indicatorCount++;
        }
    }

    // Remove the boundingRect padding.
    selectionShapesBoundingRect = selectionShapesBoundingRect.adjusted(
            +BOUNDINGRECTPADDING, +BOUNDINGRECTPADDING,
            -BOUNDINGRECTPADDING, -BOUNDINGRECTPADDING);

    if (shapeCount > 0)
    {
        this->repositionAndShowSelectionResizeHandles();
    }

    CanvasItem *new_lone_selected_item = NULL;
    if (selected_items.size() == 1)
    {
        new_lone_selected_item = selected_items.front();
    }
    if (m_lone_selected_item != new_lone_selected_item)
    {
        if (m_lone_selected_item)
        {
            m_lone_selected_item->loneSelectedChange(false);
        }
        m_lone_selected_item = new_lone_selected_item;
        if (m_lone_selected_item)
        {
            m_lone_selected_item->loneSelectedChange(true);
        }
    }

#if 0
    if (queryMode)
    {
        // Nicely handle selection changes while we are in pair_query mode.
        if (selection.size() == 1)
        {
            // We have one object selected. Set queryObj if it is a shape.
            queryObj = dynamic_cast<CanvasItem *> (selection.front());
            if (queryObj)
            {
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
            else
            {
                // Not a queryObj, so reset illumination.
                resetQueryModeIllumination(false);
            }
        }
        else
        {
            // Not a single object in the selection, so reset illumination.
            bool resetQueryObj = true;
            resetQueryModeIllumination(resetQueryObj);
        }
    }
#endif
}


void Canvas::storeSelectionResizeInfo(void)
{
    // Store info about selected shape position and dimensions in
    // relation to the boundingRect of the selection.  This way, when the
    // selection is resized, we can use this information to resize each
    // of the selected shapes.
    QPointF selectionTopLeft = selectionShapesBoundingRect.topLeft();
    QPointF selectionDimensions =
            selectionShapesBoundingRect.bottomRight() - selectionTopLeft;
    assert(selectionDimensions.x() >= 0);
    assert(selectionDimensions.y() >= 0);
    QList<CanvasItem *> selected_items = selectedItems();
    selectionResizeInfo = QVector<QRectF>(selected_items.size());
    for (int i = 0; i < selected_items.size(); ++i)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (selected_items.at(i));
        if (shape && !shape->sizeLocked())
        {
            QRectF shapeBR = shape->boundingRect().adjusted(
                    +BOUNDINGRECTPADDING, +BOUNDINGRECTPADDING,
                    -BOUNDINGRECTPADDING, -BOUNDINGRECTPADDING);
            shapeBR = shapeBR.translated(shape->scenePos());
            QPointF topLeft = (shapeBR.topLeft() - selectionTopLeft);
            topLeft = QPointF(topLeft.x() / selectionDimensions.x(),
                              topLeft.y() / selectionDimensions.y());
            QPointF bottomRight =
                    (shapeBR.bottomRight() - selectionTopLeft);
            bottomRight = QPointF(bottomRight.x() / selectionDimensions.x(),
                                  bottomRight.y() / selectionDimensions.y());
            selectionResizeInfo[i] = QRectF(topLeft, bottomRight);
        }
    }
}

void Canvas::moveSelectionResizeHandle(const int index, const QPointF pos)
{

    bool aroundCentre =
            (QApplication::keyboardModifiers() & Qt::MetaModifier);
    QPointF oppositePos = selectionShapesBoundingRect.center() - (pos -
            selectionShapesBoundingRect.center());
    switch (index)
    {
    case HAND_TOP_LEFT:
        selectionShapesBoundingRect.setTopLeft(pos);
        if (aroundCentre)
        {
            selectionShapesBoundingRect.setBottomRight(oppositePos);
        }
        break;
    case HAND_TOP_CENTRE:
        selectionShapesBoundingRect.setTop(pos.y());
        if (aroundCentre)
        {
            selectionShapesBoundingRect.setBottom(oppositePos.y());
        }
        break;
    case HAND_TOP_RIGHT:
        selectionShapesBoundingRect.setTopRight(pos);
        if (aroundCentre)
        {
            selectionShapesBoundingRect.setBottomLeft(oppositePos);
        }
        break;
    case HAND_RIGHT_CENTRE:
        selectionShapesBoundingRect.setRight(pos.x());
        if (aroundCentre)
        {
            selectionShapesBoundingRect.setLeft(oppositePos.x());
        }
        break;
    case HAND_BOTTOM_RIGHT:
        selectionShapesBoundingRect.setBottomRight(pos);
        if (aroundCentre)
        {
            selectionShapesBoundingRect.setTopLeft(oppositePos);
        }
        break;
    case HAND_BOTTOM_CENTRE:
        selectionShapesBoundingRect.setBottom(pos.y());
        if (aroundCentre)
        {
            selectionShapesBoundingRect.setTop(oppositePos.y());
        }
        break;
    case HAND_BOTTOM_LEFT:
        selectionShapesBoundingRect.setBottomLeft(pos);
        if (aroundCentre)
        {
            selectionShapesBoundingRect.setTopRight(oppositePos);
        }
        break;
    case HAND_LEFT_CENTRE:
        selectionShapesBoundingRect.setLeft(pos.x());
        if (aroundCentre)
        {
            selectionShapesBoundingRect.setRight(oppositePos.x());
        }
        break;
    default:
        break;
    }

    selectionShapesBoundingRect = selectionShapesBoundingRect.normalized();

    // Update the resize handle positions.
    this->repositionAndShowSelectionResizeHandles();

    // Calculate and apply new positions and dimensions for all shapes in
    // the selection.
    QPointF selectionTopLeft = selectionShapesBoundingRect.topLeft();
    QPointF selectionDimensions =
            selectionShapesBoundingRect.bottomRight() - selectionTopLeft;
    QList<CanvasItem *> selected_items = selectedItems();
    for (int i = 0; i < selected_items.size(); ++i)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (selected_items.at(i));
        if (shape && !shape->sizeLocked())
        {
            QRectF shapeBR = shape->boundingRect().adjusted(
                    +BOUNDINGRECTPADDING, +BOUNDINGRECTPADDING,
                    -BOUNDINGRECTPADDING, -BOUNDINGRECTPADDING);
            QPointF topLeft = selectionResizeInfo[i].topLeft();
            QPointF bottomRight = selectionResizeInfo[i].bottomRight();

            topLeft = QPointF(topLeft.x() * selectionDimensions.x(),
                              topLeft.y() * selectionDimensions.y());
            bottomRight = QPointF(bottomRight.x() * selectionDimensions.x(),
                                  bottomRight.y() * selectionDimensions.y());

            topLeft += selectionTopLeft;
            bottomRight += selectionTopLeft;

            QRectF newSize(topLeft, bottomRight);
            shape->setPosAndSize(newSize.center(),
                    QSizeF(std::max(newSize.width(), MIN_SHAPE_SIZE),
                           std::max(newSize.height(), MIN_SHAPE_SIZE)));
        }
    }

    // Cause the layout engine to notice changes to shape sizes.
    this->interrupt_graph_layout();
    this->restart_graph_layout();
}


void Canvas::hideSelectionResizeHandles(void)
{
    for (int i = 0; i < selectionResizeHandles.size(); ++i)
    {
        selectionResizeHandles[i]->setVisible(false);
    }
}

void Canvas::repositionAndShowSelectionResizeHandles(bool calculatePosition)
{
    if (m_opt_stuctural_editing_disabled)
    {
        // Don't show resize handles if structural editing is disabled.
        return;
    }

    if (calculatePosition)
    {
        selectionShapesBoundingRect = QRectF();

        QList<CanvasItem *> selected_items = selectedItems();
        for (int i = 0; i < selected_items.size(); ++i)
        {
            ShapeObj *shape = dynamic_cast<ShapeObj *> (selected_items.at(i));
            if (shape && !shape->sizeLocked())
            {
                // Build the bounding rectangle from the union of all shape's
                // boundingRects in the selection.
                selectionShapesBoundingRect = selectionShapesBoundingRect.united(
                        shape->boundingRect().translated(shape->scenePos()));
            }
        }
        // Remove the boundingRect padding.
        selectionShapesBoundingRect = selectionShapesBoundingRect.adjusted(
                +BOUNDINGRECTPADDING, +BOUNDINGRECTPADDING,
                -BOUNDINGRECTPADDING, -BOUNDINGRECTPADDING);
    }

    if (selectionShapesBoundingRect.isEmpty())
    {
        return;
    }

    // Reposition resize handles.
    selectionResizeHandles[HAND_TOP_LEFT]->setPos(
            selectionShapesBoundingRect.topLeft());
    selectionResizeHandles[HAND_TOP_CENTRE]->setPos(
            selectionShapesBoundingRect.center().x(),
            selectionShapesBoundingRect.top());
    selectionResizeHandles[HAND_TOP_RIGHT]->setPos(
            selectionShapesBoundingRect.topRight());
    selectionResizeHandles[HAND_RIGHT_CENTRE]->setPos(
            selectionShapesBoundingRect.right(),
            selectionShapesBoundingRect.center().y());
    selectionResizeHandles[HAND_BOTTOM_RIGHT]->setPos(
            selectionShapesBoundingRect.bottomRight());
    selectionResizeHandles[HAND_BOTTOM_CENTRE]->setPos(
            selectionShapesBoundingRect.center().x(),
            selectionShapesBoundingRect.bottom());
    selectionResizeHandles[HAND_BOTTOM_LEFT]->setPos(
            selectionShapesBoundingRect.bottomLeft());
    selectionResizeHandles[HAND_LEFT_CENTRE]->setPos(
            selectionShapesBoundingRect.left(),
            selectionShapesBoundingRect.center().y());

    // Show resize handles.
    for (int i = 0; i < selectionResizeHandles.size(); ++i)
    {
        selectionResizeHandles[i]->setVisible(true);
    }
}


bool Canvas::singlePropUpdateID(QDomElement& node, const QString& prop,
            const QString ns)
{
    bool result = false;
    QString oldId = nodeAttribute(node, ns, prop);
    if (!oldId.isNull())
    {
        QString qName = (ns.isEmpty()) ? prop : ns + ":" + prop;
        if (m_paste_id_map.find(oldId) != m_paste_id_map.end())
        {
            // The object this ID refers to is in the selection.
            node.setAttribute(qName, m_paste_id_map[oldId]);
            result = true;
        }
        else
        {
            node.removeAttribute(prop);
        }
    }
    return result;
}



void Canvas::recursiveMapIDs(QDomNode start, const QString& ns, int pass)
{

    for (QDomNode curr = start; !curr.isNull(); curr = curr.nextSibling())
    {
        if (curr.isElement())
        {
            QDomElement element = curr.toElement();
            QString idVal = element.attribute(x_id);

            if (!idVal.isEmpty())
            {
                if (pass == PASTE_UPDATEOBJIDS)
                {
                    QString newId = QString::number(++m_max_string_id);
                    m_paste_id_map[idVal] = newId;
                    element.setAttribute(x_id, newId);
                }
                else if (pass == PASTE_SELECTSHAPES)
                {
                    CanvasItem *obj = this->getItemByID(idVal);
                    if (obj)
                    {
                        obj->setSelected(true);
                    }
                }
                else if (pass == PASTE_REMOVEBADDISTROS)
                {
                    if (std::find(m_paste_bad_constraint_ids.begin(), m_paste_bad_constraint_ids.end(),idVal)
                            !=  m_paste_bad_constraint_ids.end())
                    {
                        // This is a bad distribution or separation, so
                        // remove its dunnart type so that it is ignored.
                        element.removeAttribute(x_type);
                    }
                }
            }

            if (pass == PASTE_UPDATEIDPROPS)
            {
                // Update single properties that refer to IDs.
                singlePropUpdateID(element, x_srcID, ns);
                singlePropUpdateID(element, x_dstID, ns);
                singlePropUpdateID(element, x_constraintID);
                singlePropUpdateID(element, x_objOneID);
                singlePropUpdateID(element, x_objTwoID);
            }
            else if (pass == PASTE_FINDBADDISTROS)
            {
                if (nodeAttribute(element, ns, x_type) == x_constraint)
                {
                    QString relType = nodeAttribute(element, ns, x_relType);
                    if ((relType ==  x_distribution) || (relType == x_separation))
                    {
                        QString indicatorID = essentialProp<QString>(element, x_constraintID);
                        QString objID1 = element.attribute(x_objOneID);
                        QString objID2 = element.attribute(x_objTwoID);

                        if (objID1.isEmpty() || objID2.isEmpty())
                        {
                            // This is a distribution or separation
                            // relationship for a guideline or guidelines
                            // that are not in the selection.

                            // Add distribution or separation to bad list.
                            m_paste_bad_constraint_ids.push_back(indicatorID);
                            // Remove the type attribute so that this node
                            // is effectively ignored in later processing.
                            element.removeAttribute(x_type);
                        }
                    }
                    if (relType == x_alignment)
                    {
                        QString objID1 = element.attribute(x_objOneID);

                        if (objID1.isEmpty())
                        {
                            // This is an alignment relationship for a
                            // shape that is not in the selection.

                            // Remove the type attribute so that this node
                            // is effectively ignored in later processing.
                            element.removeAttribute(x_type);
                        }
                    }
                }
            }
        }
        recursiveMapIDs(curr.firstChild(), ns, pass);
    }
}


// Return true for namespaces that are not used by Dunnart
// (since we will be copy elements/properties
//  in such namespaces straight through)
static bool is_external_ns(const QString& ns)
{
    if (ns.isEmpty() || (ns == x_dunnartNs) || (ns == "inkscape") ||
            (ns ==  "sodipodi") )
    {
        return false;
    }
    else
    {
        return true;
    }
}

void Canvas::loadSVGRootNodeAttributes(const QDomElement& svgRoot)
{
    if (svgRoot.hasAttribute("viewBox"))
    {
        QString value = svgRoot.attribute("viewBox");
        if (!value.isEmpty())
        {
            double pageX, pageY, pageW, pageH;
            sscanf(value.toLatin1().data(), "%lf %lf %lf %lf",
                    &pageX, &pageY, &pageW, &pageH);
            this->setPageRect(QRectF(pageX, pageY, pageW, pageH));
        }
    }
}

void Canvas::recursiveReadSVG(const QDomNode& start, const QString& dunnartNS,
        int pass)
{
    for (QDomNode curr = start; !curr.isNull(); curr = curr.nextSibling())
    {
        if (!curr.prefix().isEmpty())
        {
            if (is_external_ns(curr.prefix()))
            {
                m_extra_namespaces_map[curr.prefix()] = curr.namespaceURI();
            }
        }

        if (curr.isElement())
        {
            const QDomElement element = curr.toElement();

            if (pass == 0)
            {
                if ((element.localName() == "options") &&
                    (element.prefix() == x_dunnartNs))
                {
                    this->loadLayoutOptionsFromDomElement(element);
                }
                else if ((element.localName() == "identification") &&
                        (element.prefix() == "proorigami"))
                {
                    // For Pro-origami diagrams, use orthogonal connectors.
                    this->m_force_orthogonal_connectors = true;
                    // Don't allow the user to change diagram structure.
                    this->setOptStructuralEditingDisabled(true);
                    // Prevent overlaps.
                    this->m_opt_prevent_overlaps = true;
                }
                else if ((element.localName() == "svg") &&
                         element.prefix().isEmpty())
                {
                    this->loadSVGRootNodeAttributes(element);
                }
                else if (is_external_ns(element.prefix()))
                {
                    // Save nodes for external namespaces to output
                    // unchanged on saving.
                    // [ADS] FIXME: the tree structure of these external
                    // nodes will be lost, we are just storing them in
                    // a list regardless of depth.
                    QDomNode nodecopy = curr.cloneNode();
                    m_external_node_list.push_back(nodecopy);
                }
            }
            if (nodeHasAttribute(element, dunnartNS, x_type))
            {
                CanvasItem::create(this, element, dunnartNS, pass);
            }
        }
        this->recursiveReadSVG(curr.firstChild(), dunnartNS, pass);
    }
}

bool Canvas::forceOrthogonalConnectors(void) const
{
    return m_force_orthogonal_connectors;
}

// Atributes:
static const char *x_EXPERIMENTAL_rect = "EXP-rectConstraint";
static const char *x_layoutMethod = "layoutMethod";
static const char *x_layoutMode = "layoutMode";
static const char *x_layoutBeautification = "layoutBeautify";
static const char *x_preventOverlaps = "preventOverlaps";
static const char *x_automaticGraphLayout =
        "automaticGraphLayout";
static const char *x_avoidBuffer = "avoidBuffer";
static const char *x_routingBuffer = "routingBuffer";
static const char *x_downwardSeparation = "downwardSeparation";
static const char *x_defaultIdealConnectorLength =
        "defaultIdealConnectorLength";
static const char *x_pageBoundaryConstraints =
        "pageBoundaryConstraints";
static const char *x_penaliseCrossings = "penaliseCrossings";
static const char *x_segmentPenalty = "segmentPenalty";
static const char *x_colourInterferingConnectors =
        "colourInterferingConnectors";
static const char *x_rubberBandRouting =
        "rubberBandRouting";
static const char *x_interferingConnectorColours =
        "interferingConnectorColours";

static QString nodeToString(const QDomNode& node)
{
    QString nodeString;
    QTextStream nodeTextStream(&nodeString);
    node.save(nodeTextStream, 4);

    return nodeString;
}

void Canvas::saveDiagramAsSVG(QString outputFilename)
{
    QSize size = pageRect().size().toSize();
    QRectF viewBox = pageRect();

    QBuffer buffer;
    buffer.open(QBuffer::WriteOnly);

    QSvgGenerator generator;
    generator.setOutputDevice(&buffer);

    generator.setSize(size);
    generator.setViewBox(viewBox);
    generator.setDescription(tr("This SVG file was exported from "
            "Dunnart, a constraint-based diagram editor."));

    QPainter painter;
    if (painter.begin(&generator))
    {
        // Don't paint any objects into the generator, be just want to capture
        // the header and footer SVG tags, that we can later wrap around the
        // individual drawing tags for all canvas items.
        painter.end();
    }
    buffer.close();

    // Remove SVG tags up to beginning of first group tag, which will be
    // the beginning of the definition of the
    QString svgStr(buffer.data());
    int contentStart = svgStr.indexOf("<g ");
    int contentEnd = svgStr.indexOf("</g>") + 5;
    svgStr = svgStr.remove(contentStart, svgStr.length() - contentStart);

    // Add namespaces.
    int namespaceInsertPos = svgStr.indexOf(" xmlns");
    // Add Dunnart namespace.
    svgStr.insert(namespaceInsertPos,
            QString(" xmlns:dunnart=\"%1\"\n").arg(x_dunnartURI));
    // Now, add any namespaces used in the input document that we don't use
    // (since we will be copying properties in such namespaces straight
    // through).
    QMap<QString, QString>::const_iterator i =
            m_extra_namespaces_map.constBegin();
    while (i != m_extra_namespaces_map.constEnd())
    {
        const QString& nsName = i.key();
        const QString& nsURI  = i.value();

        svgStr.insert(namespaceInsertPos,
                QString(" xmlns:%1=\"%2\"\n").arg(nsName).arg(nsURI));
        ++i;
    }

    QFile svgFile(outputFilename);
    svgFile.open(QIODevice::WriteOnly);
    svgFile.write(svgStr.toUtf8());

    setRenderingForPrinting(true);
    QList<CanvasItem *> canvas_items = items();
    int canvas_count = canvas_items.size();
    for (int i = 0; i < canvas_count; ++i)
    {
        // Consider all the canvas items in reverse order, so they get drawn
        // into the SVG file with the correct z-order.
        CanvasItem *cobj = canvas_items.at(canvas_count - 1 - i);
        QString svg = cobj->svgCodeAsString(size, viewBox);

        // Less than three lines represents a open and close group tag
        // (setting style), with no content between, so only output it if
        // there are more than three lines.
        int lines = svg.count('\n');
        if (lines > 3)
        {
            svgFile.write(svg.toUtf8());
        }
    }
    setRenderingForPrinting(false);

    svgFile.write("<!-- Dunnart description -->\n");
    QDomDocument doc("svg");

    QDomElement optionsNode = this->writeLayoutOptionsToDomElement(doc);
    QString optionsNodeString = nodeToString(optionsNode);
    svgFile.write(optionsNodeString.toUtf8());

    for (int i = 0; i < canvas_count; ++i)
    {
        // Consider all the canvas items in reverse order, so they get drawn
        // into the SVG file with the correct z-order.
        CanvasItem *canvasObj = canvas_items.at(i);

        QDomNode node = canvasObj->to_QDomElement(XMLSS_ALL, doc);
        QString svgNodeString = nodeToString(node);
        svgFile.write(svgNodeString.toUtf8());
    }

    // Copy XML for any external namespaces we have saved
    if ( ! m_external_node_list.empty() )
    {
        svgFile.write("<!-- External namespace nodes -->\n");
        QDomNode externalNode;
        foreach (externalNode, m_external_node_list)
        {
            QString externalNodeString = nodeToString(externalNode);
            svgFile.write(externalNodeString.toUtf8());
        }
    }

    svgFile.write(QByteArray("</svg>\n"));
    svgFile.close();
}


// XXX Clean up the methods below.

void Canvas::loadLayoutOptionsFromDomElement(const QDomElement& options)
{
    GraphLayout *gl = this->layout();
    assert(gl);

    int method = gl->optimizationMethod;
    if(optionalProp(options,x_layoutMethod,method)) {
        gl->setOptimizationMethod((GraphLayout::OptimizationMethod)method);
    }
    int mode = gl->mode;
    if(optionalProp(options,x_layoutMode,mode)) {
        GraphLayout::Mode gmode=static_cast<GraphLayout::Mode>(mode);
        //QT selectModeButton(gmode);
        gl->setLayoutMode(gmode);
    }

    bool booleanVal = false;
    if (optionalProp(options,x_automaticGraphLayout,booleanVal))
    {
        setOptAutomaticGraphLayout(booleanVal);
    }

    if (optionalProp(options,x_layoutBeautification,booleanVal))
    {
        setOptPreserveTopology(booleanVal);
    }
    gl->runLevel = optPreserveTopology();

    if (optionalProp(options,x_preventOverlaps,booleanVal))
    {
        setOptPreventOverlaps(booleanVal);
    }

    if (optionalProp(options,x_pageBoundaryConstraints,booleanVal))
    {
        setOptFitWithinPage(booleanVal);
    }

    if (optionalProp(options,x_rubberBandRouting,booleanVal))
    {
        setOptRubberBandRouting(booleanVal);
    }

    optionalProp(options,x_EXPERIMENTAL_rect,m_rectangle_constraint_test);
    optionalProp(options,x_avoidBuffer,avoidBuffer);

    if (!optionalProp(options,x_routingBuffer,routingBuffer))
    {
        routingBuffer = avoidBuffer;
    }
    optionalProp(options,x_downwardSeparation,m_directed_edge_height_modifier);

    double ideal_connector_length_modifier;
    if (optionalProp(options,x_defaultIdealConnectorLength,
           ideal_connector_length_modifier))
    {
        set_ideal_connector_length(ideal_connector_length_modifier);
    }

    optionalProp(options,x_penaliseCrossings,m_avoid_connector_crossings);
    optionalProp(options,x_segmentPenalty,
            router()->penaltyRef(Avoid::segmentPenalty));
    optionalProp(options,x_colourInterferingConnectors,
            m_opt_colour_interfering_connectors);

    if (options.hasAttribute(x_interferingConnectorColours))
    {
        setInterferingConnectorColours(
                options.attribute(x_interferingConnectorColours));
    }

    unsigned int fileFontSize;
    if (optionalProp(options,x_fontSize,fileFontSize) && fileFontSize>0)
    {
        shapeFontSize = fileFontSize;
    }
}



QDomElement Canvas::writeLayoutOptionsToDomElement(QDomDocument& doc) const
{
    // Store Dunnart options.
    QDomElement dunOpts = doc.createElement("dunnart:options");

    // layout properties
    GraphLayout* gl = this->layout();
    newProp(dunOpts, x_automaticGraphLayout, optAutomaticGraphLayout());
    newProp(dunOpts, x_layoutMethod, (int)gl->optimizationMethod);
    newProp(dunOpts, x_layoutMode, (int)gl->mode);
    newProp(dunOpts, x_layoutBeautification, optPreserveTopology());
    newProp(dunOpts, x_preventOverlaps, optPreventOverlaps());
    newProp(dunOpts, x_avoidBuffer, avoidBuffer);
    if (routingBuffer != avoidBuffer)
    {
        newProp(dunOpts, x_routingBuffer, routingBuffer);
    }
    newProp(dunOpts, x_downwardSeparation, m_directed_edge_height_modifier);
    newProp(dunOpts, x_pageBoundaryConstraints, optFitWithinPage());
    newProp(dunOpts, x_defaultIdealConnectorLength,
            optIdealConnectorLengthModifier());
    newProp(dunOpts, x_penaliseCrossings, m_avoid_connector_crossings);
    newProp(dunOpts, x_segmentPenalty,
            router()->routingPenalty(Avoid::segmentPenalty));
    newProp(dunOpts, x_colourInterferingConnectors,
            m_opt_colour_interfering_connectors);
    newProp(dunOpts, x_rubberBandRouting, optRubberBandRouting());
    if (!m_interfering_connector_colours.isEmpty() &&
            (m_interfering_connector_colours != m_default_connector_colours))
    {
        QStringList colourStrings;
        QColor colour;
        foreach (colour, m_interfering_connector_colours)
        {
            // name is #RRGGBB, but just want RRGGBB
            colourStrings.append(colour.name().right(6));
        }
        // Build string of colours separated by commmas.
        newProp(dunOpts, x_interferingConnectorColours,
                colourStrings.join(",").toLatin1().data());
    }

    if (shapeFontSize != defaultShapeFontSize)
    {
        newProp(dunOpts,x_fontSize,shapeFontSize);
    }

    return dunOpts;
}


//
// setInterferingConnectorColors() - set list of colors to color interfering
//                                   connectors
//
// Parameters:
//  color_list_str - comma-delmited list of hex RRGGBB color values (no '#' prefix)
//
void Canvas::setInterferingConnectorColours(const QString colourListString)
{
    QStringList colourList = colourListString.split(QChar(','));

    m_interfering_connector_colours.clear();

    QString colourStr;
    foreach (colourStr, colourList)
    {
        // Put in the form "#RRGGBB"
        colourStr.prepend("#");
        QColor colour(colourStr);
        if (colour.isValid())
        {
            m_interfering_connector_colours.append(colour);
        }
    }
}

//
// interferingConnectorColors() - get list of colors to color intefering
//                                   connectors
//
// Returns a list of QColors previously set up with the
// setInterferiongConenctorColours method, or if unset then
// the default list of colors.
//
const QList<QColor> Canvas::interferingConnectorColours(void) const
{
    if (m_interfering_connector_colours.isEmpty())
    {
        return m_default_connector_colours;
    }
    else
    {
        return m_interfering_connector_colours;
    }
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

