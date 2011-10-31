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
//! Contains the interface for Dunnart File IO plugins.


#ifndef FILEIOPLUGININTERFACE_H
#define FILEIOPLUGININTERFACE_H

#include <QString>
#include <QStringList>
#include <QFileInfo>

namespace dunnart {

class Canvas;

//! @brief   Dunnart File IO Plugin Interface.
//!
//! Classes that implement this interface will be used by Dunnart for loading
//! and saving diagrams.  They need to advertise the file extensions for which
//! they can load and save diagrams.  Then, Dunnart will call into the
//! plugin when it needs to load or save a file of that type.
//!
class FileIOPluginInterface
{
    public:
        virtual ~FileIOPluginInterface() {}

        //! @brief   Returns a list of file extensions this plugin can load.
        //!
        virtual QStringList saveableFileExtensions(void) const = 0;

        //! @brief   Returns a list of file extensions this plugin can save to.
        //!
        virtual QStringList loadableFileExtensions(void) const = 0;

        //! @brief   Returns a string describing the specified file extension.
        //!
        //! For example "dot" might return "GraphViz DOT Language".
        //!
        //! Dunnart will only call this with an extension that the plugin
        //! says it supports loading or saving of.
        //!
        virtual QString fileExtensionDescription(const QString& extension)
                const = 0;

        //! @brief   This method is called when the plugin is asked to save
        //!          the diagram on a specified canvas to the given file.
        //!
        //! The file specified will be of a type supported by this plugin.
        //!
        //! @returns Whether this action diagram was saved successfully.
        //!
        virtual bool saveDiagramToFile(Canvas *canvas,
                const QFileInfo& fileInfo, QString& errorMessage) = 0;

        //! @brief   This method is called when the plugin is asked to load
        //!          the diagram in the specified file onto the given canvas.
        //!
        //! The file specified will be of a type supported by this plugin.
        //!
        //! @returns Whether this action diagram was loaded successfully.
        //!
        virtual bool loadDiagramFromFile(Canvas *canvas,
                const QFileInfo& fileInfo, QString& errorMessage) = 0;

};

}

Q_DECLARE_INTERFACE(dunnart::FileIOPluginInterface,
        "org.dunnart.dunnart.FileIOPluginInterface/1.0")


#endif // FILEIOPLUGININTERFACE_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
