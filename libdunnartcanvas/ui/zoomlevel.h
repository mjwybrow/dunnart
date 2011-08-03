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

#ifndef ZOOMLEVEL_H_
#define ZOOMLEVEL_H_

#include "libdunnartcanvas/gen-ui/ui_ZoomLevel.h"

namespace dunnart {


class CanvasView;

class ZoomLevel : public QDockWidget, private Ui::ZoomLevel
{
    Q_OBJECT

    public:
        ZoomLevel(CanvasView *canvasview, QWidget *parent = NULL);

    private slots:
        void changeZoomLevel(int zoom);
        void changeCanvasView(CanvasView *canvasview);
    private:
        CanvasView *m_canvasview;
        double m_zoom_level;
};


}
#endif // ZOOMLEVEL_H_
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

