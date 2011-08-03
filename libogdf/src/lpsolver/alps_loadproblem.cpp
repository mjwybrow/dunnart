/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief The routine for loading the problem.
 * 
 * This is only a preliminary -- \e Text missing??
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

namespace ogdf {

int alps_loadproblem(
					 lpstruct *lp,
					 int nrows,
					 int ncols,
					 int nonz,
					 int minmax, 
					 double *obj,
					 int *matbeg,
					 int *matcount,
					 int *matind,
					 double *matcoeff,
					 double *rhs,
					 char *csense,
					 double *lb,
					 double *ub,
					 char *colnamestr,
					 int *colnameind,
					 char *rownamestr,
					 int *rownameind,
					 int ncolstrchars,
					 int nrowstrchars
					 )

{
	/** Declare alps_loadproblem scalars */
	int i,j,k,ind;

	/** Allocate space for original LP data */
	lp->an = ncols;
	lp->am = nrows;
	lp->amatcolbeg = (int *) malloc( lp->an * sizeof(int) );
	lp->amatcolcount = (int *) malloc( lp->an * sizeof(int) );
	lp->amatcolind = (int *) malloc( nonz * sizeof(int) );
	lp->amatcolcoeff = (double *) malloc( nonz * sizeof(double) );
	lp->aupperbound = (double *) malloc( lp->an * sizeof(double) );
	lp->alowerbound = (double *) malloc( lp->an * sizeof(double) );
	lp->ax = (double *) malloc( lp->an * sizeof(double) );
	lp->aslack = (double *) malloc( lp->am * sizeof(double) );
	lp->acolstat = (int *) malloc( (lp->an+lp->am) * sizeof(int) );
	lp->arhs = (double *) malloc( lp->am * sizeof(double) );
	lp->aobj = (double *) malloc( lp->an * sizeof(double) );
	lp->aconstrainttype = (char *) malloc( lp->am * sizeof(char) );
	if (!lp->amatcolbeg || ! lp->amatcolcount || !lp->amatcolind || !lp->amatcolcoeff
		|| !lp->aupperbound || !lp->alowerbound || !lp->ax || !lp->aslack
		|| !lp->acolstat || !lp->arhs || !lp->aobj || !lp->aconstrainttype)
		// run out of memory
	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	if (nrowstrchars) {
		lp->arownameind = (int *) malloc( lp->am * sizeof(int) );
		lp->arownamestr = (char *) malloc( nrowstrchars * sizeof(char) );
		if (!lp->arownameind || !lp->arownamestr)
			// run out of memory
		{
			fprintf (stderr, "run out of memory\n");
			return(ALPS_RUNOUTOFMEMORY);
		}
	}
	else {
		lp->arownameind = NULL;
		lp->arownamestr = NULL;
	}

	if (ncolstrchars) {
		lp->acolnameind = (int *) malloc( lp->an * sizeof(int) );
		lp->acolnamestr = (char *) malloc( ncolstrchars * sizeof(char) );
		if (!lp->acolnameind || !lp->acolnamestr) 
			// run out of memory
		{
			fprintf (stderr, "run out of memory\n");
			return(ALPS_RUNOUTOFMEMORY);
		}
	}
	else {
		lp->acolnameind = NULL;
		lp->acolnamestr = NULL;
	}

	/** Setup original LP */
	/*
	*  The original LP is set up in data structures starting with the letter
	* ``a''. This formulation is left untouched. (For later optimization the
	* data is copied into the internal data structures starting with the
	* letter ``i''.)
	*/
	ind = 0;
	for (j=0;j<ncols;j++) {
		lp->amatcolbeg[j] = matbeg[j];
		lp->amatcolcount[j] = matcount[j];
	}
	for (k=0;k<nonz;k++) {
		lp->amatcolind[k] = matind[k];
		lp->amatcolcoeff[k] = matcoeff[k];
	}
	for (i=0;i<nrows;i++) lp->arhs[i] = rhs[i];
	for (i=0;i<nrows;i++) lp->aconstrainttype[i] = csense[i];
	for (j=0;j<ncols;j++) lp->alowerbound[j] = lb[j];
	for (j=0;j<ncols;j++) lp->aupperbound[j] = ub[j];
	for (j=0;j<ncols;j++) lp->aobj[j] = obj[j];
	lp->aobjminmax = minmax;

	if (nrowstrchars) {
		for (i=0;i<nrows;i++) lp->arownameind[i] = rownameind[i];
		for (k=0;k<nrowstrchars;k++) lp->arownamestr[k] = rownamestr[k];
	}
	if (ncolstrchars) {
		for (j=0;j<ncols;j++) lp->acolnameind[j] = colnameind[j];
		for (k=0;k<ncolstrchars;k++) lp->acolnamestr[k] = colnamestr[k];
	}


	/** Initialize timers, iteration counts and set parameters to default values */
	/*
	*  We initialize the timers and iteration counts and set the parameters
	* to default values.
	*/
	lp->numfactor = 0;
	lp->sumfill = 0.0;
	lp->totaltime = 0;
	lp->factortime = 0;
	lp->piseltime = 0;
	lp->recoheaptime = 0;
	lp->fsolvetime = 0;
	lp->bsolvetime = 0;
	lp->ftrantime = 0;
	lp->btrantime = 0;
	lp->nrphase1iter = 0;
	lp->nrphase2iter = 0;
	lp->scaling = 3;
	lp->preprocessing = 0;
	lp->etamax = 80;
	lp->etaclear = 20;
	lp->maxconsider = 20;
	lp->howtopisel = ALPS_FULLEASYPISEL;
	lp->basisstatus = ALPS_NOBASIS;
	lp->initialbasis = ALPS_INITBASIS;

	return 0;
}

} // end namespace ogdf
