/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2011  Monash University
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
//! Contains the interface for Dunnart Shape plugins.


#ifndef SHAPEPLUGININTERFACE_H
#define SHAPEPLUGININTERFACE_H

#include <QString>
#include <QStringList>
#include <QtPlugin>


namespace dunnart {

class ShapeObj;

//! @brief   Dunnart Shape Plugin Interface.
//!
//! Classes that implement this interface will be used by Dunnart for
//! providing drawing and interaction functionality for custom shapes.
//!
class ShapePluginInterface
{
    public:

        virtual ~ShapePluginInterface() {}

        //! @brief   Returns a short label that describes the group of shapes
        //!          provided by this plugin.
        //!
        virtual QString shapesClassLabel(void) const = 0;

        //! @brief   Returns a list of shapes that this plugin produces.
        //!
        //! The shape identifier strings should be fully qualified, e.g.,
        //! "org.dunnart.shapes.diamond"
        //!
        virtual QStringList producableShapeTypes(void) const = 0;

        //! @brief   Given a shape identifier string, this method should
        //!          return a new object of that type.
        //!
        //! Dunnart will only call this method with shapes that the plugin
        //! has declared itself as producing.
        //!
        virtual ShapeObj *generateShapeOfType(QString shapeType) = 0;
};

}

#define ShapePluginInterface_iid "org.dunnart.dunnart.ShapePluginInterface/1.0"

Q_DECLARE_INTERFACE(dunnart::ShapePluginInterface, ShapePluginInterface_iid)

#endif // SHAPEPLUGININTERFACE_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
