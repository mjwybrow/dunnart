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

#ifndef FREEHAND_H
#define FREEHAND_H

#include <utility>
using namespace std;

// For geometry:
#include "libdunnartcanvas/shape.h"

#include <list>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <stdint.h>

typedef double                               StlClassProbabilityT;
typedef std::map<int, StlClassProbabilityT>  StlClassProbabilities;

typedef double                               StlFeatureResultT;
typedef std::vector<StlFeatureResultT>       StlFeatureVec;


typedef float     StlStrokeCoordT;
typedef float     StlStrokePressureT;
typedef uint32_t  StlStrokeTimeT;

struct StlStrokePoint {
  StlStrokePoint( StlStrokeCoordT x = 0.0,
		  StlStrokeCoordT y = 0.0,
		  StlStrokePressureT pressure  = 0.0,
		  StlStrokeTimeT milliTime = 0 )
    : x(x),
      y(y),
      pressure(pressure),
      milliTime(milliTime)
  {}

  StlStrokeCoordT x;
  StlStrokeCoordT y;
  StlStrokePressureT pressure;
  StlStrokeTimeT milliTime;
#if 0
  float rotation;
  float xTilt;
  float yTilt;
#endif
};

typedef std::vector<StlStrokePoint>  StlStroke;
typedef std::list<StlStroke>         StlStrokeList;


template <class T>
bool from_string( T& t,
                  const std::string& s, 
                  std::ios_base& (*f)(std::ios_base&) = std::dec ) {
  std::istringstream iss(s);
  return !(iss >> f >> t).fail();
}



namespace Avoid {
class ShapeRef;
}

namespace dunnart {


using Avoid::ShapeRef;

class FreehandGeometry
{
    public:
        FreehandGeometry();
        void setInitialOffset(double x, double y);
        void addPoint(double x, double y);
        void addPoint(double x, double y, unsigned int t);
        void removeOffset(void);
        void startNewStroke(void);
        void addStroke(const FreehandGeometry& geometry);
        void calcBBox(void);

        StlStrokeList strokes;
        StlStroke *currStroke, flattened_stroke;
        double x, y, w, h;
        unsigned int startTime;
        double initialOffsetX, initialOffsetY;
};

extern void freehand_time_expired(void);

class FreehandShape: public ShapeObj
{
    public:
        FreehandShape();

        virtual void initWithXMLProperties(Canvas *canvas,
                const QDomElement& node, const QString& ns);
        QDomElement to_QDomElement(const unsigned int subset, 
                QDomDocument& doc);

        void finalise(void);
        void addStroke(FreehandGeometry& geometry);
        Avoid::Polygon *poly(const double buffer, Avoid::Polygon *poly = NULL);
        FreehandGeometry& getGeometry(void);
        void setPosition(int x, int y, bool from_cider);
        void setColour(unsigned int r, unsigned int g, unsigned int b);
        virtual QPainterPath buildPainterPath(void);

        int get_mostlikely_id() const;
        StlClassProbabilityT get_mostlikely_prob() const;
        StlClassProbabilities get_probability_map() const;

    private:
        FreehandGeometry _geometry;
        QColor _colour;

        StlClassProbabilities class_probs;
        int mostlikely_class_id;
        StlClassProbabilityT mostlikely_class_prob;

        void cache_mostlikely_class();
};

extern void start_freehand_drawing(void);
extern bool handle_freehand_shape_click(void);


}
#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

