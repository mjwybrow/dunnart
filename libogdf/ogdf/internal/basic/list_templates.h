/*
 * $Revision: 1.3 $
 * 
 * last checkin:
 *   $Author: chimani $ 
 *   $Date: 2008-02-20 20:33:07 +1100 (Wed, 20 Feb 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of algorithms as templates working with
 *        different list types
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

#ifndef OGDF_LIST_TEMPLATES_H
#define OGDF_LIST_TEMPLATES_H


#include <ogdf/basic/Array.h>


namespace ogdf {

// sorts list L using quicksort
template<class LIST>
void quicksortTemplate(LIST &L)
{
	const int n = L.size();
	Array<typename LIST::value_type> A(n);

	int i = 0;
	typename LIST::iterator it;
	for (it = L.begin(); it.valid(); ++it)
		A[i++] = *it;

	A.quicksort();

	for (i = 0, it = L.begin(); i < n; i++)
		*it++ = A[i];
}


// sorts list L using quicksort and compare element comp
template<class LIST, class COMPARER>
void quicksortTemplate(LIST &L, COMPARER &comp)
{
	const int n = L.size();
	Array<typename LIST::value_type> A(n);

	int i = 0;
	typename LIST::iterator it;
	for (it = L.begin(); it.valid(); ++it)
		A[i++] = *it;

	A.quicksort(comp);

	for (i = 0, it = L.begin(); i < n; i++)
		*it++ = A[i];
}


} // end namespace ogdf

#endif
