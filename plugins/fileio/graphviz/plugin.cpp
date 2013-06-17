/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2012-2013  Monash University
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

//! @file
//! Plugin that adds support for reading and writing Graphviz's 'dot'
//! file format.

#include <QtWidgets>
#include <QObject>
#include <QFileInfo>
#include <QDomDocument>
#include <QSvgGenerator>

#include "libdunnartcanvas/fileioplugininterface.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/pluginshapefactory.h"

#include <cgraph.h>

using namespace dunnart;


struct CmpEdges
{
    bool operator()(Agedge_t *lhs, Agedge_t *rhs)
    {
        // An edge is uniquely identified by its endpoints and its name key
        // attribute (if there are multiple edges).

        Agnode_t *lhsMinEnd;
        Agnode_t *lhsMaxEnd;

        Agnode_t *rhsMinEnd;
        Agnode_t *rhsMaxEnd;

        if (aghead(lhs) < agtail(lhs))
        {
            lhsMinEnd = aghead(lhs);
            lhsMaxEnd = agtail(lhs);
        }
        else
        {
            lhsMinEnd = agtail(lhs);
            lhsMaxEnd = aghead(lhs);
        }

        if (aghead(rhs) < agtail(rhs))
        {
            rhsMinEnd = aghead(rhs);
            rhsMaxEnd = agtail(rhs);
        }
        else
        {
            rhsMinEnd = agtail(rhs);
            rhsMaxEnd = aghead(rhs);
        }

        if (lhsMinEnd != rhsMinEnd)
        {
            return lhsMinEnd < rhsMinEnd;
        }
        if (lhsMaxEnd != rhsMaxEnd)
        {
            return lhsMaxEnd < rhsMaxEnd;
        }
        char *lhsKey = agnameof(lhs);
        char *rhsKey = agnameof(rhs);
        if (lhsKey && rhsKey)
        {
            return strcmp(lhsKey, rhsKey) < 0;
        }
        return false;
    }
};

//! @brief  Plugin class that adds support for loading Graphviz's 'dot'
//!         file format.
//!
class GraphvizFileIOPlugin : public QObject, public FileIOPluginInterface
{
    Q_OBJECT
        Q_INTERFACES (dunnart::FileIOPluginInterface)
        Q_PLUGIN_METADATA (IID "org.dunnart.GraphvizFileIOPlugin")

