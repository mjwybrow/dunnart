/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2011  Monash University
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

#include <QtDebug>
#include <QMenu>
#include <QToolBar>
#include <QUndoGroup>
#include <QMainWindow>
#include <QMessageBox>
#include <QFileDialog>
#include <QPushButton>
#include <QAbstractButton>

#include "libdunnartcanvas/canvastabwidget.h"

#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/pluginfileiofactory.h"

namespace dunnart {

CanvasTabWidget::CanvasTabWidget(QMainWindow *window) :
    QTabWidget(window),
    m_window(window),
    m_canvas(NULL)
{
    connect(this, SIGNAL(currentChanged(int)),
            this, SLOT(currentChanged(int)));
    connect(this, SIGNAL(tabCloseRequested(int)),
            this, SLOT(tabCloseRequested(int)));

    m_action_undo = new QAction(QIcon(":/resources/nuvola_icons/undo.png"),
            tr("Undo"), this);
    m_action_undo->setShortcut(QKeySequence::Undo);
    m_action_undo->setEnabled(false);

    m_action_redo = new QAction(QIcon(":/resources/nuvola_icons/redo.png"),
            tr("Redo"), this);
    m_action_redo->setShortcut(QKeySequence::Redo);
    m_action_redo->setEnabled(false);

    m_undo_group = new QUndoGroup(this);
    connect(m_undo_group, SIGNAL(canUndoChanged(bool)), m_action_undo,
            SLOT(setEnabled(bool)));
    connect(m_undo_group, SIGNAL(canRedoChanged(bool)), m_action_redo,
            SLOT(setEnabled(bool)));
    connect(m_action_undo, SIGNAL(triggered()), m_undo_group, SLOT(undo()));
    connect(m_action_redo, SIGNAL(triggered()), m_undo_group, SLOT(redo()));
    connect(m_undo_group, SIGNAL(cleanChanged(bool)), this,
            SLOT(documentCleanChanged(bool)));

    this->setDocumentMode(true);
    this->setMovable(true);

    // Set up actions.
    m_action_bring_to_front = new QAction(QIcon(":/resources/images/bringtop.png"),
            tr("Bring to Front"), this);
    m_action_bring_to_front->setIconText(tr("To Front"));

    m_action_send_to_back = new QAction(QIcon(":/resources/images/bringbot.png"),
            tr("Send to Back"), this);
    m_action_send_to_back->setIconText(tr("To Back"));

    m_action_cut = new QAction(QIcon(":/resources/nuvola_icons/editcut.png"),
            tr("Cut"), this);
    m_action_cut->setShortcut(QKeySequence::Cut);
    m_action_cut->setEnabled(false);

    m_action_copy = new QAction(QIcon(":/resources/nuvola_icons/editcopy.png"),
            tr("Copy"), this);
    m_action_copy->setShortcut(QKeySequence::Copy);
    m_action_copy->setEnabled(false);

    m_action_paste = new QAction(QIcon(":/resources/nuvola_icons/editpaste.png"),
            tr("Paste"), this);
    m_action_paste->setShortcut(QKeySequence::Paste);
    m_action_paste->setEnabled(false);

    m_action_selection_mode = new QAction(
            QIcon(":/resources/images/arrow.png"),
            tr("Selection Mode"), this);
    m_action_selection_mode->setIconText(tr("Select"));
    m_action_selection_mode->setCheckable(true);

    m_action_connection_mode = new QAction(
            QIcon(":/resources/images/connector.png"),
            tr("Connection Mode"), this);
    m_action_connection_mode->setIconText(tr("Connect"));
    m_action_connection_mode->setCheckable(true);

    m_action_mode_group = new QActionGroup(this);
    m_action_mode_group->addAction(m_action_selection_mode);
    m_action_mode_group->addAction(m_action_connection_mode);
    m_action_selection_mode->setChecked(true);
    connect(m_action_mode_group, SIGNAL(triggered(QAction*)),
            this, SLOT(setCanvasEditModeFromAction(QAction*)));

    m_action_delete = new QAction(tr("Delete"), this);
#ifdef Q_WS_MAC
    m_action_delete->setShortcut(tr("Backspace"));
#else
    m_action_delete->setShortcuts(QKeySequence::Delete);
#endif
    m_action_delete->setEnabled(false);

    m_action_edit_separator = new QAction(this);
    m_action_edit_separator->setSeparator(true);

    m_action_edit_separator2 = new QAction(this);
    m_action_edit_separator2->setSeparator(true);

    m_action_select_all = new QAction(tr("Select All"), this);
    m_action_select_all->setShortcut(QKeySequence::SelectAll);

    m_action_automatic_layout = new QAction(QIcon(":/resources/images/layout.png"),
            tr("Automatic Graph Layout"), this);
    m_action_automatic_layout->setCheckable(true);
    m_action_automatic_layout->setIconText(tr("Graph Layout"));

    m_action_cola_debug_output = new QAction(tr("Output COLA debug files"), this);
    m_action_cola_debug_output->setCheckable(true);

    m_action_lock = new QAction(QIcon(":/resources/images/locklayout.png"),
            tr("Pin/Unpin"), this);

    m_action_overlay_router_obstacles = new QAction(
            tr("Routing - Obstacles"), this);
    m_action_overlay_router_obstacles->setCheckable(true);

    m_action_overlay_router_visgraph = new QAction(
            tr("Routing - Polyline visibility graph"), this);
    m_action_overlay_router_visgraph->setCheckable(true);

    m_action_overlay_router_orthogonal_visgraph = new QAction(
            tr("Routing - Orthogonal visibility graph"), this);
    m_action_overlay_router_orthogonal_visgraph->setCheckable(true);
}

CanvasView *CanvasTabWidget::currentCanvasView(void) const
{
    return qobject_cast<CanvasView *>(currentWidget());
}

CanvasView *CanvasTabWidget::canvasViewAt(int index) const
{
    return qobject_cast<CanvasView *> (widget(index));
}

Canvas *CanvasTabWidget::currentCanvas(void) const
{
    return currentCanvasView()->canvas();
}

QUndoGroup *CanvasTabWidget::undoGroup(void) const
{
    return m_undo_group;
}

void CanvasTabWidget::currentCanvasEditModeChanged(const int mode)
{
    if (mode == ModeSelection)
    {
        m_action_selection_mode->setChecked(true);
    }
    else if (mode == ModeConnection)
    {
        m_action_connection_mode->setChecked(true);
    }
}

void CanvasTabWidget::setCanvasEditModeFromAction(QAction *action)
{
    if (action == m_action_selection_mode)
    {
        m_canvas->setEditMode(ModeSelection);
    }
    else if (action == m_action_connection_mode)
    {
        m_canvas->setEditMode(ModeConnection);
    }
}

void CanvasTabWidget::diagramFilenameChanged(const QFileInfo& fileinfo)
{
    Canvas *canvas = qobject_cast<Canvas *>(sender());
    QGraphicsView *view;
    foreach (view, canvas->views())
    {
        int index = indexOf(view);
        if (index != -1)
        {
            setTabText(index, fileinfo.fileName());
            setTabToolTip(index, fileinfo.absoluteFilePath());
        }
    }
    emit currentCanvasFileInfoChanged(fileinfo);
}

void CanvasTabWidget::currentChanged(int index)
{
    if (index == -1)
    {
        // No new tab.
        return;
    }

    CanvasView *canvasview = currentCanvasView();

    emit currentCanvasViewChanged(canvasview);
    emit currentCanvasChanged(canvasview->canvas());

    if (m_canvas)
    {
        disconnect(m_canvas, 0, this, 0);
        disconnect(this, 0, m_canvas, 0);
    }
    m_canvas = canvasview->canvas();

    m_undo_group->setActiveStack(m_canvas->undoStack());
    m_window->setWindowModified(!m_undo_group->isClean());
    currentCanvasEditModeChanged(m_canvas->editMode());

    connect(m_canvas, SIGNAL(clipboardContentsChanged()),
            this, SLOT(clipboardContentsChanged()));

    connect(m_canvas, SIGNAL(selectionChanged()),
            this, SLOT(selectionChanged()));
    this->selectionChanged();

    connect(m_canvas, SIGNAL(optChangedStructuralEditingDisabled(bool)),
            this, SLOT(hideEditingControls(bool)));

    connect(m_action_bring_to_front, SIGNAL(triggered()),
            m_canvas, SLOT(bringToFront()));

    connect(m_action_send_to_back,  SIGNAL(triggered()),
            m_canvas, SLOT(sendToBack()));

    connect(m_action_cut, SIGNAL(triggered()),
            m_canvas, SLOT(cutSelection()));

    connect(m_action_copy, SIGNAL(triggered()),
            m_canvas, SLOT(copySelection()));

    connect(m_action_paste, SIGNAL(triggered()),
            m_canvas, SLOT(pasteSelection()));

    connect(m_action_delete, SIGNAL(triggered()),
            m_canvas, SLOT(deleteSelection()));

    connect(m_action_select_all, SIGNAL(triggered()),
            m_canvas, SLOT(selectAll()));

    connect(m_action_automatic_layout, SIGNAL(triggered(bool)),
            m_canvas, SLOT(setOptAutomaticGraphLayout(bool)));
    connect(m_canvas, SIGNAL(optChangedAutomaticLayout(bool)),
            m_action_automatic_layout, SLOT(setChecked(bool)));
    m_action_automatic_layout->setChecked(
            m_canvas->optAutomaticGraphLayout());

    connect(m_action_cola_debug_output, SIGNAL(triggered(bool)),
            m_canvas, SLOT(setDebugCOLAOutput(bool)));

    connect(m_action_lock, SIGNAL(triggered()),
            m_canvas, SLOT(lockSelectedShapes()));

    connect(m_action_overlay_router_obstacles, SIGNAL(triggered(bool)),
            m_canvas, SLOT(setOverlayRouterObstacles(bool)));
    m_action_overlay_router_obstacles->setChecked(
            m_canvas->overlayRouterObstacles());

    connect(m_action_overlay_router_visgraph, SIGNAL(triggered(bool)),
            m_canvas, SLOT(setOverlayRouterVisGraph(bool)));
    m_action_overlay_router_visgraph->setChecked(
            m_canvas->overlayRouterVisGraph());

    connect(m_action_overlay_router_orthogonal_visgraph,
            SIGNAL(triggered(bool)),
            m_canvas, SLOT(setOverlayRouterOrthogonalVisGraph(bool)));
    m_action_overlay_router_orthogonal_visgraph->setChecked(
            m_canvas->overlayRouterOrthogonalVisGraph());

    connect(m_canvas, SIGNAL(editModeChanged(int)),
            this, SLOT(currentCanvasEditModeChanged(int)));

    bool editingDisabled = m_canvas->optStructuralEditingDisabled();
    hideEditingControls(editingDisabled);
}


void CanvasTabWidget::hideEditingControls(const bool hidden)
{
    bool showEditingControls = !hidden;
    m_action_cut->setVisible(showEditingControls);
    m_action_copy->setVisible(showEditingControls);
    m_action_paste->setVisible(showEditingControls);
    m_action_edit_separator->setVisible(showEditingControls);
    m_action_edit_separator2->setVisible(showEditingControls);
    m_action_automatic_layout->setVisible(showEditingControls);
    m_action_selection_mode->setVisible(showEditingControls);
    m_action_connection_mode->setVisible(showEditingControls);

    // If the canvas is in connector mode, put it back to selection mode.
    m_action_selection_mode->setChecked(true);
}

void CanvasTabWidget::newTab(void)
{
    Canvas *canvas = new Canvas();
    CanvasView *canvasview = new CanvasView(canvas);
    m_undo_group->addStack(canvas->undoStack());

    connect(canvasview->canvas(), SIGNAL(diagramFilenameChanged(QFileInfo)),
            this, SLOT(diagramFilenameChanged(QFileInfo)));

    insertTab(INT_MAX, canvasview, tr("untitled"));
    setCurrentWidget(canvasview);

    setTabsClosable(count() > 1);
}

bool CanvasTabWidget::closeAllRequest()
{
    QList<int> dirtyIndexes;
    for (int i = 0; i < count(); ++i)
    {
        Canvas *canvas = canvasViewAt(i)->canvas();
        if (canvas->undoStack()->isClean() == false)
        {
            dirtyIndexes.push_back(i);
        }
    }

    if (dirtyIndexes.empty())
    {
        return true;
    }
    else if (dirtyIndexes.count() == 1)
    {
        if (tabCloseRequested(dirtyIndexes.last()))
        {
            // The document is clean so the save succeeded
            return true;
        }
    }
    else
    {
        QMessageBox msgBox;
        QString text(tr("<b>You have %1 Dunnart documents with unsaved changes. Do you want to review these changes before quitting?</b>"));
        text = text.arg(dirtyIndexes.count());
        msgBox.setParent(window());
        msgBox.setText(text);
        msgBox.setInformativeText(tr("If you don't review your documents, all your changes will be lost."));
        QPushButton *reviewButton = msgBox.addButton(tr("Review Changes..."), QMessageBox::YesRole);
        QPushButton *cancelButton = msgBox.addButton(tr("Cancel"), QMessageBox::NoRole);
        QPushButton *discardButton = msgBox.addButton(tr("Discard Changes"), QMessageBox::RejectRole);
        msgBox.setDefaultButton(reviewButton);
        msgBox.setEscapeButton(cancelButton);
        msgBox.setIconPixmap(windowIcon().pixmap(MESSAGEBOX_PIXMAP_SIZE));
        msgBox.setWindowModality(Qt::WindowModal);
        msgBox.exec();

        if (msgBox.clickedButton() == discardButton)
        {
            // Happy to do nothing and close all.
            return true;
        }
        else if (msgBox.clickedButton() == reviewButton)
        {
            // Request closing each tab in turn, presenting dialog
            // that prompt the user to save.
            while (count() > 0)
            {
                if (tabCloseRequested(0) == false)
                {
                    // Stop if the user cancels any one.
                    return false;
                }
            }

            return true;
        }
    }

    // User has cancelled the close.
    return false;
}

void CanvasTabWidget::currentCanvasClose(void)
{
    bool tabWasClosed = tabCloseRequested(currentIndex());

    if (tabWasClosed && (count() == 0))
    {
        // Last tab closed, close window.
        m_window->close();
    }
}

bool CanvasTabWidget::tabCloseRequested(int index)
{
    CanvasView *view = static_cast<CanvasView *> (widget(index));
    Canvas *canvas  = view->canvas();
    bool isClean = canvas->undoStack()->isClean();

    if (!isClean)
    {
        setCurrentIndex(index);
        QMessageBox msgBox;
        QString text(tr("<b>Do you want to save changes you made in the document \"%1\"?</b>"));
        text = text.arg(tabText(index));
        msgBox.setParent(window());
        msgBox.setText(text);
        msgBox.setInformativeText(tr("Your changes will be lost if you don't save them."));
        msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        msgBox.setDefaultButton(QMessageBox::Save);
        msgBox.setIconPixmap(windowIcon().pixmap(MESSAGEBOX_PIXMAP_SIZE));
        msgBox.setWindowModality(Qt::WindowModal);
        int ret = msgBox.exec();

        if (ret == QMessageBox::Cancel)
        {
            return false;
        }
        else if (ret == QMessageBox::Save)
        {
            currentCanvasSave();
            if (m_window->isWindowModified())
            {
                // The user cancelled Save dialog, so return.
                return false;
            }
        }
    }

    removeTab(index);
    delete canvas;
    delete view;
    setTabsClosable(count() > 1);

    return true;
}

void CanvasTabWidget::addDebugOverlayMenuActions(QMenu *overlay_menu)
{
    overlay_menu->addAction(m_action_overlay_router_obstacles);
    overlay_menu->addAction(m_action_overlay_router_visgraph);
    overlay_menu->addAction(m_action_overlay_router_orthogonal_visgraph);
}

void CanvasTabWidget::addEditMenuActions(QMenu *edit_menu)
{
    edit_menu->addAction(m_action_undo);
    edit_menu->addAction(m_action_redo);
    edit_menu->addSeparator();
    edit_menu->addAction(m_action_cut);
    edit_menu->addAction(m_action_copy);
    edit_menu->addAction(m_action_paste);
    edit_menu->addAction(m_action_delete);
    edit_menu->addAction(m_action_edit_separator);
    edit_menu->addAction(m_action_select_all);
    edit_menu->addSeparator();
    edit_menu->addAction(m_action_bring_to_front);
    edit_menu->addAction(m_action_send_to_back);
}


void CanvasTabWidget::addLayoutMenuActions(QMenu *layout_menu)
{
    layout_menu->addAction(m_action_automatic_layout);
    layout_menu->addSeparator();
    layout_menu->addAction(m_action_cola_debug_output);
}


void CanvasTabWidget::addEditToolBarActions(QToolBar *edit_toolbar)
{
    edit_toolbar->addAction(m_action_undo);
    edit_toolbar->addAction(m_action_redo);
    edit_toolbar->addSeparator();
    edit_toolbar->addAction(m_action_cut);
    edit_toolbar->addAction(m_action_copy);
    edit_toolbar->addAction(m_action_paste);
    edit_toolbar->addAction(m_action_edit_separator);
    edit_toolbar->addAction(m_action_selection_mode);
    edit_toolbar->addAction(m_action_connection_mode);
    edit_toolbar->addAction(m_action_edit_separator2);

    edit_toolbar->addAction(m_action_bring_to_front);
    edit_toolbar->addAction(m_action_send_to_back);
    //edit_toolbar->addSeparator();
    //edit_toolbar->addAction(m_action_automatic_layout);
    edit_toolbar->addSeparator();
    edit_toolbar->addAction(m_action_lock);
}


void CanvasTabWidget::selectionChanged(void)
{
    int shapeCount = 0;
    int indicatorCount = 0;

    QList<CanvasItem *> selected_items = m_canvas->selectedItems();
    for (int i = 0; i < selected_items.size(); ++i)
    {
        if (dynamic_cast<ShapeObj *> (selected_items.at(i)))
        {
            shapeCount++;
        }
        else if (dynamic_cast<Indicator *> (selected_items.at(i)))
        {
            indicatorCount++;
        }
    }

#if 0
    if (shapeCount >= 1)
    {
        changeControlState(BUT_ALIGN, SDLGui::WIDGET_enable);
        changeControlState(BUT_AUTO_ALIGN, SDLGui::WIDGET_enable);
    }
    else
    {
        changeControlState(BUT_ALIGN, SDLGui::WIDGET_disable);
        changeControlState(BUT_AUTO_ALIGN, SDLGui::WIDGET_disable);
    }

    if (shapeCount >= 2)
    {
        changeControlState(BUT_SEPARATION, SDLGui::WIDGET_enable);
        changeControlState(BUT_DISTRO, SDLGui::WIDGET_enable);
    }
    else
    {
        changeControlState(BUT_SEPARATION, SDLGui::WIDGET_disable);
        changeControlState(BUT_DISTRO, SDLGui::WIDGET_disable);
    }
#endif

    if (shapeCount == 0)
    {
        m_action_lock->setEnabled(false);
        m_action_bring_to_front->setEnabled(false);
        m_action_send_to_back->setEnabled(false);
    }
    else
    {
        m_action_lock->setEnabled(true);
        m_action_bring_to_front->setEnabled(true);
        m_action_send_to_back->setEnabled(true);
    }

    if (m_canvas->optStructuralEditingDisabled() == false)
    {
        // Allow cut, copy and deletion of non-empty selections so long
        // as structural editing isn't disbabled.
        bool enabled = (selected_items.count() > 0);
        m_action_delete->setEnabled(enabled);
        m_action_cut->setEnabled(enabled);
        m_action_copy->setEnabled(enabled);
    }
    else
    {
        // Enable deletion of indicators if strucutral editing is diabled.
        bool enabled = (indicatorCount > 0);
        m_action_delete->setEnabled(enabled);
    }
}


void CanvasTabWidget::clipboardContentsChanged(void)
{
    m_action_paste->setEnabled(true);
}

void CanvasTabWidget::documentCleanChanged(bool clean)
{
    m_window->setWindowModified(!clean);
}


void CanvasTabWidget::currentCanvasSave(void)
{
    QString filename = currentCanvas()->filename();
    if (filename.isEmpty())
    {
        // Diagram is untitled, so "Save As..."
        currentCanvasSaveAs();
    }
    else
    {
        currentCanvas()->saveDiagram(filename);
    }
}


void CanvasTabWidget::currentCanvasSaveAs(void)
{
    QString currFilename = currentCanvas()->filename();
    if (currFilename.isEmpty())
    {
        currFilename = "untitled.svg";
    }

    QString defaultFilter = "Dunnart Annotated SVG (*.svg)";
    PluginFileIOFactory *fileIOFactory = sharedPluginFileIOFactory();
    QString filename = QFileDialog::getSaveFileName(m_window, tr("Save Diagram"),
            currFilename, fileIOFactory->saveableFileFiltersString(), &defaultFilter);
    if (!filename.isEmpty())
    {
        currentCanvas()->saveDiagram(filename);
        currentCanvas()->setFilename(filename);
    }
}

}
