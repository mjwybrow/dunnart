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
//! Plugin that adds support for reading and writing Dunnart's native
//! annotated SVG file format.

#include <QtWidgets>
#include <QObject>
#include <QFileInfo>
#include <QDomDocument>
#include <QSvgGenerator>

#include "libdunnartcanvas/fileioplugininterface.h"
#include "libdunnartcanvas/canvas.h"
#include "libdunnartcanvas/canvasitem.h"

using namespace dunnart;


//! @brief  Plugin class that adds support for loading and saving Dunnart's
//!         native annotated SVG file format.
//!
//! This format contains an SVG representation along with extra markup
//! describing the diagram structure in a readable way for Dunnart.
//! This is Dunnart's default file format.
//!
//! Plain SVG can be created from Dunnart by using the "export" option.
//!
//! This plugin implements the builtin SVG loading/saving functionality.
//! All the actual work of loading and saving the network is implemented in
//! libdunnartcanvas.  This is still implemented as a plugin so that
//! software built on top of Dunnart can easily exclude this functionality
//! if desired.
//!
class BuiltinSVGFileIOPlugin : public QObject, public FileIOPluginInterface
{
    Q_OBJECT
        Q_INTERFACES (dunnart::FileIOPluginInterface)
        Q_PLUGIN_METADATA (IID "org.dunnart.BuiltinSVGFileIOPlugin")

