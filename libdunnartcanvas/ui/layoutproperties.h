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

#ifndef LAYOUTPROPERTIES_H_
#define LAYOUTPROPERTIES_H_

#include "libdunnartcanvas/gen-ui/ui_LayoutProperties.h"

namespace dunnart {


class Canvas;


class LayoutPropertiesDialog : public QDockWidget, private Ui::LayoutProperties
{
    Q_OBJECT

    public:
        LayoutPropertiesDialog(Canvas *canvas, QWidget *parent = NULL);

    signals:
        // Used to pass on state changes from widgets back to the current
        // canvas.
        void optChangedAutomaticLayout(bool checked);
        void optChangedPreserveTopology(bool checked);
        void optChangedPreventOverlaps(bool checked);
        void optChangedRubberBandRouting(bool checked);
        void optChangedFitWithinPage(bool checked);

        // Used privately for passing on state changes from the
        // canvas to widgets in this dialog.
        void setOptAutomaticGraphLayout(const bool value);
        void setOptPreventOverlaps(const bool value);
        void setOptPreserveTopology(const bool value);
        void setOptRubberBandRouting(const bool value);
        void setOptFitWithinPage(const bool value);
    private slots:
        void changeAutomaticLayoutMode(bool auto_layout);
        void changeCanvas(Canvas *canvas);
    private:
        Canvas *m_canvas;
};


}
#endif // LAYOUTPROPERTIES_H_
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

