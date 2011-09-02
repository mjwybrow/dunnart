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

#include "layoutproperties.h"
#include "canvas.h"
#include "canvasview.h"


namespace dunnart {


LayoutPropertiesDialog::LayoutPropertiesDialog(Canvas *canvas, QWidget *parent)
    : QDockWidget(parent),
      m_canvas(NULL),
      m_mode_signal_mapper(NULL)
{
    setupUi(this);

    m_mode_signal_mapper = new QSignalMapper(this);
    m_mode_signal_mapper->setMapping(
            organicStructureButton, LAYOUT_STRUCTURE_ORGANIC);
    m_mode_signal_mapper->setMapping(
            flowStructureButton, LAYOUT_STRUCTURE_FLOW);
    m_mode_signal_mapper->setMapping(
            layeredStuctureButton, LAYOUT_STRUCTURE_LAYERED);

    connect(m_mode_signal_mapper, SIGNAL(mapped(int)),
            this, SIGNAL(setOptStructuralLayoutMode(int)));

    connect(organicStructureButton, SIGNAL(clicked()),
            m_mode_signal_mapper, SLOT(map()));
    connect(flowStructureButton, SIGNAL(clicked()),
            m_mode_signal_mapper, SLOT(map()));
    connect(layeredStuctureButton, SIGNAL(clicked()),
            m_mode_signal_mapper, SLOT(map()));

    connect(layoutButton, SIGNAL(clicked(bool) ),
            this, SIGNAL(setOptAutomaticGraphLayout(bool)));

    connect(preventOverlapsCheckBox, SIGNAL(clicked(bool) ),
            this, SIGNAL(setOptPreventOverlaps(bool)));
    connect(this, SIGNAL(optChangedPreventOverlaps(bool) ),
            preventOverlapsCheckBox, SLOT(setChecked(bool)));

    connect(preventOverlapsCheckBox2, SIGNAL(clicked(bool) ),
            this, SIGNAL(setOptPreventOverlaps(bool)));
    connect(this, SIGNAL(optChangedPreventOverlaps(bool) ),
            preventOverlapsCheckBox2, SLOT(setChecked(bool)));

    connect(preserveTopologyCheckBox, SIGNAL(clicked(bool) ),
            this, SIGNAL(setOptPreserveTopology(bool)));
    connect(this, SIGNAL(optChangedPreserveTopology(bool) ),
            preserveTopologyCheckBox, SLOT(setChecked(bool)));

    connect(rubberBandRoutingCheckBox, SIGNAL(clicked(bool) ),
            this, SIGNAL(setOptRubberBandRouting(bool)));
    connect(this, SIGNAL(optChangedRubberBandRouting(bool) ),
            rubberBandRoutingCheckBox, SLOT(setChecked(bool)));

    connect(rubberBandRoutingCheckBox2, SIGNAL(clicked(bool) ),
            this, SIGNAL(setOptRubberBandRouting(bool)));
    connect(this, SIGNAL(optChangedRubberBandRouting(bool) ),
            rubberBandRoutingCheckBox2, SLOT(setChecked(bool)));

    connect(pageBoundaryCheckBox, SIGNAL(clicked(bool) ),
            this, SIGNAL(setOptFitWithinPage(bool)));
    connect(this, SIGNAL(optChangedFitWithinPage(bool) ),
            pageBoundaryCheckBox, SLOT(setChecked(bool)));

    connect(pageBoundaryCheckBox2, SIGNAL(clicked(bool) ),
            this, SIGNAL(setOptFitWithinPage(bool)));
    connect(this, SIGNAL(optChangedFitWithinPage(bool) ),
            pageBoundaryCheckBox2, SLOT(setChecked(bool)));

    // Preserve topology control should only be enabled when overlaps are.
    connect(this, SIGNAL(optChangedPreventOverlaps(bool) ),
            preserveTopologyCheckBox, SLOT(setEnabled(bool)));

    changeCanvas(canvas);
}


void LayoutPropertiesDialog::changeCanvas(Canvas *canvas)
{
    if (m_canvas)
    {
        disconnect(m_canvas, 0, this, 0);
        disconnect(this, 0, m_canvas, 0);
        disconnect(idealLengthSlider, 0, m_canvas, 0);
    }
    m_canvas = canvas;

    connect(this, SIGNAL(setOptAutomaticGraphLayout(bool)),
            m_canvas, SLOT(setOptAutomaticGraphLayout(bool)));
    connect(m_canvas, SIGNAL(optChangedAutomaticLayout(bool) ),
            this, SLOT(changeAutomaticLayoutMode(bool)));

    connect(this, SIGNAL(setOptStructuralLayoutMode(int)),
            m_canvas, SLOT(setOptLayoutMode(int)));
    connect(m_canvas, SIGNAL(optChangedLayoutMode(int)),
            this, SLOT(changeStructuralLayoutMode(int)));

    connect(this, SIGNAL(setOptPreventOverlaps(bool)),
            m_canvas, SLOT(setOptPreventOverlaps(bool)));
    connect(m_canvas, SIGNAL(optChangedPreventOverlaps(bool) ),
            this, SIGNAL(optChangedPreventOverlaps(bool)));

    connect(this, SIGNAL(setOptPreserveTopology(bool) ),
            m_canvas, SLOT(setOptPreserveTopology(bool) ));
    connect(m_canvas, SIGNAL(optChangedPreserveTopology(bool) ),
            this, SIGNAL(optChangedPreserveTopology(bool) ));

    connect(this, SIGNAL(setOptRubberBandRouting(bool)),
            m_canvas, SLOT(setOptRubberBandRouting(bool)));
    connect(m_canvas, SIGNAL(optChangedRubberBandRouting(bool) ),
            this, SIGNAL(optChangedRubberBandRouting(bool) ));

    connect(this, SIGNAL(setOptFitWithinPage(bool)),
            m_canvas, SLOT(setOptFitWithinPage(bool)));
    connect(m_canvas, SIGNAL(optChangedFitWithinPage(bool) ),
            this, SIGNAL(optChangedFitWithinPage(bool) ));

    connect(idealLengthSlider, SIGNAL(sliderMoved(int)),
            m_canvas, SLOT(setOptIdealEdgeLengthModifierFromSlider(int)));
    connect(m_canvas, SIGNAL(optChangedIdealEdgeLengthModifier(double)),
            this, SLOT(changeIdealEdgeLength(double)));

    connect(downwardSeparationSlider, SIGNAL(sliderMoved(int)),
            m_canvas, SLOT(setOptDirectedEdgeSeparationModifierFromSlider(int)));
    connect(m_canvas, SIGNAL(optChangedDirectedEdgeSeparationModifier(double)),
            this, SLOT(changeDirectedEdgeSeparationModifier(double)));

    // Set initial control values.
    idealLengthSlider->setSliderPosition(
            m_canvas->optIdealEdgeLengthModifier() * 100);
    downwardSeparationSlider->setSliderPosition(
            m_canvas->optDirectedEdgeSeparationModifier() * 100);
    bool value = m_canvas->optPreventOverlaps();
    preventOverlapsCheckBox->setChecked(value);
    preventOverlapsCheckBox2->setChecked(value);
    preserveTopologyCheckBox->setEnabled(value);

    value = m_canvas->optPreserveTopology();
    preserveTopologyCheckBox->setChecked(value);

    value = m_canvas->optRubberBandRouting();
    rubberBandRoutingCheckBox->setChecked(value);
    rubberBandRoutingCheckBox2->setChecked(value);

    value = m_canvas->optFitWithinPage();
    pageBoundaryCheckBox->setChecked(value);
    pageBoundaryCheckBox2->setChecked(value);

#ifdef NOGRAPHVIZ
    layeredStuctureButton->setDisabled(true);
#endif

    changeAutomaticLayoutMode(m_canvas->optAutomaticGraphLayout());
    changeStructuralLayoutMode(m_canvas->optLayoutMode());
}

void LayoutPropertiesDialog::changeStructuralLayoutMode(int mode)
{
    switch(mode)
    {
        case LAYOUT_STRUCTURE_ORGANIC:
            organicStructureButton->setChecked(true);
            flowStructureButton->setChecked(false);
            layeredStuctureButton->setChecked(false);
            downwardSeparationLabel->setEnabled(false);
            downwardSeparationSlider->setEnabled(false);
            break;
        case LAYOUT_STRUCTURE_FLOW:
            organicStructureButton->setChecked(false);
            flowStructureButton->setChecked(true);
            layeredStuctureButton->setChecked(false);
            downwardSeparationLabel->setEnabled(true);
            downwardSeparationSlider->setEnabled(true);
            break;
        case LAYOUT_STRUCTURE_LAYERED:
            organicStructureButton->setChecked(false);
            flowStructureButton->setChecked(false);
            layeredStuctureButton->setChecked(true);
            downwardSeparationLabel->setEnabled(false);
            downwardSeparationSlider->setEnabled(false);
            break;
        default:
            break;
    }
}

void LayoutPropertiesDialog::changeAutomaticLayoutMode(bool auto_layout)
{
    layoutButton->setChecked(auto_layout);
    if (auto_layout)
    {
        layoutModeLabel->setText(tr("Graph layout mode:\nAutomatic layout"));
        layoutOptionPages->setCurrentIndex(1);
    }
    else
    {
        layoutModeLabel->setText(tr("Graph layout mode:\nManual layout"));
        layoutOptionPages->setCurrentIndex(0);
    }
}

void LayoutPropertiesDialog::changeDirectedEdgeSeparationModifier(double value)
{
    downwardSeparationSlider->setSliderPosition(value * 100);
}

void LayoutPropertiesDialog::changeIdealEdgeLength(double value)
{
    idealLengthSlider->setSliderPosition(value * 100);
}

}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
