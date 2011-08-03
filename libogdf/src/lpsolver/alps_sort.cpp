/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief The sort function. This is a general purpuse quicksort.
 * 
 * \author Michael Juenger, Gerhard Reinelt
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

namespace ogdf {

/** The sort function */
/* 
*  Here is the sort function.
*/
void quick_sort(double *A, int *index, int l, int r)
{
	double pivot;
	double dt;
	int it;

	if (r>l) {
		pivot = A[r]; /* Pivotelement */
		do {
			dt = A[l];
			A[l] = A[r];
			A[r] = dt;
			it = index[l];
			index[l] = index[r];
			index[r] = it;
			while (A[l] < pivot) l++;
			while (A[r] >= pivot) r--;
		} while (l <= r);
		quick_sort(A,index,l,l-1);
		quick_sort(A,index,l,r);
	}
} /* quick_sort */


/** The second sort function */
/*
*  Here is the second sort function.
*/
void
quick_sort2(
			double *A,
			int *index,
			int l,
			int r
			)
			/*=== Sortiere A[l]...A[r] aufsteigend ===*/
{
	int i,j,t;
	double v;
	double td;

	if (r<=l) return;

	if (r==l+1) {
		if (A[l]>A[r]) {
			td = A[l];
			A[l] = A[r];
			A[r] = td;
			t = index[l];
			index[l] = index[r];
			index[r] = t;
		}
	}

	i = l-1;
	j = r;
	v = A[r];

	do {
		do i++; while(A[i]<=v && i<r);

		do j--; while (A[j]>v && j>=l);
		if (j==l && A[j]>v) j = l-1;

		if (j>i) {
			td = A[i];
			A[i] = A[j];
			A[j] = td;
			t = index[i];
			index[i] = index[j];
			index[j] = t;
		}
		else {
			td = A[i];
			A[i] = A[r];
			A[r] = td;
			t = index[i];
			index[i] = index[r];
			index[r] = t;
		}

	} while (j>i);

	quick_sort2(A,index,l,i-1);
	quick_sort2(A,index,i+1,r);

}/*quick_sort2*/

} // end namespace ogdf
