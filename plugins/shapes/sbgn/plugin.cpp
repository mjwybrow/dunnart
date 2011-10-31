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

#include "libdunnartcanvas/shapeplugininterface.h"
#include "libdunnartcanvas/shape.h"
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
        QStringList producableShapeTypes() const
        {
            QStringList shapes;
            shapes << "org.sbgn.pd.UnspecifiedEPN";
            shapes << "org.sbgn.pd.SourceOrSink";
            shapes << "org.sbgn.pd.SimpleChemEPN";
            shapes << "org.sbgn.pd.MacromolEPN";
            shapes << "org.sbgn.pd.NucleicAcidEPN";
            shapes << "org.sbgn.pd.ComplexEPN";
            shapes << "org.sbgn.pd.PerturbingEPN";
            shapes << "org.sbgn.pd.ProcessNode";
            shapes << "org.sbgn.pd.UnknownProcessNode";
            shapes << "org.sbgn.pd.OmittedProcessNode";
            shapes << "org.sbgn.pd.AssociationProcessNode";
            shapes << "org.sbgn.pd.DissociationProcessNode";
            shapes << "org.sbgn.pd.PhenotypeProcessNode";
            return shapes;
        }
        ShapeObj *generateShapeOfType(QString shapeType)
        {
            if (shapeType == "org.sbgn.pd.UnspecifiedEPN")
            {
                return new UnspecifiedEPN("Unspecified cloned", true);
            }
            else if (shapeType == "org.sbgn.pd.SourceOrSink")
            {
                return new SourceOrSink();
            }
            else if (shapeType == "org.sbgn.pd.SimpleChemEPN")
            {
                return new SimpleChemEPN("Hello World 1", true, true);
            }
            else if (shapeType == "org.sbgn.pd.MacromolEPN")
            {
                return new MacromolEPN("Cloned Macromol", false, "", false);
            }
            else if (shapeType == "org.sbgn.pd.NucleicAcidEPN")
            {
                return new NucleicAcidEPN("Cloned Nucleic Acid", true, "Clone marker", true);
            }
            else if (shapeType == "org.sbgn.pd.ComplexEPN")
            {
                return new ComplexEPN("Cloned Complex", true, "Clone label", true);
            }
            else if (shapeType == "org.sbgn.pd.PerturbingEPN")
            {
                return new PerturbingEPN("Perturbing EPN cloned", true);
            }
            else if (shapeType == "org.sbgn.pd.ProcessNode")
            {
                return new ProcessNode(Qt::Horizontal, ProcessNode::PROCESS);
            }
            else if (shapeType == "org.sbgn.pd.UnknownProcessNode")
            {
                return new ProcessNode(Qt::Vertical, ProcessNode::UNCERTAIN);
            }
            else if (shapeType == "org.sbgn.pd.OmittedProcessNode")
            {
                return new ProcessNode(Qt::Horizontal, ProcessNode::OMITTED);
            }
            else if (shapeType == "org.sbgn.pd.AssociationProcessNode")
            {
                return new ProcessNode(Qt::Horizontal, ProcessNode::ASSOCIATION);
            }
            else if (shapeType == "org.sbgn.pd.DissociationProcessNode")
            {
                return new ProcessNode(Qt::Horizontal, ProcessNode::DISSOCIATION);
            }
            else if (shapeType == "org.sbgn.pd.PhenotypeProcessNode")
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
