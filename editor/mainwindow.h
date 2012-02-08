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

#ifndef MAINWINDOW_H_
#define MAINWINDOW_H_

#include <QMainWindow>
#include <QMenu>
#include <QToolBar>
#include <QAction>

class QApplication;
class QFileInfo;
class QDockWidget;

namespace dunnart {

class Application;
class CanvasView;
class Canvas;
class CanvasTabWidget;
class ZoomLevel;
class LayoutPropertiesDialog;
class ConnectorPropertiesDialog;
class CreateAlignmentDialog;
class CreateDistributionDialog;
class CreateSeparationDialog;
class CreateTemplateDialog;
class PropertiesEditorDialog;
class ShapePickerDialog;
class UndoHistoryDialog;

static const int MAX_RECENT_FILES = 10;

class MainWindow : public QMainWindow
{
    Q_OBJECT;

    public:
        MainWindow(Application *app);
        virtual ~MainWindow() { }

        Canvas *canvas(void);
        CanvasView *view(void);
        QList<CanvasView *> views(void);
        void newCanvasTab(void);
        bool loadDiagram(const QString& filename);
    private slots:
        void documentNew(void);
        void documentOpen(void);
        void documentOpenRecent();
        void documentExport(void);
        void documentPrint(void);
        void about(void);
        void openHomepage(void);
        void clearRecentFileMenu(void);
        void canvasChanged(Canvas *);
        void canvasFileInfoChanged(const QFileInfo& fileinfo);

    protected:
        virtual void closeEvent(QCloseEvent *event);

    private:
        void updateRecentFileActions(void);
        QString strippedName(const QString& fullFileName);

        CanvasTabWidget *m_tab_widget;

        QToolBar *m_edit_toolbar;

        QMenu *m_file_menu;
        QMenu *m_edit_menu;
        QMenu *m_view_menu;
        QMenu *m_layout_menu;
        QMenu *m_help_menu;

        QAction *m_new_action;
        QAction *m_open_action;
        QAction *m_close_action;
        QAction *m_save_action;
        QAction *m_print_action;
        QAction *m_save_as_action;
        QAction *m_export_action;
        QAction *m_quit_action;
        QAction *m_action_show_zoom_level_dialog;
        QAction *m_action_show_layout_properties_dialog;
        QAction *m_action_show_connector_properties_dialog;
        QAction *m_action_show_create_alignment_dialog;
        QAction *m_action_show_create_distribution_dialog;
        QAction *m_action_show_create_separation_dialog;
        QAction *m_action_show_create_template_dialog;
        QAction *m_action_show_properties_editor_dialog;
        QAction *m_action_show_shape_picker_dialog;
        QAction *m_action_show_undo_history_dialog;
        QAction *m_action_open_recent_file[MAX_RECENT_FILES];
        QAction *m_action_recent_file_separator;
        QAction *m_action_clear_recent_files;

        QAction *m_about_action;
        QAction *m_homepage_action;

        Application *m_application;

        ZoomLevel *m_dialog_zoomLevel;
        LayoutPropertiesDialog *m_dialog_layoutProps;
        ConnectorPropertiesDialog *m_dialog_connectorProps;
        CreateAlignmentDialog *m_dialog_alignment;
        CreateDistributionDialog *m_dialog_distribution;
        CreateSeparationDialog *m_dialog_separation;
        CreateTemplateDialog *m_dialog_template;
        PropertiesEditorDialog *m_dialog_properties_editor;
        ShapePickerDialog *m_dialog_shape_picker;
        UndoHistoryDialog *m_dialog_undo_history;
};


}
#endif // MAINWINDOW_H_
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

