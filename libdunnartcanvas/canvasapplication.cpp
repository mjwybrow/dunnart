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

#include <QtCore>

#include "canvasapplication.h"

#include "libdunnartcanvas/pluginshapefactory.h"
#include "libdunnartcanvas/pluginfileiofactory.h"
#include "libdunnartcanvas/pluginapplicationmanager.h"
#include "libdunnartcanvas/shapeplugininterface.h"
#include "libdunnartcanvas/fileioplugininterface.h"
#include "libdunnartcanvas/applicationplugininterface.h"
#include "libdunnartcanvas/canvastabwidget.h"

namespace dunnart {

CanvasApplication::CanvasApplication(int & argc, char ** argv)
    : QApplication(argc, argv),
      m_main_window(NULL),
      m_canvas_tab_widget(NULL)
{
    QDir pluginsDir = QDir(qApp->applicationDirPath());
#if defined(Q_OS_MAC)
    if (pluginsDir.dirName() == "MacOS") {
        pluginsDir.cdUp();
    }
    pluginsDir.cd("Plugins");
#else
    pluginsDir.cd("plugins");
#endif

    // Dynamically load each library in the plugins directory, and then
    // register them if they implement a given Dunnart plugin interface.
    PluginShapeFactory *shapeFactory = sharedPluginShapeFactory();
    PluginFileIOFactory *fileIOFactory = sharedPluginFileIOFactory();
    PluginApplicationManager *appPluginManager = sharedPluginApplicationManager();
    foreach (QString fileName, pluginsDir.entryList(QDir::Files))
    {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin == NULL)
        {
            // The plugin either didn't load or couldn't be instantiated.
            qCritical("Plugin \"%s\" failed to load: %s", qPrintable(fileName),
                      qPrintable(loader.errorString()));
            continue;
        }

        ShapePluginInterface *shapePlugin =
                qobject_cast<ShapePluginInterface *> (plugin);
        if (shapePlugin)
        {
            shapeFactory->registerShapePlugin(shapePlugin);
        }

        FileIOPluginInterface *fileIOPlugin =
                qobject_cast<FileIOPluginInterface *> (plugin);
        if (fileIOPlugin)
        {
            fileIOFactory->registerFileIOPlugin(fileIOPlugin);
        }

        ApplicationPluginInterface *appPlugin =
                qobject_cast<ApplicationPluginInterface *> (plugin);
        if (appPlugin)
        {
            appPluginManager->registerApplicationPlugin(appPlugin);
        }
    }
}

QMainWindow *CanvasApplication::mainWindow(void) const
{
    return m_main_window;
}


void CanvasApplication::setMainWindow(QMainWindow *window)
{
    m_main_window = window;
}

void CanvasApplication::setCanvasTabWidget(CanvasTabWidget *tabWidget)
{
    if (m_canvas_tab_widget)
    {
        disconnect(m_canvas_tab_widget, 0, this, 0);
    }
    m_canvas_tab_widget = tabWidget;

    if (m_canvas_tab_widget)
    {
        // Relay a few signals.
        connect(m_canvas_tab_widget, SIGNAL(currentCanvasChanged(Canvas*)),
                this, SIGNAL(currentCanvasChanged(Canvas*)));
        connect(m_canvas_tab_widget, SIGNAL(currentCanvasViewChanged(CanvasView*)),
                this, SIGNAL(currentCanvasViewChanged(CanvasView*)));
        connect(m_canvas_tab_widget, SIGNAL(currentCanvasFileInfoChanged(QFileInfo)),
                this, SIGNAL(currentCanvasFileInfoChanged(QFileInfo)));
    }
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
