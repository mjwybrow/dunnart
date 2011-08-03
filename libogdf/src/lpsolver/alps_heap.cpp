/*
 * $Revision: 1.2 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-11-27 03:02:28 +1100 (Thu, 27 Nov 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief The heap functions.
 * 
 * We implement a priority queue as a heap with the operations 
 * |decreasekey|, |increasekey|, |removekey| and |minkey|. The
 * initial heap is made by the |makeheap| function. We encode rows
 * canonically by |0| to |dim-1|, and columns from |dim| to
 * |2*dim-1|. The heap data structure consists of \item{--} 
 * |int*heapcard| (cardinality of the heap) \item{--} |int*p| 
 * (array holding the rows and columns in positions |1| to 
 * |2*dim|) \item{--} |int*pindex| (index into |p| for a row or 
 * column).
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


#include <stdlib.h>
#include <stdio.h>

namespace ogdf {

/** The decreasekey function */
/*
*  We assume that the key of element has been decreased, and we perform
* the necessary swaps towards the root.
*/
void decreasekey ( int element,
				  int dim,
				  int * /* heapcard */,
				  int *p,
				  int *pindex,
				  int *rownonzeroes,
				  int *colnonzeroes)
{
	int    pos,compos,ppos,pcompos;
	int    wppos,wpcompos,finished;

	pos = pindex[element];
	// Go towards the root

	finished = 0;
	while ((!finished)&&(pos>1)) {
		compos = pos/2;
		ppos = p[pos];
		if (ppos<dim) wppos = rownonzeroes[ppos];
		else          wppos = colnonzeroes[ppos-dim];
		pcompos = p[compos];
		if (pcompos<dim) wpcompos = rownonzeroes[pcompos];
		else             wpcompos = colnonzeroes[pcompos-dim];
		if (wpcompos>wppos) {
			p[pos] = pcompos;
			pindex[pcompos] = pos;
			p[compos] = ppos;
			pindex[ppos] = compos;
		}
		else finished = 1;
		pos = compos;
	}


}


/** The increasekey function */
/*
*  We assume that the key of element has been increased, and we perform
* the necessary swaps towards the leaves.
*/
void increasekey (
				  int element,
				  int dim,
				  int *heapcard,
				  int *p,
				  int *pindex,
				  int *rownonzeroes,
				  int *colnonzeroes
				  )
{
	int    pos,compos,compos2,ppos,pcompos,pcompos2;
	int    wppos,wpcompos,wpcompos2,finished;

	pos = pindex[element];
	// Go towards the leaves

	finished = 0;
	while ((!finished)&&(2*pos<=(*heapcard))) {
		ppos = p[pos];
		if (ppos<dim) wppos = rownonzeroes[ppos];
		else          wppos = colnonzeroes[ppos-dim];
		compos = 2*pos;
		pcompos = p[compos];
		if (pcompos<dim) wpcompos = rownonzeroes[pcompos];
		else             wpcompos = colnonzeroes[pcompos-dim];
		if (compos<(*heapcard)) {
			compos2 = compos+1; 
			pcompos2 = p[compos2];
			if (pcompos2<dim) wpcompos2 = rownonzeroes[pcompos2];
			else              wpcompos2 = colnonzeroes[pcompos2-dim];
			if (wpcompos2<wpcompos) {
				compos = compos2;
				pcompos = pcompos2;
				wpcompos = wpcompos2;
			}
		}
		if (wpcompos<wppos) {
			p[pos] = pcompos;
			pindex[pcompos] = pos;
			p[compos] = ppos;
			pindex[ppos] = compos;
		}
		else finished = 1;
		pos = compos;
	}
}


