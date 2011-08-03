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


#ifndef PDPPROCESSNODE_H
#define PDPROCESSNODE_H

#include "pdepn.h"

using namespace dunnart;

/***************************     Process Nodes     ***************************/
// Should process nodes just have a fixed size?  This is what I have done here ...
class ProcessNode: public ShapeObj
{
    Q_OBJECT
    Q_PROPERTY (PDProcessNodeType Process_Type READ getProcessType WRITE setProcessType)
    Q_ENUMS (PDProcessNodeType)

public:
    enum PDProcessNodeType { PROCESS, OMITTED, UNCERTAIN, ASSOCIATION, DISSOCIATION, PHENOTYPE };

    ProcessNode() : ShapeObj(), orientation(Qt::Horizontal), rect(16), processType(PROCESS)  {}
    ProcessNode(int o, PDProcessNodeType pt);
    QPainterPath buildPainterPath();
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QAction* buildAndExecContextMenu(QGraphicsSceneMouseEvent *event, QMenu& menu);
    void setProcessType(ProcessNode::PDProcessNodeType pnt);

    PDProcessNodeType getProcessType() { return processType; }

protected:
    int orientation;
    int rect;
    PDProcessNodeType processType;
};

Q_DECLARE_METATYPE (ProcessNode::PDProcessNodeType)

#endif // PDPROCESSNODE_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
