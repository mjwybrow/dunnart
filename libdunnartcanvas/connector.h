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

extern bool createDirectedConns;
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

        void rerouteIntersect(void);
        virtual void cascade_distance(int dist, unsigned int dir,
                CanvasItem **path);
        virtual void deactivateAll(CanvasItemSet& selSet);

        void setRoutingCheckPoints(const QList<QPointF>& checkpoints);
        void move_endpoint(endPt ep, int diff_x, int diff_y);
        void update_and_reroute(bool reroute = true);
        void addXmlProps(const unsigned int subset, QDomElement& node,
                QDomDocument& doc);
        QPair<CPoint, CPoint> get_connpts(void) const;
        void update_endpoints(void);
        void disconnect_from(ShapeObj *shape, uint pinClassID = 0);
        QPair<ShapeObj *, ShapeObj *> getAttachedShapes(void);
        bool hasSameEndpoints(void);
        //QT virtual void addContextMenuItems(MenuItems& items);
        //QT virtual void changeContextMenuState(Menu *menu);
        Avoid::ConnRef *avoidRef;
        void updateFromPathChange(void);
        bool isCyclic();
        void isCyclic(bool value);
        bool isInEa();
        void isInEa(bool value);
        void overrideColour(QColor col);
        void restoreColour();
        void setDotted(bool dotted) {this->dotted = dotted;}
        void setInEa(bool value);
        virtual void move_diff_points(int diff_x, int diff_y);
        void adjust_endpoint_for_vis(int type, Avoid::Point& adjpt,
                Avoid::Vector slp = Avoid::Vector());
        virtual void UpdateEndptVis(const int type);
        void reapplyRoute(void);
        void applyNewRoute(const Avoid::Polygon& route);
        void applyNewRoute(const Avoid::PolyLine& route, bool updateLibavoid);
        void updateFromLibavoid(void);
        virtual void write_svg_path(QDomElement& node);
        virtual void write_libavoid_path(QDomElement& node,
                QDomDocument& doc);
        virtual QDomElement to_QDomElement(const unsigned int subset,
                QDomDocument& doc);
        QRectF boundingRect(void) const;
        QPainterPath shape() const;
        void paint(QPainter *painter,
                const QStyleOptionGraphicsItem *option, QWidget *widget);
        virtual void setPainterPath(QPainterPath path);
        virtual void loneSelectedChange(const bool value);
        void buildArrowHeadPath(void);
        void setNewEndpointPos(const int endptType, QPointF pos,
            ShapeObj *shape = NULL, uint pinClassID = 0);
        static bool drawArrow(QPainterPath& painter_path, double srcx,
                double srcy, double dstx, double dsty,
                Connector::ArrowHeadType arrow_type = Connector::normal);
        static QString valueStringForEnum(const char *enumName,
                int enumValue);

        bool multiEdge;
        unsigned multiEdgeSize;
        unsigned multiEdgeInd;
        OrthogonalConstraint orthogonalConstraint;

    public slots:
        void swapDirection(void);

    protected:
        virtual QAction *buildAndExecContextMenu(
                QGraphicsSceneMouseEvent *event, QMenu& menu);
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change,
                const QVariant &value);
        void initialiser(void);
        virtual void calc_layout(void);
        virtual void routerAdd(void);
        virtual void routerRemove(void);
        void updateConnections(void);

        double m_ideal_length;
        QColor m_colour;
        QColor m_saved_colour;
        int num_points;
        unsigned int srctype, dsttype;
        Avoid::Polygon offset_route;
        Avoid::PolyLine offset_obs_route;
        CPoint srcpt, dstpt;
        int bbx, bby, bbw, bbh;
        bool m_directed;
        RoutingType m_routing_type;
        bool cyclic;
        bool inEa;
        ArrowHeadType m_arrow_head_type;
        bool m_arrow_head_outline;
        bool dotted;
        void applyMultiEdgeOffset(Avoid::Point& p1, Avoid::Point& p2,
                bool justSecond = true);
        QPainterPath m_arrow_path;
        // Unclosed and unreversed representation of connector path route.
        QPainterPath m_conn_path;
        QPainterPath m_shape_path;
        QVector<Handle *> m_handles;
};



// Connector representation
struct ConnRepStruct
{
    int width, height;
    int xpos, ypos;
    int type;

    int numpoints;
    CPoint *points;
    double idealLength;
};



}

Q_DECLARE_METATYPE (dunnart::Connector::RoutingType)
Q_DECLARE_METATYPE (dunnart::Connector::ArrowHeadType)

#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

