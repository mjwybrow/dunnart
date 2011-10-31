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


#include "libdunnartcanvas/pluginshapefactory.h"
#include "libdunnartcanvas/shapeplugininterface.h"
#include "libdunnartcanvas/shape.h"

namespace dunnart {

// Shared singleton:
static PluginShapeFactory *shared_plugin_shape_factory = NULL;

PluginShapeFactory *sharedPluginShapeFactory(void)
{
    if (shared_plugin_shape_factory == NULL)
    {
        shared_plugin_shape_factory = new PluginShapeFactory();
    }
    return shared_plugin_shape_factory;
}

PluginShapeFactory::PluginShapeFactory()
{
}

QStringList PluginShapeFactory::shapeClassLabels(void) const
{
    return m_shape_class_labels;
}

QStringList PluginShapeFactory::shapeTypesForClassLabel(
    const QString& classLabel) const
{
    int index =  m_shape_class_labels.indexOf(classLabel);
    QStringList shapeTypes;
    if (index == -1)
    {
        return shapeTypes;
    }

    foreach (QString shapeType, m_shape_builders.at(index).keys())
    {
        shapeTypes << shapeType;
    }
    return shapeTypes;
}

void PluginShapeFactory::registerShapePlugin(ShapePluginInterface *builder)
{
    m_shape_class_labels.push_back(builder->shapesClassLabel());
    m_shape_builders.push_back(ShapeBuilders());
    ShapeBuilders& shapeBuilders = m_shape_builders.last();
    foreach (QString shapeType, builder->producableShapeTypes())
    {
        shapeBuilders[shapeType] = builder;
    }
}

ShapeObj *PluginShapeFactory::createShape(const QString& shapeType)
{
    ShapeObj *newShape = NULL;
    // We allow the user to use unqualified names from the default namespace,
    // so we also check the qualified version.
    QString qualifiedShapeType = "org.dunnart.shapes." + shapeType;

    foreach (ShapeBuilders shapeBuilders, m_shape_builders)
    {
        if (shapeBuilders.contains(shapeType))
        {
            newShape = shapeBuilders[shapeType]->generateShapeOfType(shapeType);
        }
        else if (shapeBuilders.contains(qualifiedShapeType))
        {
            newShape = shapeBuilders[qualifiedShapeType]->generateShapeOfType(
                    qualifiedShapeType);
        }

        if (newShape)
        {
            // Set the type and return the shape.
            newShape->setItemType(shapeType);
            return newShape;
        }
    }
    return NULL;
}

}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
