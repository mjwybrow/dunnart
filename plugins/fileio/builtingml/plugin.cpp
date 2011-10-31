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
 * Author(s): Michael Wybrow  <http://michael.wybrow.info/>
*/

//! @file
//! Plugin that adds support for reading GML documents.

#include <QtGui>
#include <QObject>
#include <QFileInfo>

#include "libdunnartcanvas/fileioplugininterface.h"
#include "libdunnartcanvas/canvas.h"

using namespace dunnart;


//! @brief  Plugin class that adds support for reading GML documents.
//!
//! This plugin implements the builtin GML reading functionality.
//! All the actual work of loading the network is implemented in
//! libdunnartcanvas.  This is still implemented as a plugin so
//! that software built on top of Dunnart can easily exclude this
//! functionality if desired.
//!
class BuiltinGMLFileIOPlugin : public QObject, public FileIOPluginInterface
{
    Q_OBJECT
        Q_INTERFACES (dunnart::FileIOPluginInterface)

    public:
        BuiltinGMLFileIOPlugin()
        {
        }
        QStringList saveableFileExtensions(void) const
        {
            // No save functionality in this plugin.
            return QStringList();
        }
        QString fileExtensionDescription(const QString& extension) const
        {
            if (extension == "gml")
            {
                return "GML graph";
            }
            return QString();
        }
        QStringList loadableFileExtensions(void) const
        {
            QStringList fileTypes;
            fileTypes << "gml";
            return fileTypes;
        }
        bool saveDiagramToFile(Canvas *canvas, const QFileInfo& fileInfo,
                QString& errorMessage)
        {
            Q_UNUSED(canvas);
            Q_UNUSED(fileInfo);
            Q_UNUSED(errorMessage);

            // No save functionality in this plugin.
            return false;
        }
        bool loadDiagramFromFile(Canvas *canvas, const QFileInfo& fileInfo,
                QString& errorMessage)
        {
            Q_UNUSED(errorMessage);

            return canvas->loadGmlDiagram(fileInfo);
        }
};

Q_EXPORT_PLUGIN2(fileio_builtingml, BuiltinGMLFileIOPlugin)

// Because there is no header file, we need to load the MOC file here to 
// cause Qt to generate it for us.
#include "plugin.moc"

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
