/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2012  Monash University
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

//! @file
//! CanvasApplication class.  You should use or extend this class rather
//! than QApplication for canvas-based applications.

#ifndef CANVASAPPLICATION_H
#define CANVASAPPLICATION_H

#include <QApplication>
#include <QFileInfo>

class QEvent;
class QMainWindow;

namespace dunnart {

class CanvasTabWidget;
class Canvas;
class CanvasView;

//! @brief   CanvasApplication interface.
//!
//! Canvas-based applications should subclass this instead of QApplication
//! so that DunnartCanvas will load its plugins and have them be able to
//! track and operate on the main window, and see when the active canvas
//! changes.  Subclasseses will need to implement all the pure virtual
//! methods.
//!
//! Dunnart's own subclass of this is dunnart::Application.
//!
class CanvasApplication : public QApplication
{
    Q_OBJECT

    public:
        CanvasApplication(int & argc, char ** argv);
        
        //! @brief  Open a new diagram of the given file.
        //!
        //! @param  file  The file to open.
        virtual bool openDiagram(const QFileInfo& file) = 0;

        QMainWindow *mainWindow(void) const;
        void setMainWindow(QMainWindow *window);

        //! @brief  Can be used to let CanvasApplication know if the
        //!         application uses a CanvasTabWidget.
        //!
        //! This just sets up CanvasApplication to repeat various signals
        //! from the tab widget.
        //!
        //! dunnart::Application calls this automatically.  If you subclass
        //! CanvasApplication you should call this if your application uses
        //! a CanvasTabWidget.  If not, then you should emit the various
        //! currentCanvas* signals yourself.
        void setCanvasTabWidget(CanvasTabWidget *tabWidget);

    signals:

        //! @brief  Signal emitted when the active CanvasView changes.
        void currentCanvasViewChanged(CanvasView *canvasview);

        //! @brief  Signal emitted when the active Canvas changes.
        void currentCanvasChanged(Canvas *canvas);

        //! @brief  Signal emitted when the file associated with the active
        //!         canvas changes.
        void currentCanvasFileInfoChanged(const QFileInfo& file);

    private:
        QMainWindow *m_main_window;
        CanvasTabWidget *m_canvas_tab_widget;
};

}
#endif // CANVASAPPLICATION_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
