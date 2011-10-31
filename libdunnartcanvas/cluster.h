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

#ifndef CLUSTER_H
#define CLUSTER_H

#include "libavoid/geomtypes.h"

#include "libdunnartcanvas/shape.h"

class QGraphicsSceneWheelEvent;

namespace Avoid {
class ClusterRef;
}

namespace dunnart {


using Avoid::ShapeRef;

typedef std::set<Connector *> ConnSet;
typedef std::list<std::pair<Connector *, QDomElement> > XmlNodeList;
 
typedef std::map<unsigned int, QRectF> ClusterShapeMap;


class Cluster: public ShapeObj
{
    public:
        Cluster()
            : ShapeObj(x_cluster),
              rectangular(false),
              psn(0)
        {
        }
        Cluster(CanvasItemList& memberList, QString id);
        Cluster(Canvas *canvas, const QDomElement& node, const QString& ns);

        QDomElement to_QDomElement(const unsigned int subset,
                QDomDocument& doc);
        Avoid::Polygon *poly(const double buffer, Avoid::Polygon *poly = NULL);
        virtual void setLabel(const QString& label);
        void recomputeBoundary(void);
        void setNewBoundary(std::vector<Avoid::Point>& points);
        void computeBoundingBox(void);
        virtual bool canBe(const unsigned int flags);
        ShapeList& getMembers(void);
        ClusterShapeMap& getSmallShapeInfo(void)
        {
            return smallShapeInfo;
        }
        Avoid::ClusterRef *avoidClusterRef;
        virtual void setCentrePos(const QPointF & pos);
        virtual QPainterPath buildPainterPath(void);
        
        bool rectangular;
   protected:
        virtual QAction *buildAndExecContextMenu(
                QGraphicsSceneMouseEvent *event, QMenu& menu);
        virtual void routerAdd(void);
        virtual void routerRemove(void);
        virtual void routerMove(void);
        virtual void routerResize(void);
        virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
 
    private:
        virtual void userMoveBy(qreal dx, qreal dy);

        void calculateBoundary(void);
        void changeDetailLevel(bool expand);

        ShapeList members;
        std::vector<Avoid::Point> boundary;
        int psn;
        double m_detail_level;
        QSizeF m_expanded_size;
        ClusterShapeMap largeShapeInfo;
        ClusterShapeMap smallShapeInfo;
        ConnSet internalConns;
        XmlNodeList expandedConnRoutes;
        
};


extern Cluster *newCluster(CanvasItemList& memberList, QString id = QString());


}
#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

