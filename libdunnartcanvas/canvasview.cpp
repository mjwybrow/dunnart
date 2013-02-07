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

#include <QTimer>
#include <QMouseEvent>
#include <QApplication>
#include <QScrollBar>
#include <QMenu>

#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/pluginshapefactory.h"

namespace dunnart {

CanvasView *canvas_view = NULL;

CanvasView::CanvasView(Canvas *canvas)
    : QGraphicsView(),
      m_hand_scrolling(false)
{
    // We'd like antialiasing.
    setRenderHints(QPainter::Antialiasing);

    setViewportUpdateMode(QGraphicsView::BoundingRectViewportUpdate);

    setDragMode(QGraphicsView::RubberBandDrag);

    setRubberBandSelectionMode(Qt::ContainsItemShape);

    // Always show the scrollbars.
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    setResizeAnchor(QGraphicsView::AnchorViewCenter);

    setAcceptDrops(true);

    setScene(canvas);
}


Canvas *CanvasView::canvas(void)
{
    // If the scene is set, then it should be a Canvas object
    // rather than just a QGraphicsScene.
    assert((scene() == NULL) || (qobject_cast<Canvas *> (scene()) != NULL));

    return qobject_cast<Canvas *> (scene());
}


void CanvasView::setScene(QGraphicsScene *scene)
{
    if (scene == canvas())
    {
        return;
    }

    if (canvas())
    {
        disconnect(canvas(), 0, this, 0);
    }
    QGraphicsView::setScene(scene);

    if (canvas())
    {
        connect(canvas(), SIGNAL(sceneRectChanged(QRectF)),
                this, SLOT(adjustSceneRect(QRectF)));
        connect(canvas(), SIGNAL(debugOverlayEnabled(bool)),
                this, SLOT(debugOverlayEnabled(bool)));
        connect(canvas(), SIGNAL(editModeChanged(int)),
                this, SLOT(editModeChanged(int)));
        connect(canvas(), SIGNAL(canvasDrawingChanged()),
                this, SLOT(repaintCanvasViewport()));
    }
}


void CanvasView::debugOverlayEnabled(bool enabled)
{
    if (enabled)
    {
        setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    }
    else
    {
        setViewportUpdateMode(QGraphicsView::MinimalViewportUpdate);
    }
}

void CanvasView::editModeChanged(int mode)
{
    setDragMode((mode == ModeSelection) ?
            QGraphicsView::RubberBandDrag : QGraphicsView::NoDrag);
}

void CanvasView::adjustSceneRect(QRectF new_scene_rect)
{
    Q_UNUSED (new_scene_rect)

    if (canvas() == NULL)
    {
        return;
    }

    QRectF scene_rect = sceneRect();
    // XXX We use scene_.itemsBoundingRect() here rather than new_scene_rect
    //     since new_scene_rect has strange larger values, possibly due to 
    //     connectors returning incorrect sizes at some point.
    scene_rect = scene_rect.united(canvas()->itemsBoundingRect());
    setSceneRect(scene_rect);
}

void CanvasView::repaintCanvasViewport(void)
{
    // Trigger a repaint of the entire visible canvas viewport.
    canvas()->update(viewportRect());
}


static QRectF expandRect(const QRectF& origRect, double amount)
{
    return (origRect.isEmpty()) ?
            origRect : origRect.adjusted(-amount, -amount, amount, amount);
}


void CanvasView::dragEnterEvent(QDragEnterEvent *event)
{
    if (canvas()->optStructuralEditingDisabled())
    {
        // Ignore drop event if structural editing is disabled.
        event->ignore();
        return;
    }

    if (event->mimeData()->hasFormat("application/x-dunnartshapetype"))
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void CanvasView::dragMoveEvent(QDragMoveEvent *event)
{
    if (canvas()->optStructuralEditingDisabled())
    {
        // Ignore drop event if structural editing is disabled.
        event->ignore();
        return;
    }

    if (event->mimeData()->hasFormat("application/x-dunnartshapetype"))
    {
        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void CanvasView::scrollContentsBy(int dx, int dy)
{
    QGraphicsView::scrollContentsBy(dx, dy);
    emit viewportChanged(viewportRect());
}

void CanvasView::dropEvent(QDropEvent *event)
{
    if (canvas()->optStructuralEditingDisabled())
    {
        // Ignore drop event if structural editing is disabled.
        event->ignore();
        return;
    }

    if (event->mimeData()->hasFormat("application/x-dunnartshapetype"))
    {
        QByteArray itemData =
                event->mimeData()->data("application/x-dunnartshapetype");
        QDataStream dataStream(&itemData, QIODevice::ReadOnly);

        QString type;
        QPoint offset;
        QPointF size;
        dataStream >> type >> offset >> size;

        QPointF shapeCentrePos =
                mapToScene(event->pos() - offset) + (size / 2);

        // Load this shape from a plugin if the factory supports it.
        PluginShapeFactory *factory = sharedPluginShapeFactory();
        ShapeObj *shape = factory->createShape(type);
        shape->initWithDimensions(0, shapeCentrePos.x(), shapeCentrePos.y(),
                size.x(), size.y());

        UndoMacro *macro = canvas()->beginUndoMacro(tr("Create Shape"));
        QUndoCommand *cmd = new CmdCanvasSceneAddItem(canvas(), shape);
        macro->addCommand(cmd);

        canvas()->interrupt_graph_layout();

        // Make the new shape the only selected one.
        canvas()->deselectAll();
        shape->setSelected(true);

        event->acceptProposedAction();
    }
    else
    {
        event->ignore();
    }
}

void CanvasView::mousePressEvent(QMouseEvent *event)
{
    if (canvas() == NULL)
    {
        return;
    }

    // Allow middle-click to scroll the canvas.
    if (event->button() == Qt::MidButton)
    {
        m_last_mouse_pos = mapFromGlobal(event->globalPos());
        m_hand_scrolling = true;
        QApplication::setOverrideCursor(Qt::ClosedHandCursor);
        return;
    }
    else if (event->button() == Qt::RightButton)
    {
        bool consumed = handleContextMenuEvent(event);
        if (consumed)
        {
            return;
        }
    }
#if 0
    else if (event->button() == Qt::LeftButton)
    {
        canvas()->interrupt_graph_layout();
        canvas()->restart_graph_layout();
    }
#endif

    QGraphicsView::mousePressEvent(event);
}

void CanvasView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (canvas() == NULL)
    {
        return;
    }

    // Allow double-click to fit canvas to diagram.
    if (event->button() == Qt::LeftButton)
    {
        if( !itemAt(event->pos()) )
        {
            double padding = 10.0;
            zoomToShowRect(expandRect(
                    diagramBoundingRect(canvas()->items()), padding));

            return;
        }
    }

    // Pass-through so double-clicks on canvas objects can go to them.
    QGraphicsView::mouseDoubleClickEvent(event);
}

void CanvasView::mouseReleaseEvent(QMouseEvent *event)
{
    if (canvas() == NULL)
    {
        return;
    }

    if (event->button() == Qt::MidButton)
    {
        // Allow middle-click to scroll the canvas.
        m_hand_scrolling = false;
        QApplication::restoreOverrideCursor();
        return;
    }

    QGraphicsView::mouseReleaseEvent(event);
}


void CanvasView::mouseMoveEvent(QMouseEvent* event)
{
    if (canvas() == NULL)
    {
        return;
    }

    // Allow middle-click to scroll the canvas.
    if (m_hand_scrolling)
    {
        QScrollBar* hBar = horizontalScrollBar();
        QScrollBar* vBar = verticalScrollBar();
        QPoint delta = mapFromGlobal(event->globalPos()) - m_last_mouse_pos;
        hBar->setValue(hBar->value() + 
                (isRightToLeft() ? delta.x() : -delta.x()));
        vBar->setValue(vBar->value() - delta.y());
        m_last_mouse_pos = mapFromGlobal(event->globalPos());
        return;
    }

    QGraphicsView::mouseMoveEvent(event);
}

void CanvasView::keyPressEvent(QKeyEvent *keyEvent)
{
    if (canvas() == NULL)
    {
        return;
    }

    if (keyEvent->key() == Qt::Key_Alt)
    {
        // The user can temporarily suspend the layout while holding a key.
        // This is known as ALT-dragging and can be used to add or remove
        // shapes from constraint relationships and alter the network
        // topology when topology preservation is turned on.
        canvas()->setLayoutSuspended(true);

        keyEvent->accept();
    }
    else
    {
        keyEvent->ignore();
    }
}

void CanvasView::keyReleaseEvent(QKeyEvent *keyEvent)
{
    if (canvas() == NULL)
    {
        return;
    }

    if (keyEvent->key() == Qt::Key_Alt)
    {
        if (canvas()->isLayoutSuspended())
        {
            // Cancel ALT-dragging when the user releases the key,
            // if the layout is still suspended at this point.
            canvas()->setLayoutSuspended(false);
        }

        keyEvent->accept();
    }
    else if (keyEvent->key() == Qt::Key_Plus ||
             keyEvent->key() == Qt::Key_Equal)
    {
        // The + (=) key will increase the detail level of any selected shapes.
        CanvasItemList selection = canvas()->selectedItems();
        for (int i = 0; i < selection.size(); ++i)
        {
            ShapeObj *shape = dynamic_cast<ShapeObj *> (selection.at(i));
            if (shape)
            {
                shape->changeDetailLevel(true);
            }
        }
    }
    else if (keyEvent->key() == Qt::Key_Minus)
    {
        // The - key will decrease the detail level of any selected shapes.
        CanvasItemList selection = canvas()->selectedItems();
        for (int i = 0; i < selection.size(); ++i)
        {
            ShapeObj *shape = dynamic_cast<ShapeObj *> (selection.at(i));
            if (shape)
            {
                shape->changeDetailLevel(false);
            }
        }
    }
    else
    {
        keyEvent->ignore();
    }
}


QAction *CanvasView::buildAndExecContextMenu(QMouseEvent *event,
        QMenu& menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }
    QAction *zoomToFitDiagram =
            menu.addAction(tr("Zoom to fit entire diagram"));
    QAction *zoomToFitSelection =
            menu.addAction(tr("Zoom to fit current selection"));
    QAction *zoomToFitPage =
            menu.addAction(tr("Zoom to fit page"));
    menu.addSeparator();
    QAction* fitToDiagram = menu.addAction(
            tr("Fit page to entire diagram"));
    QAction* fitToSelection = menu.addAction(
            tr("Fit page to current selection"));
    QAction* fitToViewport = menu.addAction(
            tr("Fit page to current viewport"));

    if (canvas()->selectedItems().empty())
    {
        zoomToFitSelection->setDisabled(true);
        fitToSelection->setDisabled(true);
    }

    if (canvas()->pageRect().isEmpty())
    {
        zoomToFitPage->setDisabled(true);
    }

    QAction *action = menu.exec(event->globalPos(), fitToDiagram);

    double padding = 10;
    if (action == zoomToFitDiagram)
    {
        zoomToShowRect(
                expandRect(diagramBoundingRect(canvas()->items()), padding));
    }
    else if (action == zoomToFitSelection)
    {
        zoomToShowRect(
                expandRect(diagramBoundingRect(canvas()->selectedItems()),
                padding));
    }
    else if (action == zoomToFitPage)
    {
        zoomToShowRect(expandRect(canvas()->pageRect(), padding));
    }
    else if (action == fitToViewport)
    {
        canvas()->setPageRect(viewportRect());
    }
    else if (action == fitToDiagram)
    {
        canvas()->setPageRect(
                expandRect(diagramBoundingRect(
                        canvas()->items()), padding));
    }
    else if (action == fitToSelection)
    {
        canvas()->setPageRect(expandRect(diagramBoundingRect(
                canvas()->selectedItems()), padding));
    }
    event->accept();

    return action;
}

// Get the current viewport rect.
QRectF CanvasView::viewportRect(void) const
{
    QRectF visibleRect(mapToScene(0,0),
            mapToScene(viewport()->width(), viewport()->height()));
    return visibleRect;
}


QPointF CanvasView::centre(void) const
{
    return viewportRect().center();
}

void CanvasView::setCentre(QPointF newCentre)
{
    centerOn(newCentre);
}

void CanvasView::zoomToShowRect(const QRectF& rect)
{
    fitInView(rect, Qt::KeepAspectRatio);

    // XXX There is no signal in Qt for when the QGraphicsView changed its
    //     transform. So we do this ourselves to allow things like the zoom
    //     slider to update when the view transform changes.
    if (transform() != m_last_transform)
    {
        m_last_transform = transform();
        emit canvasTransformChanged(m_last_transform);
    }
}

bool CanvasView::handleContextMenuEvent(QMouseEvent *event)
{
    if (canvas() == NULL)
    {
        return false;
    }
    event->setAccepted(false);

    if( !itemAt(event->pos()) )
    {
        QMenu menu;
        buildAndExecContextMenu(event, menu);
    }

    return event->isAccepted();
}


/**
 * Do tasks that need to happen after the diagram has been loaded.
 */
void CanvasView::postDiagramLoad(void)
{
    if (canvas() == NULL)
    {
        return;
    }

    canvas()->postDiagramLoad();

    // We make the initial scene large, so there will be some initial scroll
    // room for small diagrams and the starting diagram won't be smaller than
    // the view, which can cause jumps when the scene rect is resized and gets
    // re-centred in the view.
    qreal border = 2000;
    QPolygonF scene_poly = mapToScene(-border, -border,
            viewport()->width() + (2 * border),
            viewport()->height() + (2 * border));
    QRectF view_scene_rect = scene_poly.boundingRect();
    QRectF scene_rect = scene()->sceneRect();
    scene_rect = scene_rect.united(view_scene_rect);
    setSceneRect(scene_rect);

    // Start with the canvas view centred on the diagram content.
    centerOn(diagramBoundingRect(canvas()->items()).center());
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

