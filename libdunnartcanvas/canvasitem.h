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

#ifndef CANVASITEM_H
#define CANVASITEM_H

#include <QtXml>
#include <QRectF>
#include <QString>
#include <QGraphicsItem>
#include <QGraphicsSvgItem>
#include <QSet>
#include <QMenu>
#include <QAction>
#include <QUndoCommand>


#include <set>
#include <iostream>


class QGraphicsSceneMouseEvent;

namespace dunnart {


static const unsigned CASCADE_VERT = 1;
static const unsigned CASCADE_HORI = 2;
static const unsigned CASCADE_ALL  = 3;

static const double BOUNDINGRECTPADDING = 3;


// SVG:
extern const char *x_id;
// Namespace:
extern const QString x_dunnartNs;
// Dunnarts:
extern const char *x_type;
extern const char *x_connector;
extern const char *x_xPos;
extern const char *x_yPos;
extern const char *x_centreX;
extern const char *x_centreY;
extern const char *x_width;
extern const char *x_height;
extern const char *x_transparent;
extern const char *x_lineCol;
extern const char *x_fillCol;
extern const char *x_label;
extern const char *x_idealLength;
extern const char *x_srcX;
extern const char *x_srcY;
extern const char *x_srcID;
extern const char *x_srcFlags;
extern const char *x_srcPinID;
extern const char *x_dstX;
extern const char *x_dstY;
extern const char *x_dstID;
extern const char *x_dstFlags;
extern const char *x_dstPinID;
extern const char *x_directed;
extern const char *x_reversed;
extern const char *x_rectangular;
extern const char *x_obeysDirEdgeConstraints;
extern const char *x_orthogonalConstraint;
extern const char *x_guideline;
extern const char *x_cluster;
extern const char *x_shPolygon;
extern const char *x_shTextShape;
extern const char *x_shFreehand;
extern const char *x_constraintA;
extern const char *x_constraintD;
extern const char *x_constraintS;
extern const char *x_contains;
extern const char *x_constraint;
extern const char *x_position;
extern const char *x_direction;
extern const char *x_objOneID;
extern const char *x_objTwoID;
extern const char *x_constraintID;
extern const char *x_alignmentPos;
extern const char *x_node;
extern const char *x_relType;
extern const char *x_alignment;
extern const char *x_distribution;
extern const char *x_separation;
extern const char *x_isMultiway;
extern const char *x_sepDistance;
extern const char *x_fontSize;
extern const char *x_geometry;
extern const char *x_dunnartURI;
extern const char *x_lockedPosition;
extern const char *x_libavoidPath;

//-------------------------------------------------

static const unsigned int XMLSS_INONE    = 0;
static const unsigned int XMLSS_IOTHER   = 2;
static const unsigned int XMLSS_IMOVE    = 4;
static const unsigned int XMLSS_IRESIZE  = 8;
static const unsigned int XMLSS_ILABEL   = 16;
static const unsigned int XMLSS_ICONNS   = 32;

static const unsigned int XMLSS_XMOVE    = 64;

// XMLSS_ALL should be all of XMLSS_I*:
static const unsigned int XMLSS_ALL      = XMLSS_XMOVE - 1;

//-------------------------------------------------

static const QColor HAZARD_COLOUR = QColor(255, 80,  0);


static const int HANDLE_PADDING = 3;

// Only display guides to edge of things they are connected to.
static const bool two_tier_indicators = true;

enum loadPass
{
    PASS_SHAPES,
    PASS_CLUSTERS,
    PASS_CONNECTORS,
    PASS_RELATIONSHIPS,
    PASS_LAST
};

class Canvas;
class CanvasItem;
typedef QSet<CanvasItem *> CanvasItemSet;


class CanvasItem: public QGraphicsSvgItem
{
    Q_OBJECT
    Q_PROPERTY (QString id READ getIdString)

    public:
        CanvasItem(QGraphicsItem *parent, QString id, unsigned int lev);
        virtual ~CanvasItem();

        virtual void initWithXMLProperties(Canvas *canvas,
                const QDomElement& node, const QString& ns);

        QString getIdString(void) const;
        uint internalId(void) const;

