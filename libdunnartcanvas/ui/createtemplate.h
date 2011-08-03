/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2008-2010  Monash University
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

#ifndef CREATETEMPLATEDIALOG_H
#define CREATETEMPLATEDIALOG_H

#include "libdunnartcanvas/gen-ui/ui_createtemplate.h"
#include <QDockWidget>

class QSignalMapper;

namespace dunnart {

class Canvas;

class CreateTemplateDialog : public QDockWidget,
    private Ui::CreateTemplateDialog
{
    Q_OBJECT

    public:
        CreateTemplateDialog(Canvas *canvas, QWidget *parent = NULL);
        ~CreateTemplateDialog();

    signals:
    private slots:
        void changeCanvas(Canvas *canvas);
    private:
        Canvas *m_canvas;
        QSignalMapper *m_signal_mapper;
};

}

#endif // CREATETEMPLATEDIALOG_H
