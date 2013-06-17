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
#include <QtGlobal>

#include "libdunnartcanvas/shapeplugininterface.h"
#include "libdunnartcanvas/shape.h"
using namespace dunnart;

#include "helix.h"
#include "strand.h"

class ProOrigamiShapesPlugin : public QObject, public ShapePluginInterface
{
    Q_OBJECT
        Q_INTERFACES (dunnart::ShapePluginInterface)
        Q_PLUGIN_METADATA (IID "org.dunnart.ProOrigamiShapesPlugin")

    public:
        ProOrigamiShapesPlugin()
        {
        }
        QString shapesClassLabel(void) const
        {
            return "Pro-origami";
        }
        QStringList producableShapeTypes() const
        {
            QStringList shapes;
            shapes << "org.dunnart.shapes.bioStrand";
            shapes << "org.dunnart.shapes.bioHelix";
            return shapes;
        }
        ShapeObj *generateShapeOfType(QString shapeType)
        {
            if (shapeType == "org.dunnart.shapes.bioStrand")
            {
                return new BioStrand();
            }
            else if (shapeType == "org.dunnart.shapes.bioHelix")
            {
                return new BioHelix();
            }
            return NULL;
        }
};


// Because there is no header file, we need to load the MOC file here to 
// cause Qt to generate it for us.
#include "plugin.moc"

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
