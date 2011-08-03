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
#include "libdunnartcanvas/instrument.h"
#include "libdunnartcanvas/visibility.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/utility.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/canvas.h"

#include "libavoid/router.h"
#include "libavoid/debug.h"

#include "libdunnartcanvas/pluginshapefactory.h"

using namespace dunnart;

bool use_recogniser = false;

#include "mainwindow.h"

// We supply our own since the MinGW people don't have non-GPLed headers
// in their compiler, which we need on windows.
#include "getopt.h"

FILE *ins_fp = NULL;



static void usage(char *title, char *editor);


static void cleanup(void)
{

    ins_end();
}


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

    PluginShapeFactory *factory = sharedPluginShapeFactory();
    foreach (QString fileName, pluginsDir.entryList(QDir::Files))
    {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        ShapePluginInterface *shapePlugin = qobject_cast<ShapePluginInterface *> (plugin);
        if (shapePlugin)
        {
            factory->registerShapePlugin(shapePlugin);
        }
    }

    namespaces.setPrefix(x_dunnartNs, x_dunnartURI);
    namespaces.setPrefix("xmlns", "http://www.w3.org/2000/svg");
    namespaces.setPrefix("sodipodi", 
            "http://sodipodi.sourceforge.net/DTD/sodipodi-0.dtd");
    namespaces.setPrefix("xlink", "http://www.w3.org/1999/xlink");

    // the filename of a gesture recogniser, if one is to be loaded
    std::string recogniser_filename;

    bool save_svg_and_exit = false;

    MainWindow window(&app);

    QIcon appIcon(":/resources/nuvola_icons/kfig.png");
    app.setWindowIcon(appIcon);
    window.setWindowIcon(appIcon);

    int c = -1;
    char args[] = "jbd:fhinqrst:vw:xyz:";
    while ((c = mj_getopt(argc, argv, args)) != -1)
    {
        switch (c)
        {
            case 'd':
                recogniser_filename = mj_optarg;
                use_recogniser = true;
                break;
            case 'b':
                //QT window.canvas()->m_batch_diagram_layout = true;
                break;
            case 'i':
            case 'f':
            case 'n':
            case 'j':
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
                fprintf(stderr, "Please run `%s -h' to see valid options.\n",
                        argv[0]);
                exit(EXIT_FAILURE);
                break;

            default:
                qFatal("?? getopt returned character code 0%o ??\n", c);
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
    ShapeObj *shape1 = new Rect(0, 0);
    window.canvas()->addItem(shape1);
    ShapeObj *shape2 = new Rect(0, 0);
    window.canvas()->addItem(shape2);
    ShapeObj *shape3 = new Rect(0, 0);
    window.canvas()->addItem(shape3);
    ShapeObj *shape4 = new Rect(0, 0);
    window.canvas()->addItem(shape4);
    ShapeObj *shape5 = new Rect(0, 0);
    window.canvas()->addItem(shape5);

    AvoidingConn *conn = new AvoidingConn(shape1, shape2);
    window.canvas()->addItem(conn);
    conn = new AvoidingConn(shape3, shape2);
    window.canvas()->addItem(conn);
    conn = new AvoidingConn(shape3, shape1);
    window.canvas()->addItem(conn);
    conn = new AvoidingConn(shape4, shape1);
    window.canvas()->addItem(conn);
    conn = new AvoidingConn(shape5, shape4);
    window.canvas()->addItem(conn);
    conn = new AvoidingConn(shape1, shape5);
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
        ins_end();
        exit(EXIT_SUCCESS);
    }
#endif
    
    atexit(cleanup);

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
"Visibility Options:\n"
"   -t ptvisalgo      Set the algorithm used for point visibility.\n"
"                     ptvisalgo must be one of the following:\n"
"                        n  naive\n"
"                        s  sweep\n"
"   -i                If this option is given, don't ignore regions that \n"
"                     can't contain the shortest path.  Keep all valid edges\n"
"                     in the visibility graph.\n"
"   -e                If this option is given, don't include endpoints in\n"
"                     the visibility graph.  Instead generate pt visibility\n"
"                     for endpoints prior to planning paths for a connector.\n"
"   -f                Use Partial Feedback, rather than Immediate Feedback\n"
"                     for visibility graph actions.\n"
"   -n                If this option is given, the *invisibility* graph won't\n"
"                     be used.  This generally results in a slowdown.\n"
"   -s                Don't selectively calculate and reroute connectors\n"
"                     after changes to the visibility graph.  Instead,\n"
"                     recalculate the paths for all connectors.\n"
"\n"
"General Options:\n"
"   -q                Record the session into the file.svg.bnlg binary log.\n"
"   -r                Replay the session saved in the file.svg.bnlg binary log.\n"
"   -h                Show usage information.\n"
"   -v                Show version information.\n"
"   -x                Load the diagram, then immediately write out SVG and quit.\n"
"   -b                Batch processing: Run graphlayout, reroute connectors,\n"
"                     Fit page to diagram, write out SVG and then exit.\n"
"   -y                Enable rounded poly-line segment corners on connectors.\n"
"   -z xing_penalty   Set the connector crossing penalty (0 to 500).\n"
"   -w nudge_distance 'Nudge' connectors by this amount to separate then.\n"
"                     Only valid in batch mode (-b), otherwise use CTRL-N.\n"
"\n"
"General Information:\n"
"\n"
"   A diagram can be opened by giving the filename as an argument when\n"
"   starting Dunnart.  New diagrams can be created by specifying a\n"
"   non-existent diagram on the command line, modifying the diagram and\n"
"   then saving it.  (A modification of a diagram can be created by\n"
"   making a copy of a diagram through the shell, and then opening and\n"
"   modifying the copy.  Drawing should have a `svg' extension.  They are\n"
"   stored as Scalable Vector Graphics (SVG) with added markup.\n"
"\n"
"   When using the record (-q) feature or opening an `exd' (experiment\n"
"   diagram) file all interactions with the diagram are written out to\n"
"   a binary log file (.bnlg).  This can then be used by Dunnart to play\n"
"   back the entire session using the `-r' option.\n"
"\n"
"   Saving diagrams with an exd extension creates a new revision of the\n"
"   file each time as `origfile-XX.exd' where XX is the revision number.\n"
"   This is useful for saving a copy of the state of the diagram at\n"
"   particular points during experiments.\n\n", title, editor);
      
}


// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

