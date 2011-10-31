/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2011  Monash University
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
//! Plugin that adds support for reading and writing .layout documents.

#include <QtGui>
#include <QObject>
#include <QFileInfo>
#include <QDomDocument>

#include "libdunnartcanvas/fileioplugininterface.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/pluginshapefactory.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/connectionpininfo.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/undo.h"


using namespace dunnart;


//! @brief  Plugin class that adds support for loading and saving .layout
//!         documents.
//!
//! This is the layout description file format used by the Garuda project.
//!
//! This plugin implements the builtin .layout loading/saving functionality.
//! All the actual work of loading and saving the network is implemented in
//! libdunnartcanvas.  This is still implemented as a plugin so that
//! software built on top of Dunnart can easily exclude this functionality
//! if desired.
//!
class BuiltinLayoutFileIOPlugin : public QObject, public FileIOPluginInterface
{
    Q_OBJECT
        Q_INTERFACES (dunnart::FileIOPluginInterface)

    public:
        BuiltinLayoutFileIOPlugin()
        {
        }
        QStringList saveableFileExtensions(void) const
        {
            QStringList fileTypes;
            fileTypes << "layout";
            return fileTypes;
        }
        QStringList loadableFileExtensions(void) const
        {
            QStringList fileTypes;
            fileTypes << "layout";
            return fileTypes;
        }
        QString fileExtensionDescription(const QString& extension) const
        {
            if (extension == "layout")
            {
                return "Layout description";
            }
            return QString();
        }
        bool saveDiagramToFile(Canvas *canvas, const QFileInfo& fileInfo,
                QString& errorMessage);
        bool loadDiagramFromFile(Canvas *canvas, const QFileInfo& fileInfo,
                QString& errorMessage);
        void recursiveReadLayoutXML(Canvas *canvas, const QDomNode& start,
                const QString& dunnartNS, int pass);
        void createFromLayoutXML(Canvas *canvas, const QDomElement& node,
                const QString& dunnartURI, int pass);
};

bool BuiltinLayoutFileIOPlugin::saveDiagramToFile(Canvas *canvas,
        const QFileInfo& fileInfo, QString& errorMessage)
{
    QString outputFilename = fileInfo.absoluteFilePath();
    QDomDocument doc("layout");

    QDomElement root = doc.createElement("layout");
    doc.appendChild(root);

    QList<CanvasItem *> canvas_items = canvas->items();
    int canvas_count = canvas_items.size();
    for (int i = 0; i < canvas_count; ++i)
    {
        // Consider all the canvas items in reverse order, so they get
        // drawn into the saved file with the correct z-order.
        CanvasItem *canvasObj = canvas_items.at(i);

        // The layout format differs only slightly from Dunnart's native
        // annotated SVG ouput, so we rewrite the few attributes necessary.
        QDomElement node = canvasObj->to_QDomElement(XMLSS_ALL, doc);
        if (node.attribute("type") == "connector")
        {
            // Edges are "edge" elements and don't have the type attribute.
            node.setPrefix(QString());
            node.setTagName("edge");
            node.removeAttribute("type");

            // Layout format doesn't have the libavoidPath attribute.
            node.removeAttribute("libavoidPath");
        }
        else
        {
            // Nodes are "node" elements.
            node.setPrefix(QString());
            node.setTagName("node");

            if (node.attribute("type") == "rect")
            {
                // Standard nodes are rectangles, so only retain the type
                // attributes for non-rects.
                node.removeAttribute("type");
            }
        }
        root.appendChild(node);
    }

    // Write DOM document to the file.
    QFile xmlFile(outputFilename);
    if ( ! xmlFile.open(QIODevice::WriteOnly) )
    {
        errorMessage = tr("File could not be opened for writing.");
        return false;
    }
    xmlFile.write(doc.toString().toUtf8());
    xmlFile.close();

    return true;
}


bool BuiltinLayoutFileIOPlugin::loadDiagramFromFile(Canvas *canvas,
        const QFileInfo& fileInfo, QString& errorMessage)
{
    QString filename = fileInfo.absoluteFilePath();
    QDomDocument doc(filename);
    QFile file(filename);
    if ( ! file.open(QIODevice::ReadOnly) )
    {
        errorMessage = tr("File could not be opened for reading.");
        return false;
    }

    QString parsingError;
    int errorLine;
    int errorColumn;
    if (!doc.setContent(&file, true, &parsingError, &errorLine, &errorColumn))
    {
        file.close();
        errorMessage = tr("Error reading XML: %1:%2: %3").arg(errorLine).
                arg(errorColumn).arg(parsingError);
        return false;
    }
    file.close();

    QDomElement root = doc.documentElement();

    // The layout format is intended for using to layout diagrams
    // and networks of a fixed strucutre, so we disable structural
    // editing.
    canvas->setOptStructuralEditingDisabled(true);

    for (int pass = 0; pass < PASS_LAST; ++pass)
    {
        recursiveReadLayoutXML(canvas, root, x_dunnartNs, pass);
    }
    return true;
}


