// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2010  Monash University
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

#include <QPainter>
#include <QLabel>
#include <QGridLayout>
#include <QMouseEvent>
#include <QDrag>

#include "shapepickerdialog.h"
#include "canvas.h"
#include "canvasitem.h"
#include "shape.h"
#include "pluginshapefactory.h"

namespace dunnart {

ShapePickerDialog::ShapePickerDialog(Canvas *canvas, QWidget *parent)
    : QDockWidget(parent),
      m_canvas(NULL)
{
    this->setupUi(this);

    PluginShapeFactory *factory = sharedPluginShapeFactory();
    QStringList shapeClasses = factory->shapeClassLabels();
    comboBox->addItems(shapeClasses);

    m_grid_layout = new QGridLayout();
    scrollAreaWidgetContents->setLayout(m_grid_layout);
    // Add a stretchable third column.
    m_grid_layout->setColumnStretch(2, 1);

    connect(comboBox, SIGNAL(currentIndexChanged(QString)),
            this, SLOT(changeIndex(QString)));

    if (shapeClasses.empty())
    {
        comboBox->setEnabled(false);
    }
    else
    {
        changeIndex(shapeClasses.first());
    }
    changeCanvas(canvas);
}

ShapePickerDialog::~ShapePickerDialog()
{
}


void ShapePickerDialog::mousePressEvent(QMouseEvent *event)
{
    QPoint adjustedPos =
            scrollAreaWidgetContents->mapFromGlobal(event->globalPos());
    QLabel *child = dynamic_cast<QLabel*>
            (scrollAreaWidgetContents->childAt(adjustedPos));
    if (!child)
    {
        return;
    }

    QPixmap pixmap = *child->pixmap();

    QByteArray itemData;
    QDataStream dataStream(&itemData, QIODevice::WriteOnly);
    dataStream << child->property("type").toString() <<
                  QPoint(adjustedPos - child->pos()) <<
                  QPointF(child->width(), child->height());

    QMimeData *mimeData = new QMimeData;
    mimeData->setData("application/x-dunnartshapetype", itemData);

    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->setHotSpot(adjustedPos - child->pos());

    drag->exec(Qt::CopyAction);
    /*
    QPixmap tempPixmap = pixmap;
    QPainter painter;
    painter.begin(&tempPixmap);
    painter.fillRect(pixmap.rect(), QColor(127, 127, 127, 127));
    painter.end();

    child->setPixmap(tempPixmap);

    if (drag->exec(Qt::CopyAction) == Qt::MoveAction)
        child->close();
    else {
        child->show();
        child->setPixmap(pixmap);
    }
    */
}

void ShapePickerDialog::changeCanvas(Canvas *canvas)
{
    if (m_canvas)
    {
        disconnect(m_canvas, 0, this, 0);
        disconnect(this, 0, m_canvas, 0);
    }
    m_canvas = canvas;
}

void ShapePickerDialog::changeIndex(const QString& text)
{
    // Delete any existing icons shown.
    foreach (QObject *icon, m_shape_icons)
    {
        icon->deleteLater();
    }
    m_shape_icons.clear();

    PluginShapeFactory *factory = sharedPluginShapeFactory();
    QStringList shapeTypes = factory->shapeTypesForClassLabel(text);
    int counter = 0;
    foreach (QString shapeType, shapeTypes)
    {
        // Create a transparent pixmap.
        QPixmap pixmap(70, 50);
        pixmap.fill(Qt::transparent);

        // Create a shape instance for drawing with.
        ShapeObj *shape = factory->createShape(shapeType);
        shape->initWithDimensions(0, 0, 0,
                pixmap.width() - 10, pixmap.height() - 10);
        shape->setupForShapePickerPreview();

        // Render the shape into the pixmap, at the center.
        QPainter painter;
        QStyleOptionGraphicsItem style;
        painter.begin(&pixmap);
        painter.translate(pixmap.width() / 2, pixmap.height() / 2);
        painter.setRenderHint(QPainter::Antialiasing);
        shape->paint(&painter, &style, NULL);
        painter.end();

        // Create a icon label object from the pixmap.
        QLabel *label = new QLabel(scrollAreaWidgetContents);
        label->setPixmap(pixmap);
        label->setProperty("type", shapeType);

        // Position the icon in correct row and column.
        m_grid_layout->addWidget(label, (counter / 2), (counter % 2));
        label->show();
        // Don't stretch this row.
        m_grid_layout->setRowStretch((counter / 2), 0);

        // Store this label for deleteing later if changing shape types.
        m_shape_icons.push_back(label);

        ++counter;
        delete shape;
    }
    // Add a stretchable row after the last one.
    m_grid_layout->setRowStretch(((counter - 1) / 2) + 1, 1);

}

}
