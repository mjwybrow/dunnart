/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2010  Monash University
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

#ifndef SHAPE_H
#define SHAPE_H

#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/guideline.h"

namespace Avoid {

class Polygon;
class ShapeRef;
class Point;
class ShapeConnectionPin;

}

namespace dunnart {


extern double avoidBuffer;
extern double routingBuffer;

static const double AVOID_BUFFER_SMALL  = 2.0;
static const double AVOID_BUFFER_DIFFERENCE  = 3.5;

static const double MIN_SHAPE_SIZE = 5;

static const uint CENTRE_CONNECTION_PIN = 100;
static const uint DEPRECATED_CONNECTION_PIN_TOP = 101;
static const uint DEPRECATED_CONNECTION_PIN_BOTTOM = 102;
static const uint DEPRECATED_CONNECTION_PIN_LEFT = 103;
static const uint DEPRECATED_CONNECTION_PIN_RIGHT = 104;


class Relationship;
class Handle;
class Cluster;
class ShapeObj;
class Connector;
class Canvas;
class ConnectionPinInfo;


typedef std::multiset<Connector *> ConnMultiset;
  
class ShapeObj: public CanvasItem
{
    Q_OBJECT
    Q_PROPERTY (QPointF position READ centrePos WRITE cmd_setCentrePos)
    Q_PROPERTY (QString label READ getLabel WRITE setLabel)
    Q_PROPERTY (QSizeF size READ size WRITE cmd_setSize)
    Q_PROPERTY (QColor fillColour READ fillColour WRITE setFillColour)
    Q_PROPERTY (QColor strokeColour READ strokeColour WRITE setStrokeColour)
    Q_PROPERTY (bool sizeLocked READ sizeLocked WRITE setSizeLocked)

    UNDO_ACTION (ShapeObj, QPointF, centrePos, setCentrePos, UNDO_SHAPE_POS, "move shape")
    UNDO_ACTION (ShapeObj, QSizeF, size, setSize, UNDO_SHAPE_SIZE, "resize shape")

    public:
        ShapeObj();
        virtual ~ShapeObj();

        virtual void initWithXMLProperties(Canvas *canvas,
                const QDomElement& node, const QString& ns);
        virtual void initWithDimensions(QString id, const double x,
                const double y, const double w, const double h);

        QString getLabel(void) const;
        void setLabel(const QString label);

        QColor strokeColour(void) const;
        void setStrokeColour(const QColor& colour);
        QColor fillColour(void) const;
        void setFillColour(const QColor& colour);

        bool sizeLocked(void) const;
        void setSizeLocked(const bool locked);

        virtual void cascade_distance(int dist, unsigned int dir,
                CanvasItem **path);
        void addXmlProps(const unsigned int subset, QDomElement& node,
                QDomDocument& doc);
        void addContainedShape(ShapeObj *shape);
        void addContainedShapes(QList<ShapeObj *>& shapes);
        void removeContainedShape(ShapeObj *shape);
        void removeContainedShapes(QList<ShapeObj *>& shapes);
        QList<ShapeObj *> containedShapes(void) const;
        void showConnectionPoints(void);
        void hideConnectionPoints(void);
        bool change_detail_level(bool expand);
        virtual QRectF shapeRect(const double buffer = 0.0) const;
        void setPos(const QPointF& pos);
        virtual void setSize(const QSizeF& size);
        void setPosAndSize(const QPointF& newCentrePos, const QSizeF& newSize);
        virtual void setCentrePos(const QPointF & pos);
        virtual Avoid::Polygon *poly(const double buffer,
                Avoid::Polygon *poly = NULL);
        void paintLabel(QPainter *painter);
        virtual QRectF labelBoundingRect(void) const;
        void drawLabelAndImage(QPixmap *target, const int x, const int y);
        virtual void change_label(void);
        virtual void set_label(const char *l);
        virtual bool canBe(const unsigned int flags);
        Guideline *get_guide(atypes type);
        Guideline *new_guide(atypes type);
        double attachedGuidelinePosition(atypes type) const;
        static double attachedGuidelinePosition(atypes type,
                const QRectF& shapeRect);
        void setLockedPosition(const bool val);
        bool hasLockedPosition(void);
        virtual void on_resize(bool setDetailLevel = false);
        ConnMultiset getConnMultiset(void);
        virtual void deactivateAll(CanvasItemSet& selSet);
        virtual void findAttachedSet(CanvasItemSet& objSet);
        void setDecorativeImageFile(const std::string fileName);
        void determine_good_text_dimensions(int *w, int *h);
        //virtual void determine_best_dimensions(int *w, int *h);
        //virtual void determine_small_dimensions(int *w, int *h);
        virtual QPainterPath buildPainterPath(void);
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                        QWidget *widget);
        void updateContainment(void);
        uint connectionPinForConnectionFlags(uint flags);
        void addConnectionPin(ConnectionPinInfo pinInfo);
        virtual QPointF centrePos(void) const;

        Relationship *rels[6];
        Avoid::ShapeRef *avoidRef;
        void setBeingResized(bool isResizing);
        bool isBeingResized(void);
    protected:
        virtual QAction *buildAndExecContextMenu(
                QGraphicsSceneMouseEvent *event, QMenu& menu);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);

        void paintShapeDecorations(QPainter *painter);
        void move_to(const int x, const int y, bool store_undo,
                bool from_solver, bool from_cider);

        bool m_has_locked_position;
        QGraphicsSvgItem *m_lock_icon;
        QPixmap* decorativeImage;
        QPixmap* smallDecorativeImage;
        double smallDecorativeScale;
        double detailLevel;
        bool beingResized;

    protected:
        virtual void routerAdd(void);
        virtual void routerRemove(void);
        virtual void routerMove(void);
        virtual void routerResize(void);

    private:
        virtual void userMoveBy(qreal dx, qreal dy);

        QString m_label;
        QColor m_fill_colour;
        QColor m_stroke_colour;
        QSet<ShapeObj *> m_parent_shapes;
        QSet<ShapeObj *> m_child_shapes;
        QList<ConnectionPinInfo> m_connection_pins;
        bool m_size_locked;
        QVector<Handle *> m_handles;

        friend class Cluster;
};

typedef std::list<ShapeObj *> ShapeList;


//===========================================================================
// Shape types:


class RectangleShape: public ShapeObj
{
    public:
        RectangleShape()
            : ShapeObj()
        {
        }
        virtual QDomElement to_QDomElement(const unsigned int subset, 
                QDomDocument& doc);
};


extern const QColor shLineCol;
extern const QColor shFillCol;

static const unsigned int defaultShapeFontSize = 11;
extern unsigned int shapeFontSize;

extern ShapeObj *isShapeForLayout(QGraphicsItem *object);

class ShapePluginInterface
{
    public:
        virtual ~ShapePluginInterface() {}

        virtual QString shapesClassLabel(void) const = 0;
        virtual QStringList shapes(void) const = 0;
        virtual ShapeObj *generateShape(QString shapeType) = 0;
};

}

Q_DECLARE_INTERFACE(dunnart::ShapePluginInterface,
        "org.dunnart.dunnart.ShapePluginInterface/1.0")

#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

