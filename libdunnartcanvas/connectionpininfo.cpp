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

#include <QStringList>

#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/connectionpininfo.h"

namespace dunnart {

ConnectionPinInfo::ConnectionPinInfo(
        const unsigned int classId, const double xPortionOffset,
        const double yPortionOffset, const double insideOffset,
        const Avoid::ConnDirFlags visDirs, const bool exclusive)
    : classId(classId),
      xPortionOffset(xPortionOffset),
      yPortionOffset(yPortionOffset),
      insideOffset(insideOffset),
      visDirs(visDirs),
      exclusive(exclusive),
      pin(NULL)
{
}

ConnectionPinInfo::ConnectionPinInfo(QString strRep)
    : classId(0),
      xPortionOffset(Avoid::ATTACH_POS_CENTRE),
      yPortionOffset(Avoid::ATTACH_POS_CENTRE),
      insideOffset(0.0),
      visDirs(Avoid::ConnDirNone),
      exclusive(false),
      pin(NULL)
{
    QStringList list = strRep.split(" ");
    bool okay;

    classId = list[0].toUInt(&okay);
    if (!okay)
    {
        qWarning("Could not read classId from \"%s\"",
                qPrintable(strRep));
    }

    xPortionOffset = list[1].toDouble(&okay);
    if (!okay)
    {
        qWarning("Could not read xPortionOffset from \"%s\"",
                qPrintable(strRep));
    }

    yPortionOffset = list[2].toDouble(&okay);
    if (!okay)
    {
        qWarning("Could not read yPortionOffset from \"%s\"",
                qPrintable(strRep));
    }

    visDirs = directionFlagsFromString(strRep);
    for (int i = 3; i < list.size(); ++i)
    {
        double valueD = list[i].toDouble(&okay);
        if (okay)
        {
            insideOffset = valueD;
        }
        else if (list[i] == "exclusive")
        {
            exclusive = true;
        }
    }
}


Avoid::ConnDirFlags ConnectionPinInfo::directionFlagsFromString(
        const QString& string)
{
    Avoid::ConnDirFlags flags = Avoid::ConnDirNone;
    QStringList list = string.split(" ");

    for (int i = 0; i < list.size(); ++i)
    {
        if (list[i] == "up")
        {
            flags |= Avoid::ConnDirUp;
        }

        if (list[i] == "down")
        {
            flags |= Avoid::ConnDirDown;
        }

        if (list[i] == "left")
        {
            flags |= Avoid::ConnDirLeft;
        }

        if (list[i] == "right")
        {
            flags |= Avoid::ConnDirRight;
        }
    }

    if (flags == Avoid::ConnDirNone)
    {
        flags = Avoid::ConnDirAll;
    }
    return flags;
}


void ConnectionPinInfo::createPin(Avoid::ShapeRef *avoidRef)
{
    pin = new Avoid::ShapeConnectionPin(avoidRef, classId,
            xPortionOffset, yPortionOffset, insideOffset, visDirs);
    pin->setExclusive(exclusive);
}

QDomElement ConnectionPinInfo::pinAsDomElement(QDomDocument& doc) const
{
    QDomElement node = doc.createElement("pin");

    newProp(node, "classID", classId);
    newProp(node, "nodeXProp", xPortionOffset);
    newProp(node, "nodeYProp", yPortionOffset);
    if (insideOffset != 0)
    {
        newProp(node, "insideOffset", insideOffset);
    }

    if (visDirs != Avoid::ConnDirAll)
    {
        QStringList directions;
        if (visDirs == Avoid::ConnDirUp)
        {
            directions << "up";
        }
        if (visDirs == Avoid::ConnDirDown)
        {
            directions << "down";
        }
        if (visDirs == Avoid::ConnDirLeft)
        {
            directions << "left";
        }
        if (visDirs == Avoid::ConnDirRight)
        {
            directions << "right";
        }
        newProp(node, "pinDirections", directions.join(" "));
    }

    if (exclusive)
    {
        newProp(node, "exclusive", "1");
    }

    return node;
}



QString ConnectionPinInfo::writeToString(void) const
{
    QString strRep("%1 %2 %3");
    strRep = strRep.arg(classId);
    strRep = strRep.arg(xPortionOffset);
    strRep = strRep.arg(yPortionOffset);
    if (insideOffset)
    {
        strRep += QString(" %1").arg(insideOffset);
    }
    if (visDirs != Avoid::ConnDirAll)
    {
        if (visDirs == Avoid::ConnDirUp)
        {
            strRep += " up";
        }
        if (visDirs == Avoid::ConnDirDown)
        {
            strRep += " down";
        }
        if (visDirs == Avoid::ConnDirLeft)
        {
            strRep += " left";
        }
        if (visDirs == Avoid::ConnDirRight)
        {
            strRep += " right";
        }
    }
    if (exclusive)
    {
        strRep += " exclusive";
    }
    return strRep;
}


bool ConnectionPinInfo::operator==(const ConnectionPinInfo& rhs)
{
    return ((classId == rhs.classId) &&
            (xPortionOffset == rhs.xPortionOffset) &&
            (yPortionOffset == rhs.yPortionOffset) &&
            (insideOffset == rhs.insideOffset) &&
            (visDirs == rhs.visDirs) &&
            (exclusive == rhs.exclusive));
}

}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
