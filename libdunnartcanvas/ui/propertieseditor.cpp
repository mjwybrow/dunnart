/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 20108  Monash University
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
 * Author(s): Michael Wybrow
*/

#include "qtpropertybrowser/qttreepropertybrowser.h"
#include "qtpropertybrowser/qtvariantproperty.h"
#include "qtpropertybrowser/qtpropertymanager.h"
#include "qtpropertybrowser/qteditorfactory.h"


#include "propertieseditor.h"
#include "canvas.h"
#include "canvasitem.h"
#include "connector.h"

namespace dunnart {

PropertiesEditorDialog::PropertiesEditorDialog(Canvas *canvas, QWidget *parent)
    : QDockWidget(parent),
      m_canvas(NULL)
{
    this->setAllowedAreas(Qt::DockWidgetAreas(
            Qt::BottomDockWidgetArea | Qt::LeftDockWidgetArea |
            Qt::RightDockWidgetArea));
    this->setWindowTitle(tr("Properties Editor"));
    this->setObjectName("PropertiesEditorDialog");

    m_variant_manager = new QtVariantPropertyManager(this);
    m_read_only_manager = new QtVariantPropertyManager(this);

    connect(m_variant_manager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
                this, SLOT(valueChanged(QtProperty *, const QVariant &)));

    QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory(this);

    m_property_editor = new QtTreePropertyBrowser(this);
    m_property_editor->setFactoryForManager(m_variant_manager, variantFactory);
    m_property_editor->setResizeMode(QtTreePropertyBrowser::Interactive);
    m_property_editor->setSplitterPosition(width() / 3);
    this->setWidget(m_property_editor);

    changeCanvas(canvas);
}

PropertiesEditorDialog::~PropertiesEditorDialog()
{
}

void PropertiesEditorDialog::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED (event)

    m_property_editor->setSplitterPosition(width() / 3);
}

void PropertiesEditorDialog::canvasSelectionChanged()
{
    // Clear the existing properties.
    QMap<QtProperty *, QString>::ConstIterator itProp =
            m_property_to_id.constBegin();
    while (itProp != m_property_to_id.constEnd())
    {
        delete itProp.key();
        itProp++;
    }
    m_property_to_id.clear();
    m_id_to_property.clear();

    if (!m_canvas)
    {
        return;
    }
    QList<CanvasItem *> list = m_canvas->selectedItems();

    QObject *item = m_canvas;
    int firstProperty = Canvas::staticMetaObject.propertyOffset();
    bool canvasItemProperties = false;
    if (list.count() > 0)
    {
        item = list.front();
        canvasItemProperties = true;
        firstProperty = CanvasItem::staticMetaObject.propertyOffset();
    }

    QtVariantProperty *property;

    const QMetaObject* metaObject = item->metaObject();
    QStringList properties;
    for(int i = firstProperty; i < metaObject->propertyCount(); ++i)
    {
        //qDebug("== %s", metaObject->property(i).name());

        const QMetaProperty& prop = metaObject->property(i);

        int type = prop.userType();
        if (type == QVariant::UInt)
        {
            type = QVariant::Int;
        }

        //qDebug("## %s,  %d,  %s", prop.name(), prop.userType(), item->property(prop.name()).typeName());

        if (prop.isEnumType())
        {
            property = m_variant_manager->addProperty(QtVariantPropertyManager::enumTypeId(), QLatin1String(prop.name()));
            QMetaEnum metaEnum = prop.enumerator();
            QMap<int, bool> valueMap; // dont show multiple enum values which have the same values
            QStringList enumNames;
            for (int i = 0; i < metaEnum.keyCount(); i++) {
                int value = metaEnum.value(i);
                if (!valueMap.contains(value)) {
                    valueMap[value] = true;
                    enumNames.append(QLatin1String(metaEnum.key(i)));
                }
            }
            property->setAttribute(QLatin1String("enumNames"), enumNames);
            //qDebug() << prop.name() << "= " << item->property(prop.name()).value<dunnart::Connector::RoutingType>();
            int enumValueIndex = *reinterpret_cast<const int *>
                    (item->property(prop.name()).constData());
            property->setValue(metaEnum.value(enumValueIndex));
        }
        else if (m_variant_manager->isPropertyTypeSupported(type))
        {
            if (!prop.isWritable())
            {
                property = m_read_only_manager->addProperty(type,  QLatin1String(prop.name()) + QLatin1String(" (read-only)"));
            }
            else
            {
                property = m_variant_manager->addProperty(type,  QLatin1String(prop.name()));
            }
            property->setValue(item->property(prop.name()));
        }
        else
        {
            property = m_read_only_manager->addProperty(QVariant::String,  QLatin1String(prop.name()));
            property->setValue(QLatin1String("< Unknown Type >"));
            property->setEnabled(false);
        }
        Q_ASSERT(property);

        //property->setAttribute(QLatin1String("minimum"), 0);
        //property->setAttribute(QLatin1String("maximum"), 100);
        addProperty(property, prop.name());
    }
    QList<QByteArray> propertyList = item->dynamicPropertyNames();
    for (int i = 0; i < propertyList.size(); ++i)
    {
        //qDebug("-- %s", propertyList.at(i).constData());

        const char *propName = propertyList.at(i).constData();
        const QVariant& propVariant = item->property(propName);
        property = m_variant_manager->addProperty(propVariant.userType(), QString(propName));
        //property->setAttribute(QLatin1String("minimum"), 0);
        //property->setAttribute(QLatin1String("maximum"), 100);
        property->setValue(propVariant);
        addProperty(property, propName);
    }
}

