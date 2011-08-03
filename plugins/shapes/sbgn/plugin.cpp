/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2010-2011  Monash University
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
 * Author(s): Sarah Boyd  <Sarah.Boyd@monash.edu>
*/


#include <QtGui>
#include <QObject>

#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/canvasitem.h"
using namespace dunnart;

#include "pdepn.h"
#include "pdunspecifiedepn.h"
#include "pdsourcesink.h"
#include "pdsimplechemepn.h"
#include "pdmacromolepn.h"
#include "pdnucleicepn.h"
#include "pdcomplexepn.h"
#include "pdperturbingepn.h"
#include "pdphenotypeprocessnode.h"
//#include "pdprocessnode.h"

class SBGNShapesPlugin : public QObject, public ShapePluginInterface
{
    Q_OBJECT
        Q_INTERFACES (dunnart::ShapePluginInterface)

    public:
        SBGNShapesPlugin()
        {
        }
        QString shapesClassLabel(void) const
        {
            return "SBGN";
        }
        QStringList shapes() const
        {
            QStringList shapes;
            shapes << "org.dunnart.shapes.sbgn.UnspecifiedEPN";
            shapes << "org.dunnart.shapes.sbgn.SourceOrSink";
            shapes << "org.dunnart.shapes.sbgn.SimpleChemEPN";
            shapes << "org.dunnart.shapes.sbgn.MacromolEPN";
            shapes << "org.dunnart.shapes.sbgn.NucleicAcidEPN";
            shapes << "org.dunnart.shapes.sbgn.ComplexEPN";
            shapes << "org.dunnart.shapes.sbgn.PerturbingEPN";
            shapes << "org.dunnart.shapes.sbgn.ProcessNode";
            shapes << "org.dunnart.shapes.sbgn.UnknownProcessNode";
            shapes << "org.dunnart.shapes.sbgn.OmittedProcessNode";
            shapes << "org.dunnart.shapes.sbgn.AssociationProcessNode";
            shapes << "org.dunnart.shapes.sbgn.DissociationProcessNode";
            shapes << "org.dunnart.shapes.sbgn.PhenotypeProcessNode";
            return shapes;
        }
        ShapeObj *generateShape(QString shapeType)
        {
            if (shapeType == "org.dunnart.shapes.sbgn.UnspecifiedEPN")
            {
                return new UnspecifiedEPN("Unspecified cloned", true);
            }
            else if (shapeType == "org.dunnart.shapes.sbgn.SourceOrSink")
            {
                return new SourceOrSink();
            }
            else if (shapeType == "org.dunnart.shapes.sbgn.SimpleChemEPN")
            {
                return new SimpleChemEPN("Hello World 1", true, true);
            }
            else if (shapeType == "org.dunnart.shapes.sbgn.MacromolEPN")
            {
                return new MacromolEPN("Cloned Macromol", true, "Label ...", true);
            }
            else if (shapeType == "org.dunnart.shapes.sbgn.NucleicAcidEPN")
            {
                return new NucleicAcidEPN("Cloned Nucleic Acid", true, "Clone marker", true);
            }
            else if (shapeType == "org.dunnart.shapes.sbgn.ComplexEPN")
            {
                return new ComplexEPN("Cloned Complex", true, "Clone label", true);
            }
            else if (shapeType == "org.dunnart.shapes.sbgn.PerturbingEPN")
            {
                return new PerturbingEPN("Perturbing EPN cloned", true);
            }
            else if (shapeType == "org.dunnart.shapes.sbgn.ProcessNode")
            {
                return new ProcessNode(Qt::Horizontal, ProcessNode::PROCESS);
            }
            else if (shapeType == "org.dunnart.shapes.sbgn.UnknownProcessNode")
            {
                return new ProcessNode(Qt::Vertical, ProcessNode::UNCERTAIN);
            }
            else if (shapeType == "org.dunnart.shapes.sbgn.OmittedProcessNode")
            {
                return new ProcessNode(Qt::Horizontal, ProcessNode::OMITTED);
            }
            else if (shapeType == "org.dunnart.shapes.sbgn.AssociationProcessNode")
            {
                return new ProcessNode(Qt::Horizontal, ProcessNode::ASSOCIATION);
            }
            else if (shapeType == "org.dunnart.shapes.sbgn.DissociationProcessNode")
            {
                return new ProcessNode(Qt::Horizontal, ProcessNode::DISSOCIATION);
            }
            else if (shapeType == "org.dunnart.shapes.sbgn.PhenotypeProcessNode")
            {
                return new PhenotypeProcessNode("Phenotype cloned", true);
            }
            return NULL;
        }
};

Q_EXPORT_PLUGIN2(shapes_sbgn, SBGNShapesPlugin)

// Because there is no header file, we need to load the MOC file here to 
// cause Qt to generate it for us.
#include "plugin.moc"

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