    public:
        GraphvizFileIOPlugin()
        {
        }
        QStringList saveableFileExtensions(void) const
        {
            QStringList fileTypes;
            return fileTypes;
        }
        QStringList loadableFileExtensions(void) const
        {
            QStringList fileTypes;
            fileTypes << "gv";
            fileTypes << "dot";
            return fileTypes;
        }
        QString fileExtensionDescription(const QString& extension) const
        {
            if ((extension == "gv") || (extension == "dot"))
            {
                return "Graphviz";
            }
            return QString();
        }
        bool saveDiagramToFile(Canvas *canvas, const QFileInfo& fileInfo,
                QString& errorMessage)
        {
            Q_UNUSED (canvas)
            Q_UNUSED (fileInfo)
            Q_UNUSED (errorMessage)

            return false;
        }
        bool loadDiagramFromFile(Canvas *canvas, const QFileInfo& fileInfo,
                QString& errorMessage)
        {
            QString filename = fileInfo.absoluteFilePath();

            FILE *fp = fopen(filename.toLatin1().data(), "r");
            if (fp == NULL)
            {
                errorMessage = tr("File could not be opened for reading.");
                return false;
            }

            // Determine view DPI.  Assume it is same in both dimensions.
            int dpi = canvas->views().first()->logicalDpiX();

            Agraph_t *g = agread(fp, NULL);

            Agnode_t *v;
            QMap<QString, dunnart::ShapeObj *> nodeShapeMap;
            std::set<Agedge_t *, CmpEdges> processedEdges;
            for (v = agfstnode(g); v; v = agnxtnode(g,v))
            {
                QString shapeType = "org.dunnart.shapes.rect";
                char *shape = agget(v, (char *) "shape");
                if (shape)
                {
                    if (strcmp(shape, "diamond") == 0)
                    {
                        shapeType = "org.dunnart.shapes.diamond";
                    }
                    else if ((strcmp(shape, "ellipse") == 0) ||
                             (strcmp(shape, "oval") == 0))
                    {
                        shapeType = "org.dunnart.shapes.ellipse";
                    }
                }

                // Load this shape from a plugin if the factory supports it.
                dunnart::PluginShapeFactory *factory = sharedPluginShapeFactory();
                dunnart::ShapeObj *newShape = factory->createShape(shapeType);

                // Default values for nodes without these properties
                qreal x = 0;
                qreal y= 0;
                qreal width = 20;
                qreal height = 20;

                char *posStr = agget(v, (char *) "pos");
                if (posStr)
                {
                    bool fixedPosition = false;
                    // Of format "x,y{,z}{!}"
                    // Bang means the node has a fixed position.
                    QString positionStr(posStr);

                    // Remove the bang.
                    if (positionStr.endsWith('!'))
                    {
                        fixedPosition = true;
                        positionStr.remove('!');
                    }

                    QStringList values = positionStr.split(",", QString::SkipEmptyParts);
                    if (values.count() >= 2)
                    {
                        x = values[0].toDouble() * dpi;
                        y = values[1].toDouble() * dpi;
                        // TODO Do something with Z value.
                    }
                }

                // Look for and process width and height properties.
                char *widthStr = agget(v, (char *) "width");
                if (widthStr)
                {
                    width = atof(widthStr) * dpi;
                }
                char *heightStr = agget(v, (char *) "height");
                if (height == 0)
                {
                    height = atof(heightStr) * dpi;
                }

                //qDebug() << QPointF(x, y) << QSizeF(width, height) << QString(agnameof(v));
                newShape->initWithDimensions(QString(), x, y, width, height);

                newShape->setFillColour(Qt::lightGray);

                // Set a label, if necessary.
                char *labelStr = agget(v, (char *) "label");
                if (labelStr)
                {
                    newShape->setLabel(labelStr);
                }

                // Set a stroke colour, if necessary
                char *colourStr = agget(v, (char *) "color");
                if (colourStr)
                {
                    newShape->setStrokeColour(QColor(colourStr));
                }


                newShape->setIdString(agnameof(v));
                nodeShapeMap[QString(agnameof(v))] = newShape;

                canvas->addItem(newShape);
            }

            Agedge_t *e;
            for (v = agfstnode(g); v; v = agnxtnode(g,v))
            {
                for (e = agfstedge(g, v); e; e = agnxtedge(g, e, v))
                {
                    if (processedEdges.find(e) != processedEdges.end())
                    {
                        continue;
                    }
                    processedEdges.insert(e);
                    QString tarName(agnameof(aghead(e)));
                    QString srcName(agnameof(agtail(e)));

                    dunnart::Connector *newConn = new Connector();
                    newConn->initWithConnection(nodeShapeMap[srcName], nodeShapeMap[tarName]);

                    // Determine if the edge is directed.
                    bool directed = true;
                    char *dir = agget(e, (char *) "dir");
                    if (dir)
                    {
                        if (strcmp(dir, "none") == 0)
                        {
                            directed = false;
                        }
                    }
                    newConn->setDirected(directed);

                    char *len = agget(e, (char *) "len");
                    if (len)
                    {
                        newConn->setIdealLength(atof(len) * dpi);
                    }
                    else
                    {
                        // neato default 1.0
                        newConn->setIdealLength(1.0 * dpi);
                    }

                    canvas->addItem(newConn);
                }
            }

            Agraph_t *sg;
            for (sg = agfstsubg(g); sg; sg = agnxtsubg(sg))
            {
                qDebug("Warning: Subgraph not handled");
            }

            agclose(g);
            fclose(fp);

            return true;
        }
};


// Because there is no header file, we need to load the MOC file here to 
// cause Qt to generate it for us.
#include "plugin.moc"

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