    public:
        BuiltinSVGFileIOPlugin()
        {
        }
        QStringList saveableFileExtensions(void) const
        {
            QStringList fileTypes;
            fileTypes << "svg";
            return fileTypes;
        }
        QStringList loadableFileExtensions(void) const
        {
            QStringList fileTypes;
            fileTypes << "svg";
            return fileTypes;
        }
        QString fileExtensionDescription(const QString& extension) const
        {
            if (extension == "svg")
            {
                return "Dunnart Annotated SVG";
            }
            return QString();
        }
        bool saveDiagramToFile(Canvas *canvas, const QFileInfo& fileInfo,
                QString& errorMessage)
        {
            QString outputFilename = fileInfo.absoluteFilePath();
            QRectF viewBox = canvas->pageRect();
            if (viewBox.size().isEmpty())
            {
                // There is no page set, so set a viewbox equal to diagram bounds
                double padding = 10;
                viewBox = expandRect(diagramBoundingRect(canvas->items()), padding);
            }

            QBuffer buffer;
            buffer.open(QBuffer::WriteOnly);

            QSvgGenerator generator;
            generator.setOutputDevice(&buffer);

            QSize size = viewBox.size().toSize();
            generator.setSize(size);
            generator.setViewBox(viewBox);
            generator.setTitle(QFileInfo(outputFilename).fileName());
            generator.setDescription(
                    tr("This SVG file was saved from Dunnart.  "
                       "http://www.dunnart.org/"));

            QPainter painter;
            if (painter.begin(&generator))
            {
                // Don't paint any objects into the generator, be just want to capture
                // the header and footer SVG tags, that we can later wrap around the
                // individual drawing tags for all canvas items.
                painter.end();
            }
            buffer.close();

            // Remove SVG tags up to beginning of first group tag, which will be
            // the beginning of the definition of the
            QString svgStr(buffer.data());
            int contentStart = svgStr.indexOf("<g ");
            svgStr = svgStr.remove(contentStart, svgStr.length() - contentStart);

            // Add namespaces.
            int namespaceInsertPos = svgStr.indexOf(" xmlns");
            // Add Dunnart namespace.
            svgStr.insert(namespaceInsertPos,
                    QString(" xmlns:dunnart=\"%1\"\n").arg(x_dunnartURI));
            // Now, add any namespaces used in the input document that we don't use
            // (since we will be copying properties in such namespaces straight
            // through).
            QMap<QString, QString>::const_iterator i =
                    canvas->m_extra_namespaces_map.constBegin();
            while (i != canvas->m_extra_namespaces_map.constEnd())
            {
                const QString& nsName = i.key();
                const QString& nsURI  = i.value();

                svgStr.insert(namespaceInsertPos,
                        QString(" xmlns:%1=\"%2\"\n").arg(nsName).arg(nsURI));
                ++i;
            }

            QFile svgFile(outputFilename);
            if ( ! svgFile.open(QIODevice::WriteOnly) )
            {
                errorMessage = tr("File could not be opened for writing.");
                return false;
            }
            svgFile.write(svgStr.toUtf8());

            canvas->setRenderingForPrinting(true);
            QList<CanvasItem *> canvas_items = canvas->items();
            int canvas_count = canvas_items.size();
            for (int i = 0; i < canvas_count; ++i)
            {
                // Consider all the canvas items in reverse order, so they get drawn
                // into the SVG file with the correct z-order.
                CanvasItem *cobj = canvas_items.at(canvas_count - 1 - i);
                QString svg = cobj->svgCodeAsString(size, viewBox);

                // Less than three lines represents a open and close group tag
                // (setting style), with no content between, so only output it if
                // there are more than three lines.
                int lines = svg.count('\n');
                if (lines > 3)
                {
                    svgFile.write(svg.toUtf8());
                }
            }
            canvas->setRenderingForPrinting(false);

            svgFile.write("<!-- Dunnart description -->\n");
            QDomDocument doc("svg");

            QDomElement optionsNode = canvas->writeLayoutOptionsToDomElement(doc);
            QString optionsNodeString = nodeToString(optionsNode);
            svgFile.write(optionsNodeString.toUtf8());

            for (int i = 0; i < canvas_count; ++i)
            {
                // Consider all the canvas items in reverse order, so they get drawn
                // into the SVG file with the correct z-order.
                CanvasItem *canvasObj = canvas_items.at(i);

                QDomNode node = canvasObj->to_QDomElement(XMLSS_ALL, doc);
                QString svgNodeString = nodeToString(node);
                svgFile.write(svgNodeString.toUtf8());
            }

            // Copy XML for any external namespaces we have saved
            if ( ! canvas->m_external_node_list.empty() )
            {
                svgFile.write("<!-- External namespace nodes -->\n");
                QDomNode externalNode;
                foreach (externalNode, canvas->m_external_node_list)
                {
                    QString externalNodeString = nodeToString(externalNode);
                    svgFile.write(externalNodeString.toUtf8());
                }
            }

            svgFile.write(QByteArray("</svg>\n"));
            svgFile.close();
            return true;
        }
        bool loadDiagramFromFile(Canvas *canvas, const QFileInfo& fileInfo,
                QString& errorMessage)
        {
            QString filename = fileInfo.absoluteFilePath();
            QDomDocument doc(filename);
            QFile file(filename);
            if (!file.open(QIODevice::ReadOnly))
            {
                errorMessage = tr("File could not be opened for reading.");
                return false;
            }

            QString parsingError;
            int errorLine;
            int errorColumn;
#if 1
            // We can open files from Dunnart Ver 1 by rewriting some
            // properties.  This is still required for opening older
            // Pro-origami files
            QTextStream stream(&file);
            QString content = stream.readAll();
            if (content.contains("sodipodi:guide dunnart:type=\"indGuide\""))
            {
                // This has v1 guideline descriptions that need to be rewritten.
                content.replace(" direction=", " foo=");
                content.replace(" position=", " bar=");
                content.replace("sodipodi:guide", "dunnart:node");
            }
            content.replace("node dunnart:type=", "node type=");
            content.replace("dunnart:direction=", "direction=");
            content.replace("dunnart:position=", "position=");
            content.replace("dunnart:sepDistance=", "sepDistance=");
            content.replace("avoidBuffer=\"10\"", "avoidBuffer=\"6\"");
            content.replace("dunnart:xPos=", "dunnart:cx=");
            content.replace("dunnart:yPos=", "dunnart:cy=");
            if (!doc.setContent(content, true, &parsingError, &errorLine, &errorColumn))
#else
            // Once we don't care, this is simpler.
            if (!doc.setContent(&file, true, &parsingError, &errorLine, &errorColumn))
#endif
            {
                file.close();
                errorMessage = tr("Error reading SVG: %1:%2: %3").arg(errorLine).
                        arg(errorColumn).arg(parsingError);
                return false;
            }
            file.close();
            canvas->setSvgRendererForFile(filename);

            QDomElement root = doc.documentElement();

            for (int pass = 0; pass < PASS_LAST; ++pass)
            {
                canvas->recursiveReadSVG(root, x_dunnartNs, pass);
            }

            return true;
        }
        static QString nodeToString(const QDomNode& node)
        {
            QString nodeString;
            QTextStream nodeTextStream(&nodeString);
            node.save(nodeTextStream, 4);

            return nodeString;
        }
};

Q_PLUGIN_METADATA (IID "org.dunnart.BuiltinSVGFileIOPlugin")

// Because there is no header file, we need to load the MOC file here to 
// cause Qt to generate it for us.
#include "plugin.moc"

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