/** The removekey function */
/*
*  We assume that the key of element has been set to zero, because the
* corresponding row or column has been removed from the working matrix.
* Therefore, we can first do the same as in the |decreasekey| function,
* and we are sure that the element will move to the root (it should be
* the only element with key |0|). Then we copy the last heap element to
* the root position and behave exactly as in the |increasekey| function.
*/
void removekey (int element,
				int dim,
				int *heapcard,
				int *p,
				int *pindex,
				int *rownonzeroes,
				int *colnonzeroes)
{
	int    pos,compos,compos2,ppos,pcompos,pcompos2;
	int    wppos,wpcompos,wpcompos2,finished;

	pos = pindex[element];

	// Go towards the root
	finished = 0;
	while ((!finished)&&(pos>1)) {
		compos = pos/2;
		ppos = p[pos];
		if (ppos<dim) wppos = rownonzeroes[ppos];
		else          wppos = colnonzeroes[ppos-dim];
		pcompos = p[compos];
		if (pcompos<dim) wpcompos = rownonzeroes[pcompos];
		else             wpcompos = colnonzeroes[pcompos-dim];
		if (wpcompos>wppos) {
			p[pos] = pcompos;
			pindex[pcompos] = pos;
			p[compos] = ppos;
			pindex[ppos] = compos;
		}
		else finished = 1;
		pos = compos;
	}


	if (pos!=1) {
		printf("pos=/=1 in removekey !!!\n");
		exit(7);
	}
	pcompos = p[*heapcard];
	p[pos] = pcompos;
	pindex[pcompos] = pos;
	(*heapcard)--;
	// Go towards the leaves

	finished = 0;
	while ((!finished)&&(2*pos<=(*heapcard))) {
		ppos = p[pos];
		if (ppos<dim) wppos = rownonzeroes[ppos];
		else          wppos = colnonzeroes[ppos-dim];
		compos = 2*pos;
		pcompos = p[compos];
		if (pcompos<dim) wpcompos = rownonzeroes[pcompos];
		else             wpcompos = colnonzeroes[pcompos-dim];
		if (compos<(*heapcard)) {
			compos2 = compos+1; 
			pcompos2 = p[compos2];
			if (pcompos2<dim) wpcompos2 = rownonzeroes[pcompos2];
			else              wpcompos2 = colnonzeroes[pcompos2-dim];
			if (wpcompos2<wpcompos) {
				compos = compos2;
				pcompos = pcompos2;
				wpcompos = wpcompos2;
			}
		}
		if (wpcompos<wppos) {
			p[pos] = pcompos;
			pindex[pcompos] = pos;
			p[compos] = ppos;
			pindex[ppos] = compos;
		}
		else finished = 1;
		pos = compos;
	}
}


/** The minkey function */
/*
*  The |minkey| function just acesses the root elemnt of the heap.
*/
int minkey (int /* element */,
			int /* dim */,
			int * /* heapcard */,
			int *p,
			int * /* pindex */,
			int * /* rownonzeroes */,
			int * /* colnonzeroes */)
{
	return(p[1]);
}


/** The insertkey function */
/*
*  The |insertkey| function must only be called if the heap size does
* not increase above the initial size, otherwise a severe error will
* occur (which is not checked in this version)
*/
void insertkey (int element,
				int dim,
				int *heapcard,
				int *p,
				int *pindex,
				int *rownonzeroes,
				int *colnonzeroes)
{
	int    pos,compos,ppos,pcompos;
	int    wppos,wpcompos,finished;

	(*heapcard)++;
	pindex[element] = *heapcard;
	p[*heapcard] = element;
	pos = *heapcard;

	// Go towards the root
	finished = 0;
	while ((!finished)&&(pos>1)) {
		compos = pos/2;
		ppos = p[pos];
		if (ppos<dim) wppos = rownonzeroes[ppos];
		else          wppos = colnonzeroes[ppos-dim];
		pcompos = p[compos];
		if (pcompos<dim) wpcompos = rownonzeroes[pcompos];
		else             wpcompos = colnonzeroes[pcompos-dim];
		if (wpcompos>wppos) {
			p[pos] = pcompos;
			pindex[pcompos] = pos;
			p[compos] = ppos;
			pindex[ppos] = compos;
		}
		else finished = 1;
		pos = compos;
	}
}


/** The makeheap function */
/*
*  In order to make the initial heap, we assign all rows and columns to
* the heap position in canonical order, first the rows, then the
* columns. Then, going backwards, we make a heap out of this by swapping
* towards the leaves, just like in the |increasekey| function.
*/
void makeheap (int /* element */,
			   int dim,
			   int *heapcard,
			   int *p,
			   int *pindex,
			   int *rownonzeroes,
			   int *colnonzeroes )
{
	int    pos,compos,compos2,ppos,pcompos,pcompos2,startpos;
	int    wppos,wpcompos,wpcompos2,finished;

	for (pos=1; pos<=2*dim; pos++) {
		p[pos] = pos - 1;
		pindex[pos-1] = pos;
	}
	(*heapcard) = 2*dim;
	for (startpos=(*heapcard)/2; startpos>=1; startpos--) {
		pos = startpos;

		// Go towards the leaves
		finished = 0;
		while ((!finished)&&(2*pos<=(*heapcard))) {
			ppos = p[pos];
			if (ppos<dim) wppos = rownonzeroes[ppos];
			else          wppos = colnonzeroes[ppos-dim];
			compos = 2*pos;
			pcompos = p[compos];
			if (pcompos<dim) wpcompos = rownonzeroes[pcompos];
			else             wpcompos = colnonzeroes[pcompos-dim];
			if (compos<(*heapcard)) {
				compos2 = compos+1; 
				pcompos2 = p[compos2];
				if (pcompos2<dim) wpcompos2 = rownonzeroes[pcompos2];
				else              wpcompos2 = colnonzeroes[pcompos2-dim];
				if (wpcompos2<wpcompos) {
					compos = compos2;
					pcompos = pcompos2;
					wpcompos = wpcompos2;
				}
			}
			if (wpcompos<wppos) {
				p[pos] = pcompos;
				pindex[pcompos] = pos;
				p[compos] = ppos;
				pindex[ppos] = compos;
			}
			else finished = 1;
			pos = compos;
		}
	}
}

} // end namespace ogdf
