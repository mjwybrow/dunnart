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

#ifndef CONNECTORPROPERTIES_H
#define CONNECTORPROPERTIES_H

#include "libdunnartcanvas/gen-ui/ui_connectorproperties.h"
#include <QDockWidget>

namespace dunnart {

class Canvas;

class ConnectorPropertiesDialog : public QDockWidget, private Ui::ConnectorProperties
{
    Q_OBJECT

    public:
        ConnectorPropertiesDialog(Canvas *canvas, QWidget *parent = NULL);
        ~ConnectorPropertiesDialog();

    signals:
        // Used to pass on state changes from widgets back to the current
        // canvas.
        void optChangedConnRoundingDist(int value);
        void optChangedConnPenaltyCrossing(int value);
        void optChangedRoutingShapeBuffer(int value);
        void optChangedConnPenaltySegment(int value);
        void optChangedConnPenaltyFixedSharedPath(int value);
        void optChangedConnPerformNudging(bool checked);

        // Used privately for passing on state changes from the
        // canvas to widgets in this dialog.
        void setOptChangedConnRoundingDist(const int value);
        void setOptChangedConnPenaltyCrossing(const int value);
        void setOptChangedRoutingShapeBuffer(const int value);
        void setOptChangedConnPenaltySegment(const int value);
        void setOptChangedConnPenaltyFixedSharedPath(const int value);
        void setOptChangedConnPerformNudging(const bool checked);
    private slots:
        void changeCanvas(Canvas *canvas);
    private:
        Canvas *m_canvas;
};

}

#endif // CONNECTORPROPERTIES_H
