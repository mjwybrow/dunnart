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

#include <QMenuBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QUrl>
#include <QStatusBar>
#include <QPrinter>
#include <QPrintDialog>
#include <QDesktopServices>
#include <QSettings>
#include <QSvgGenerator>
#include <QDebug>
#include <QCloseEvent>
#include <QFileInfo>

#include "mainwindow.h"
#include "canvastabwidget.h"
#include "application.h"

#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/pluginfileiofactory.h"

#include "libdunnartcanvas/githash.h"

#include "libdunnartcanvas/ui/layoutproperties.h"
#include "libdunnartcanvas/ui/zoomlevel.h"
#include "libdunnartcanvas/ui/connectorproperties.h"
#include "libdunnartcanvas/ui/createalignment.h"
#include "libdunnartcanvas/ui/createdistribution.h"
#include "libdunnartcanvas/ui/createtemplate.h"
#include "libdunnartcanvas/ui/createseparation.h"
#include "libdunnartcanvas/ui/propertieseditor.h"
#include "libdunnartcanvas/ui/shapepickerdialog.h"
#include "libdunnartcanvas/ui/undohistorydialog.h"
#include "libdunnartcanvas/pluginapplicationmanager.h"


namespace dunnart {


MainWindow::MainWindow(Application *app)
    : QMainWindow(),
      m_application(app)
{
    app->setMainWindow(this);
    setUnifiedTitleAndToolBarOnMac(true);
    setDocumentMode(true);

    QCoreApplication::setOrganizationName("Dunnart");
    QCoreApplication::setOrganizationDomain("dunnart.org");
    QCoreApplication::setApplicationName("Dunnart");

    // Correct the look of the tab bar on OS X cocoa.
    app->setStyleSheet(
        "QGraphicsView {"
            "border: 0px;"
        "}"
#ifdef Q_WS_MAC
        "QTabBar::tab:top {"
            "font-family: \"Lucida Grande\";"
            "font-size: 11px;"
        "}"
#endif
    );

    // Set the window title.
    setWindowTitle("Dunnart");

    m_tab_widget = new CanvasTabWidget(this);    
    connect(m_tab_widget, SIGNAL(currentCanvasChanged(Canvas*)),
            this, SLOT(canvasChanged(Canvas*)));
    connect(m_tab_widget, SIGNAL(currentCanvasFileInfoChanged(QFileInfo)),
            this, SLOT(canvasFileInfoChanged(QFileInfo)));
    m_tab_widget->newTab();
    setCentralWidget(m_tab_widget);

    // Inital window size.
    resize(1020, 743);

    m_new_action = new QAction("New", this);
    m_new_action->setShortcut(QKeySequence::New);
    connect(m_new_action, SIGNAL(triggered()), this, SLOT(documentNew()));

    m_open_action = new QAction("Open...", this);
    m_open_action->setShortcut(QKeySequence::Open);
    connect(m_open_action, SIGNAL(triggered()), this, SLOT(documentOpen()));

    for (int i = 0; i < MAX_RECENT_FILES; ++i)
    {
        m_action_open_recent_file[i] = new QAction(this);
        m_action_open_recent_file[i]->setVisible(false);
        connect(m_action_open_recent_file[i], SIGNAL(triggered()),
                this, SLOT(documentOpenRecent()));
    }

    m_close_action = new QAction("Close", this);
    m_close_action->setShortcut(QKeySequence::Close);
    connect(m_close_action, SIGNAL(triggered()),
            m_tab_widget, SLOT(currentCanvasClose()));

    m_save_action = new QAction("Save", this);
    m_save_action->setShortcut(QKeySequence::Save);
    connect(m_save_action, SIGNAL(triggered()),
            m_tab_widget, SLOT(currentCanvasSave()));

    m_save_as_action = new QAction("Save As...", this);
    m_save_as_action->setShortcut(QKeySequence::SaveAs);
    connect(m_save_as_action, SIGNAL(triggered()),
            m_tab_widget, SLOT(currentCanvasSaveAs()));

    m_export_action = new QAction("Export...", this);
    connect(m_export_action, SIGNAL(triggered()), this, SLOT(documentExport()));

    m_print_action = new QAction("Print...", this);
    m_print_action->setShortcut(QKeySequence::Print);
    connect(m_print_action, SIGNAL(triggered()), this, SLOT(documentPrint()));

    m_quit_action = new QAction(tr("Quit"), this);
    m_quit_action->setShortcut(QKeySequence::Quit);
    connect(m_quit_action, SIGNAL(triggered()),
            this, SLOT(close()));

    m_about_action = new QAction(tr("About"), this);
    connect(m_about_action, SIGNAL(triggered()), this, SLOT(about()));

    m_homepage_action = new QAction(tr("Dunnart homepage"), this);
    connect(m_homepage_action, SIGNAL(triggered()), this, SLOT(openHomepage()));

    m_action_show_zoom_level_dialog = new QAction(
            tr("Zoom Level"), this);
    m_action_show_zoom_level_dialog->setCheckable(true);

    m_action_show_properties_editor_dialog = new QAction(
            tr("Properties Editor"), this);
    m_action_show_properties_editor_dialog->setCheckable(true);

    m_action_show_layout_properties_dialog = new QAction(
            tr("Layout Properties"), this);
    m_action_show_layout_properties_dialog->setCheckable(true);

    m_action_show_connector_properties_dialog = new QAction(
            tr("Connector Properties"), this);
    m_action_show_connector_properties_dialog->setCheckable(true);

    m_action_show_create_alignment_dialog = new QAction(
            tr("Create Alignments"), this);
    m_action_show_create_alignment_dialog->setCheckable(true);

    m_action_show_create_distribution_dialog = new QAction(
            tr("Create Distributions"), this);
    m_action_show_create_distribution_dialog->setCheckable(true);

    m_action_show_create_separation_dialog = new QAction(
            tr("Create Separations"), this);
    m_action_show_create_separation_dialog->setCheckable(true);

    m_action_show_create_template_dialog = new QAction(
            tr("Create Templates"), this);
    m_action_show_create_template_dialog->setShortcut(tr("Ctrl+T"));
    m_action_show_create_template_dialog->setCheckable(true);

    m_action_show_shape_picker_dialog = new QAction(
            tr("Shape Picker"), this);
    m_action_show_shape_picker_dialog->setCheckable(true);

    m_action_clear_recent_files = new QAction(tr("Clear Menu"), this);
    connect(m_action_clear_recent_files, SIGNAL(triggered()),
            this, SLOT(clearRecentFileMenu()));

    m_action_show_undo_history_dialog = new QAction(
            tr("Undo History"), this);
    m_action_show_undo_history_dialog->setCheckable(true);

    CanvasView *canvasview = m_tab_widget->currentCanvasView();
    Canvas *canvas = m_tab_widget->currentCanvas();

    // Create statusBar, and assign it to the canvas.
    canvas->setStatusBar(statusBar());

#ifdef Q_WS_MAC
    // Make the status bar font size slightly smaller.
    QFont statusBarFont = statusBar()->font();
    statusBarFont.setPointSize(statusBarFont.pointSize() - 2);
    statusBar()->setFont(statusBarFont);
#endif

    m_file_menu = menuBar()->addMenu("File");
    m_file_menu->addAction(m_new_action);
    m_file_menu->addAction(m_open_action);
    QMenu *recentsMenu = m_file_menu->addMenu(tr("Open Recent"));
    for (int i = 0; i < MAX_RECENT_FILES; ++i)
    {
        recentsMenu->addAction(m_action_open_recent_file[i]);
    }
    m_action_recent_file_separator = recentsMenu->addSeparator();
    recentsMenu->addAction(m_action_clear_recent_files);
    updateRecentFileActions();

    m_file_menu->addSeparator();
    m_file_menu->addAction(m_close_action);
    m_file_menu->addAction(m_save_action);
    m_file_menu->addAction(m_save_as_action);
    m_file_menu->addSeparator();
    m_file_menu->addAction(m_export_action);
    m_file_menu->addSeparator();
    m_file_menu->addAction(m_print_action);
    m_file_menu->addSeparator();
    m_file_menu->addAction(m_quit_action);

    m_edit_menu = menuBar()->addMenu(tr("Edit"));
    m_tab_widget->addEditMenuActions(m_edit_menu);

    m_view_menu = menuBar()->addMenu(tr("View"));
    QMenu *dialogs_menu = m_view_menu->addMenu(tr("Show Dialogs"));
    dialogs_menu->addAction(m_action_show_zoom_level_dialog);
    dialogs_menu->addSeparator();
    dialogs_menu->addAction(m_action_show_shape_picker_dialog);
    dialogs_menu->addAction(m_action_show_undo_history_dialog);
    dialogs_menu->addAction(m_action_show_properties_editor_dialog);
    dialogs_menu->addSeparator();
    dialogs_menu->addAction(m_action_show_create_alignment_dialog);
    dialogs_menu->addAction(m_action_show_create_distribution_dialog);
    dialogs_menu->addAction(m_action_show_create_separation_dialog);
    dialogs_menu->addAction(m_action_show_create_template_dialog);
    dialogs_menu->addSeparator();
    dialogs_menu->addAction(m_action_show_layout_properties_dialog);
    dialogs_menu->addAction(m_action_show_connector_properties_dialog);
    QMenu *overlays_menu = m_view_menu->addMenu(tr("Canvas Debug Overlays"));
    m_tab_widget->addDebugOverlayMenuActions(overlays_menu);

    m_layout_menu = menuBar()->addMenu("Layout");
    m_tab_widget->addLayoutMenuActions(m_layout_menu);
    
    m_edit_toolbar = addToolBar(tr("Edit toolbar"));
    m_edit_toolbar->setIconSize(QSize(24, 24));
    m_edit_toolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    m_tab_widget->addEditToolBarActions(m_edit_toolbar);

    m_dialog_zoomLevel = new ZoomLevel(canvasview);
    connect(m_tab_widget, SIGNAL(currentCanvasViewChanged(CanvasView*)),
            m_dialog_zoomLevel, SLOT(changeCanvasView(CanvasView*)));
    connect(m_action_show_zoom_level_dialog,  SIGNAL(triggered(bool)),
            m_dialog_zoomLevel, SLOT(setVisible(bool)));
    connect(m_dialog_zoomLevel, SIGNAL(visibilityChanged(bool)),
            m_action_show_zoom_level_dialog,  SLOT(setChecked(bool)));
    addDockWidget(Qt::RightDockWidgetArea, m_dialog_zoomLevel);
    m_dialog_zoomLevel->show();

    m_dialog_properties_editor = new PropertiesEditorDialog(canvas);
    connect(m_tab_widget, SIGNAL(currentCanvasChanged(Canvas*)),
            m_dialog_properties_editor, SLOT(changeCanvas(Canvas*)));
    connect(m_action_show_properties_editor_dialog, SIGNAL(triggered(bool)),
            m_dialog_properties_editor, SLOT(setVisible(bool)));
    connect(m_dialog_properties_editor, SIGNAL(visibilityChanged(bool)),
            m_action_show_properties_editor_dialog,  SLOT(setChecked(bool)));
    addDockWidget(Qt::BottomDockWidgetArea, m_dialog_properties_editor);
    m_dialog_properties_editor->hide();

    m_dialog_shape_picker = new ShapePickerDialog(canvas);
    connect(m_tab_widget, SIGNAL(currentCanvasChanged(Canvas*)),
            m_dialog_shape_picker, SLOT(changeCanvas(Canvas*)));
    connect(m_action_show_shape_picker_dialog,  SIGNAL(triggered(bool)),
            m_dialog_shape_picker, SLOT(setVisible(bool)));
    connect(m_dialog_shape_picker, SIGNAL(visibilityChanged(bool)),
            m_action_show_shape_picker_dialog,  SLOT(setChecked(bool)));
    addDockWidget(Qt::LeftDockWidgetArea, m_dialog_shape_picker);
    m_dialog_shape_picker->show();

    m_dialog_layoutProps = new LayoutPropertiesDialog(canvas);
    connect(m_tab_widget, SIGNAL(currentCanvasChanged(Canvas*)),
            m_dialog_layoutProps, SLOT(changeCanvas(Canvas*)));
    connect(m_action_show_layout_properties_dialog,  SIGNAL(triggered(bool)),
            m_dialog_layoutProps, SLOT(setVisible(bool)));
    connect(m_dialog_layoutProps, SIGNAL(visibilityChanged(bool)),
            m_action_show_layout_properties_dialog,  SLOT(setChecked(bool)));
    addDockWidget(Qt::LeftDockWidgetArea, m_dialog_layoutProps);
    m_dialog_layoutProps->show();

    m_dialog_connectorProps = new ConnectorPropertiesDialog(canvas);
    connect(m_tab_widget, SIGNAL(currentCanvasChanged(Canvas*)),
            m_dialog_connectorProps, SLOT(changeCanvas(Canvas*)));
    connect(m_action_show_connector_properties_dialog,  SIGNAL(triggered(bool)),
            m_dialog_connectorProps, SLOT(setVisible(bool)));
    connect(m_dialog_connectorProps, SIGNAL(visibilityChanged(bool)),
            m_action_show_connector_properties_dialog,  SLOT(setChecked(bool)));
    addDockWidget(Qt::LeftDockWidgetArea, m_dialog_connectorProps);
    m_dialog_connectorProps->hide();

    m_dialog_alignment = new CreateAlignmentDialog(canvas);
    connect(m_tab_widget, SIGNAL(currentCanvasChanged(Canvas*)),
            m_dialog_alignment, SLOT(changeCanvas(Canvas*)));
    connect(m_action_show_create_alignment_dialog,  SIGNAL(triggered(bool)),
            m_dialog_alignment, SLOT(setVisible(bool)));
    connect(m_dialog_alignment, SIGNAL(visibilityChanged(bool)),
            m_action_show_create_alignment_dialog,  SLOT(setChecked(bool)));
    addDockWidget(Qt::RightDockWidgetArea, m_dialog_alignment);
    m_dialog_alignment->show();

    m_dialog_distribution = new  CreateDistributionDialog(canvas, this);
    connect(m_tab_widget, SIGNAL(currentCanvasChanged(Canvas*)),
            m_dialog_distribution, SLOT(changeCanvas(Canvas*)));
    connect(m_action_show_create_distribution_dialog,  SIGNAL(triggered(bool)),
            m_dialog_distribution, SLOT(setVisible(bool)));
    connect(m_dialog_distribution, SIGNAL(visibilityChanged(bool)),
            m_action_show_create_distribution_dialog,  SLOT(setChecked(bool)));
    addDockWidget(Qt::RightDockWidgetArea, m_dialog_distribution);
    m_dialog_distribution->show();

    m_dialog_separation = new CreateSeparationDialog(canvas, this);
    connect(m_tab_widget, SIGNAL(currentCanvasChanged(Canvas*)),
            m_dialog_separation, SLOT(changeCanvas(Canvas*)));
    connect(m_action_show_create_separation_dialog,  SIGNAL(triggered(bool)),
            m_dialog_separation, SLOT(setVisible(bool)));
    connect(m_dialog_separation, SIGNAL(visibilityChanged(bool)),
            m_action_show_create_separation_dialog,  SLOT(setChecked(bool)));
    addDockWidget(Qt::RightDockWidgetArea, m_dialog_separation);
    m_dialog_separation->show();

    m_dialog_template = new CreateTemplateDialog(canvas, this);
    connect(m_tab_widget, SIGNAL(currentCanvasChanged(Canvas*)),
            m_dialog_template, SLOT(changeCanvas(Canvas*)));
    connect(m_action_show_create_template_dialog,  SIGNAL(triggered(bool)),
            m_dialog_template, SLOT(setVisible(bool)));
    connect(m_dialog_template, SIGNAL(visibilityChanged(bool)),
            m_action_show_create_template_dialog,  SLOT(setChecked(bool)));
    m_dialog_template->hide();

    m_dialog_undo_history = new UndoHistoryDialog(
            m_tab_widget->undoGroup(), this);
    connect(m_action_show_undo_history_dialog,  SIGNAL(triggered(bool)),
            m_dialog_undo_history, SLOT(setVisible(bool)));
    connect(m_dialog_undo_history, SIGNAL(visibilityChanged(bool)),
            m_action_show_undo_history_dialog,  SLOT(setChecked(bool)));
    addDockWidget(Qt::LeftDockWidgetArea, m_dialog_undo_history);
    m_dialog_undo_history->hide();

    // Allow plugins to initialise themselves and add things like
    // menu items and dock widgets to the main window.
    PluginApplicationManager *appPluginManager =
            sharedPluginApplicationManager();
    appPluginManager->applicationMainWindowInitialised(app);

    // Add help menu after everything else (if should be rightmost).
    m_help_menu = menuBar()->addMenu(tr("Help"));
    m_help_menu->addAction(m_homepage_action);
    m_help_menu->addSeparator();
    m_help_menu->addAction(m_about_action);

    // Restore window geometry and Dock Widget geometry.
    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->setAccepted(m_tab_widget->closeAllRequest());
    if (event->isAccepted())
    {
        PluginApplicationManager *appPluginManager =
                sharedPluginApplicationManager();
        appPluginManager->applicationWillClose();

        // Save window geometry and Dock Widget geometry.
        QSettings settings;
        settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
        QMainWindow::closeEvent(event);
    }
}

void MainWindow::openHomepage(void)
{
    QDesktopServices::openUrl(QUrl("http://www.dunnart.org/"));
}


void MainWindow::about(void)
{
    QMessageBox *box = new QMessageBox(QMessageBox::NoIcon, "About Dunnart",
            "Dunnart", QMessageBox::NoButton, this, 
            Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);
    box->setIconPixmap(windowIcon().pixmap(MESSAGEBOX_PIXMAP_SIZE));
    QString hash = GITHASH;
    hash = hash.toUpper();

    QString infoStr = 
            "Version 2.0  (" + hash + ")<br />"
            "Built on " __DATE__ "<br />"
            "Based on Qt " QT_VERSION_STR "<br /><br />"
            "Copyright &copy; 2003&ndash;2008 Michael Wybrow<br />"
            "Copyright&nbsp;&copy;&nbsp;2006-2012&nbsp;Monash&nbsp;University";
    box->setInformativeText(infoStr);
    box->show();
}


Canvas *MainWindow::canvas(void)
{
    return m_tab_widget->currentCanvas();
}


QList<CanvasView *> MainWindow::views(void)
{
    QList<CanvasView *> viewList;
    for (int i = 0; i < m_tab_widget->count(); ++i)
    {
        viewList.append(m_tab_widget->canvasViewAt(i));
    }
    return viewList;
}

CanvasView *MainWindow::view(void)
{
    return m_tab_widget->currentCanvasView();
}

bool MainWindow::loadDiagram(const QString& filename)
{
    newCanvasTab();

    bool successful = canvas()->loadDiagram(filename);

    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    files.removeAll(filename);
    files.prepend(filename);
    while (files.size() > MAX_RECENT_FILES)
    {
        files.removeLast();
    }
    settings.setValue("recentFileList", files);
    updateRecentFileActions();

    QFileInfo fileinfo(filename);
    canvasFileInfoChanged(fileinfo);

    view()->postDiagramLoad();

    return successful;
}

void MainWindow::documentOpen(void)
{
    PluginFileIOFactory *fileIOFactory = sharedPluginFileIOFactory();
    QString filter = tr("Dunnart diagrams (") +
            fileIOFactory->openableFileTypesString() + ")";

    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open Diagram"), QDir::homePath(), filter);
    if (!fileName.isEmpty())
    {
        loadDiagram(fileName);
    }
}


void MainWindow::documentOpenRecent()
{
     QAction *action = qobject_cast<QAction *>(sender());
     if (action)
     {
         loadDiagram(action->data().toString());
     }
}


void MainWindow::newCanvasTab(void)
{
    if ((m_tab_widget->tabText(m_tab_widget->currentIndex()) == "untitled") &&
            m_tab_widget->currentCanvas()->items().empty())
    {
        return;
    }
    m_tab_widget->newTab();
    canvas()->setStatusBar(statusBar());
}

void MainWindow::documentNew(void)
{
    newCanvasTab();
    view()->postDiagramLoad();
}


void MainWindow::documentExport(void)
{
    Canvas *currCanvas = canvas();
    QString currFilename = currCanvas->filename();
    currFilename.chop(4);
    if (currFilename.isEmpty())
    {
        currFilename = "untitled";
    }
    QString filename = QFileDialog::getSaveFileName(this, tr("Export Diagram"),
            currFilename, tr("SVG (*.svg);;PDF (*.pdf);;Postscript (*.ps)"));
    if (filename.isEmpty())
    {
        return;
    }

    QFileInfo file(filename);
    if (file.suffix() == "svg")
    {
        QSvgGenerator generator;
        generator.setFileName(filename);

        generator.setSize(currCanvas->pageRect().size().toSize());
        QRectF targetRect(QPointF(0, 0), QSizeF(currCanvas->sceneRect().size()));

        QRectF viewbox(currCanvas->pageRect().topLeft() -
                currCanvas->sceneRect().topLeft(),
                currCanvas->pageRect().size());
        generator.setViewBox(viewbox);

        generator.setTitle(QFileInfo(filename).fileName());
        generator.setDescription(tr("This file was exported from Dunnart.  "
                                    "http://www.dunnart.org/"));

        QPainter painter;
        if (painter.begin(&generator))
        {
            painter.setRenderHint(QPainter::Antialiasing);
            currCanvas->setRenderingForPrinting(true);
            currCanvas->render(&painter, targetRect,
                    currCanvas->sceneRect(),
                    Qt::IgnoreAspectRatio);
            currCanvas->setRenderingForPrinting(false);

            painter.end();
        }
        else
        {
            qDebug("Export SVG painter failed to begin.");
        }
    }
    else
    {
        // Use QPrinter for PDF and PS.
        QPrinter printer;
        printer.setOutputFileName(filename);
        printer.setPaperSize(currCanvas->pageRect().size(),
                QPrinter::Millimeter);
        QPainter painter;
        if (painter.begin(&printer))
        {
            painter.setRenderHint(QPainter::Antialiasing);
            currCanvas->setRenderingForPrinting(true);
            currCanvas->render(&painter, QRectF(),
                    currCanvas->pageRect().adjusted(+3, +3, -3, -3),
                    Qt::IgnoreAspectRatio);
            currCanvas->setRenderingForPrinting(false);
        }
        else
        {
            qDebug("Export PDF/PS painter failed to begin.");
        }
    }
}

void MainWindow::documentPrint(void)
{
    Canvas *currCanvas = canvas();
    QPrinter printer;

    QPrintDialog *dialog = new QPrintDialog(&printer, this);
    dialog->setWindowTitle(tr("Print Document"));
    if (dialog->exec() == QDialog::Accepted)
    {
        QPainter painter(&printer);
        painter.setRenderHint(QPainter::Antialiasing);
        currCanvas->setRenderingForPrinting(true);
        currCanvas->render(&painter, QRectF(),
                currCanvas->pageRect());
        currCanvas->setRenderingForPrinting(false);
    }
}


void MainWindow::clearRecentFileMenu(void)
{
    QSettings settings;
    settings.setValue("recentFileList", QStringList());
    updateRecentFileActions();
}

void MainWindow::updateRecentFileActions()
{
     QSettings settings;
     QStringList files = settings.value("recentFileList").toStringList();

     int numRecentFiles = qMin(files.size(), MAX_RECENT_FILES);

     for (int i = 0; i < numRecentFiles; ++i)
     {
         QString text = strippedName(files[i]);
         m_action_open_recent_file[i]->setText(text);
         m_action_open_recent_file[i]->setData(files[i]);
         m_action_open_recent_file[i]->setVisible(true);
     }
     for (int j = numRecentFiles; j < MAX_RECENT_FILES; ++j)
     {
         m_action_open_recent_file[j]->setVisible(false);
     }

     m_action_recent_file_separator->setVisible(numRecentFiles > 0);
     m_action_clear_recent_files->setEnabled(numRecentFiles > 0);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
     return QFileInfo(fullFileName).fileName();
}

void MainWindow::canvasFileInfoChanged(const QFileInfo& fileinfo)
{
    if (fileinfo.fileName().isEmpty())
    {
        setWindowFilePath(QString());
        setWindowTitle("untitled[*] - Dunnart");
    }
    else
    {
        setWindowFilePath(fileinfo.absoluteFilePath());
        setWindowTitle(fileinfo.fileName() + "[*] - Dunnart");
    }
}

void MainWindow::canvasChanged(Canvas *canvas)
{
    QFileInfo fileinfo(canvas->filename());
    canvasFileInfoChanged(fileinfo);
}

}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

