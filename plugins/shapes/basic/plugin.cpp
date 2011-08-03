/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2010  Monash University
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

#include <QtGui>
#include <QObject>

#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/canvasitem.h"
using namespace dunnart;

#include "diamond.h"
#include "ellipse.h"
#include "inputoutput.h"
#include "userinteraction.h"

class BasicShapesPlugin : public QObject, public ShapePluginInterface
{
    Q_OBJECT
        Q_INTERFACES (dunnart::ShapePluginInterface)

    public:
        BasicShapesPlugin()
        {
        }
        QString shapesClassLabel(void) const
        {
            return "Basic";
        }
        QStringList shapes() const
        {
            QStringList shapes;
            shapes << "org.dunnart.shapes.flowDiamond";
            shapes << "org.dunnart.shapes.flowInOutput";
            shapes << "org.dunnart.shapes.flowEndOProc";
            shapes << "org.dunnart.shapes.flowUserInt";
            return shapes;
        }
        ShapeObj *generateShape(QString shapeType)
        {
            if (shapeType == "org.dunnart.shapes.flowDiamond")
            {
                return new DiamondShape();
            }
            else if (shapeType == "org.dunnart.shapes.flowInOutput")
            {
                return new InputOutputShape();
            }
            else if (shapeType ==  "org.dunnart.shapes.flowEndOProc")
            {
                return new EllipseShape();
            }
            else if (shapeType == "org.dunnart.shapes.flowUserInt")
            {
                return new UserInterationShape();
            }
            return NULL;
        }
};

Q_EXPORT_PLUGIN2(shapes_basic, BasicShapesPlugin)

// Because there is no header file, we need to load the MOC file here to 
// cause Qt to generate it for us.
#include "plugin.moc"

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
