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

#include "connectorproperties.h"
#include "canvas.h"

namespace dunnart {

ConnectorPropertiesDialog::ConnectorPropertiesDialog(Canvas *canvas, QWidget *parent)
    : QDockWidget(parent),
      m_canvas(NULL)
{
    this->setupUi(this);

    connect(roundedCornersSlider, SIGNAL(sliderMoved(int)),
            this, SIGNAL(setOptChangedConnRoundingDist(int)));

    connect(nudgingCheckBox, SIGNAL(clicked(bool)),
            this, SIGNAL(setOptChangedConnPerformNudging(bool)));

    connect(segmentPenaltySlider, SIGNAL(sliderMoved(int)),
            this, SIGNAL(setOptChangedConnPenaltySegment(int)));

    connect(crossingPenaltySlider, SIGNAL(sliderMoved(int)),
            this, SIGNAL(setOptChangedConnPenaltyCrossing(int)));

    connect(pathOverlapPenaltySlider, SIGNAL(sliderMoved(int)),
            this, SIGNAL(setOptChangedConnPenaltyFixedSharedPath(int)));

    connect(shapePaddingSlider, SIGNAL(sliderMoved(int)),
            this, SIGNAL(setOptChangedRoutingShapeBuffer(int)));

    changeCanvas(canvas);
}

ConnectorPropertiesDialog::~ConnectorPropertiesDialog()
{
}

void ConnectorPropertiesDialog::changeCanvas(Canvas *canvas)
{
    if (m_canvas)
    {
        disconnect(m_canvas, 0, this, 0);
        disconnect(this, 0, m_canvas, 0);
    }
    m_canvas = canvas;

    connect(this, SIGNAL(setOptChangedConnPenaltySegment(int)),
            m_canvas, SLOT(setOptRoutingPenaltySegment(int)));

    connect(this, SIGNAL(setOptChangedConnRoundingDist(int)),
            m_canvas, SLOT(setOptConnRoundingDist(int)));

    connect(this, SIGNAL(setOptChangedRoutingShapeBuffer(int)),
            m_canvas, SLOT(setOptRoutingShapePadding(int)));

    // Set initial control values.
    roundedCornersSlider->setSliderPosition(
            m_canvas->optConnectorRoundingDistance());
    segmentPenaltySlider->setSliderPosition(
            m_canvas->optRoutingPenaltySegment());
    shapePaddingSlider->setSliderPosition(
            m_canvas->optRoutingShapePadding());
}

}
