/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
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
 *
 * Author(s): Michael Wybrow  <http://michael.wybrow.info/>
*/

#ifndef CANVASVIEW_H_
#define CANVASVIEW_H_

#include <QGraphicsView>
#include <QList>

class QMenu;

namespace dunnart {


class CanvasItem;
class Canvas;

class CanvasView : public QGraphicsView
{
    Q_OBJECT;

    public:
        CanvasView(Canvas *c);
        virtual ~CanvasView() { }
        void postDiagramLoad(void);
        Canvas *canvas(void);
        void setScene(QGraphicsScene *scene);
    signals:
        void canvasTransformChanged(const QTransform& transform);
    protected:
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);
        virtual void keyPressEvent(QKeyEvent *keyEvent);
        virtual void keyReleaseEvent(QKeyEvent *keyEvent);
        virtual void dropEvent(QDropEvent *event);
        virtual void dragEnterEvent(QDragEnterEvent *event);
        virtual void dragMoveEvent(QDragMoveEvent *event);
        virtual QAction *buildAndExecContextMenu(QMouseEvent *event,
                QMenu& menu);
        bool handleContextMenuEvent(QMouseEvent * event);
    private slots:
        void adjustSceneRect(QRectF rect);
        void debugOverlayEnabled(bool enabled);
        void editModeChanged(int mode);
    private:
        void zoomToShowRect(const QRectF& rect);

        QPoint m_last_mouse_pos;
        bool m_hand_scrolling;
        QTransform m_last_transform;
};


}
#endif // CANVASVIEW_H_

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

