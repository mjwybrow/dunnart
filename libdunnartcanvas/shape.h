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
  
/**
  * @brief Base class for canvas shapes.
  *
  *
  * @par Detail Levels (Contextual Zoom)
  * When you create your own shapes, you can specify that they have
  * additional detail levels by overriding ShapeObj::levelsOfDetail().
  * You can then specify sizes for each detail level by overriding
  * ShapeObj::sizeForDetailLevel(uint level).
  *
  * @par
  * The base level of detail is 1.  Each additional level is numbered 2, 3,
  * etc and should usually make the shape larger and have increasing amounts
  * of detail.
  *
  * @par
  * You probably want to draw different amounts of text (or draw the shape
  * completely differently) depending on the detail level.  You can check
  * the current detail level by calling ShapeObj::currentDetailLevel().
  *
  * @par
  * You can resize a shape by moving a mouse scroll wheel when hovering
  * over the shape (this will only work for actual mice, not trackpads), or
  * by selecting the shape and hitting the '+' key to increase the detail
  * level or the '-' key to decrease it.
  */
class ShapeObj: public CanvasItem
{
    Q_OBJECT
    Q_PROPERTY (QPointF position READ centrePos WRITE cmd_setCentrePos)
    Q_PROPERTY (QString label READ getLabel WRITE setLabel)
    Q_PROPERTY (QSizeF size READ size WRITE cmd_setSize)
    Q_PROPERTY (QColor fillColour READ fillColour WRITE setFillColour)
    Q_PROPERTY (QColor strokeColour READ strokeColour WRITE setStrokeColour)
    Q_PROPERTY (bool sizeLocked READ sizeLocked WRITE setSizeLocked)
    Q_PROPERTY (bool pinned READ isPinned WRITE setPinned)

    // QMarginsF not handled by qtpropertybrowser.
    //Q_PROPERTY (QMarginsF containmentPadding READ containmentPadding() WRITE setContainmentPadding())

    UNDO_ACTION (ShapeObj, QPointF, centrePos, setCentrePos, UNDO_SHAPE_POS, "move shape")
    UNDO_ACTION (ShapeObj, QSizeF, size, setSize, UNDO_SHAPE_SIZE, "resize shape")

    public:
        ShapeObj(const QString& itemType);
        virtual ~ShapeObj();

        virtual void initWithXMLProperties(Canvas *canvas,
                const QDomElement& node, const QString& ns);
        virtual void initWithDimensions(QString id, const double x,
                const double y, const double w, const double h);

        //! @brief Override this to set up any label text, colours, etc for the
        //!        instance of the shape to be shown in the shape picker.
        //!
        virtual void setupForShapePickerPreview(void);

        //! @brief Override this to set up any label text, colours, etc for the
        //!        instance of the shape created on the canvas when the users
        //!        drags shapes from the shape picker.
        //!
        virtual void setupForShapePickerDropOnCanvas(void);

        QString getLabel(void) const;
        virtual void setLabel(const QString& label);

        QColor strokeColour(void) const;
        void setStrokeColour(const QColor& colour);
        QColor fillColour(void) const;
        void setFillColour(const QColor& colour);

        bool sizeLocked(void) const;
        void setSizeLocked(const bool locked);

        virtual void cascade_distance(int dist, unsigned int dir,
                CanvasItem **path);
        virtual void addXmlProps(const unsigned int subset, QDomElement& node,
                QDomDocument& doc);

        void addContainedShape(ShapeObj *shape);
        void addContainedShapes(QList<ShapeObj *>& shapes);
        void removeContainedShape(ShapeObj *shape);
        void removeContainedShapes(QList<ShapeObj *>& shapes);

        //! @brief Returns the internal padding specified for containment
        //!        constraints for any contained shapes.
        //!
        //! @return A QMarginsF object representing the padding values.
        //!
        QMarginsF containmentPadding(void) const;

        //! @brief Specify internal padding values to be used for constraining
        //!        contained shapes within this shape.
        //!
        //! @param padding A QMarginsF object specifying padding values.
        //!
        void setContainmentPadding(const QMarginsF& padding);

        QList<ShapeObj *> containedShapes(void) const;
        void showConnectionPoints(void);
        void hideConnectionPoints(void);
        bool changeDetailLevel(bool expand);
        virtual QRectF shapeRect(const double buffer = 0.0) const;
        void setPos(const QPointF& pos);
        virtual void setSize(const QSizeF& size);
        void setPosAndSize(const QPointF& newCentrePos, const QSizeF& newSize);
        virtual void setCentrePos(const QPointF & pos);
        virtual Avoid::Polygon polygon(void) const;
        void paintLabel(QPainter *painter);
        virtual QRectF labelBoundingRect(void) const;
        void drawLabelAndImage(QPixmap *target, const int x, const int y);
        virtual void changeLabel(void);
        Guideline *attachedGuidelineOfType(atypes type);
        Guideline *newGuidelineOfType(atypes type);
        double attachedGuidelinePosition(atypes type) const;
        static double attachedGuidelinePosition(atypes type,
                const QRectF& shapeRect);
        void setPinned(const bool val);
        bool isPinned(void);
        ConnMultiset getConnMultiset(void);
        virtual void deactivateAll(CanvasItemSet& selSet);
        virtual void findAttachedSet(CanvasItemSet& objSet);
        void setDecorativeImageFile(const std::string fileName);

        //void determine_good_text_dimensions(int *w, int *h);
        //virtual void determine_best_dimensions(int *w, int *h);
        //virtual void determine_small_dimensions(int *w, int *h);
        virtual QPainterPath buildPainterPath(void);
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                        QWidget *widget);
        void updateContainment(void);
        uint connectionPinForConnectionFlags(uint flags);
        void addConnectionPin(ConnectionPinInfo pinInfo);
        virtual QPointF centrePos(void) const;
        void setBeingResized(bool isResizing);
        bool isBeingResized(void);

        Relationship *rels[6];
        Avoid::ShapeRef *avoidRef;
    protected:
        virtual QAction *buildAndExecContextMenu(
                QGraphicsSceneMouseEvent *event, QMenu& menu);
        virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void wheelEvent(QGraphicsSceneWheelEvent *event);

        virtual void routerAdd(void);
        virtual void routerRemove(void);
        virtual void routerMove(void);
        virtual void routerResize(void);

        void paintShapeDecorations(QPainter *painter);
        void move_to(const int x, const int y, bool store_undo,
                bool from_solver, bool from_cider);

        /**
          * Override this to specify how many levels of detail your shape has.
         */
        virtual uint levelsOfDetail(void) const;

        /**
          * Override this to specify the size that your shape should be
          * expanded to at each detail level.  The base level is 1.  Any
          * subsequent levels will be 2, 3, ...
         */
        virtual QSizeF sizeForDetailLevel(uint level);

        /**
          * Check the current detail level.  You can use this when painting
          * the shape.
         */
        uint currentDetailLevel(void) const;

        bool m_is_pinned;
        QGraphicsSvgItem *m_lock_icon;
        QPixmap* decorativeImage;
        QPixmap* smallDecorativeImage;
        double smallDecorativeScale;

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
        uint m_detail_level;
        bool m_being_resized;
        QMarginsF m_containment_padding;

        friend class Cluster;
};

typedef std::list<ShapeObj *> ShapeList;


//===========================================================================
// Shape types:


class RectangleShape: public ShapeObj
{
    public:
        RectangleShape()
            : ShapeObj("rect")
        {
        }
};


extern const QColor shLineCol;
extern const QColor shFillCol;

extern ShapeObj *isShapeForLayout(QGraphicsItem *object);


}
#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

