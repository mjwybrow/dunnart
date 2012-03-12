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

//! @brief   CanvasApplication interface.
//!
//! Canvas-based applications should subclass this inseatd of QApplication
//! so that DunnartCanvas will load its plugins and have them be able to
//! track and operate on the main window.  Subclasseses will need to
//! implement all the pure virtual methods.
//!
//! Dunnart's own subclass of this is dunnart::Application.
//!
class CanvasApplication : public QApplication
{
    public:
        CanvasApplication(int & argc, char ** argv);
        
        //! @brief   Open a new diagram of the given file.
        //!
        //! @param  file  The file to open.
        virtual bool openDiagram(const QFileInfo& file) = 0;

        QMainWindow *mainWindow(void) const;
        void setMainWindow(QMainWindow *window);
    private:
        QMainWindow *m_main_window;
};

}
#endif // CANVASAPPLICATION_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
