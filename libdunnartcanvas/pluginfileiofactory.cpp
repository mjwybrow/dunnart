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
 * Author(s): Michael Wybrow  <http://michael.wybrow.info/>
*/

#include <QFileInfo>

#include "libdunnartcanvas/pluginfileiofactory.h"
#include "libdunnartcanvas/fileioplugininterface.h"


namespace dunnart {

// Shared singleton:
static PluginFileIOFactory *shared_plugin_fileio_factory = NULL;

PluginFileIOFactory *sharedPluginFileIOFactory(void)
{
    if (shared_plugin_fileio_factory == NULL)
    {
        shared_plugin_fileio_factory = new PluginFileIOFactory();
    }
    return shared_plugin_fileio_factory;
}

PluginFileIOFactory::PluginFileIOFactory()
{
}

QString PluginFileIOFactory::openableFileFiltersString(void) const
{
    QString fileTypes;
    foreach (QString fileExtension, m_file_loading_handlers.keys())
    {
        // Separator.
        fileTypes += ";;";

        fileTypes += m_file_loading_handlers[fileExtension]->fileExtensionDescription(fileExtension) + " (*." + fileExtension + ")";
    }
    return fileTypes;
}

QString PluginFileIOFactory::saveableFileFiltersString(void) const
{
    QString fileTypes;
    foreach (QString fileExtension, m_file_saving_handlers.keys())
    {
        if (fileTypes.length() > 0)
        {
            // Separator.
            fileTypes += ";;";
        }
        fileTypes += m_file_saving_handlers[fileExtension]->fileExtensionDescription(fileExtension) + " (*." + fileExtension + ")";
    }
    return fileTypes;
}


QString PluginFileIOFactory::openableFileTypesString(void) const
{
    QString fileTypes;
    foreach (QString fileExtension, m_file_loading_handlers.keys())
    {
        if (fileTypes.length() > 0)
        {
            // Separator.
            fileTypes += " ";
        }
        fileTypes += "*." + fileExtension;
    }
    return fileTypes;
}

void PluginFileIOFactory::registerFileIOPlugin(FileIOPluginInterface *fileIOPlugin)
{
    foreach (QString fileType, fileIOPlugin->saveableFileExtensions())
    {
        m_file_saving_handlers[fileType] = fileIOPlugin;
    }

    foreach (QString fileType, fileIOPlugin->loadableFileExtensions())
    {
        m_file_loading_handlers[fileType] = fileIOPlugin;
    }
}

bool PluginFileIOFactory::saveDiagramToFile(Canvas *canvas,
        const QFileInfo& fileInfo, QString& errorMessage)
{
    QString fileExtension = fileInfo.suffix();
    if (m_file_saving_handlers.contains(fileExtension))
    {
        return m_file_saving_handlers[fileExtension]->
                saveDiagramToFile(canvas, fileInfo, errorMessage);
    }

    // This shouldn't really happen, but just in case.
    errorMessage = QObject::tr("File handler plugin was not available.");
    return false;
}

bool PluginFileIOFactory::loadDiagramFromFile(Canvas *canvas,
        const QFileInfo& fileInfo, QString& errorMessage)
{
    QString fileExtension = fileInfo.suffix();
    if (m_file_loading_handlers.contains(fileExtension))
    {
        return m_file_loading_handlers[fileExtension]->
                loadDiagramFromFile(canvas, fileInfo, errorMessage);
    }

    // This shouldn't really happen, but just in case.
    errorMessage = QObject::tr("File handler plugin was not available.");
    return false;
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
