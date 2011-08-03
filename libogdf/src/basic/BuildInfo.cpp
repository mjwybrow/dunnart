/*
 * $Revision: 1.7 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-11-28 19:46:00 +1100 (Fri, 28 Nov 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief This file always rebuilds to set the creation timestamp
 * 
 * Well... i guess it doesn't with the new makefile...
 * 
 * \author Joachim Kupke
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2007
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 or 3 as published by the Free Software Foundation
 * and appearing in the files LICENSE_GPL_v2.txt and
 * LICENSE_GPL_v3.txt included in the packaging of this file.
 *
 * \par
 * In addition, as a special exception, you have permission to link
 * this software with the libraries of the COIN-OR Osi project
 * (http://www.coin-or.org/projects/Osi.xml), all libraries required
 * by Osi, and all LP-solver libraries directly supported by the
 * COIN-OR Osi project, and distribute executables, as long as
 * you follow the requirements of the GNU General Public License
 * in regard to all of the software in the executable aside from these
 * third-party libraries.
 * 
 * \par
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * \par
 * You should have received a copy of the GNU General Public 
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 * 
 * \see  http://www.gnu.org/copyleft/gpl.html
 ***************************************************************/


#include <stdio.h>
#include <string.h>

#include "ogdf/basic/basic.h"
#include "ogdf/BuildInfo.h"

namespace ogdf {

////////////////////////////////////////////////////
///
///  Hier kann die Versionsnummer modifiziert werden
///  bitte IMMER die ReleaseVersion sowie major und
///  minor Release anpassen 
///
////////////////////////////////////////////////////

// liefert die aktuelle Versionsnummer   <=== regelm??ig ?berpr?fen!!!
const char *BuildInfo::releaseVersion()
{
	return "1.7";
}

// liefert den aktuellen major release der versionsnummer als INT
const int BuildInfo::majorRelease()
{
	return 1;
}

// liefert den aktuellen minor release als INT
const int BuildInfo::minorRelease()
{
	return 7;
}

/////////////////////////////////////////////////////
///
/// ab hier Update nicht mehr zwingend erforderlich
///
/////////////////////////////////////////////////////

/////////////////////////////////////////////////////
///
/// der Applikationsname
///
/////////////////////////////////////////////////////

// liefert den Namen der Applikation
const char *BuildInfo::applicationName()
{
	return "ogdf";
}


// Konstanten
static const char *month[] = {
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "Jun",
    "Jul",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};

// Datum des build
const char *BuildInfo::buildDate()
{
	return __DATE__;
}

// Zeit des build
const char *BuildInfo::buildTime()
{
	return __TIME__;
}

// Zeitstempel des build, Achtung: funktioniert nicht unter LINUX!
const char *BuildInfo::buildStamp()
{
#ifdef _MSC_VER
	return __TIMESTAMP__;
#else
	return "Not implemented.";
#endif
}

// Liefert eine Zahl zwischen 1 und 9999, die das Builddatum verschlüsselt
// es gilt:    zahl1 < zahl2  <==>  buildDate(zahl1) < buildDate(zahl2)
long BuildInfo::buildDate(const char *date)
{
    int retVal;
    int d, m, y;

    char buf[8];

    sscanf(date, "%s%d%d", buf, &d, &y);

    int i;
    for (i = 0; i < 12; ++i)
        if (!strncmp(buf, month[i], 3))
            break;

    if (i == 12)
        retVal = 0;
    else {
        m = i+1;

        retVal  = (y - 2000) << 9;
        retVal |= (m << 5);
        retVal |= d;
    }


    return retVal;
}

// Liefert wieder das Builddatum aus der verschl?sselten Zahl
const char *BuildInfo::buildDate(long date)
{
	static char buf[16];
	long d, m, y; 
	d = date & 0x1f;
	m = (date >> 5) & 0x0f;
	y = (date >> 9);

	ogdf::sprintf(buf, 16, "%d. %s %d", d, month[m-1], y+2000);
	return buf;
}

// Liefert die Compilerversion
const char *BuildInfo::compilerVersion()
{
	static char buffer[128];
#ifdef __BORLANDC__
	sprintf(buffer, 128, "bcc version %d.", __BORLANDC__);
#elif defined(_MSC_VER)
	sprintf(buffer, 128, "msc version %d.", _MSC_VER);
#else
	sprintf(buffer, 128, "stdc version: %d.", "no info" /*__STDC_VERSION__*/);
#endif
	return buffer;
}

}

