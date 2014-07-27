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

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <list>
#include <utility>

#include <QColor>

#include "libdunnartcanvas/canvasitem.h"

#include "libavoid/geomtypes.h"


class QGraphicsSceneMouseEvent;

namespace Avoid {
class ConnRef;
class Router;
}

namespace dunnart {

class ShapeObj;

extern const QColor defaultConnLineCol;

typedef struct ConnRepStruct ConnRep;

class Handle;

class CPoint
{
    public:
        CPoint()
            : x(0),
              y(0),
              shape(NULL),
              pinClassID(0)
        {
        }
        
        double x, y;
        ShapeObj *shape;
        uint pinClassID;

        CPoint& operator= (const CPoint& f)
        {
            // Gracefully handle self assignment
            if (this == &f) return *this;
            
            x = f.x;
            y = f.y;
            shape = f.shape;
            pinClassID = f.pinClassID;

            return *this;
        }
};


//===========================================================================
// Connector types:


enum endPt { SRCPT, DSTPT };


class Connector : public CanvasItem
{
    Q_OBJECT
    Q_PROPERTY (RoutingType routingType READ routingType WRITE setRoutingType)
    Q_PROPERTY (bool directed READ isDirected WRITE setDirected)
    Q_PROPERTY (double idealLength READ idealLength WRITE setIdealLength)
    Q_PROPERTY (ArrowHeadType arrowHeadType READ arrowHeadType WRITE setArrowHeadType)
    Q_PROPERTY (QColor colour READ colour WRITE setColour)
    Q_PROPERTY (QString label READ label WRITE setLabel)
    Q_PROPERTY (bool obeysDirectedConstraint READ obeysDirectedEdgeConstraints WRITE setObeysDirectedEdgeConstraints)
    Q_ENUMS (RoutingType)
    Q_ENUMS (ArrowHeadType)

    public:
        enum RoutingType { orthogonal, polyline };
        enum OrthogonalConstraint { NONE, HORIZONTAL, VERTICAL };
        enum ArrowHeadType
        {
            normal,
            triangle_outline,
            triangle_filled,
            circle_outline,
            circle_filled,
            diamond_outline,
            diamond_filled,
            cross,
            cross_triangle_outline,
            cross_triangle_filled
        };

        friend void spline_preview(CanvasItem **c);
        Connector();
        ~Connector();

        virtual void initWithConnection(ShapeObj *sh1, ShapeObj *sh2);
        virtual void initWithXMLProperties(Canvas *canvas,
                const QDomElement& node, const QString& ns);

        bool isDirected(void) const;
        void setDirected(const bool directed);

        RoutingType routingType(void) const;
        void setRoutingType(const RoutingType routingType);

        QColor colour(void) const;
        void setColour(const QColor colour);

        ArrowHeadType arrowHeadType(void) const;
        void setArrowHeadType(const ArrowHeadType arrowHeadType);

        double idealLength(void) const;
        void setIdealLength(double length);

        //! @brief Return the current label for the connector.
        //!
        //! @return The current label, a QString.
        //!
        virtual QString label(void) const;

        //! @brief Set a new label for the connector.
        //!
        //! @param label  A QString with the new label.
        //!
        virtual void setLabel(const QString& label);

        void rerouteAvoidingIntersections(void);
        virtual void cascade_distance(int dist, unsigned int dir,
                CanvasItem **path);
        virtual void deactivateAll(CanvasItemSet& selSet);

        void setRoutingCheckPoints(const QList<QPointF>& checkpoints);
        void forceReroute(void);
        virtual void addXmlProps(const unsigned int subset, QDomElement& node,
                QDomDocument& doc);
        QPair<CPoint, CPoint> get_connpts(void) const;
        void disconnect_from(ShapeObj *shape, uint pinClassID = 0);
        QPair<ShapeObj *, ShapeObj *> getAttachedShapes(void);
        bool hasSameEndpoints(void);
        Avoid::ConnRef *avoidRef;
        bool hasDownwardConstraint(void) const;
        void setHasDownwardConstraint(const bool value);
        bool obeysDirectedEdgeConstraints(void) const;
        void setObeysDirectedEdgeConstraints(const bool value);
        void overrideColour(QColor col);
        void restoreColour();
        void setDotted(bool dotted) {this->m_is_dotted = dotted;}
        virtual void setNewLibavoidEndpoint(const int type);
        void reapplyRoute(void);
        void applyNewRoute(const Avoid::Polygon& route);
        void applyNewRoute(const Avoid::PolyLine& route, bool updateLibavoid);
        void updateFromLibavoid(void);
        virtual void write_libavoid_path(QDomElement& node,
                QDomDocument& doc);
        QRectF boundingRect(void) const;
        QPainterPath shape() const;
        void paint(QPainter *painter,
                const QStyleOptionGraphicsItem *option, QWidget *widget);
        virtual void setPainterPath(QPainterPath path);
        virtual void loneSelectedChange(const bool value);
        void buildArrowHeadPath(void);
        void setNewEndpoint(const int endptType, QPointF pos,
                ShapeObj *shape = NULL, uint pinClassID = 0);
        static bool drawArrow(QPainterPath& painter_path, double srcx,
                double srcy, double dstx, double dsty,
                Connector::ArrowHeadType arrow_type = Connector::normal);
        static QString valueStringForEnum(const char *enumName,
                int enumValue);

        bool m_is_multiedge;
        unsigned m_multiedge_size;
        unsigned m_multiedge_index;
        OrthogonalConstraint m_orthogonal_constraint;

    public slots:
        void swapDirection(void);

    protected:
        virtual QAction *buildAndExecContextMenu(
                QGraphicsSceneMouseEvent *event, QMenu& menu);
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
                const QVariant &value);
        void initialiser(void);
        virtual void triggerReroute(bool now = false);
        virtual void routerAdd(void);
        virtual void routerRemove(void);
        void applySimpleRoute(void);
        void applyMultiEdgeOffset(Avoid::Point& p1, Avoid::Point& p2,
                bool justSecond = true);

        QString m_label;
        double m_ideal_length;
        QColor m_colour;
        QColor m_saved_colour;
        Avoid::Polygon m_offset_route;
        CPoint m_src_pt;
        CPoint m_dst_pt;
        bool m_is_directed;
        RoutingType m_routing_type;
        bool m_has_downward_constraint;
        bool m_obeys_directed_edge_constraints;
        ArrowHeadType m_arrow_head_type;
        bool m_arrow_head_outline;
        bool m_is_dotted;
        QPainterPath m_arrow_path;
        // Unclosed and unreversed representation of connector path route.
        QPainterPath m_conn_path;
        QPainterPath m_shape_path;
        QVector<Handle *> m_handles;
        bool m_is_lone_selected;
};


}

Q_DECLARE_METATYPE (dunnart::Connector::RoutingType)
Q_DECLARE_METATYPE (dunnart::Connector::ArrowHeadType)

#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

