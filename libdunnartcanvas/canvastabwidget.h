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

#ifndef CANVASTABWIDGET_H
#define CANVASTABWIDGET_H

#include <QTabWidget>

class QMenu;
class QAction;
class QToolBar;
class QUndoGroup;
class QMainWindow;
class QFileInfo;
class QActionGroup;

namespace dunnart {

class Canvas;
class CanvasView;

class CanvasTabWidget : public QTabWidget
{
Q_OBJECT
public:
    explicit CanvasTabWidget(QMainWindow *window);
    void newTab(void);
    CanvasView *currentCanvasView(void) const;
    Canvas *currentCanvas(void) const;
    CanvasView *canvasViewAt(int index) const;
    QUndoGroup *undoGroup(void) const;

    void addDebugOverlayMenuActions(QMenu *overlay_menu);
    void addEditMenuActions(QMenu *edit_menu);
    void addLayoutMenuActions(QMenu *layout_menu);
    void addEditToolBarActions(QToolBar *edit_toolbar);

signals:
    void currentCanvasViewChanged(CanvasView *canvasview);
    void currentCanvasChanged(Canvas *canvas);
    void currentCanvasFileInfoChanged(const QFileInfo& fileinfo);

public slots:
    void diagramFilenameChanged(const QFileInfo& fileinfo);
    void currentChanged(int index);
    bool tabCloseRequested(int index);
    void currentCanvasClose(void);
    void currentCanvasSave(void);
    void currentCanvasSaveAs(void);
    bool closeAllRequest(void);
    void hideEditingControls(const bool hidden);
    void currentCanvasEditModeChanged(const int mode);

private slots:
    void setCanvasEditModeFromAction(QAction *action);
    void selectionChanged(void);
    void clipboardContentsChanged(void);
    void documentCleanChanged(bool);

private:
    QMainWindow *m_window;
    Canvas *m_canvas;
    QUndoGroup *m_undo_group;

    QAction *m_action_undo;
    QAction *m_action_redo;
    QAction *m_action_cut;
    QAction *m_action_copy;
    QAction *m_action_paste;
    QAction *m_action_delete;
    QAction *m_action_select_all;
    QAction *m_action_bring_to_front;
    QAction *m_action_send_to_back;
    QAction *m_action_selection_mode;
    QAction *m_action_connection_mode;
    QActionGroup *m_action_mode_group;
    QAction *m_action_lock;
    QAction *m_action_automatic_layout;
    QAction *m_action_cola_debug_output;
    QAction *m_action_overlay_router_obstacles;
    QAction *m_action_overlay_router_visgraph;
    QAction *m_action_overlay_router_orthogonal_visgraph;
    QAction *m_action_overlay_router_raw_routes;
    QAction *m_action_overlay_router_display_routes;
    QAction *m_action_edit_separator;
    QAction *m_action_edit_separator2;
};

}
#endif // CANVASTABWIDGET_H
