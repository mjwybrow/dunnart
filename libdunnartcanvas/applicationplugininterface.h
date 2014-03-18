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
//! Contains the interface for Dunnart application plugins.


#ifndef APPLICATIONPLUGININTERFACE_H
#define APPLICATIONPLUGININTERFACE_H

namespace dunnart {

class Canvas;
class CanvasApplication;

//! @brief   Dunnart application plugin interface.
//!
//! Classes that implement this interface will be used by Dunnart as general
//! application plugins.  They will be initialised when Dunnart or the host
//! application is started and they will receive notification of important 
//! events.
//!
class ApplicationPluginInterface
{
    public:
        virtual ~ApplicationPluginInterface() {}

        virtual void applicationMainWindowInitialised(
                CanvasApplication *canvasApplication) = 0;
        virtual void applicationWillClose(
                CanvasApplication *canvasApplication) = 0;
};

}

Q_DECLARE_INTERFACE(dunnart::ApplicationPluginInterface,
        "org.dunnart.dunnart.ApplicationPluginInterface/1.0")


#endif // APPLICATIONPLUGININTERFACE_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
