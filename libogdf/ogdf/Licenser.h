/*
 * $Revision: 1.3 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-03-09 05:35:47 +1100 (Sun, 09 Mar 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class Licenser which contains the license
 *        mechanism used in OREAS applications
 * 
 * \author Carsten Gutwenger
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


#ifdef _MSC_VER
#pragma once
#endif


#ifndef OGDF_LICENSER_H
#define OGDF_LICENSER_H


#include <ogdf/basic/basic.h>
#include <time.h>

#include <sstream> // MCh: compatibility (was: strstream)
using std::istringstream;


namespace ogdf {


/**
 * \deprecated The class License is no longer needed in the OGDF and will
 * be removed in future releases. Possibly, some functionality of the class
 * will be provided by new functions or classes.
 */
class OGDF_EXPORT Licenser {
public:

	// return value of license validation
	enum ErrorCode {
		valid                 = 0, // license is valid (no error)
        noLicense             = 1, // no license is given
		getHostnameFailed     = 2, // getting host name of current machine failed
		getSerialNumberFailed = 3, // getting serial number of current machine
		                           // failed
		wrongSoftwareName     = 4, // software name in license string is wrong
		wrongVersionNumber    = 5, // version number in license string is wrong
		dateFormatError       = 6, // error in date format in license string
		expired               = 7, // license has already expired
		wrongHostName         = 8, // host name does not match current machine
		wrongHostId           = 9, // host id (or serial number of drive c:) does
		                           // not match current machine
		wrongDigest           = 10,// digest (check sum) of license string is wrong
        illegalKey            = 11 // key belongs to the Red List of illegal keys
	};

    Licenser() : m_daysLeft(0) {}


	// creates a license string
	void create(istream &info,  // info-file
		int duration,           // duration of license (0 = unlimited)
        const char *licenseType,// license type
		int licenseNumber,      // license number
        const char *companyName,// name of company
		ostream &license,       // output for created license string
        const char *installPath = "License", // path, where to install the license-string
        bool oldStyle = false);


	// validates a license
	ErrorCode checkLicense(
		const char *theSoftwareName,  // software name of product
		const char *theVersionNumber, // version of product
		istream &license);            // license string

    ErrorCode checkLicense(
        const char *theSoftwareName,
        const char *theVersionNumber,
        const char *licString) {
            char *ls = const_cast<char*>(licString);
            istringstream str(ls);
            return checkLicense(theSoftwareName, theVersionNumber, str);
    }

    const char *getCompany(const char *lic);
    const char *getProduct(const char *lic);
    const char *getExpirationDate(const char *lic);

	void makeExpirationDate(int duration, char *expirationDate);


	// md5-coding algorithm
	// computes in digest the md5-digest of string in input. len is the length
	// of input, input may contain 0's.
	static void md5(const char *input, int len, char *digest);

	static const char *monthName[];

    const long daysLeft() { return m_daysLeft; }

    bool validDate(int d, int m, int y);

private:
	void makeDigest(const char *softwareName,
		char *versionNumber,
        const char *licenseType,
		int licenseNumber,
        const char *companyName,
		const char *expirationDate,
		const char *hostName,
		const char *hostId,
		char *digest);

	time_t getExpirationTime(char*expDate);

    long m_daysLeft;
};


} // end namespace ogdf


#endif
