/*
 * $Revision: 1.5 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-01-17 22:56:41 +0100 (Thu, 17 Jan 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of BuildInfo class.
 * 
 * The BuildInfo class provides information about the build of
 * the library.
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


#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_BUILD_INFO_H
#define OGDF_BUILD_INFO_H


namespace ogdf {


//!Provides information about the build of the library.
class OGDF_EXPORT BuildInfo {

public:
	//! Returns the version number of the library.
	static const char *releaseVersion();
	//! Returns the major digit of the release number.
	static const int   majorRelease();
	//! Returns the minor digit of the release number.
	static const int   minorRelease();
	//! Returns the name of the library.
	static const char *applicationName();

	//! Returns the date of the build.
	static const char *buildDate();
	//! Returns the time of the build.
	static const char *buildTime();
	/**
	 * \deprecated This function will not be supported in future releases.
	 * It is not reuqired for OGDF.
	 */
	static const char *buildStamp();
	
	//! Returns the build date encoded as long int.
	/**
	 * \bug The date encoding reserves 5 bits for the year, where 0 refers to
	 * the year 2000. In particular, year 2032 will clash with the month.
	 * Furthermore, why do we need a long if the encode number is at most 9999
	 * (according to documentation)?
	 */
	static long  buildDate(const char *date);

	//! Converts the date encoding to a string.
	static const char *buildDate(long date);

	//! Returns the compiler and version used to build the library.
	/**
	 * \todo Provide more information about compiler; adjust to further systems.
	 */
	static const char *compilerVersion();


};

}

#endif
