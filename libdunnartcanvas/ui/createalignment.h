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

#ifndef CREATEALIGNMENTDIALOG_H
#define CREATEALIGNMENTDIALOG_H

#include "ui_createalignment.h"
#include <QDockWidget>

class QSignalMapper;

namespace dunnart {

class Canvas;

class CreateAlignmentDialog : public QDockWidget, 
	private Ui::CreateAlignmentDialog
{
    Q_OBJECT

    public:
        CreateAlignmentDialog(Canvas *canvas, QWidget *parent = NULL);
        ~CreateAlignmentDialog();

    signals:
    private slots:
        void changeCanvas(Canvas *canvas);
    private:
        Canvas *m_canvas;
        QSignalMapper *m_signal_mapper;
};

}

#endif // CREATEALIGNMENTDIALOG_H
