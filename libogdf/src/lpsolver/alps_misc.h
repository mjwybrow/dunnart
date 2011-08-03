/*
 * $Revision: 1.4 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-01-17 10:47:44 +0100 (Thu, 17 Jan 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Additional declarations for the LP solver "alps"
 * 
 * \author Michael Juenger, Gerhard Reinelt
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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

#ifndef OGDF_ALPS_MISC_H
#define OGDF_ALPS_MISC_H

#include <time.h>

namespace ogdf {

inline void ffree(char **a)
{
	if (*a) {
		free( (char *) *a);
		*a = 0;
	}
}

inline int cputime()
{
	return (int) clock();
}

/** [0,1] random generator */
/*
*  The follwoing is a standard random number generator suitable for
* simple applications.
*/
inline double rand01(int *randseed)
{
	int iy;
	double yfl;

	iy = *randseed * 65539;
	if (iy<0) iy = iy + 2147483647 + 1;
	yfl = (double) iy;
	*randseed = iy;

	return( (double) yfl * 0.46566128e-9 );
}/*rand01*/

void quick_sort2(double *A,  int *index,  int l,  int r);


#if defined(unix) || defined(__unix__) || defined(__unix) || defined(_AIX)

inline int strcmpi(const char *s1, const char *s2)
{
	return strcasecmp(s1, s2);
}

#endif

} // end namespace ogdf

#endif
