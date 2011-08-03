// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
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
 * Author: Michael Wybrow <mjwybrow@users.sourceforge.net>
*/



#ifndef SHAPEPICKERDIALOG_H
#define SHAPEPICKERDIALOG_H

#include <QDockWidget>
#include <QObjectList>
#include "libdunnartcanvas/gen-ui/ui_shapepickerdialog.h"

class QGridLayout;
class QMouseEvent;

namespace dunnart {

class Canvas;

class ShapePickerDialog : public QDockWidget, private Ui::ShapePickerDialog
{
    Q_OBJECT

public:
    ShapePickerDialog(Canvas *canvas, QWidget *parent = NULL);
    ~ShapePickerDialog();
protected:
    void mousePressEvent(QMouseEvent *event);
private slots:
    void changeCanvas(Canvas *canvas);
    void changeIndex(const QString& text);
private:
    Canvas *m_canvas;
    QGridLayout *m_grid_layout;
    QObjectList m_shape_icons;
};

}

#endif // SHAPEPICKERDIALOG_H
