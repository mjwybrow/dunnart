/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-11-27 03:02:28 +1100 (Thu, 27 Nov 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Installing and checking the external basis.
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

#include "alps.h"
#include "alps_misc.h"

namespace ogdf {

int alps_installextbasis( lpstruct *lp, int /* pril */)
{
	/** Declare alps_installextbasis scalars */
	int j,k,lpcj;
	int row_ind_of_slack_art;

	/** Allocate space for solution and basis */
	lp->x = (double *) malloc( lp->in * sizeof(double) );
	lp->redcost = (double *) malloc( lp->in * sizeof(double) );
	lp->y = (double *) malloc( lp->im * sizeof(double) );
	lp->ysval = (double *) malloc( lp->im * sizeof(double) );
	lp->ysind = (int *) malloc( lp->im * sizeof(int) );
	lp->d = (double *) malloc( lp->im * sizeof(double) );
	lp->dsval = (double *) malloc( lp->im * sizeof(double) );
	lp->dsind = (int *) malloc( lp->im * sizeof(int) );
	lp->colstat = (int *) malloc( lp->in * sizeof(int) );
	lp->basisheader = (int *) malloc( lp->im * sizeof(int) );
	lp->etacol = (int *) malloc(lp->etamax * sizeof(int));
	lp->etastart = (int *) malloc(lp->etamax * sizeof(int));
	lp->etacount = (int *) malloc(lp->etamax * sizeof(int));
	lp->eta = (double *) malloc(lp->etamax * sizeof(double));
	lp->etaval = (double *) malloc(lp->etamax * lp->im * sizeof(double));
	lp->etaind = (int *) malloc(lp->etamax * lp->im * sizeof(int));
	if (!lp->x ||!lp->redcost ||!lp->y ||!lp->ysval ||!lp->ysind 
		|| !lp->d || !lp->dsval || !lp->dsind || !lp->colstat 
		|| !lp->basisheader ||!lp->etacol ||!lp->etastart
		|| !lp->etacount ||!lp->eta || !lp->etaval ||!lp->etaind)
		// run out of memory
	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	/** Construct the internal basis */
	/*
	*  The internal basis is copied from the external basis.
	*/
	k = 0;
	for (j=0;j<lp->an;j++) {
		lpcj = lp->extcolstat[j];
		lp->colstat[j] = lpcj;
		if (lpcj==ALPS_BASIC) lp->basisheader[k++] = j;
	}
	for (j=lp->an;j<lp->in;j++) {
		row_ind_of_slack_art = lp->imatcolind[lp->imatcolbeg[j]];
		lpcj = lp->extrowstat[row_ind_of_slack_art];
		lp->colstat[j] = lpcj;
		if (lpcj==ALPS_BASIC) lp->basisheader[k++] = j;
	}
	ffree((char **) &lp->extcolstat);
	ffree((char **) &lp->extrowstat);

	return 0;
}

} // end namespace ogdf
