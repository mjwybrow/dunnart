/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief The recoheap functions.
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
//
// The recoheap functions. For the reduced cost, we implement a priority
// queue as a heap with the operations |increaserecoheap|,
// |decreaserecoheap|, |removemaxrecoheap|, |removerecoheap|,
// |insertrecoheap| and |makerecoheap|.
//

/*
*  The definitions of the reduced cost heap functions are written into
* the file {\tt alps_recoheap.c}.
*/


#include <stdio.h>
#include "alps.h"

namespace ogdf {

/** The increaserecoheap function */
/*
*  We assume that the key of element has been increased, and we perform
* the necessary swaps towards the root.
*/
void increaserecoheap (
					   lpstruct *lp,
					   int element
					   )
{
	int    pos,compos,ppos,pcompos,finished;
	double wppos,wpcompos;

	pos = lp->recoheappindex[element];
	// Go towards the root

	finished = 0;
	while ((!finished)&&(pos>1)) {
		compos = pos/2;
		ppos = lp->recoheapp[pos];
		wppos = fabs(lp->reducedcost[ppos]);
		pcompos = lp->recoheapp[compos];
		wpcompos = fabs(lp->reducedcost[pcompos]);
		if (wpcompos<wppos) {
			lp->recoheapp[pos] = pcompos;
			lp->recoheappindex[pcompos] = pos;
			lp->recoheapp[compos] = ppos;
			lp->recoheappindex[ppos] = compos;
		}
		else finished = 1;
		pos = compos;
	}


}


/** The decreaserecoheap function */
/*
*  We assume that the key of element has been decreased, and we perform
* the necessary swaps towards the leaves.
*/
void decreaserecoheap (
					   lpstruct *lp,
					   int element
					   )
{
	int    pos,compos,compos2,ppos,pcompos,pcompos2,finished;
	double wppos,wpcompos,wpcompos2;

	pos = lp->recoheappindex[element];
	// Go towards the leaves

	finished = 0;
	while ((!finished)&&(2*pos<=lp->recoheapcard)) {
		ppos = lp->recoheapp[pos];
		wppos = fabs(lp->reducedcost[ppos]);
		compos = 2*pos;
		pcompos = lp->recoheapp[compos];
		wpcompos = fabs(lp->reducedcost[pcompos]);
		if (compos<lp->recoheapcard) {
			compos2 = compos+1; 
			pcompos2 = lp->recoheapp[compos2];
			wpcompos2 = fabs(lp->reducedcost[pcompos2]);
			if (wpcompos2>wpcompos) {
				compos = compos2;
				pcompos = pcompos2;
				wpcompos = wpcompos2;
			}
		}
		if (wpcompos>wppos) {
			lp->recoheapp[pos] = pcompos;
			lp->recoheappindex[pcompos] = pos;
			lp->recoheapp[compos] = ppos;
			lp->recoheappindex[ppos] = compos;
		}
		else finished = 1;
		pos = compos;
	}


}


/** The removemaxrecoheap function */

int removemaxrecoheap (
					   lpstruct *lp
					   )
{
	int    pos,compos,compos2,ppos,pcompos,pcompos2,finished,element;
	double    wppos,wpcompos,wpcompos2;

	element = lp->recoheapp[1];
	ppos = lp->recoheapp[lp->recoheapcard];
	(lp->recoheapcard)--;
	lp->recoheapp[1] = ppos;
	lp->recoheappindex[ppos] = 1;
	pos = 1;
	// Go towards the leaves

	finished = 0;
	while ((!finished)&&(2*pos<=lp->recoheapcard)) {
		ppos = lp->recoheapp[pos];
		wppos = fabs(lp->reducedcost[ppos]);
		compos = 2*pos;
		pcompos = lp->recoheapp[compos];
		wpcompos = fabs(lp->reducedcost[pcompos]);
		if (compos<lp->recoheapcard) {
			compos2 = compos+1; 
			pcompos2 = lp->recoheapp[compos2];
			wpcompos2 = fabs(lp->reducedcost[pcompos2]);
			if (wpcompos2>wpcompos) {
				compos = compos2;
				pcompos = pcompos2;
				wpcompos = wpcompos2;
			}
		}
		if (wpcompos>wppos) {
			lp->recoheapp[pos] = pcompos;
			lp->recoheappindex[pcompos] = pos;
			lp->recoheapp[compos] = ppos;
			lp->recoheappindex[ppos] = compos;
		}
		else finished = 1;
		pos = compos;
	}


	lp->recoheappindex[element] = -1;
	return(element);
}


/** The removerecoheap function */

void removerecoheap (
					 lpstruct *lp,
					 int element
					 )
{
	int    pos,compos,compos2,ppos,pcompos,pcompos2,finished;
	double    wppos,wpcompos,wpcompos2,origreco;

	origreco = lp->reducedcost[element];
	lp->reducedcost[element] = ALPS_REAL_INFINITY;
	pos = lp->recoheappindex[element];
	// Go towards the root

	finished = 0;
	while ((!finished)&&(pos>1)) {
		compos = pos/2;
		ppos = lp->recoheapp[pos];
		wppos = fabs(lp->reducedcost[ppos]);
		pcompos = lp->recoheapp[compos];
		wpcompos = fabs(lp->reducedcost[pcompos]);
		if (wpcompos<wppos) {
			lp->recoheapp[pos] = pcompos;
			lp->recoheappindex[pcompos] = pos;
			lp->recoheapp[compos] = ppos;
			lp->recoheappindex[ppos] = compos;
		}
		else finished = 1;
		pos = compos;
	}


	if (pos!=1) {
		printf("pos=/=1 in removerecoheap !!!\n");
		exit(7);
	}
	pcompos = lp->recoheapp[lp->recoheapcard];
	(lp->recoheapcard)--;
	lp->recoheapp[pos] = pcompos;
	lp->recoheappindex[pcompos] = pos;
	// Go towards the leaves

	finished = 0;
	while ((!finished)&&(2*pos<=lp->recoheapcard)) {
		ppos = lp->recoheapp[pos];
		wppos = fabs(lp->reducedcost[ppos]);
		compos = 2*pos;
		pcompos = lp->recoheapp[compos];
		wpcompos = fabs(lp->reducedcost[pcompos]);
		if (compos<lp->recoheapcard) {
			compos2 = compos+1; 
			pcompos2 = lp->recoheapp[compos2];
			wpcompos2 = fabs(lp->reducedcost[pcompos2]);
			if (wpcompos2>wpcompos) {
				compos = compos2;
				pcompos = pcompos2;
				wpcompos = wpcompos2;
			}
		}
		if (wpcompos>wppos) {
			lp->recoheapp[pos] = pcompos;
			lp->recoheappindex[pcompos] = pos;
			lp->recoheapp[compos] = ppos;
			lp->recoheappindex[ppos] = compos;
		}
		else finished = 1;
		pos = compos;
	}


	lp->reducedcost[element] = origreco;
	lp->recoheappindex[element] = -1;
}


/** The insertrecoheap function */

void insertrecoheap (
					 lpstruct *lp,
					 int element
					 )
{
	int    pos,compos,ppos,pcompos,finished;
	double    wppos,wpcompos;

	(lp->recoheapcard)++;
	lp->recoheappindex[element] = lp->recoheapcard;
	lp->recoheapp[lp->recoheapcard] = element;
	pos = lp->recoheapcard;
	// Go towards the root

	finished = 0;
	while ((!finished)&&(pos>1)) {
		compos = pos/2;
		ppos = lp->recoheapp[pos];
		wppos = fabs(lp->reducedcost[ppos]);
		pcompos = lp->recoheapp[compos];
		wpcompos = fabs(lp->reducedcost[pcompos]);
		if (wpcompos<wppos) {
			lp->recoheapp[pos] = pcompos;
			lp->recoheappindex[pcompos] = pos;
			lp->recoheapp[compos] = ppos;
			lp->recoheappindex[ppos] = compos;
		}
		else finished = 1;
		pos = compos;
	}


}


/** The makerecoheap function */

void makerecoheap (
				   lpstruct *lp
				   )
{
	int    pos,compos,compos2,ppos,pcompos,pcompos2,startpos,finished;
	double    wppos,wpcompos,wpcompos2;

	for (startpos=(lp->recoheapcard)/2; startpos>=1; startpos--) {
		pos = startpos;
		// Go towards the leaves

		finished = 0;
		while ((!finished)&&(2*pos<=lp->recoheapcard)) {
			ppos = lp->recoheapp[pos];
			wppos = fabs(lp->reducedcost[ppos]);
			compos = 2*pos;
			pcompos = lp->recoheapp[compos];
			wpcompos = fabs(lp->reducedcost[pcompos]);
			if (compos<lp->recoheapcard) {
				compos2 = compos+1; 
				pcompos2 = lp->recoheapp[compos2];
				wpcompos2 = fabs(lp->reducedcost[pcompos2]);
				if (wpcompos2>wpcompos) {
					compos = compos2;
					pcompos = pcompos2;
					wpcompos = wpcompos2;
				}
			}
			if (wpcompos>wppos) {
				lp->recoheapp[pos] = pcompos;
				lp->recoheappindex[pcompos] = pos;
				lp->recoheapp[compos] = ppos;
				lp->recoheappindex[ppos] = compos;
			}
			else finished = 1;
			pos = compos;
		}


	}
}

} // end namespace ogdf
