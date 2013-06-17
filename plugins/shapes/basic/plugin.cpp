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

#include <QtWidgets>
#include <QObject>

#include "libdunnartcanvas/shapeplugininterface.h"
#include "libdunnartcanvas/shape.h"
using namespace dunnart;

#include "diamond.h"
#include "roundedrect.h"
#include "ellipse.h"

class BasicShapesPlugin : public QObject, public ShapePluginInterface
{
    Q_OBJECT
        Q_INTERFACES (dunnart::ShapePluginInterface)
        Q_PLUGIN_METADATA (IID "org.dunnart.BasicShapesPlugin")

    public:
        BasicShapesPlugin()
        {
        }
        QString shapesClassLabel(void) const
        {
            return "Basic";
        }
        QStringList producableShapeTypes() const
        {
            QStringList shapes;
            shapes << "org.dunnart.shapes.rect";
            shapes << "org.dunnart.shapes.diamond";
            shapes << "org.dunnart.shapes.roundedRect";
            shapes << "org.dunnart.shapes.ellipse";
            return shapes;
        }
        ShapeObj *generateShapeOfType(QString shapeType)
        {
            if (shapeType == "org.dunnart.shapes.rect")
            {
                return new RectangleShape();
            }
            else if (shapeType == "org.dunnart.shapes.diamond")
            {
                return new DiamondShape();
            }
            else if (shapeType ==  "org.dunnart.shapes.roundedRect")
            {
                return new RoundedRectShape();
            }
            else if (shapeType ==  "org.dunnart.shapes.ellipse")
            {
                return new EllipseShape();
            }
            return NULL;
        }
};


// Because there is no header file, we need to load the MOC file here to 
// cause Qt to generate it for us.
#include "plugin.moc"

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
