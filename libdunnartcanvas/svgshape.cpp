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
 * Author(s): Michael Wybrow  <http://michael.wybrow.info/>
*/

#include <QGraphicsSvgItem>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <cassert>

#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/svgshape.h"

#include "libavoid/geomtypes.h"

namespace dunnart {

SvgShape::SvgShape()
    : ShapeObj("svgNode")
{
    // Restore the cache type from normal QGraphicsSVGItems.
    // XXX Don't enable caching, as there seems to be a bug that
    //     prevents SVG output of these cached shapes.
    //setCacheMode(QGraphicsItem::DeviceCoordinateCache);

    setSizeLocked(true);
}

void SvgShape::initWithXMLProperties(Canvas *canvas,
        const QDomElement& node, const QString& ns)
{
    // Call equivalent superclass method.
    ShapeObj::initWithXMLProperties(canvas, node, ns);

    m_svg_id = nodeAttribute(node, QString(), x_id);

    QString value = nodeAttribute(node, QString(), "transform");
    if (!value.isEmpty())
    {
        // Take position from transform, which will be of the form:
        //      "translate(xx.xx yy.yy)"
        QStringList list = value.split(QRegExp(" |\\(|\\)"));
        if (list.at(0) == "translate")
        {
            m_svg_pos = QPointF(list.at(1).toDouble(), list.at(2).toDouble());
        }
        else if (list.at(0) == "matrix")
        {
            QTransform transform(list.at(1).toDouble(),
                    list.at(2).toDouble(), list.at(3).toDouble(),
                    list.at(4).toDouble(), list.at(5).toDouble(),
                    list.at(6).toDouble());
            m_svg_pos = QPointF(list.at(5).toDouble(), list.at(6).toDouble());
        }
    }
}


QVariant SvgShape::itemChange(GraphicsItemChange change,
        const QVariant &value)
{
    if (change == QGraphicsItem::ItemSceneChange)
    {
        Canvas *canvas = dynamic_cast<Canvas *> (value.value<QGraphicsScene *>());
        if (canvas)
        {
            setSharedRenderer(canvas->svgRenderer());
            setElementId(m_svg_id);
        }
    }
    else if (change == QGraphicsItem::ItemSceneHasChanged)
    {
        if (canvas())
        {
            QRectF svgBoundingRect = QGraphicsSvgItem::boundingRect();

            setSize(svgBoundingRect.size());

            QPointF offset(svgBoundingRect.width() / 2,
                    svgBoundingRect.height() / 2);
            //QTransform transform;
            //transform.translate(-offset.x(), -offset.y());
            //this->setTransform(transform);

            QGraphicsItem::setPos(m_svg_pos - offset);
        }
    }
    return ShapeObj::itemChange(change, value);
}

QRectF SvgShape::boundingRect(void) const
{
    double b = 4;
    return QGraphicsSvgItem::boundingRect().adjusted(-b, -b, b, b);
}


QPointF SvgShape::centrePos(void) const
{
    // Pos is the top left.
    QPointF offset(size().width() / 2, size().height() / 2);
    return pos() + offset;
}

void SvgShape::setCentrePos(const QPointF& newPos)
{
    if (canvas() && !canvas()->processingLayoutUpdates())
    {
        Actions& actions = canvas()->getActions();
        actions.moveList.push_back(this);
    }
    QPointF offset(size().width() / 2, size().height() / 2);
    CanvasItem::setPos(newPos - offset);
}


void SvgShape::paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
           QWidget *widget)
{
    assert(painter->isActive());

    if (isSelected())
    {
        QPen highlight;
        highlight.setColor(QColor(0, 255, 255, 100));
        highlight.setWidth(7);
        highlight.setCosmetic(true);
        // Draw selection cue.
        painter->setPen(highlight);
        //painter->setBrush(QBrush(QColor(0, 255, 255, 100)));

        QRectF rect = shapeRect();
        rect.moveTopLeft(QPointF(0,0));
        painter->drawRect(rect);
    }

    QStyleOptionGraphicsItem styleOption(*option);
    styleOption.state &= ! QStyle::State_Selected;
    QGraphicsSvgItem::paint(painter, &styleOption, widget);
}

QPainterPath SvgShape::shape(void) const
{
    return QGraphicsItem::shape();
}


};
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