int PropertiesEditorDialog::enumToInt(const QMetaEnum &metaEnum, int enumValue) const
{
    QMap<int, int> valueMap; // dont show multiple enum values which have the same values
    int pos = 0;
    for (int i = 0; i < metaEnum.keyCount(); i++) {
        int value = metaEnum.value(i);
        if (!valueMap.contains(value)) {
            if (value == enumValue)
                return pos;
            valueMap[value] = pos++;
        }
    }
    return -1;
}

void PropertiesEditorDialog::valueChanged(QtProperty *property, const QVariant &value)
{
    if (!m_property_to_id.contains(property))
    {
        return;
    }

    if (!m_canvas)
    {
        return;
    }
    QList<CanvasItem *> list = m_canvas->selectedItems();

    m_canvas->beginUndoMacro(tr("Change Property"));
    if (list.empty())
    {
        // Set the property directly on the canvas:
        m_canvas->setProperty(m_property_to_id[property].toLatin1().constData(), value);
    }
    else
    {
        // Otherwise, set it on the first item in the selection:
        CanvasItem *item = list.front();
        item->setProperty(m_property_to_id[property].toLatin1().constData(), value);
    }
}

void PropertiesEditorDialog::addProperty(QtVariantProperty *property, const QString &id)
{
    m_property_to_id[property] = id;
    m_id_to_property[id] = property;
    //QtBrowserItem *item =
    m_property_editor->addProperty(property);
    /*
    if (idToExpanded.contains(id))
        propertyEditor->setExpanded(item, idToExpanded[id]);
    */
}

void PropertiesEditorDialog::changeCanvas(Canvas *canvas)
{
    if (m_canvas)
    {
        disconnect(m_canvas, 0, this, 0);
        disconnect(this, 0, m_canvas, 0);
    }
    m_canvas = canvas;

    connect(m_canvas, SIGNAL(selectionChanged()),
            this, SLOT(canvasSelectionChanged()));

    canvasSelectionChanged();

    //connect(variantManager, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
    //            this, SLOT(valueChanged(QtProperty *, const QVariant &)));


    //connect(this, SIGNAL(setOptChangedConnPenaltySegment(int)),
    //        m_canvas, SLOT(setOptConnPenaltySegment(int)));

}

}
