/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2010-2011  Monash University
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
 * Author(s): Sarah Boyd  <Sarah.Boyd@monash.edu>
*/


#include "pdprocessnode.h"

using namespace dunnart;

////////////////////////////////////////////////////////////////////////////////////////////////////
//                                   PROCESS NODE FUNCTIONS
////////////////////////////////////////////////////////////////////////////////////////////////////

ProcessNode::ProcessNode(int o, PDProcessNodeType pt) : ShapeObj(), orientation(o), rect(16), processType(pt)
{
    if (processType == UNCERTAIN) { setLabel("?"); }
    else if (processType == OMITTED) { setLabel("\\\\"); }
}

QPainterPath ProcessNode::buildPainterPath(void)
{
    QPainterPath p;

    if (processType == ASSOCIATION) { // AssociationProcessNode is a single circle (filled with black)
        p.addEllipse( -rect/2, -rect/2, rect, rect );
    }
    else if (processType == DISSOCIATION) { // DissociationProcessNode is circle within a circle
        p.setFillRule(Qt::WindingFill);
        p.addEllipse( -rect/2+4, -rect/2+4, rect-8, rect-8 );
        p.addEllipse( -rect/2, -rect/2, rect, rect );
    }
    // Otherwise, this must be one of the square-shaped process nodes:
    else  {
        p.addRect(-rect/2, -rect/2, rect, rect);
    }

    // ******* IN THIS CASE, STEMS ARE DRAWN IN THE HORIZONTAL DIRECTION:
    if (this->orientation == Qt::Horizontal) {
        p.moveTo( -0.5*width(), 0 );
        p.lineTo( -rect/2, 0 );
        p.moveTo( rect/2, 0);
        p.lineTo( width()/2, 0);
    }
    // ******* IN THIS CASE, STEMS ARE DRAWN IN THE VERTICAL DIRECTION:
    else {
        p.moveTo( 0, -0.5*height() );
        p.lineTo( 0, -rect/2 );
        p.moveTo( 0, rect/2 );
        p.lineTo( 0, 0.5*height() );
    }

    return p;
}

// This is needed to make sure nodes of type ASSOCIATION are filled with black
void ProcessNode::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    // Call the parent paint method, to draw the node and label
    ShapeObj::paint(painter, option, widget);

    // Then fill the node with black if necessary
    if (processType == ProcessNode::ASSOCIATION) {
        painter->fillPath(painterPath(), Qt::black);
    }
}

QAction *ProcessNode::buildAndExecContextMenu(QGraphicsSceneMouseEvent *event,
                                              QMenu& menu)
{
    if (!menu.isEmpty())
    {
        menu.addSeparator();
    }

    QMenu *changeTypeMenu = new QMenu("Change Process Type", NULL);
    menu.addMenu(changeTypeMenu);
    QAction* changeToProcess = changeTypeMenu->addAction(tr("Process"));
    QAction* changeToOmitted = changeTypeMenu->addAction(tr("Omitted"));
    QAction* changeToUncertain = changeTypeMenu->addAction(tr("Uncertain"));
    QAction* changeToAssociation = changeTypeMenu->addAction(tr("Association"));
    QAction* changeToDissociation = changeTypeMenu->addAction(tr("Dissociation"));
    QAction* changeToPhenotype = changeTypeMenu->addAction(tr("Phenotype"));

    QAction* switchNodeOrientation = menu.addAction(tr("Switch node orientation"));

    QAction *action = ShapeObj::buildAndExecContextMenu(event, menu);

    if (action == changeToProcess) { setProcessType(ProcessNode::PROCESS); }
    else if (action == changeToOmitted) { setProcessType(ProcessNode::OMITTED); }
    else if (action == changeToUncertain) { setProcessType(ProcessNode::UNCERTAIN); }
    else if (action == changeToAssociation) { setProcessType(ProcessNode::ASSOCIATION); }
    else if (action == changeToDissociation) { setProcessType(ProcessNode::DISSOCIATION); }
    else if (action == changeToPhenotype) { setProcessType(ProcessNode::PHENOTYPE); }
    else if (action == switchNodeOrientation) {
        orientation = (!orientation);
        prepareGeometryChange();
        setPainterPath(buildPainterPath());
    }

    return action;
}

void ProcessNode::setProcessType(ProcessNode::PDProcessNodeType pnt)
{
    processType = pnt;
    switch(pnt) {
    case (ProcessNode::OMITTED) :
        ShapeObj::setLabel(QString("\\\\"));
        break;
    case (ProcessNode::UNCERTAIN) :
        ShapeObj::setLabel(QString("?"));
        break;
    case (ProcessNode::PHENOTYPE) :
        ShapeObj::setLabel(QString("?"));
        break;
    default :
        ShapeObj::setLabel(QString(""));
    }
    setPainterPath(buildPainterPath());
    update();
}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