        Canvas *canvas(void) const;
        void bringToFront(void);
        void sendToBack(void);
        static CanvasItem *create(Canvas *canvas, const QDomElement& node, 
                const QString& dunnartURI, int pass);
        virtual QDomElement to_QDomElement(const unsigned int subset, 
                QDomDocument& doc);
        virtual void cascade_distance(int dist, unsigned int dir,
                CanvasItem **path) = 0;
        void update_after_unhide(void);
        bool cascade_logic(int& nextval, int dist, unsigned int dir,
                CanvasItem **path);
        void move_to(const int x, const int y, bool store_undo);
        void glowSetClipRect(QPixmap *surface);
        void glowClearClipRect(QPixmap *surface);
        virtual void resizedCanvas(void)  { }
        virtual bool canBe(const unsigned int flags);
        virtual bool isCollapsed(void);
        void setAsCollapsed(bool collapsed);
        virtual void deactivateAll(CanvasItemSet& selSet) = 0;
        bool isInactive(void) const;
        void setAsInactive(bool inactive, 
                CanvasItemSet fullSet = CanvasItemSet());
        void maybeReturn(void);
        virtual void findAttachedSet(CanvasItemSet& objSet)
        {
            Q_UNUSED(objSet);
        }
        //! @brief   Returns the bounding rectangle required to render the
        //!          canvas item within.
        //! @note    This can't be called from within the buildPainterPath()
        //!          method, since it depends on the painter path already
        //!          hacing been built.
        //! @returns A QRectF preresenting the item's bounding rectangle.
        virtual QRectF boundingRect() const;
        virtual QPainterPath shape() const;
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option,
                                        QWidget *widget);
        double width(void) const;
        double height(void) const;
        void setSize(const double w, const double h);
        void dragReleaseEvent(QGraphicsSceneMouseEvent *event);
        QSizeF size(void) const;
        virtual void setSize(const QSizeF& size);
        void setConstraintConflict(const bool conflict);
        bool constraintConflict(void) const;
        virtual void loneSelectedChange(const bool value);
        QString svgCodeAsString(const QSize& size, const QRectF& viewBox);

        int distance;
        int cascade_glow;
        CanvasItem *connectedObjs[2];
    protected:
        void setHoverMessage(const QString& message);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event); 
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        virtual void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual QPainterPath buildPainterPath(void);
        QPainterPath painterPath(void) const;
        virtual void setPainterPath(QPainterPath path);

        // This method resizes the canvas item and also triggers the
        // painter path used for drawing to be recreated.
        void setSizeAndUpdatePainterPath(const QSizeF& newSize);

        QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
                const QVariant &value);
        void addXmlProps(const unsigned int subset, QDomElement& node,
                QDomDocument& doc);
        virtual QAction *buildAndExecContextMenu(
                QGraphicsSceneMouseEvent *event, QMenu& menu);

        // Libavoid related methods
        virtual void routerAdd(void);
        virtual void routerRemove(void);
        virtual void routerMove(void);
        virtual void routerResize(void);

        QString m_string_id;
        uint m_internal_id;
        bool m_is_collapsed;
        bool _inactive;
    private:
        friend class AlterCanvasItemProperty;
        virtual void userMoveBy(qreal dx, qreal dy);

        QPainterPath m_painter_path;
        QSizeF m_size;
        QString m_hover_message;
        bool m_constraint_conflict;
};

typedef QList<CanvasItem *> CanvasItemList;


enum {
    ZORD_Template = 1,
    ZORD_Cluster,
    ZORD_Guideline,
    ZORD_Distribution,
    ZORD_TransSh,
    ZORD_Conn,
    ZORD_Shape
};


static const unsigned int C_NONE = 0;
static const unsigned int C_ALIGNED = 1;
static const unsigned int C_CLUSTERED = 2;

inline QString qualify(const QString& prefix, const QString& name)
{
    return (prefix.isEmpty()) ? name : prefix + ":" + name;
}

extern QString nodeAttribute(const QDomElement& node, const QString& ns,
        const QString& prop);
extern bool nodeHasAttribute(const QDomElement& node, const QString& ns,
        const QString& prop);

template <typename T>
bool optionalProp(const QDomElement& node, const QString& prop, T &arg, 
        const QString& ns = QString())
{
    bool gotProp = false;
    QString value = nodeAttribute(node, ns, prop);
    if (!value.isNull())
    {
        QVariant variant(value);
        if (qVariantCanConvert<T>(variant))
        {
            arg = qVariantValue<T>(variant);

            //std::cout << "Read \""<<prop.toStdString()<<"\"="<<
            //        variant.toString().toLatin1().constData()<<std::endl;
            gotProp = true;
        } 
        else
        {
            std::cout << "WARNING: couldn't parse value for property \"" <<
                    prop.toStdString() << "\": \"" << value.toStdString() <<
                    "\"" << std::endl;
        }
    }
    return gotProp;
}


template <typename T>
T essentialProp(const QDomElement& node, const QString& prop,
        const QString& nsURI = QString())
{
    T value;
    bool result = optionalProp(node, prop, value, nsURI);

    if (!result)
    {
        qFatal("Essential property \"%s:%s\" not found in node.",
                qPrintable(nsURI), qPrintable(prop));
    }
    return value;
}

template <typename T>
void newProp(QDomElement& node, const QString& prop, T argument,
        const char *append = "")
{
    QString value = QString("%1%2").arg(argument).arg(append);
    node.setAttribute(prop, value);
}


template <typename T>
void newNsProp(QDomElement& node, const QString& ns, const QString& prop,
        T argument, const char *append = "")
{
    QString value = QString("%1%2").arg(argument).arg(append);
    node.setAttribute(qualify(ns, prop), value);
}


extern void returnAllInactive(void);
extern void returnAppropriateConnectors(void);
extern QColor QColorFromRRGGBBAA(char *str);


}
#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

