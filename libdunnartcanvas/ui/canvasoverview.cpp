/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2012  Monash University
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
 * Author: Michael Wybrow <mjwybrow@users.sourceforge.net>
*/

#include <QtCore>
#include <QtGui>

#include "canvasoverview.h"

#include "libdunnartcanvas/gen-ui/ui_canvasoverview.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/connector.h"


namespace dunnart {

class CanvasOverviewWidget : public QWidget
{
    public:
        CanvasOverviewWidget(QWidget *parent = 0, Qt::WindowFlags f = 0)
            : QWidget(parent, f),
              m_canvasview(NULL)
        {
        }
        void setCanavasView(CanvasView *canvasview)
        {
            m_canvasview = canvasview;
            this->update();
        }
    protected:
        void mouseReleaseEvent(QMouseEvent *event)
        {
            // Centre the canvas where the user clicked on the overview.

            // Recentre the canvas view, animating the "centre" property.
            QPropertyAnimation *animation =
                    new QPropertyAnimation(m_canvasview, "centre");

            animation->setDuration(500);
            //animation->setEasingCurve(QEasingCurve::OutInCirc);
            animation->setStartValue(m_canvasview->centre());
            animation->setEndValue(m_transform.inverted().map(event->pos()));
            animation->start();
        }

        void paintEvent(QPaintEvent *event)
        {
            Q_UNUSED (event)

            // XXX Maybe we should cache this and recompute separately for
            //     transform changes, shape movement and viewport changes?

            // Draw a white background.
            QPainter painter(this);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.setPen(Qt::white);
            painter.setBrush(Qt::white);
            QRectF drawingRect = QRectF(0, 0, width(), height());
            painter.drawRect(drawingRect);

            if (m_canvasview == NULL)
            {
                return;
            }

            // Compute the diagram bounds.
            Canvas *canvas = m_canvasview->canvas();
            QRectF diagramBounds =  diagramBoundingRect(canvas->items());
            double buffer = 50;
            diagramBounds.adjust(-buffer, -buffer, buffer, buffer);

            // Compute the scale to with the drawing into the overview rect.
            qreal xscale = drawingRect.width() / diagramBounds.width();
            qreal yscale = drawingRect.height() /  diagramBounds.height();

            // Choose the smallest of the two scale values.
            qreal scale = std::min(xscale, yscale);

            // Scale uniformly, and transform to center in the overview.
            QTransform scaleTransform = QTransform::fromScale(scale, scale);
            QRectF targetRect = scaleTransform.mapRect(diagramBounds);
            QPointF diff = drawingRect.center() - targetRect.center();
            m_transform = QTransform();
            m_transform.translate(diff.x(), diff.y());
            m_transform.scale(scale, scale);

            // Draw edges in overview for each connector on the canvas.
            painter.setPen(QColor(0, 0, 0, 100));
            QList<CanvasItem *> items = canvas->items();
            for (int i = 0; i < items.count(); ++i)
            {
                Connector *connector = dynamic_cast<Connector *> (items.at(i));
                if (connector)
                {
                    QPair<ShapeObj *, ShapeObj *> endShapes =
                            connector->getAttachedShapes();
                    if (!endShapes.first || !endShapes.second)
                    {
                        continue;
                    }

                    QLineF line(endShapes.first->centrePos(),
                                endShapes.second->centrePos());
                    painter.drawLine(m_transform.map(line));
                }
            }

            // Draw Rectangles in overview for each shape on the canvas.
            painter.setPen(Qt::black);
            painter.setBrush(Qt::darkGray);
            QRectF shapeRect;
            for (int i = 0; i < items.count(); ++i)
            {
                ShapeObj *shape = dynamic_cast<ShapeObj *> (items.at(i));
                if (shape)
                {
                    shapeRect.setSize(shape->size());
                    shapeRect.moveCenter(shape->centrePos());
                    painter.drawRect(m_transform.mapRect(shapeRect));
                }
            }

            // Show where the visible viewport is (by making everything
            // outside this have a light grey overlay).
            QColor grey(0, 0, 0, 60);
            painter.setPen(QPen(Qt::transparent));
            painter.setBrush(QBrush(grey));
            QRectF viewRect = m_transform.mapRect(m_canvasview->viewportRect());
            QPolygon polygon = QPolygon(drawingRect.toRect()).subtracted(
                    QPolygon(viewRect.toRect()));
            painter.drawPolygon(polygon);
        }
    private:
        CanvasView *m_canvasview;
        QTransform m_transform;
};

void CanvasOverviewDialog::canvasViewportChanged(QRectF viewRect)
{
    Q_UNUSED (viewRect)

    update();
}

void CanvasOverviewDialog::canvasSceneChanged(QList<QRectF> rects)
{
    Q_UNUSED (rects)

    update();
}

CanvasOverviewDialog::CanvasOverviewDialog(CanvasView *view, QWidget *parent)
    : QDockWidget(parent),
      ui(new Ui::CanvasOverview),
      m_canvasview(view)
{
    ui->setupUi(this);

    m_canvasoverview = new CanvasOverviewWidget();
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_canvasoverview);

    ui->frame->setLayout(layout);

    changeCanvasView(view);
}

CanvasOverviewDialog::~CanvasOverviewDialog()
{
    delete ui;
}

void CanvasOverviewDialog::changeCanvasView(CanvasView *canvasview)
{
    if (m_canvasview)
    {
        disconnect(m_canvasview, 0, this, 0);
        disconnect(this, 0, m_canvasview, 0);
    }
    m_canvasview = canvasview;
    m_canvasoverview->setCanavasView(m_canvasview);

    // Trigger redraw when the viewport is scrolled or changes size.
    connect(m_canvasview, SIGNAL(viewportChanged(QRectF)),
            this, SLOT(canvasViewportChanged(QRectF)));

    // Trigger redraw when the diagram layout changes.
    connect(m_canvasview->canvas(), SIGNAL(changed(QList<QRectF>)),
            this, SLOT(canvasSceneChanged(QList<QRectF>)));
}

}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
