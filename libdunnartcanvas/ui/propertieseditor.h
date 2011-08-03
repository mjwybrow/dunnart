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

#ifndef PROPERTIESEDITOR_H
#define PROPERTIESEDITOR_H

#include <QMap>
#include <QDockWidget>

class QtTreePropertyBrowser;
class QtVariantPropertyManager;
class QtEnumPropertyManager;
class QtVariantProperty;
class QtProperty;

namespace dunnart {

class Canvas;

class PropertiesEditorDialog : public QDockWidget
{
    Q_OBJECT

    public:
        PropertiesEditorDialog(Canvas *canvas, QWidget *parent = NULL);
        ~PropertiesEditorDialog();

    signals:
    private slots:
        void changeCanvas(Canvas *canvas);
        void canvasSelectionChanged();
        void valueChanged(QtProperty *property, const QVariant &value);
    protected:
        virtual void resizeEvent(QResizeEvent *event);
    private:
        void addProperty(QtVariantProperty *property, const QString &id);
        int enumToInt(const QMetaEnum &metaEnum, int enumValue) const;

        Canvas *m_canvas;
        QtTreePropertyBrowser *m_property_editor;
        QtVariantPropertyManager *m_variant_manager;
        QtVariantPropertyManager *m_read_only_manager;
        QtEnumPropertyManager *m_enum_manager;
        QMap<QtProperty *, QString> m_property_to_id;
        QMap<QString, QtVariantProperty *> m_id_to_property;
};

}

#endif // PROPERTIESEDITOR_H
