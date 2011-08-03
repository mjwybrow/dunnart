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

#include <QSignalMapper>

#include "createdistribution.h"
#include "canvas.h"
#include "guideline.h"

namespace dunnart {

CreateDistributionDialog::CreateDistributionDialog(Canvas *canvas, QWidget *parent)
    : QDockWidget(parent),
      m_canvas(NULL)
{
    this->setupUi(this);

    m_signal_mapper = new QSignalMapper(this);
    m_signal_mapper->setMapping(horiLeftButton, (int) DIST_LEFT);
    m_signal_mapper->setMapping(horiCentreButton, (int) DIST_CENTER);
    m_signal_mapper->setMapping(horiRightButton, (int) DIST_RIGHT);
    m_signal_mapper->setMapping(vertTopButton, (int) DIST_TOP);
    m_signal_mapper->setMapping(vertCentreButton, (int) DIST_MIDDLE);
    m_signal_mapper->setMapping(vertBottomButton, (int) DIST_BOTTOM);

    connect(horiLeftButton, SIGNAL(clicked()), m_signal_mapper, SLOT(map()));
    connect(horiCentreButton, SIGNAL(clicked()), m_signal_mapper, SLOT(map()));
    connect(horiRightButton, SIGNAL(clicked()), m_signal_mapper, SLOT(map()));
    connect(vertTopButton, SIGNAL(clicked()), m_signal_mapper, SLOT(map()));
    connect(vertCentreButton, SIGNAL(clicked()), m_signal_mapper, SLOT(map()));
    connect(vertBottomButton, SIGNAL(clicked()), m_signal_mapper, SLOT(map()));

    changeCanvas(canvas);
}

CreateDistributionDialog::~CreateDistributionDialog()
{
    delete m_signal_mapper;
}

void CreateDistributionDialog::changeCanvas(Canvas *canvas)
{
    if (m_canvas)
    {
        disconnect(m_canvas, 0, this, 0);
        disconnect(this, 0, m_canvas, 0);
        disconnect(m_signal_mapper, 0, m_canvas, 0);
    }
    m_canvas = canvas;

    connect(m_signal_mapper, SIGNAL(mapped(int)),
            m_canvas, SLOT(distributeSelection(int)));
}

}
