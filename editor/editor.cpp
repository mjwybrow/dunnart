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

#include <QApplication>
#include <QIcon>
#include <QThread>
#include <QByteArray>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cstring>

#include <unistd.h>

#include "application.h"

#include "libdunnartcanvas/shared.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/placement.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/visibility.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/utility.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/canvas.h"

#include "libavoid/router.h"
#include "libavoid/debug.h"

#include "libdunnartcanvas/pluginshapefactory.h"
#include "libdunnartcanvas/pluginfileiofactory.h"
#include "libdunnartcanvas/shapeplugininterface.h"
#include "libdunnartcanvas/fileioplugininterface.h"

#include "mainwindow.h"


using namespace dunnart;

// We supply our own since the MinGW people don't have non-GPLed headers
// in their compiler, which we need on windows.
#include "getopt.h"


static void usage(char *title, char *editor);



int main(int argc, char *argv[])
{
    Application app(argc, argv);

    //QCoreApplication::addLibraryPath("libs/plugins");

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
    // register them if they implement a given DUnnart plugin interface.
    PluginShapeFactory *shapeFactory = sharedPluginShapeFactory();
    PluginFileIOFactory *fileIOFactory = sharedPluginFileIOFactory();
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
    }

    namespaces.setPrefix(x_dunnartNs, x_dunnartURI);
    namespaces.setPrefix("xmlns", "http://www.w3.org/2000/svg");
    namespaces.setPrefix("sodipodi",
            "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd");
    namespaces.setPrefix("xlink", "http://www.w3.org/1999/xlink");

    bool save_svg_and_exit = false;

    MainWindow window(&app);

    QIcon appIcon(":/resources/nuvola_icons/kfig.png");
    app.setWindowIcon(appIcon);
    window.setWindowIcon(appIcon);

    int c = -1;
    char args[] = "bhvw:xyz:";
    while ((c = mj_getopt(argc, argv, args)) != -1)
    {
        switch (c)
        {
            case 'b':
                //QT window.canvas()->m_batch_diagram_layout = true;
                break;
            case 'x':
                save_svg_and_exit = true;
                break;
            case 'h':
                usage("Dunnart 2", argv[0]);
                exit(EXIT_SUCCESS);
            case 'v':
                printf("%s\n\n", "Dunnart 2");
                exit(EXIT_SUCCESS);
                break;
            case 'y':
                window.canvas()->setOptConnRoundingDist(7);
                break;
            case 'z':
                window.canvas()->router()->setRoutingPenalty(
                        Avoid::crossingPenalty, atof(mj_optarg));
                break;
            case'w':
                window.canvas()->setNudgeDistance(atof(mj_optarg));
                break;
            case '?':
                qFatal("Please run `%s -h' to see valid options.", argv[0]);
                break;

            default:
                qFatal("?? getopt returned character code 0%o ??", c);
        }
    }


#if 1
    int diagrams = 1;
    while (mj_optind < argc)
    {
        if (diagrams > 1)
        {
            window.newCanvasTab();
        }
        int o = mj_optind;
        
        window.loadDiagram(QString(argv[o]));
        ++mj_optind;
        ++diagrams;
    }
#else
    ShapeObj *shape1 = new RectangleShape();
    shape1->setPosAndSize(QPointF(0,0), QSizeF(20,20));
    window.canvas()->addItem(shape1);
    ShapeObj *shape2 = new RectangleShape();
    shape2->setPosAndSize(QPointF(0,0), QSizeF(20,20));
    window.canvas()->addItem(shape2);
    ShapeObj *shape3 = new RectangleShape();
    shape3->setPosAndSize(QPointF(0,0), QSizeF(20,20));
    window.canvas()->addItem(shape3);
    ShapeObj *shape4 = new RectangleShape();
    shape4->setPosAndSize(QPointF(0,0), QSizeF(20,20));
    window.canvas()->addItem(shape4);
    ShapeObj *shape5 = new RectangleShape();
    shape5->setPosAndSize(QPointF(0,0), QSizeF(20,20));
    window.canvas()->addItem(shape5);

    Connector *conn = new Connector();
    conn->initWithConnection(shape1, shape2);
    window.canvas()->addItem(conn);
    conn = new Connector();
    conn->initWithConnection(shape3, shape2);
    window.canvas()->addItem(conn);
    conn = new Connector();
    conn->initWithConnection(shape3, shape1);
    window.canvas()->addItem(conn);
    conn = new Connector();
    conn->initWithConnection(shape4, shape1);
    window.canvas()->addItem(conn);
    conn = new Connector();
    conn->initWithConnection(shape5, shape4);
    window.canvas()->addItem(conn);
    conn = new Connector();
    conn->initWithConnection(shape1, shape5);
    window.canvas()->addItem(conn);
#endif

    window.activateWindow();
    window.raise();
    window.show();
    app.processEvents();

    // Trigger initial layout
    QList<CanvasView *> views = window.views();
    CanvasView *view;
    foreach (view, views)
    {
        view->postDiagramLoad();
    }

#if 0
    if (save_svg_and_exit)
    {
        // Save the SVG and exit.
        save_svg_action(NULL);
        free_xml_savespace();
        freeInterferingConnectorColours();
        exit(EXIT_SUCCESS);
    }
#endif

    //QT iteration_handler = do_response_tasks;
    //QT canvas_repaint_handler = repaint_canvas_wrapper;
    // Don't count initial diagram loading time.
    //QT router->timers.Reset();

    // Main GUI event loop:
    return app.exec();
}


static void usage(char *title, char *editor)
{
    printf("%s\n\n"
"Usage: %s [options] file\n"
"\n"
"General Options:\n"
"   -h                Show usage information.\n"
"   -v                Show version information.\n"
"   -x                Load the diagram, then immediately write out SVG and quit.\n"
"   -b                Batch processing: Run graphlayout, reroute connectors,\n"
"                     Fit page to diagram, write out SVG and then exit.\n"
"   -y                Enable rounded poly-line segment corners on connectors.\n"
"   -z xing_penalty   Set the connector crossing penalty (0 to 500).\n"
"   -w nudge_distance 'Nudge' connectors by this amount to separate then.\n"
"                     Only valid in batch mode (-b), otherwise use CTRL-N.\n"
"\n", title, editor);
      
}


// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