void BuiltinLayoutFileIOPlugin::recursiveReadLayoutXML(Canvas *canvas,
        const QDomNode& start, const QString& dunnartNS, int pass)
{
    for (QDomNode curr = start; !curr.isNull(); curr = curr.nextSibling())
    {
        if (curr.isElement())
        {
            const QDomElement element = curr.toElement();

            if (pass == PASS_SHAPES)
            {
                if ((element.localName() == "options") &&
                    (element.prefix() == x_dunnartNs))
                {
                    canvas->loadLayoutOptionsFromDomElement(element);
                }
            }

            if ( (element.localName() == "node") ||
                 (element.localName() == "edge") )
            {
                createFromLayoutXML(canvas, element, "", pass);
            }
        }
        recursiveReadLayoutXML(canvas, curr.firstChild(), dunnartNS, pass);
    }
}


void BuiltinLayoutFileIOPlugin::createFromLayoutXML(Canvas *canvas,
        const QDomElement& node, const QString& dunnartURI, int pass)
{
    CanvasItem *newObj = NULL;

    QString nodeType = node.localName();

    if (pass == PASS_SHAPES)
    {
        if (nodeType == "node")
        {
            QString type = nodeAttribute(node, dunnartURI, x_type);
            if (type.isEmpty())
            {
                type = "node";
            }
            // Load this shape from a plugin if the factory supports it.
            PluginShapeFactory *factory = sharedPluginShapeFactory();
            ShapeObj *shape = factory->createShape(type);

            if (!shape)
            {
                shape = new RectangleShape();
            }

            if (shape)
            {
                newObj = shape;

                for (QDomNode curr = node.firstChild(); !curr.isNull();
                     curr = curr.nextSibling())
                {
                    if (curr.isElement())
                    {
                        const QDomElement element = curr.toElement();
                        if (element.localName() == "pin")
                        {
                            uint classId;
                            double xProp, yProp;
                            double insideOffset = 0.0;
                            bool isExclusive = false;
                            Avoid::ConnDirFlags visDirs = Avoid::ConnDirAll;

                            classId = essentialProp<uint>(element, "classID");
                            xProp = essentialProp<double>(element, "nodeXProp");
                            yProp = essentialProp<double>(element, "nodeYProp");

                            optionalProp(node, "insideOffset", insideOffset);
                            optionalProp(node, "exclusive", isExclusive);

                            QString directions;
                            if (optionalProp<QString>(element, "pinDirections",
                                    directions))
                            {
                                visDirs = ConnectionPinInfo::
                                        directionFlagsFromString(directions);
                            }
                            shape->addConnectionPin(ConnectionPinInfo(
                                    classId, xProp, yProp, insideOffset,
                                    visDirs, isExclusive));
                        }
                    }
                }

                double x = 0, y = 0, w = 10, h = 10;

                optionalProp(node, "width", w);
                optionalProp(node, "height", h);

                optionalProp(node, "cx", x);
                optionalProp(node, "cy", y);

                shape->setIdString(essentialProp<QString>(node, "id"));
                shape->CanvasItem::setPos(x, y);
                shape->CanvasItem::setSize(w, h);

                QString value = nodeAttribute(node, QString(), "label");
                if (!value.isNull())
                {
                    shape->setLabel(value);
                }

                // Set dynamic properties for any attributes not recognised and handled
                // by Dunnart.
                QDomNamedNodeMap attrs = node.attributes();
                for (uint i = 0; i < attrs.length(); ++i)
                {
                    QDomNode prop = attrs.item(i);
                    QString name = prop.localName();
                    if ( ! prop.prefix().isEmpty() && prop.prefix() != x_dunnartNs)
                    {
                        QString propName = prop.prefix() + ":" + prop.localName();
                        shape->setProperty(propName.toLatin1().constData(), prop.nodeValue());
                    }
                }
            }
        }
    }
    else if (pass == PASS_CONNECTORS)
    {
        if (nodeType == "edge")
        {
            Connector *connector = new Connector();
            connector->initWithXMLProperties(canvas, node, dunnartURI);
            newObj = connector;
        }
    }

    if (newObj)
    {
        QUndoCommand *cmd = new CmdCanvasSceneAddItem(canvas, newObj);
        canvas->currentUndoMacro()->addCommand(cmd);
    }
}


Q_EXPORT_PLUGIN2(fileio_builtinlayout, BuiltinLayoutFileIOPlugin)

// Because there is no header file, we need to load the MOC file here to 
// cause Qt to generate it for us.
#include "plugin.moc"

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
