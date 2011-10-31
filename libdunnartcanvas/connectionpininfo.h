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
 * Author: Michael Wybrow <mjwybrow@users.sourceforge.net>
*/



#ifndef CONNECTIONPININFO_H
#define CONNECTIONPININFO_H

#include <QString>
#include <QDomElement>

#include "libavoid/connectionpin.h"
#include "libavoid/connend.h"

namespace dunnart {

class ConnectionPinInfo
{
public:
    ConnectionPinInfo(const unsigned int classId,
            const double xPortionOffset, const double yPortionOffset,
            const double insideOffset = 0.0,
            const Avoid::ConnDirFlags visDirs = Avoid::ConnDirAll,
            const bool exclusive = false);
    ConnectionPinInfo(QString strRep);
    void createPin(Avoid::ShapeRef *avoidRef);
    QString writeToString(void) const;
    bool operator==(const ConnectionPinInfo& rhs);
    QDomElement pinAsDomElement(QDomDocument& doc) const;

    static Avoid::ConnDirFlags directionFlagsFromString(const QString& string);

    unsigned int classId;
    double xPortionOffset;
    double yPortionOffset;
    double insideOffset;
    Avoid::ConnDirFlags visDirs;
    bool exclusive;
    Avoid::ShapeConnectionPin *pin;
};


}

#endif // CONNECTIONPININFO_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
