/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow  <mjwybrow@users.sourceforge.net>
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
*/

#include <QDebug>

#include "zoomlevel.h"
#include "canvas.h"

#include "canvasview.h"

namespace dunnart {


ZoomLevel::ZoomLevel(CanvasView *canvasview, QWidget *parent)
    : QDockWidget(parent),
      m_canvasview(canvasview),
      m_zoom_level(1.0)
{
    setupUi(this);

    changeCanvasView(canvasview);

    connect(zoomSlider, SIGNAL(valueChanged(int)),
            this, SLOT(changeZoomLevel(int)));
}

void ZoomLevel::changeCanvasView(CanvasView *canvasview)
{
    m_canvasview = canvasview;

    QTransform transform = m_canvasview->transform();
    // m11 is horizontal scale.  Should match m22
    double scale = transform.m11();
    Q_ASSERT(scale == transform.m22());

    // Update the slider position to the zoom for the current canvas.
    int zoom = scale * 100;
    m_zoom_level = zoom / 100.0;
    zoomSlider->setSliderPosition(zoom);
}

void ZoomLevel::changeZoomLevel(int zoom)
{
    // Set a new zoom level for this canvas.
    double scale = ((zoom / 100.0) / m_zoom_level);
    m_canvasview->scale(scale, scale);
    m_zoom_level = zoom / 100.0;
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
