/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Primal (phase 1 and 2) and dual simplex algorithm.
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

/*
* The primal simplex algorithm starts by constructing the basis matrix
* from the basis header information, and computing an initial basis
* factorization. Then the primal simples loop is started. We assume that
* it has been verified that the basis is primal feasible. The dual
* simplex algorithm starts by constructing the basis matrix from the
* basis header information, and computing an initial basis
* factorization. Then the dual simples loop is started. We assume that
* it has been verified that the basis is dual feasible.
*/


#include "alps.h"
#include "alps_misc.h"
#include "alps_preproc.h"

#define MAXINFERROR 200
#define MAXOBJERROR 200

namespace ogdf {

int primalsimplex(
				  lpstruct *lp,
				  int pril
				  )

{
	/** Declare simplex scalars */

	int totalfill;
	int totalcancel;
	int status;
	int basnonz;
	int ind;
	int enter;
	int leaveindex;
	int leave;
	int i,j,k,l;
	int nriter;
	int dirindl;
	int eff;
	int dsc;
	int enr;
	int dsindl;
	int objincr;
	int starttime;
	int stoptime;
	int primalalgorithm;
	int basisvariable;
	int ageobjval;
	int degenage;
	int degeneracy;
	double dsvall;
	double dirvall;
	double redcost;
	double change;
	double yl;
	double zl;
	double oldobjval;
	double oldobjcoeff;
	double oldx;
	double newx;
	double lb;
	double ub;
	double lbviol;
	double ubviol;
	double preobjval;
	double absdiffub,absdifflb;


	/** Declare simplex arrays */

	int *pivrow;
	int *pivcol;
	double *pivvalue;
	double *cbas;
	double *xbas;
	double *trhs;
	double *entcol;



	/** Allocate space for simplex arrays */

	pivrow = NULL;
	pivcol = NULL;
	pivvalue = NULL;
	cbas = (double *) malloc( lp->im * sizeof(double) );
	xbas = (double *) malloc( lp->im * sizeof(double) );
	trhs = (double *) malloc( lp->im * sizeof(double) );
	entcol = (double *) malloc( lp->im * sizeof(double) );
	lp->w = (double *) malloc( lp->in * sizeof(double) );
	lp->wsval = (double *) malloc( lp->in * sizeof(double) );
	lp->wsind = (int *) malloc( lp->in * sizeof(int) );
	if (!cbas ||!xbas ||!trhs || !entcol )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}





	primalalgorithm = ALPS_TRUE;
	/** Construct basis matrix */
	/*
	*  In order to construct the basis matrix, all we have to do is counting
	* the number of nonzeroes in the columns corresponding to the variables
	* listed in the |lp->basisheader|, and then allocating the right amount
	* of space, and copying the data.
	*/
	ffree( (char **) &lp->basmatbeg);
	ffree( (char **) &lp->basmatcount);
	ffree( (char **) &lp->basmatind);
	ffree( (char **) &lp->basmatcoeff);
	basnonz = 0;
	for(k=0;k<lp->im;k++) basnonz += lp->imatcolcount[lp->basisheader[k]];
	lp->basmatbeg = (int *) malloc( lp->im * sizeof(int) );
	lp->basmatcount = (int *) malloc( lp->im * sizeof(int) );
	lp->basmatind = (int *) malloc( basnonz * sizeof(int) );
	lp->basmatcoeff = (double *) malloc( basnonz * sizeof(double) );
	if (!lp->basmatbeg || ! lp->basmatcount || !lp->basmatind 
		|| !lp->basmatcoeff )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}



	ind=0;
	for(k=0;k<lp->im;k++) {
		j = lp->basisheader[k];
		lp->basmatbeg[k] = ind;
		lp->basmatcount[k] = lp->imatcolcount[j];
		for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) {
			lp->basmatind[ind] = lp->imatcolind[l];
			lp->basmatcoeff[ind] =  lp->imatcolcoeff[l];
			ind++;
		}
	}



	/** Factor basis matrix */
	/*
	*  Here we compute an $LU$-factorization of the basis matrix. Later, we
	* should add here a fresh computation of |lp->x|.
	*/
	ffree( (char **) &pivrow);
	ffree( (char **) &pivcol);
	ffree( (char **) &pivvalue);
	ffree( (char **) &lp->rumatbeg);
	ffree( (char **) &lp->rumatend);
	ffree( (char **) &lp->rumatind);
	ffree( (char **) &lp->rumatcoeff);
	ffree( (char **) &lp->clmatbeg);
	ffree( (char **) &lp->clmatend);
	ffree( (char **) &lp->clmatind);
	ffree( (char **) &lp->clmatcoeff);
	ffree( (char **) &lp->cumatbeg);
	ffree( (char **) &lp->cumatend);
	ffree( (char **) &lp->cumatind);
	ffree( (char **) &lp->cumatcoeff);
#ifdef ALPS_TIMING_ENABLED
	starttime = cputime();
#endif
	status= alps_lufac (
		pril,
		(int) 0,
		lp->im,
		lp->basmatbeg,
		lp->basmatcount,
		lp->basmatind,
		lp->basmatcoeff,
		&pivrow,
		&pivcol,
		&pivvalue,
		&lp->rumatbeg,
		&lp->rumatend,
		&lp->rumatind,
		&lp->rumatcoeff,
		&lp->cumatbeg,
		&lp->cumatend,
		&lp->cumatind,
		&lp->cumatcoeff,
		&lp->clmatbeg,
		&lp->clmatend,
		&lp->clmatind,
		&lp->clmatcoeff,
		&totalfill,
		&totalcancel
		);
#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->factortime += (stoptime - starttime);
#endif
	if (status==ALPS_RUNOUTOFMEMORY) {
		fprintf(stderr,"LU factorization failed: run out of memory.\n");
		return ALPS_RUNOUTOFMEMORY;
	}
	else if (status) {
		fprintf(stderr,"LU factorization failed. Matrix singular.\n");
		return ALPS_LU_NOPIVOT;
	}
	lp->etanr = 0;
	lp->etafirstfree = 0;
	lp->numfactor++;
	lp->sumfill += ( (double) totalfill ) / ( (double) lp->im );


	/** Compute |x| and value from scratch */
	/*
	*  After each factorization, we compute the solution vector |lp->x| and
	* the objective function value from scratch.
	*/
	for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
	lp->objval = 0.0;
	for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) {
		switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		lp->x[j] = lp->ilowerbound[j];
		lp->objval += lp->iobj[j]*lp->x[j];
		break;
	case ALPS_NONBASICUPB:
		lp->x[j] = lp->iupperbound[j];
		lp->objval += lp->iobj[j]*lp->x[j];
		break;
	case ALPS_NONBASICFREE:
		lp->x[j] = 0.0;
		break;
		}
		for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			i = lp->imatcolind[k];
			trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
		}
	}
#ifdef ALPS_TIMING_ENABLED
	starttime = cputime();
#endif
	status = alps_fsolveeqs(
		lp,
		lp->im,
		pivrow,
		pivcol,
		pivvalue,
		lp->rumatbeg,
		lp->rumatend,
		lp->rumatind,
		lp->rumatcoeff,
		lp->clmatbeg,
		lp->clmatend,
		lp->clmatind,
		lp->clmatcoeff,
		lp->etanr,
		lp->etacol,
		lp->etaval,
		lp->etaind,
		lp->etastart,
		lp->etacount,
		lp->eta,
		trhs,
		xbas
		);

#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->fsolvetime += (stoptime - starttime);
#endif
	if (status==ALPS_RUNOUTOFMEMORY) {
		fprintf(stderr,"FSOLVE failed: run out of memory.\n");
		exit(1000);
	}
	for (i=0; i<lp->im; i++) {
		j = lp->basisheader[i];
		lp->x[j] = xbas[i];
		if (primalalgorithm) {
			if (lp->x[j]<lp->ilowerbound[j]) lp->x[j] = lp->ilowerbound[j];
			if (lp->x[j]>lp->iupperbound[j]) lp->x[j] = lp->iupperbound[j];
		}
		lp->objval += lp->iobj[j]*lp->x[j];
	}


	/** Primal phase2 simplex loop */
	/*
	*  The primal simplex loop doesn't go forever (hopefully) since on
	* various termination conditions we jump to
	* |terminate_primal_simplex_loop|. The layout is textbook-like.
	*/
	oldobjval = 1.0E20;
	preobjval = -1.0E20;
	ageobjval = 0;
	degenage = 2;
	ageobjval = 0;
	degeneracy = ALPS_FALSE;
	objincr = 0;
	nriter = lp->nrphase1iter;
	while(ALPS_TRUE) {
start_primal_simplex_loop:
		// Internal constraint check

#ifdef CHECK
	{ int i,j,k,nviol;

	nviol = 0;
	for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
	for (j=0;j<lp->in;j++) {
		for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j]; k++) {
			i = lp->imatcolind[k];
			trhs[i] -=  lp->x[j]*lp->imatcolcoeff[k];
		}
	}
	for (i=0; i<lp->im; i++) { 
		if (trhs[i]>0.01) {
			printf("Constraint violation (%.4lf) in constraint %d (%s)!\n",
				trhs[i],i,lp->arownamestr+lp->arownameind[org_rownr[i]]);
			nviol++;
		}
	}
#ifdef EXIT
	if (nviol) {
		exit(1000);
	}
#endif
	}
#endif


	// Internal bound check
	/*
	*  Feasibility check for internal problem.
	*/
#ifdef CHECK
	{ int j,nviol;
	double viol;

	nviol = 0;
	for (j=0;j<lp->in;j++) {
		viol = lp->x[j] - lp->iupperbound[j];
		if (viol>0.0001) {
			printf("\nUpper bound violation (%.4lf>%.4lf) of variable %d",
				lp->x[j],lp->iupperbound[j],j);
			if (j<lp->inorig) printf(" (%s)",lp->acolnamestr+lp->acolnameind[j]);
			printf("!\n");
			nviol++;
		}
		viol = lp->ilowerbound[j] - lp->x[j];
		if (viol>0.0001) {
			printf("\nLower bound violation (%.4lf<%.4lf) of variable %d",
				lp->x[j],lp->ilowerbound[j],j);
			if (j<lp->inorig) printf(" (%s)",lp->acolnamestr+lp->acolnameind[j]);
			printf("!\n");
			nviol++;
		}
	}
#ifdef EXIT
	if (nviol) {
		exit(1000);
	}
#endif
	}
#endif


#ifdef ORIG
	if (lp->etanr==0) {
		// Compute multipliers
		/*
		*  The multipliers $y$, here |lp->y|, are found by solving the system
		* $y^TB=c_B$.
		*/
		for (k=0;k<lp->im;k++) cbas[k] = lp->iobj[lp->basisheader[k]];
#ifdef ALPS_TIMING_ENABLED
		starttime = cputime();
#endif
		status = alps_bsolveeqs(
			lp,
			lp->im,
			pivrow,
			pivcol,
			pivvalue,
			lp->cumatbeg,
			lp->cumatend,
			lp->cumatind,
			lp->cumatcoeff,
			lp->clmatbeg,
			lp->clmatend,
			lp->clmatind,
			lp->clmatcoeff,
			lp->etanr,
			lp->etacol,
			lp->etaval,
			lp->etaind,
			lp->etastart,
			lp->etacount,
			lp->eta,
			cbas,
			lp->y
			);

#ifdef ALPS_TIMING_ENABLED
		stoptime = cputime();
		lp->bsolvetime += (stoptime - starttime);
#endif
		if (status==ALPS_RUNOUTOFMEMORY) {
			fprintf(stderr,"BSOLVE failed: run out of memory.\n");
			exit(1000);
		}
		k = 0;
		for (l=0;l<lp->im;l++) {
			yl = lp->y[l];
			if ((yl<-ALPS_ZEROEPS)||(yl>ALPS_ZEROEPS)) {
				lp->ysind[k] = l;
				lp->ysval[k] = yl;
				k++;
			}
		}
		lp->yscount = k;
		if (pril>=4) {
			printf("\ncbas: "); for (i=0;i<lp->im;i++) printf(" %.2lf",cbas[i]);printf("\n");
			printf("y   : "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->y[i]);printf("\n");
		}


	}
#endif
	// Compute multipliers
	/*
	*  The multipliers $y$, here |lp->y|, are found by solving the system
	* $y^TB=c_B$.
	*/
	for (k=0;k<lp->im;k++) cbas[k] = lp->iobj[lp->basisheader[k]];
#ifdef ALPS_TIMING_ENABLED
	starttime = cputime();
#endif
	status = alps_bsolveeqs(
		lp,
		lp->im,
		pivrow,
		pivcol,
		pivvalue,
		lp->cumatbeg,
		lp->cumatend,
		lp->cumatind,
		lp->cumatcoeff,
		lp->clmatbeg,
		lp->clmatend,
		lp->clmatind,
		lp->clmatcoeff,
		lp->etanr,
		lp->etacol,
		lp->etaval,
		lp->etaind,
		lp->etastart,
		lp->etacount,
		lp->eta,
		cbas,
		lp->y
		);

#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->bsolvetime += (stoptime - starttime);
#endif
	if (status==ALPS_RUNOUTOFMEMORY) {
		fprintf(stderr,"BSOLVE failed: run out of memory.\n");
		exit(1000);
	}
	k = 0;
	for (l=0;l<lp->im;l++) {
		yl = lp->y[l];
		if ((yl<-ALPS_ZEROEPS)||(yl>ALPS_ZEROEPS)) {
			lp->ysind[k] = l;
			lp->ysval[k] = yl;
			k++;
		}
	}
	lp->yscount = k;
	if (pril>=4) {
		printf("\ncbas: "); for (i=0;i<lp->im;i++) printf(" %.2lf",cbas[i]);printf("\n");
		printf("y   : "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->y[i]);printf("\n");
	}


	// Make primal pivot selection
	/*
	*  For primal pivot selection, we call pisel or fulleasypisel, and then,
	* before the pivot, we compute $z$ by solving $z^TA_B=u^T$ for the unit
	* vector $u$ with a $1$ in the |leaveindex| position.
	*/
#ifdef ALPS_TIMING_ENABLED
	starttime = cputime();
#endif
	if (degeneracy) {
		status = alps_primalrandompisel(
			lp,
			pril,
			&enter,
			&leave,
			&leaveindex,
			&redcost,
			&change,
			pivrow,
			pivcol,
			pivvalue,
			&oldobjcoeff
			);
	}
	else {
		status = alps_primaleasypisel(
			lp,
			pril,
			&enter,
			&leave,
			&leaveindex,
			&redcost,
			&change,
			pivrow,
			pivcol,
			pivvalue,
			&oldobjcoeff
			);
	}
#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->piseltime += (stoptime - starttime);
#endif
	if ((lp->problemstatus==ALPS_UNBOUNDED)||(lp->problemstatus==ALPS_OPTIMAL)) {
		if (!lp->etanr) goto terminate_primal_simplex_loop;
		else {
			// Construct basis matrix
			/*
			*  In order to construct the basis matrix, all we have to do is counting
			* the number of nonzeroes in the columns corresponding to the variables
			* listed in the |lp->basisheader|, and then allocating the right amount
			* of space, and copying the data.
			*/
			ffree( (char **) &lp->basmatbeg);
			ffree( (char **) &lp->basmatcount);
			ffree( (char **) &lp->basmatind);
			ffree( (char **) &lp->basmatcoeff);
			basnonz = 0;
			for(k=0;k<lp->im;k++) basnonz += lp->imatcolcount[lp->basisheader[k]];
			lp->basmatbeg = (int *) malloc( lp->im * sizeof(int) );
			lp->basmatcount = (int *) malloc( lp->im * sizeof(int) );
			lp->basmatind = (int *) malloc( basnonz * sizeof(int) );
			lp->basmatcoeff = (double *) malloc( basnonz * sizeof(double) );
			if (!lp->basmatbeg || ! lp->basmatcount || !lp->basmatind 
				|| !lp->basmatcoeff )
				// run out of memory

			{
				fprintf (stderr, "run out of memory\n");
				return(ALPS_RUNOUTOFMEMORY);
			}



			ind=0;
			for(k=0;k<lp->im;k++) {
				j = lp->basisheader[k];
				lp->basmatbeg[k] = ind;
				lp->basmatcount[k] = lp->imatcolcount[j];
				for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) {
					lp->basmatind[ind] = lp->imatcolind[l];
					lp->basmatcoeff[ind] =  lp->imatcolcoeff[l];
					ind++;
				}
			}



			// Factor basis matrix
			/*
			*  Here we compute an $LU$-factorization of the basis matrix. Later, we
			* should add here a fresh computation of |lp->x|.
			*/
			ffree( (char **) &pivrow);
			ffree( (char **) &pivcol);
			ffree( (char **) &pivvalue);
			ffree( (char **) &lp->rumatbeg);
			ffree( (char **) &lp->rumatend);
			ffree( (char **) &lp->rumatind);
			ffree( (char **) &lp->rumatcoeff);
			ffree( (char **) &lp->clmatbeg);
			ffree( (char **) &lp->clmatend);
			ffree( (char **) &lp->clmatind);
			ffree( (char **) &lp->clmatcoeff);
			ffree( (char **) &lp->cumatbeg);
			ffree( (char **) &lp->cumatend);
			ffree( (char **) &lp->cumatind);
			ffree( (char **) &lp->cumatcoeff);
#ifdef ALPS_TIMING_ENABLED
			starttime = cputime();
#endif
			status= alps_lufac (
				pril,
				(int) 0,
				lp->im,
				lp->basmatbeg,
				lp->basmatcount,
				lp->basmatind,
				lp->basmatcoeff,
				&pivrow,
				&pivcol,
				&pivvalue,
				&lp->rumatbeg,
				&lp->rumatend,
				&lp->rumatind,
				&lp->rumatcoeff,
				&lp->cumatbeg,
				&lp->cumatend,
				&lp->cumatind,
				&lp->cumatcoeff,
				&lp->clmatbeg,
				&lp->clmatend,
				&lp->clmatind,
				&lp->clmatcoeff,
				&totalfill,
				&totalcancel
				);
#ifdef ALPS_TIMING_ENABLED
			stoptime = cputime();
			lp->factortime += (stoptime - starttime);
#endif
			if (status==ALPS_RUNOUTOFMEMORY) {
				fprintf(stderr,"LU factorization failed: run out of memory.\n");
				return ALPS_RUNOUTOFMEMORY;
			}
			else if (status) {
				fprintf(stderr,"LU factorization failed. Matrix singular.\n");
				return ALPS_LU_NOPIVOT;
			}
			lp->etanr = 0;
			lp->etafirstfree = 0;
			lp->numfactor++;
			lp->sumfill += ( (double) totalfill ) / ( (double) lp->im );


			if (pril>=3) printf("=== Refactorization performed. ===\n");
			// Compute |x| and value from scratch
			/*
			*  After each factorization, we compute the solution vector |lp->x| and
			* the objective function value from scratch.
			*/
			for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
			lp->objval = 0.0;
			for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) {
				switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		lp->x[j] = lp->ilowerbound[j];
		lp->objval += lp->iobj[j]*lp->x[j];
		break;
	case ALPS_NONBASICUPB:
		lp->x[j] = lp->iupperbound[j];
		lp->objval += lp->iobj[j]*lp->x[j];
		break;
	case ALPS_NONBASICFREE:
		lp->x[j] = 0.0;
		break;
				}
				for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
					i = lp->imatcolind[k];
					trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
				}
			}
#ifdef ALPS_TIMING_ENABLED
			starttime = cputime();
#endif
			status = alps_fsolveeqs(
				lp,
				lp->im,
				pivrow,
				pivcol,
				pivvalue,
				lp->rumatbeg,
				lp->rumatend,
				lp->rumatind,
				lp->rumatcoeff,
				lp->clmatbeg,
				lp->clmatend,
				lp->clmatind,
				lp->clmatcoeff,
				lp->etanr,
				lp->etacol,
				lp->etaval,
				lp->etaind,
				lp->etastart,
				lp->etacount,
				lp->eta,
				trhs,
				xbas
				);

#ifdef ALPS_TIMING_ENABLED
			stoptime = cputime();
			lp->fsolvetime += (stoptime - starttime);
#endif
			if (status==ALPS_RUNOUTOFMEMORY) {
				fprintf(stderr,"FSOLVE failed: run out of memory.\n");
				exit(1000);
			}
			for (i=0; i<lp->im; i++) {
				j = lp->basisheader[i];
				lp->x[j] = xbas[i];
				if (primalalgorithm) {
					if (lp->x[j]<lp->ilowerbound[j]) lp->x[j] = lp->ilowerbound[j];
					if (lp->x[j]>lp->iupperbound[j]) lp->x[j] = lp->iupperbound[j];
				}
				lp->objval += lp->iobj[j]*lp->x[j];
			}


			if (pril>=4) printprimalsol(lp);
			goto start_primal_simplex_loop;
		}
	}
	else if (leaveindex>=0) {
		if (pril>=4) printf("Leaving variable: %d [change = %.2lf]\n",leave,change);
		lp->u[leaveindex] = 1.0;
#ifdef ALPS_TIMING_ENABLED
		starttime = cputime();
#endif
		status = alps_bsolveeqs(
			lp,
			lp->im,
			pivrow,
			pivcol,
			pivvalue,
			lp->cumatbeg,
			lp->cumatend,
			lp->cumatind,
			lp->cumatcoeff,
			lp->clmatbeg,
			lp->clmatend,
			lp->clmatind,
			lp->clmatcoeff,
			lp->etanr,
			lp->etacol,
			lp->etaval,
			lp->etaind,
			lp->etastart,
			lp->etacount,
			lp->eta,
			lp->u,
			lp->z
			);

#ifdef ALPS_TIMING_ENABLED
		stoptime = cputime();
		lp->bsolvetime += (stoptime - starttime);
#endif
		if (status==ALPS_RUNOUTOFMEMORY) {
			fprintf(stderr,"BSOLVE failed: run out of memory.\n");
			exit(1000);
		}
		k = 0;
		for (l=0;l<lp->im;l++) {
			zl = lp->z[l];
			if ((zl<-ALPS_ZEROEPS)||(zl>ALPS_ZEROEPS)) {
				lp->zsind[k] = l;
				lp->zsval[k] = zl;
				k++;
			}
		}
		lp->zscount = k;
		if (pril>=4) {
			printf("\nu: "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->u[i]);printf("\n");
			printf("z: "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->z[i]);printf("\n");
		}

		lp->u[leaveindex] = 0.0;
	}
	else {
		leave = enter;
		if (pril>=3) printf("Variable %d changing bounds [%.2lf units]\n",leave,change);
	}

	if (pril>=4) {
		for (k=0;k<lp->im;k++) entcol[k] = 0.0;
		for (l=lp->imatcolbeg[enter];l<lp->imatcolbeg[enter]+lp->imatcolcount[enter];l++) 
			entcol[lp->imatcolind[l]] = lp->imatcolcoeff[l];
		printf("\nentcol: "); for (i=0;i<lp->im;i++) printf(" %.2lf",entcol[i]);printf("\n");
		printf("d     : "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->d[i]);printf("\n");
	}


	// Basis update (primal)
	/*
	*  Here we update the basis information. If there is no real pivot
	* |leave==enter|, the ``entering/leaving'' just switches bounds which is
	* recorded in |lp->colstat[enter]|. Otherwise, the entering variable
	* takes the leaving variable's place in the |lp->basisheader| and is
	* declared basic. A variable leaving the basis goes to one of the
	* bounds. (Free basic variables never leave.) If we are in phase 1 and
	* there were artificial bounds, the original bounds are restored and the
	* phase 1 objective function coefficient is set to zero.
	*/
	enr = lp->etanr;
	if (((!enr)&&(pril==1))||(pril>=2)) printf("%6d:   ",nriter);
	for(l=0;l<lp->dscount;l++) {
		dirindl = lp->dsind[l];
		dirvall = lp->dsval[l];
		basisvariable = lp->basisheader[dirindl];
		oldx = lp->x[basisvariable];
		newx = oldx - change*dirvall;
		// Adjust the new |x| value

		lb = lp->ilowerbound[basisvariable];
		lbviol = lb - newx;
		if (lbviol>0.0) { /* is this good or not ? */
			if (lbviol>0.001) 
				printf("\nStrong lower bound violation of basic variable %d: %lf < %lf!\n",
				basisvariable,newx,lb);
			newx = lb;
		}
		ub = lp->iupperbound[basisvariable];
		ubviol = newx - ub;
		if (ubviol>0.0) { /* is this good or not ? */
			if (lbviol>0.001) 
				printf("\nStrong upper bound violation of basic variable %d: %lf > %lf!\n",
				basisvariable,newx,ub);
			newx = ub;
		}


		lp->x[basisvariable] = newx;
		lp->objval -= (oldx-newx)*lp->iobj[basisvariable];
	}
	basisvariable = enter;
	oldx = lp->x[enter];
	newx = oldx + change;
	// Adjust the new |x| value

	lb = lp->ilowerbound[basisvariable];
	lbviol = lb - newx;
	if (lbviol>0.0) { /* is this good or not ? */
		if (lbviol>0.001) 
			printf("\nStrong lower bound violation of basic variable %d: %lf < %lf!\n",
			basisvariable,newx,lb);
		newx = lb;
	}
	ub = lp->iupperbound[basisvariable];
	ubviol = newx - ub;
	if (ubviol>0.0) { /* is this good or not ? */
		if (lbviol>0.001) 
			printf("\nStrong upper bound violation of basic variable %d: %lf > %lf!\n",
			basisvariable,newx,ub);
		newx = ub;
	}


	lp->x[basisvariable] = newx;
	lp->objval -= (oldx-newx)*lp->iobj[basisvariable];
	if (leave==enter) {
		if (lp->colstat[enter]==ALPS_NONBASICLOWB) {
			lp->colstat[enter]=ALPS_NONBASICUPB;
			lp->x[enter]=lp->iupperbound[enter];
			if (((!enr)&&(pril==1))||(pril>=2)) {
				if (enter<lp->inorig) printf("x");
				else if (enter<lp->inonartif) printf("s");
				else printf("a");
				printf("[%4d] LB -> UB            ",enter);
			}
		}
		else {
			lp->colstat[enter]=ALPS_NONBASICLOWB;
			lp->x[enter]=lp->ilowerbound[enter];
			if (((!enr)&&(pril==1))||(pril>=2)) {
				if (enter<lp->inorig) printf("x");
				else if (enter<lp->inonartif) printf("s");
				else printf("a");
				printf("[%4d] UB -> LB            ",enter);
			}
		}
	}
	else {
		if (((!enr)&&(pril==1))||(pril>=2)) {
			if (leave<lp->inorig) printf("x");
			else if (leave<lp->inonartif) printf("s");
			else printf("a");
			printf("[%4d] -> ",leave);
			if (enter<lp->inorig) printf("x");
			else if (enter<lp->inonartif) printf("s");
			else printf("a");
			printf("[%4d] ",enter);
		}
		lp->basisheader[leaveindex] = enter;
		lp->colstat[enter]=ALPS_BASIC;

		eff = lp->etafirstfree;
		dsc = lp->dscount;
		enr = lp->etanr;
		lp->etacol[enr] = leaveindex;
		lp->etastart[enr] = eff;
		lp->etacount[enr] = dsc - 1;
		for (l=0; l<dsc; l++) {
			dsindl = lp->dsind[l];
			dsvall = lp->dsval[l];
			if (dsindl == leaveindex) {
				lp->eta[enr] = dsvall;
				if (((!enr)&&(pril==1))||(pril>=2)) printf(" %8.1le",dsvall);
			}
			else {
				lp->etaind[eff] = dsindl;
				lp->etaval[eff] = dsvall;
				eff++;
			}
		}
		lp->etanr = enr + 1;
		lp->etafirstfree = eff;
		oldobjcoeff = 0.0;

		if (lp->varstat[leave]==ALPS_BOUNDEDABOVE) {
			lp->colstat[leave]=ALPS_NONBASICUPB;
			lp->x[leave] = lp->iupperbound[leave];
		}
		else if (lp->varstat[leave]==ALPS_BOUNDEDBELOW) {
			lp->colstat[leave]=ALPS_NONBASICLOWB;
			lp->x[leave] = lp->ilowerbound[leave];
		}
		else if (lp->varstat[leave]==ALPS_BOUNDEDTWICE) {
			absdifflb = fabs(lp->x[leave]-lp->ilowerbound[leave]);
			absdiffub = fabs(lp->x[leave]-lp->iupperbound[leave]);
			if (absdifflb<absdiffub) {
				lp->colstat[leave]=ALPS_NONBASICLOWB;
				lp->x[leave] = lp->ilowerbound[leave];
			}
			else {
				lp->colstat[leave]=ALPS_NONBASICUPB;
				lp->x[leave] = lp->iupperbound[leave];
			}
		}
		else if (lp->varstat[leave]==ALPS_FIXED) {
			lp->colstat[leave]=ALPS_NONBASICLOWB;
			lp->x[leave] = lp->ilowerbound[leave];
		}    
		else if (lp->varstat[leave]==ALPS_FREE) {
			lp->colstat[leave]=ALPS_NONBASICFREE;
			newx = lp->x[leave];
			printf("\nNew nonbasic free variable with value %lf!\n",newx);
		}

	}


	if (((!enr)&&(pril==1))||(pril>=2)) {
		if (lp->aobjminmax==ALPS_MAXIMIZE) 
			printf(" %15.4lf\n",lp->objval);
		else
			printf(" %15.4lf\n",-lp->objval);
	}
	if (-lp->objval>oldobjval+10.0) {
		if (pril>=2) printf("\nWARNING: Objective increased very much.\n\n");
		objincr++;
	}
	else if (-lp->objval>oldobjval+1.0) {
		if (pril>=2) printf("\nWARNING: Objective increased.\n\n");
		objincr++;
	}
	oldobjval = -lp->objval;
	if (objincr>MAXOBJERROR) {
		printf("\nERROR: Objective increased more than %d times.\n\n",MAXOBJERROR);
		exit(1000);
	}





	if (pril>=4) printprimalsol(lp);
	nriter++;
	if (lp->etanr>=lp->etaclear) {
		// Construct basis matrix
		/*
		*  In order to construct the basis matrix, all we have to do is counting
		* the number of nonzeroes in the columns corresponding to the variables
		* listed in the |lp->basisheader|, and then allocating the right amount
		* of space, and copying the data.
		*/
		ffree( (char **) &lp->basmatbeg);
		ffree( (char **) &lp->basmatcount);
		ffree( (char **) &lp->basmatind);
		ffree( (char **) &lp->basmatcoeff);
		basnonz = 0;
		for(k=0;k<lp->im;k++) basnonz += lp->imatcolcount[lp->basisheader[k]];
		lp->basmatbeg = (int *) malloc( lp->im * sizeof(int) );
		lp->basmatcount = (int *) malloc( lp->im * sizeof(int) );
		lp->basmatind = (int *) malloc( basnonz * sizeof(int) );
		lp->basmatcoeff = (double *) malloc( basnonz * sizeof(double) );
		if (!lp->basmatbeg || ! lp->basmatcount || !lp->basmatind 
			|| !lp->basmatcoeff )
			// run out of memory

		{
			fprintf (stderr, "run out of memory\n");
			return(ALPS_RUNOUTOFMEMORY);
		}



		ind=0;
		for(k=0;k<lp->im;k++) {
			j = lp->basisheader[k];
			lp->basmatbeg[k] = ind;
			lp->basmatcount[k] = lp->imatcolcount[j];
			for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) {
				lp->basmatind[ind] = lp->imatcolind[l];
				lp->basmatcoeff[ind] =  lp->imatcolcoeff[l];
				ind++;
			}
		}



		// Factor basis matrix
		/*
		*  Here we compute an $LU$-factorization of the basis matrix. Later, we
		* should add here a fresh computation of |lp->x|.
		*/
		ffree( (char **) &pivrow);
		ffree( (char **) &pivcol);
		ffree( (char **) &pivvalue);
		ffree( (char **) &lp->rumatbeg);
		ffree( (char **) &lp->rumatend);
		ffree( (char **) &lp->rumatind);
		ffree( (char **) &lp->rumatcoeff);
		ffree( (char **) &lp->clmatbeg);
		ffree( (char **) &lp->clmatend);
		ffree( (char **) &lp->clmatind);
		ffree( (char **) &lp->clmatcoeff);
		ffree( (char **) &lp->cumatbeg);
		ffree( (char **) &lp->cumatend);
		ffree( (char **) &lp->cumatind);
		ffree( (char **) &lp->cumatcoeff);
#ifdef ALPS_TIMING_ENABLED
		starttime = cputime();
#endif
		status= alps_lufac (
			pril,
			(int) 0,
			lp->im,
			lp->basmatbeg,
			lp->basmatcount,
			lp->basmatind,
			lp->basmatcoeff,
			&pivrow,
			&pivcol,
			&pivvalue,
			&lp->rumatbeg,
			&lp->rumatend,
			&lp->rumatind,
			&lp->rumatcoeff,
			&lp->cumatbeg,
			&lp->cumatend,
			&lp->cumatind,
			&lp->cumatcoeff,
			&lp->clmatbeg,
			&lp->clmatend,
			&lp->clmatind,
			&lp->clmatcoeff,
			&totalfill,
			&totalcancel
			);
#ifdef ALPS_TIMING_ENABLED
		stoptime = cputime();
		lp->factortime += (stoptime - starttime);
#endif
		if (status==ALPS_RUNOUTOFMEMORY) {
			fprintf(stderr,"LU factorization failed: run out of memory.\n");
			return ALPS_RUNOUTOFMEMORY;
		}
		else if (status) {
			fprintf(stderr,"LU factorization failed. Matrix singular.\n");
			return ALPS_LU_NOPIVOT;
		}
		lp->etanr = 0;
		lp->etafirstfree = 0;
		lp->numfactor++;
		lp->sumfill += ( (double) totalfill ) / ( (double) lp->im );


		if (pril>=3) printf("=== Refactorization performed (%1d/%1d). ===\n",lp->im,totalfill);
		// Compute |x| and value from scratch
		/*
		*  After each factorization, we compute the solution vector |lp->x| and
		* the objective function value from scratch.
		*/
		for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
		lp->objval = 0.0;
		for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) {
			switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		lp->x[j] = lp->ilowerbound[j];
		lp->objval += lp->iobj[j]*lp->x[j];
		break;
	case ALPS_NONBASICUPB:
		lp->x[j] = lp->iupperbound[j];
		lp->objval += lp->iobj[j]*lp->x[j];
		break;
	case ALPS_NONBASICFREE:
		lp->x[j] = 0.0;
		break;
			}
			for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				i = lp->imatcolind[k];
				trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
			}
		}
#ifdef ALPS_TIMING_ENABLED
		starttime = cputime();
#endif
		status = alps_fsolveeqs(
			lp,
			lp->im,
			pivrow,
			pivcol,
			pivvalue,
			lp->rumatbeg,
			lp->rumatend,
			lp->rumatind,
			lp->rumatcoeff,
			lp->clmatbeg,
			lp->clmatend,
			lp->clmatind,
			lp->clmatcoeff,
			lp->etanr,
			lp->etacol,
			lp->etaval,
			lp->etaind,
			lp->etastart,
			lp->etacount,
			lp->eta,
			trhs,
			xbas
			);

#ifdef ALPS_TIMING_ENABLED
		stoptime = cputime();
		lp->fsolvetime += (stoptime - starttime);
#endif
		if (status==ALPS_RUNOUTOFMEMORY) {
			fprintf(stderr,"FSOLVE failed: run out of memory.\n");
			exit(1000);
		}
		for (i=0; i<lp->im; i++) {
			j = lp->basisheader[i];
			lp->x[j] = xbas[i];
			if (primalalgorithm) {
				if (lp->x[j]<lp->ilowerbound[j]) lp->x[j] = lp->ilowerbound[j];
				if (lp->x[j]>lp->iupperbound[j]) lp->x[j] = lp->iupperbound[j];
			}
			lp->objval += lp->iobj[j]*lp->x[j];
		}


		if ( fabs(lp->objval)<ALPS_ZEROEPS && lp->objval<preobjval+ALPS_ZEROEPS) {
			ageobjval++;
			if ((ageobjval>degenage)&&(!degeneracy)) {
				degeneracy = ALPS_TRUE;
				if (pril) printf("Looks degenerate...\n");
			}
		}
		else if ( fabs(lp->objval)>ALPS_ZEROEPS 
			&& fabs((lp->objval-preobjval)/lp->objval)<ALPS_ZEROEPS) {
				ageobjval++;
				if ((ageobjval>degenage)&&(!degeneracy)) {
					degeneracy = ALPS_TRUE;
					if (pril) printf("Looks degenerate...\n");
				}
			}
		else {
			preobjval = lp->objval;
			ageobjval = 0;
			if (degeneracy) {
				degeneracy = ALPS_FALSE;
				if (pril) printf("Looks fine again...\n");
			}
		}
		if (pril>=4) printprimalsol(lp);
	}
	}
terminate_primal_simplex_loop:
	lp->nrphase2iter = nriter - lp->nrphase1iter;
	if (pril>=3) {
		if (lp->problemstatus==ALPS_UNKNOWN) printf("\nStatus: ALPS_UNKNOWN\n");
		if (lp->problemstatus==ALPS_INFEASIBLE) printf("\nStatus: ALPS_INFEASIBLE\n");
		if (lp->problemstatus==ALPS_UNBOUNDED) printf("\nStatus: ALPS_UNBOUNDED\n");
		if (lp->problemstatus==ALPS_OPTIMAL) printf("\nStatus: ALPS_OPTIMAL\n");
	}


	/** Free space for simplex arrays */

	ffree( (char **) &pivrow);
	ffree( (char **) &pivcol);
	ffree( (char **) &pivvalue);
	ffree( (char **) &lp->rumatbeg);
	ffree( (char **) &lp->rumatend);
	ffree( (char **) &lp->rumatind);
	ffree( (char **) &lp->rumatcoeff);
	ffree( (char **) &lp->clmatbeg);
	ffree( (char **) &lp->clmatend);
	ffree( (char **) &lp->clmatind);
	ffree( (char **) &lp->clmatcoeff);
	ffree( (char **) &lp->cumatbeg);
	ffree( (char **) &lp->cumatend);
	ffree( (char **) &lp->cumatind);
	ffree( (char **) &lp->cumatcoeff);
	ffree( (char **) &cbas);
	ffree( (char **) &xbas);
	ffree( (char **) &trhs);
	ffree( (char **) &entcol);
	ffree( (char **) &lp->w);
	ffree( (char **) &lp->wsval);
	ffree( (char **) &lp->wsind);

	return 0;
}

int primalphase1simplex(
						lpstruct *lp,
						int pril
						)

{
	/** Declare simplex scalars */

	int totalfill;
	int totalcancel;
	int status;
	int basnonz;
	int ind;
	int enter;
	int leaveindex;
	int leave;
	int i,j,k,l;
	int nriter;
	int dirindl;
	int eff;
	int dsc;
	int enr;
	int dsindl;
	int objincr;
	int starttime;
	int stoptime;
	int primalalgorithm;
	int basisvariable;
	int ageobjval;
	int degenage;
	int degeneracy;
	double dsvall;
	double dirvall;
	double redcost;
	double change;
	double yl;
	double zl;
	double oldobjval;
	double oldobjcoeff;
	double oldx;
	double newx;
	double lb;
	double ub;
	double lbviol;
	double ubviol;
	double preobjval;
	int wentfurther;
	double absdiffub,absdifflb;


	/** Declare simplex arrays */

	int *pivrow;
	int *pivcol;
	double *pivvalue;
	double *cbas;
	double *xbas;
	double *trhs;
	double *entcol;



	/** Allocate space for simplex arrays */

	pivrow = NULL;
	pivcol = NULL;
	pivvalue = NULL;
	cbas = (double *) malloc( lp->im * sizeof(double) );
	xbas = (double *) malloc( lp->im * sizeof(double) );
	trhs = (double *) malloc( lp->im * sizeof(double) );
	entcol = (double *) malloc( lp->im * sizeof(double) );
	lp->w = (double *) malloc( lp->in * sizeof(double) );
	lp->wsval = (double *) malloc( lp->in * sizeof(double) );
	lp->wsind = (int *) malloc( lp->in * sizeof(int) );
	if (!cbas ||!xbas ||!trhs || !entcol )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}





	primalalgorithm = ALPS_TRUE;
	nriter = 0;
	/** Construct basis matrix */
	/*
	*  In order to construct the basis matrix, all we have to do is counting
	* the number of nonzeroes in the columns corresponding to the variables
	* listed in the |lp->basisheader|, and then allocating the right amount
	* of space, and copying the data.
	*/
	ffree( (char **) &lp->basmatbeg);
	ffree( (char **) &lp->basmatcount);
	ffree( (char **) &lp->basmatind);
	ffree( (char **) &lp->basmatcoeff);
	basnonz = 0;
	for(k=0;k<lp->im;k++) basnonz += lp->imatcolcount[lp->basisheader[k]];
	lp->basmatbeg = (int *) malloc( lp->im * sizeof(int) );
	lp->basmatcount = (int *) malloc( lp->im * sizeof(int) );
	lp->basmatind = (int *) malloc( basnonz * sizeof(int) );
	lp->basmatcoeff = (double *) malloc( basnonz * sizeof(double) );
	if (!lp->basmatbeg || ! lp->basmatcount || !lp->basmatind 
		|| !lp->basmatcoeff )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}



	ind=0;
	for(k=0;k<lp->im;k++) {
		j = lp->basisheader[k];
		lp->basmatbeg[k] = ind;
		lp->basmatcount[k] = lp->imatcolcount[j];
		for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) {
			lp->basmatind[ind] = lp->imatcolind[l];
			lp->basmatcoeff[ind] =  lp->imatcolcoeff[l];
			ind++;
		}
	}



	/** Factor basis matrix */
	/*
	*  Here we compute an $LU$-factorization of the basis matrix. Later, we
	* should add here a fresh computation of |lp->x|.
	*/
	ffree( (char **) &pivrow);
	ffree( (char **) &pivcol);
	ffree( (char **) &pivvalue);
	ffree( (char **) &lp->rumatbeg);
	ffree( (char **) &lp->rumatend);
	ffree( (char **) &lp->rumatind);
	ffree( (char **) &lp->rumatcoeff);
	ffree( (char **) &lp->clmatbeg);
	ffree( (char **) &lp->clmatend);
	ffree( (char **) &lp->clmatind);
	ffree( (char **) &lp->clmatcoeff);
	ffree( (char **) &lp->cumatbeg);
	ffree( (char **) &lp->cumatend);
	ffree( (char **) &lp->cumatind);
	ffree( (char **) &lp->cumatcoeff);
#ifdef ALPS_TIMING_ENABLED
	starttime = cputime();
#endif
	status= alps_lufac (
		pril,
		(int) 0,
		lp->im,
		lp->basmatbeg,
		lp->basmatcount,
		lp->basmatind,
		lp->basmatcoeff,
		&pivrow,
		&pivcol,
		&pivvalue,
		&lp->rumatbeg,
		&lp->rumatend,
		&lp->rumatind,
		&lp->rumatcoeff,
		&lp->cumatbeg,
		&lp->cumatend,
		&lp->cumatind,
		&lp->cumatcoeff,
		&lp->clmatbeg,
		&lp->clmatend,
		&lp->clmatind,
		&lp->clmatcoeff,
		&totalfill,
		&totalcancel
		);
#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->factortime += (stoptime - starttime);
#endif
	if (status==ALPS_RUNOUTOFMEMORY) {
		fprintf(stderr,"LU factorization failed: run out of memory.\n");
		return ALPS_RUNOUTOFMEMORY;
	}
	else if (status) {
		fprintf(stderr,"LU factorization failed. Matrix singular.\n");
		return ALPS_LU_NOPIVOT;
	}
	lp->etanr = 0;
	lp->etafirstfree = 0;
	lp->numfactor++;
	lp->sumfill += ( (double) totalfill ) / ( (double) lp->im );


	/** Compute phase1 |x| and value from scratch */
	/*
	*  After each factorization in phase~1, we compute the solution vector
	* |lp->x| and the objective function value from scratch.
	*/
	for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
	for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) {
		switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		lp->x[j] = lp->ilowerbound[j];
		break;
	case ALPS_NONBASICUPB:
		lp->x[j] = lp->iupperbound[j];
		break;
	case ALPS_NONBASICFREE:
		lp->x[j] = 0.0;
		break;
		}
		for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			i = lp->imatcolind[k];
			trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
		}
	}
#ifdef ALPS_TIMING_ENABLED
	starttime = cputime();
#endif
	status = alps_fsolveeqs(
		lp,
		lp->im,
		pivrow,
		pivcol,
		pivvalue,
		lp->rumatbeg,
		lp->rumatend,
		lp->rumatind,
		lp->rumatcoeff,
		lp->clmatbeg,
		lp->clmatend,
		lp->clmatind,
		lp->clmatcoeff,
		lp->etanr,
		lp->etacol,
		lp->etaval,
		lp->etaind,
		lp->etastart,
		lp->etacount,
		lp->eta,
		trhs,
		xbas
		);

#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->fsolvetime += (stoptime - starttime);
#endif
	if (status==ALPS_RUNOUTOFMEMORY) {
		fprintf(stderr,"FSOLVE failed: run out of memory.\n");
		exit(1000);
	}
	lp->objval = 0.0;
	for (i=0; i<lp->im; i++) {
		j = lp->basisheader[i];
		lp->x[j] = xbas[i];
		if (lp->x[j]<lp->ilowerbound[j]) lp->x[j] = lp->ilowerbound[j];
		if (lp->x[j]>lp->iupperbound[j]) lp->x[j] = lp->iupperbound[j];
		if (lp->x[j]<lp->ioriglowerbound[j]-ALPS_ZEROEPS) {
			lp->objval -= lp->iobj[j]*(lp->ioriglowerbound[j]-lp->x[j]);
		}
		else if (lp->x[j]>lp->iorigupperbound[j]+ALPS_ZEROEPS) {
			lp->objval += lp->iobj[j]*(lp->x[j]-lp->iorigupperbound[j]);
		}
	}
	if (lp->objval>=-ALPS_EPS) {
		lp->problemstatus = ALPS_OPTIMAL;
		goto terminate_primal_phase1_simplex_loop;
	}


	/** Primal phase1 simplex loop */
	/*
	*  The primal simplex loop doesn't go forever (hopefully) since on
	* various termination conditions we jump to
	* |terminate_primal_simplex_loop|. The layout is textbook-like.
	*/
	oldobjval = 1.0E20;
	preobjval = -1.0E20;
	ageobjval = 0;
	degenage = 2;
	ageobjval = 0;
	degeneracy = ALPS_FALSE;
	wentfurther = ALPS_FALSE;
	objincr = 0;
	nriter = 0;
	while(ALPS_TRUE) {
start_primal_phase1_simplex_loop:
		// Internal constraint check

#ifdef CHECK
	{ int i,j,k,nviol;

	nviol = 0;
	for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
	for (j=0;j<lp->in;j++) {
		for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j]; k++) {
			i = lp->imatcolind[k];
			trhs[i] -=  lp->x[j]*lp->imatcolcoeff[k];
		}
	}
	for (i=0; i<lp->im; i++) { 
		if (trhs[i]>0.01) {
			printf("Constraint violation (%.4lf) in constraint %d (%s)!\n",
				trhs[i],i,lp->arownamestr+lp->arownameind[org_rownr[i]]);
			nviol++;
		}
	}
#ifdef EXIT
	if (nviol) {
		exit(1000);
	}
#endif
	}
#endif


	// Internal bound check
	/*
	*  Feasibility check for internal problem.
	*/
#ifdef CHECK
	{ int j,nviol;
	double viol;

	nviol = 0;
	for (j=0;j<lp->in;j++) {
		viol = lp->x[j] - lp->iupperbound[j];
		if (viol>0.0001) {
			printf("\nUpper bound violation (%.4lf>%.4lf) of variable %d",
				lp->x[j],lp->iupperbound[j],j);
			if (j<lp->inorig) printf(" (%s)",lp->acolnamestr+lp->acolnameind[j]);
			printf("!\n");
			nviol++;
		}
		viol = lp->ilowerbound[j] - lp->x[j];
		if (viol>0.0001) {
			printf("\nLower bound violation (%.4lf<%.4lf) of variable %d",
				lp->x[j],lp->ilowerbound[j],j);
			if (j<lp->inorig) printf(" (%s)",lp->acolnamestr+lp->acolnameind[j]);
			printf("!\n");
			nviol++;
		}
	}
#ifdef EXIT
	if (nviol) {
		exit(1000);
	}
#endif
	}
#endif


#ifdef ORIG
	if ((lp->etanr==0) || (wentfurther)) {
		// Compute multipliers
		/*
		*  The multipliers $y$, here |lp->y|, are found by solving the system
		* $y^TB=c_B$.
		*/
		for (k=0;k<lp->im;k++) cbas[k] = lp->iobj[lp->basisheader[k]];
#ifdef ALPS_TIMING_ENABLED
		starttime = cputime();
#endif
		status = alps_bsolveeqs(
			lp,
			lp->im,
			pivrow,
			pivcol,
			pivvalue,
			lp->cumatbeg,
			lp->cumatend,
			lp->cumatind,
			lp->cumatcoeff,
			lp->clmatbeg,
			lp->clmatend,
			lp->clmatind,
			lp->clmatcoeff,
			lp->etanr,
			lp->etacol,
			lp->etaval,
			lp->etaind,
			lp->etastart,
			lp->etacount,
			lp->eta,
			cbas,
			lp->y
			);

#ifdef ALPS_TIMING_ENABLED
		stoptime = cputime();
		lp->bsolvetime += (stoptime - starttime);
#endif
		if (status==ALPS_RUNOUTOFMEMORY) {
			fprintf(stderr,"BSOLVE failed: run out of memory.\n");
			exit(1000);
		}
		k = 0;
		for (l=0;l<lp->im;l++) {
			yl = lp->y[l];
			if ((yl<-ALPS_ZEROEPS)||(yl>ALPS_ZEROEPS)) {
				lp->ysind[k] = l;
				lp->ysval[k] = yl;
				k++;
			}
		}
		lp->yscount = k;
		if (pril>=4) {
			printf("\ncbas: "); for (i=0;i<lp->im;i++) printf(" %.2lf",cbas[i]);printf("\n");
			printf("y   : "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->y[i]);printf("\n");
		}


	}
#endif
	// Compute multipliers
	/*
	*  The multipliers $y$, here |lp->y|, are found by solving the system
	* $y^TB=c_B$.
	*/
	for (k=0;k<lp->im;k++) cbas[k] = lp->iobj[lp->basisheader[k]];
#ifdef ALPS_TIMING_ENABLED
	starttime = cputime();
#endif
	status = alps_bsolveeqs(
		lp,
		lp->im,
		pivrow,
		pivcol,
		pivvalue,
		lp->cumatbeg,
		lp->cumatend,
		lp->cumatind,
		lp->cumatcoeff,
		lp->clmatbeg,
		lp->clmatend,
		lp->clmatind,
		lp->clmatcoeff,
		lp->etanr,
		lp->etacol,
		lp->etaval,
		lp->etaind,
		lp->etastart,
		lp->etacount,
		lp->eta,
		cbas,
		lp->y
		);

#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->bsolvetime += (stoptime - starttime);
#endif
	if (status==ALPS_RUNOUTOFMEMORY) {
		fprintf(stderr,"BSOLVE failed: run out of memory.\n");
		exit(1000);
	}
	k = 0;
	for (l=0;l<lp->im;l++) {
		yl = lp->y[l];
		if ((yl<-ALPS_ZEROEPS)||(yl>ALPS_ZEROEPS)) {
			lp->ysind[k] = l;
			lp->ysval[k] = yl;
			k++;
		}
	}
	lp->yscount = k;
	if (pril>=4) {
		printf("\ncbas: "); for (i=0;i<lp->im;i++) printf(" %.2lf",cbas[i]);printf("\n");
		printf("y   : "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->y[i]);printf("\n");
	}


	// Make primal phase1 pivot selection
	/*
	*  For primal pivot selection, we call pisel or fulleasypisel, and then,
	* before the pivot, we compute $z$ by solving $z^TA_B=u^T$ for the unit
	* vector $u$ with a $1$ in the |leaveindex| position.
	*/
#ifdef ALPS_TIMING_ENABLED
	starttime = cputime();
#endif
	if (degeneracy) {
		status = alps_primalphase1randompisel(
			lp,
			pril,
			&enter,
			&leave,
			&leaveindex,
			&redcost,
			&change,
			pivrow,
			pivcol,
			pivvalue,
			&oldobjcoeff,
			&wentfurther
			);
	}
	else {
		status = alps_primalphase1easypisel(
			lp,
			pril,
			&enter,
			&leave,
			&leaveindex,
			&redcost,
			&change,
			pivrow,
			pivcol,
			pivvalue,
			&oldobjcoeff,
			&wentfurther
			);
	}
#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->piseltime += (stoptime - starttime);
#endif
	if (lp->problemstatus==ALPS_UNBOUNDED) {
		if (!lp->etanr) goto terminate_primal_phase1_simplex_loop;
		if (pril>=1) printf("ERROR: Phase 1 terminates with status UNBOUNDED.\n");
		if (pril>=1) printf("Try to recover by refactorizing.\n");
		// Construct basis matrix
		/*
		*  In order to construct the basis matrix, all we have to do is counting
		* the number of nonzeroes in the columns corresponding to the variables
		* listed in the |lp->basisheader|, and then allocating the right amount
		* of space, and copying the data.
		*/
		ffree( (char **) &lp->basmatbeg);
		ffree( (char **) &lp->basmatcount);
		ffree( (char **) &lp->basmatind);
		ffree( (char **) &lp->basmatcoeff);
		basnonz = 0;
		for(k=0;k<lp->im;k++) basnonz += lp->imatcolcount[lp->basisheader[k]];
		lp->basmatbeg = (int *) malloc( lp->im * sizeof(int) );
		lp->basmatcount = (int *) malloc( lp->im * sizeof(int) );
		lp->basmatind = (int *) malloc( basnonz * sizeof(int) );
		lp->basmatcoeff = (double *) malloc( basnonz * sizeof(double) );
		if (!lp->basmatbeg || ! lp->basmatcount || !lp->basmatind 
			|| !lp->basmatcoeff )
			// run out of memory

		{
			fprintf (stderr, "run out of memory\n");
			return(ALPS_RUNOUTOFMEMORY);
		}



		ind=0;
		for(k=0;k<lp->im;k++) {
			j = lp->basisheader[k];
			lp->basmatbeg[k] = ind;
			lp->basmatcount[k] = lp->imatcolcount[j];
			for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) {
				lp->basmatind[ind] = lp->imatcolind[l];
				lp->basmatcoeff[ind] =  lp->imatcolcoeff[l];
				ind++;
			}
		}



		// Factor basis matrix
		/*
		*  Here we compute an $LU$-factorization of the basis matrix. Later, we
		* should add here a fresh computation of |lp->x|.
		*/
		ffree( (char **) &pivrow);
		ffree( (char **) &pivcol);
		ffree( (char **) &pivvalue);
		ffree( (char **) &lp->rumatbeg);
		ffree( (char **) &lp->rumatend);
		ffree( (char **) &lp->rumatind);
		ffree( (char **) &lp->rumatcoeff);
		ffree( (char **) &lp->clmatbeg);
		ffree( (char **) &lp->clmatend);
		ffree( (char **) &lp->clmatind);
		ffree( (char **) &lp->clmatcoeff);
		ffree( (char **) &lp->cumatbeg);
		ffree( (char **) &lp->cumatend);
		ffree( (char **) &lp->cumatind);
		ffree( (char **) &lp->cumatcoeff);
#ifdef ALPS_TIMING_ENABLED
		starttime = cputime();
#endif
		status= alps_lufac (
			pril,
			(int) 0,
			lp->im,
			lp->basmatbeg,
			lp->basmatcount,
			lp->basmatind,
			lp->basmatcoeff,
			&pivrow,
			&pivcol,
			&pivvalue,
			&lp->rumatbeg,
			&lp->rumatend,
			&lp->rumatind,
			&lp->rumatcoeff,
			&lp->cumatbeg,
			&lp->cumatend,
			&lp->cumatind,
			&lp->cumatcoeff,
			&lp->clmatbeg,
			&lp->clmatend,
			&lp->clmatind,
			&lp->clmatcoeff,
			&totalfill,
			&totalcancel
			);
#ifdef ALPS_TIMING_ENABLED
		stoptime = cputime();
		lp->factortime += (stoptime - starttime);
#endif
		if (status==ALPS_RUNOUTOFMEMORY) {
			fprintf(stderr,"LU factorization failed: run out of memory.\n");
			return ALPS_RUNOUTOFMEMORY;
		}
		else if (status) {
			fprintf(stderr,"LU factorization failed. Matrix singular.\n");
			return ALPS_LU_NOPIVOT;
		}
		lp->etanr = 0;
		lp->etafirstfree = 0;
		lp->numfactor++;
		lp->sumfill += ( (double) totalfill ) / ( (double) lp->im );


		// Compute phase1 |x| and value from scratch
		/*
		*  After each factorization in phase~1, we compute the solution vector
		* |lp->x| and the objective function value from scratch.
		*/
		for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
		for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) {
			switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		lp->x[j] = lp->ilowerbound[j];
		break;
	case ALPS_NONBASICUPB:
		lp->x[j] = lp->iupperbound[j];
		break;
	case ALPS_NONBASICFREE:
		lp->x[j] = 0.0;
		break;
			}
			for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				i = lp->imatcolind[k];
				trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
			}
		}
#ifdef ALPS_TIMING_ENABLED
		starttime = cputime();
#endif
		status = alps_fsolveeqs(
			lp,
			lp->im,
			pivrow,
			pivcol,
			pivvalue,
			lp->rumatbeg,
			lp->rumatend,
			lp->rumatind,
			lp->rumatcoeff,
			lp->clmatbeg,
			lp->clmatend,
			lp->clmatind,
			lp->clmatcoeff,
			lp->etanr,
			lp->etacol,
			lp->etaval,
			lp->etaind,
			lp->etastart,
			lp->etacount,
			lp->eta,
			trhs,
			xbas
			);

#ifdef ALPS_TIMING_ENABLED
		stoptime = cputime();
		lp->fsolvetime += (stoptime - starttime);
#endif
		if (status==ALPS_RUNOUTOFMEMORY) {
			fprintf(stderr,"FSOLVE failed: run out of memory.\n");
			exit(1000);
		}
		lp->objval = 0.0;
		for (i=0; i<lp->im; i++) {
			j = lp->basisheader[i];
			lp->x[j] = xbas[i];
			if (lp->x[j]<lp->ilowerbound[j]) lp->x[j] = lp->ilowerbound[j];
			if (lp->x[j]>lp->iupperbound[j]) lp->x[j] = lp->iupperbound[j];
			if (lp->x[j]<lp->ioriglowerbound[j]-ALPS_ZEROEPS) {
				lp->objval -= lp->iobj[j]*(lp->ioriglowerbound[j]-lp->x[j]);
			}
			else if (lp->x[j]>lp->iorigupperbound[j]+ALPS_ZEROEPS) {
				lp->objval += lp->iobj[j]*(lp->x[j]-lp->iorigupperbound[j]);
			}
		}
		if (lp->objval>=-ALPS_EPS) {
			lp->problemstatus = ALPS_OPTIMAL;
			goto terminate_primal_phase1_simplex_loop;
		}


		goto start_primal_phase1_simplex_loop;
	}
	else if (lp->problemstatus==ALPS_OPTIMAL) {
		if (!lp->etanr) goto terminate_primal_phase1_simplex_loop;
		else {
			// Construct basis matrix
			/*
			*  In order to construct the basis matrix, all we have to do is counting
			* the number of nonzeroes in the columns corresponding to the variables
			* listed in the |lp->basisheader|, and then allocating the right amount
			* of space, and copying the data.
			*/
			ffree( (char **) &lp->basmatbeg);
			ffree( (char **) &lp->basmatcount);
			ffree( (char **) &lp->basmatind);
			ffree( (char **) &lp->basmatcoeff);
			basnonz = 0;
			for(k=0;k<lp->im;k++) basnonz += lp->imatcolcount[lp->basisheader[k]];
			lp->basmatbeg = (int *) malloc( lp->im * sizeof(int) );
			lp->basmatcount = (int *) malloc( lp->im * sizeof(int) );
			lp->basmatind = (int *) malloc( basnonz * sizeof(int) );
			lp->basmatcoeff = (double *) malloc( basnonz * sizeof(double) );
			if (!lp->basmatbeg || ! lp->basmatcount || !lp->basmatind 
				|| !lp->basmatcoeff )
				// run out of memory

			{
				fprintf (stderr, "run out of memory\n");
				return(ALPS_RUNOUTOFMEMORY);
			}



			ind=0;
			for(k=0;k<lp->im;k++) {
				j = lp->basisheader[k];
				lp->basmatbeg[k] = ind;
				lp->basmatcount[k] = lp->imatcolcount[j];
				for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) {
					lp->basmatind[ind] = lp->imatcolind[l];
					lp->basmatcoeff[ind] =  lp->imatcolcoeff[l];
					ind++;
				}
			}



			// Factor basis matrix
			/*
			*  Here we compute an $LU$-factorization of the basis matrix. Later, we
			* should add here a fresh computation of |lp->x|.
			*/
			ffree( (char **) &pivrow);
			ffree( (char **) &pivcol);
			ffree( (char **) &pivvalue);
			ffree( (char **) &lp->rumatbeg);
			ffree( (char **) &lp->rumatend);
			ffree( (char **) &lp->rumatind);
			ffree( (char **) &lp->rumatcoeff);
			ffree( (char **) &lp->clmatbeg);
			ffree( (char **) &lp->clmatend);
			ffree( (char **) &lp->clmatind);
			ffree( (char **) &lp->clmatcoeff);
			ffree( (char **) &lp->cumatbeg);
			ffree( (char **) &lp->cumatend);
			ffree( (char **) &lp->cumatind);
			ffree( (char **) &lp->cumatcoeff);
#ifdef ALPS_TIMING_ENABLED
			starttime = cputime();
#endif
			status= alps_lufac (
				pril,
				(int) 0,
				lp->im,
				lp->basmatbeg,
				lp->basmatcount,
				lp->basmatind,
				lp->basmatcoeff,
				&pivrow,
				&pivcol,
				&pivvalue,
				&lp->rumatbeg,
				&lp->rumatend,
				&lp->rumatind,
				&lp->rumatcoeff,
				&lp->cumatbeg,
				&lp->cumatend,
				&lp->cumatind,
				&lp->cumatcoeff,
				&lp->clmatbeg,
				&lp->clmatend,
				&lp->clmatind,
				&lp->clmatcoeff,
				&totalfill,
				&totalcancel
				);
#ifdef ALPS_TIMING_ENABLED
			stoptime = cputime();
			lp->factortime += (stoptime - starttime);
#endif
			if (status==ALPS_RUNOUTOFMEMORY) {
				fprintf(stderr,"LU factorization failed: run out of memory.\n");
				return ALPS_RUNOUTOFMEMORY;
			}
			else if (status) {
				fprintf(stderr,"LU factorization failed. Matrix singular.\n");
				return ALPS_LU_NOPIVOT;
			}
			lp->etanr = 0;
			lp->etafirstfree = 0;
			lp->numfactor++;
			lp->sumfill += ( (double) totalfill ) / ( (double) lp->im );


			if (pril>=3) printf("=== Refactorization performed. ===\n");
			// Compute phase1 |x| and value from scratch
			/*
			*  After each factorization in phase~1, we compute the solution vector
			* |lp->x| and the objective function value from scratch.
			*/
			for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
			for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) {
				switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		lp->x[j] = lp->ilowerbound[j];
		break;
	case ALPS_NONBASICUPB:
		lp->x[j] = lp->iupperbound[j];
		break;
	case ALPS_NONBASICFREE:
		lp->x[j] = 0.0;
		break;
				}
				for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
					i = lp->imatcolind[k];
					trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
				}
			}
#ifdef ALPS_TIMING_ENABLED
			starttime = cputime();
#endif
			status = alps_fsolveeqs(
				lp,
				lp->im,
				pivrow,
				pivcol,
				pivvalue,
				lp->rumatbeg,
				lp->rumatend,
				lp->rumatind,
				lp->rumatcoeff,
				lp->clmatbeg,
				lp->clmatend,
				lp->clmatind,
				lp->clmatcoeff,
				lp->etanr,
				lp->etacol,
				lp->etaval,
				lp->etaind,
				lp->etastart,
				lp->etacount,
				lp->eta,
				trhs,
				xbas
				);

#ifdef ALPS_TIMING_ENABLED
			stoptime = cputime();
			lp->fsolvetime += (stoptime - starttime);
#endif
			if (status==ALPS_RUNOUTOFMEMORY) {
				fprintf(stderr,"FSOLVE failed: run out of memory.\n");
				exit(1000);
			}
			lp->objval = 0.0;
			for (i=0; i<lp->im; i++) {
				j = lp->basisheader[i];
				lp->x[j] = xbas[i];
				if (lp->x[j]<lp->ilowerbound[j]) lp->x[j] = lp->ilowerbound[j];
				if (lp->x[j]>lp->iupperbound[j]) lp->x[j] = lp->iupperbound[j];
				if (lp->x[j]<lp->ioriglowerbound[j]-ALPS_ZEROEPS) {
					lp->objval -= lp->iobj[j]*(lp->ioriglowerbound[j]-lp->x[j]);
				}
				else if (lp->x[j]>lp->iorigupperbound[j]+ALPS_ZEROEPS) {
					lp->objval += lp->iobj[j]*(lp->x[j]-lp->iorigupperbound[j]);
				}
			}
			if (lp->objval>=-ALPS_EPS) {
				lp->problemstatus = ALPS_OPTIMAL;
				goto terminate_primal_phase1_simplex_loop;
			}


			if (pril>=4) printprimalsol(lp);
			goto start_primal_phase1_simplex_loop;
		}
	}
	else if (leaveindex>=0) {
#ifdef TTT
		if (fabs(change)<1.0e-7 && lp->etanr) {
			printf("\nsmall change at iter %1d: %.3le (%1d)\n",nriter,change,lp->etanr);
			// Construct basis matrix
			/*
			*  In order to construct the basis matrix, all we have to do is counting
			* the number of nonzeroes in the columns corresponding to the variables
			* listed in the |lp->basisheader|, and then allocating the right amount
			* of space, and copying the data.
			*/
			ffree( (char **) &lp->basmatbeg);
			ffree( (char **) &lp->basmatcount);
			ffree( (char **) &lp->basmatind);
			ffree( (char **) &lp->basmatcoeff);
			basnonz = 0;
			for(k=0;k<lp->im;k++) basnonz += lp->imatcolcount[lp->basisheader[k]];
			lp->basmatbeg = (int *) malloc( lp->im * sizeof(int) );
			lp->basmatcount = (int *) malloc( lp->im * sizeof(int) );
			lp->basmatind = (int *) malloc( basnonz * sizeof(int) );
			lp->basmatcoeff = (double *) malloc( basnonz * sizeof(double) );
			if (!lp->basmatbeg || ! lp->basmatcount || !lp->basmatind 
				|| !lp->basmatcoeff )
				// run out of memory

			{
				fprintf (stderr, "run out of memory\n");
				return(ALPS_RUNOUTOFMEMORY);
			}



			ind=0;
			for(k=0;k<lp->im;k++) {
				j = lp->basisheader[k];
				lp->basmatbeg[k] = ind;
				lp->basmatcount[k] = lp->imatcolcount[j];
				for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) {
					lp->basmatind[ind] = lp->imatcolind[l];
					lp->basmatcoeff[ind] =  lp->imatcolcoeff[l];
					ind++;
				}
			}



			// Factor basis matrix
			/*
			*  Here we compute an $LU$-factorization of the basis matrix. Later, we
			* should add here a fresh computation of |lp->x|.
			*/
			ffree( (char **) &pivrow);
			ffree( (char **) &pivcol);
			ffree( (char **) &pivvalue);
			ffree( (char **) &lp->rumatbeg);
			ffree( (char **) &lp->rumatend);
			ffree( (char **) &lp->rumatind);
			ffree( (char **) &lp->rumatcoeff);
			ffree( (char **) &lp->clmatbeg);
			ffree( (char **) &lp->clmatend);
			ffree( (char **) &lp->clmatind);
			ffree( (char **) &lp->clmatcoeff);
			ffree( (char **) &lp->cumatbeg);
			ffree( (char **) &lp->cumatend);
			ffree( (char **) &lp->cumatind);
			ffree( (char **) &lp->cumatcoeff);
#ifdef ALPS_TIMING_ENABLED
			starttime = cputime();
#endif
			status= alps_lufac (
				pril,
				(int) 0,
				lp->im,
				lp->basmatbeg,
				lp->basmatcount,
				lp->basmatind,
				lp->basmatcoeff,
				&pivrow,
				&pivcol,
				&pivvalue,
				&lp->rumatbeg,
				&lp->rumatend,
				&lp->rumatind,
				&lp->rumatcoeff,
				&lp->cumatbeg,
				&lp->cumatend,
				&lp->cumatind,
				&lp->cumatcoeff,
				&lp->clmatbeg,
				&lp->clmatend,
				&lp->clmatind,
				&lp->clmatcoeff,
				&totalfill,
				&totalcancel
				);
#ifdef ALPS_TIMING_ENABLED
			stoptime = cputime();
			lp->factortime += (stoptime - starttime);
#endif
			if (status==ALPS_RUNOUTOFMEMORY) {
				fprintf(stderr,"LU factorization failed: run out of memory.\n");
				return ALPS_RUNOUTOFMEMORY;
			}
			else if (status) {
				fprintf(stderr,"LU factorization failed. Matrix singular.\n");
				return ALPS_LU_NOPIVOT;
			}
			lp->etanr = 0;
			lp->etafirstfree = 0;
			lp->numfactor++;
			lp->sumfill += ( (double) totalfill ) / ( (double) lp->im );


			// Compute phase1 |x| and value from scratch
			/*
			*  After each factorization in phase~1, we compute the solution vector
			* |lp->x| and the objective function value from scratch.
			*/
			for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
			for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) {
				switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		lp->x[j] = lp->ilowerbound[j];
		break;
	case ALPS_NONBASICUPB:
		lp->x[j] = lp->iupperbound[j];
		break;
	case ALPS_NONBASICFREE:
		lp->x[j] = 0.0;
		break;
				}
				for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
					i = lp->imatcolind[k];
					trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
				}
			}
#ifdef ALPS_TIMING_ENABLED
			starttime = cputime();
#endif
			status = alps_fsolveeqs(
				lp,
				lp->im,
				pivrow,
				pivcol,
				pivvalue,
				lp->rumatbeg,
				lp->rumatend,
				lp->rumatind,
				lp->rumatcoeff,
				lp->clmatbeg,
				lp->clmatend,
				lp->clmatind,
				lp->clmatcoeff,
				lp->etanr,
				lp->etacol,
				lp->etaval,
				lp->etaind,
				lp->etastart,
				lp->etacount,
				lp->eta,
				trhs,
				xbas
				);

#ifdef ALPS_TIMING_ENABLED
			stoptime = cputime();
			lp->fsolvetime += (stoptime - starttime);
#endif
			if (status==ALPS_RUNOUTOFMEMORY) {
				fprintf(stderr,"FSOLVE failed: run out of memory.\n");
				exit(1000);
			}
			lp->objval = 0.0;
			for (i=0; i<lp->im; i++) {
				j = lp->basisheader[i];
				lp->x[j] = xbas[i];
				if (lp->x[j]<lp->ilowerbound[j]) lp->x[j] = lp->ilowerbound[j];
				if (lp->x[j]>lp->iupperbound[j]) lp->x[j] = lp->iupperbound[j];
				if (lp->x[j]<lp->ioriglowerbound[j]-ALPS_ZEROEPS) {
					lp->objval -= lp->iobj[j]*(lp->ioriglowerbound[j]-lp->x[j]);
				}
				else if (lp->x[j]>lp->iorigupperbound[j]+ALPS_ZEROEPS) {
					lp->objval += lp->iobj[j]*(lp->x[j]-lp->iorigupperbound[j]);
				}
			}
			if (lp->objval>=-ALPS_EPS) {
				lp->problemstatus = ALPS_OPTIMAL;
				goto terminate_primal_phase1_simplex_loop;
			}


			goto start_primal_phase1_simplex_loop;
		}
#endif
		if (pril>=4) printf("Leaving variable: %d [change = %.2lf]\n",leave,change);
		lp->u[leaveindex] = 1.0;
#ifdef ALPS_TIMING_ENABLED
		starttime = cputime();
#endif
		status = alps_bsolveeqs(
			lp,
			lp->im,
			pivrow,
			pivcol,
			pivvalue,
			lp->cumatbeg,
			lp->cumatend,
			lp->cumatind,
			lp->cumatcoeff,
			lp->clmatbeg,
			lp->clmatend,
			lp->clmatind,
			lp->clmatcoeff,
			lp->etanr,
			lp->etacol,
			lp->etaval,
			lp->etaind,
			lp->etastart,
			lp->etacount,
			lp->eta,
			lp->u,
			lp->z
			);

#ifdef ALPS_TIMING_ENABLED
		stoptime = cputime();
		lp->bsolvetime += (stoptime - starttime);
#endif
		if (status==ALPS_RUNOUTOFMEMORY) {
			fprintf(stderr,"BSOLVE failed: run out of memory.\n");
			exit(1000);
		}
		k = 0;
		for (l=0;l<lp->im;l++) {
			zl = lp->z[l];
			if ((zl<-ALPS_ZEROEPS)||(zl>ALPS_ZEROEPS)) {
				lp->zsind[k] = l;
				lp->zsval[k] = zl;
				k++;
			}
		}
		lp->zscount = k;
		if (pril>=4) {
			printf("\nu: "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->u[i]);printf("\n");
			printf("z: "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->z[i]);printf("\n");
		}

		lp->u[leaveindex] = 0.0;
	}
	else {
		leave = enter;
		if (pril>=3) printf("Variable %d changing bounds [%.2lf units]\n",leave,change);
	}

	if (pril>=4) {
		for (k=0;k<lp->im;k++) entcol[k] = 0.0;
		for (l=lp->imatcolbeg[enter];l<lp->imatcolbeg[enter]+lp->imatcolcount[enter];l++) 
			entcol[lp->imatcolind[l]] = lp->imatcolcoeff[l];
		printf("\nentcol: "); for (i=0;i<lp->im;i++) printf(" %.2lf",entcol[i]);printf("\n");
		printf("d     : "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->d[i]);printf("\n");
	}


	// Basis update (primal phase1)
	/*
	*  Here we update the basis information. If there is no real pivot
	* |leave==enter|, the ``entering/leaving'' just switches bounds which is
	* recorded in |lp->colstat[enter]|. Otherwise, the entering variable
	* takes the leaving variable's place in the |lp->basisheader| and is
	* declared basic. A variable leaving the basis goes to one of the
	* bounds. (Free basic variables never leave.) If we are in phase 1 and
	* there were artificial bounds, the original bounds are restored and the
	* phase 1 objective function coefficient is set to zero.
	*/
	enr = lp->etanr;
	if (((!enr)&&(pril==1))||(pril>=2)) printf("%6d:   ",nriter);
	for(l=0;l<lp->dscount;l++) {
		dirindl = lp->dsind[l];
		dirvall = lp->dsval[l];
		basisvariable = lp->basisheader[dirindl];
		oldx = lp->x[basisvariable];
		newx = oldx - change*dirvall;
		// Adjust the new |x| value

		lb = lp->ilowerbound[basisvariable];
		lbviol = lb - newx;
		if (lbviol>0.0) { /* is this good or not ? */
			if (lbviol>0.001) 
				printf("\nStrong lower bound violation of basic variable %d: %lf < %lf!\n",
				basisvariable,newx,lb);
			newx = lb;
		}
		ub = lp->iupperbound[basisvariable];
		ubviol = newx - ub;
		if (ubviol>0.0) { /* is this good or not ? */
			if (lbviol>0.001) 
				printf("\nStrong upper bound violation of basic variable %d: %lf > %lf!\n",
				basisvariable,newx,ub);
			newx = ub;
		}


		lp->x[basisvariable] = newx;
		if (lp->iartifbounds[basisvariable]) {
			lp->objval -= (oldx-newx)*lp->iobj[basisvariable];
		}
		if ((basisvariable!=leave)&&(lp->iartifbounds[basisvariable])&&(wentfurther)) {
			if ((newx<=lp->iorigupperbound[basisvariable]+ALPS_ZEROEPS)&&
				(newx>=lp->ioriglowerbound[basisvariable]-ALPS_ZEROEPS)) {
					lp->iupperbound[basisvariable] = lp->iorigupperbound[basisvariable];
					lp->ilowerbound[basisvariable] = lp->ioriglowerbound[basisvariable];
					lp->varstat[basisvariable] = lp->origvarstat[basisvariable];
					oldobjcoeff = lp->iphase1obj[basisvariable];
					lp->objval -= oldobjcoeff*lp->x[basisvariable];
					lp->iphase1obj[basisvariable] = 0.0;
					lp->iartifbounds[basisvariable] = ALPS_FALSE;
				}
		}
	}
	basisvariable = enter;
	oldx = lp->x[enter];
	newx = oldx + change;
	// Adjust the new |x| value

	lb = lp->ilowerbound[basisvariable];
	lbviol = lb - newx;
	if (lbviol>0.0) { /* is this good or not ? */
		if (lbviol>0.001) 
			printf("\nStrong lower bound violation of basic variable %d: %lf < %lf!\n",
			basisvariable,newx,lb);
		newx = lb;
	}
	ub = lp->iupperbound[basisvariable];
	ubviol = newx - ub;
	if (ubviol>0.0) { /* is this good or not ? */
		if (lbviol>0.001) 
			printf("\nStrong upper bound violation of basic variable %d: %lf > %lf!\n",
			basisvariable,newx,ub);
		newx = ub;
	}


	lp->x[basisvariable] = newx;
	if (leave==enter) {
		if (lp->colstat[enter]==ALPS_NONBASICLOWB) {
			lp->colstat[enter]=ALPS_NONBASICUPB;
			lp->x[enter]=lp->iupperbound[enter];
			if (((!enr)&&(pril==1))||(pril>=2)) {
				if (enter<lp->inorig) printf("x");
				else if (enter<lp->inonartif) printf("s");
				else printf("a");
				printf("[%4d] LB -> UB            ",enter);
			}
		}
		else {
			lp->colstat[enter]=ALPS_NONBASICLOWB;
			lp->x[enter]=lp->ilowerbound[enter];
			if (((!enr)&&(pril==1))||(pril>=2)) {
				if (enter<lp->inorig) printf("x");
				else if (enter<lp->inonartif) printf("s");
				else printf("a");
				printf("[%4d] UB -> LB            ",enter);
			}
		}
	}
	else {
		if (((!enr)&&(pril==1))||(pril>=2)) {
			if (leave<lp->inorig) printf("x");
			else if (leave<lp->inonartif) printf("s");
			else printf("a");
			printf("[%4d] -> ",leave);
			if (enter<lp->inorig) printf("x");
			else if (enter<lp->inonartif) printf("s");
			else printf("a");
			printf("[%4d] ",enter);
		}
		lp->basisheader[leaveindex] = enter;
		lp->colstat[enter]=ALPS_BASIC;

		eff = lp->etafirstfree;
		dsc = lp->dscount;
		enr = lp->etanr;
		lp->etacol[enr] = leaveindex;
		lp->etastart[enr] = eff;
		lp->etacount[enr] = dsc - 1;
		for (l=0; l<dsc; l++) {
			dsindl = lp->dsind[l];
			dsvall = lp->dsval[l];
			if (dsindl == leaveindex) {
				lp->eta[enr] = dsvall;
				if (((!enr)&&(pril==1))||(pril>=2)) printf(" %8.1le",dsvall);
			}
			else {
				lp->etaind[eff] = dsindl;
				lp->etaval[eff] = dsvall;
				eff++;
			}
		}
		lp->etanr = enr + 1;
		lp->etafirstfree = eff;
		oldobjcoeff = 0.0;
		if (leave>=lp->inonartif) { /* artificial */
			lp->ilowerbound[leave] = 0.0;
			lp->iupperbound[leave] = 0.0;
			lp->x[leave] = 0.0;
			lp->varstat[leave] = lp->origvarstat[leave];
			oldobjcoeff = lp->iphase1obj[leave];
			lp->iphase1obj[leave] = 0.0;
			lp->colstat[leave] = ALPS_NONBASICLOWB;
		}
		else if (leave>=lp->inorig) { /* slack */
			lp->ilowerbound[leave] = 0.0;
			lp->iupperbound[leave] = ALPS_REAL_INFINITY;
			lp->x[leave] = 0.0;
			lp->varstat[leave] = lp->origvarstat[leave];
			oldobjcoeff = lp->iphase1obj[leave];
			lp->iphase1obj[leave] = 0.0;
			lp->colstat[leave] = ALPS_NONBASICLOWB;
		}
		else { /* normal */
			if (lp->iartifbounds[leave]) {
				lp->iupperbound[leave] = lp->iorigupperbound[leave];
				lp->ilowerbound[leave] = lp->ioriglowerbound[leave];
				lp->varstat[leave] = lp->origvarstat[leave];
				oldobjcoeff = lp->iphase1obj[leave];
				lp->iphase1obj[leave] = 0.0;
				lp->iartifbounds[leave] = ALPS_FALSE;
			}
			if (lp->varstat[leave]==ALPS_BOUNDEDABOVE) {
				lp->colstat[leave]=ALPS_NONBASICUPB;
				lp->x[leave] = lp->iupperbound[leave];
			}
			else if (lp->varstat[leave]==ALPS_BOUNDEDBELOW) {
				lp->colstat[leave]=ALPS_NONBASICLOWB;
				lp->x[leave] = lp->ilowerbound[leave];
			}
			else if (lp->varstat[leave]==ALPS_BOUNDEDTWICE) {
				absdifflb = fabs(lp->x[leave]-lp->ilowerbound[leave]);
				absdiffub = fabs(lp->x[leave]-lp->iupperbound[leave]);
				if (absdifflb<absdiffub) {
					lp->colstat[leave]=ALPS_NONBASICLOWB;
					lp->x[leave] = lp->ilowerbound[leave];
				}
				else {
					lp->colstat[leave]=ALPS_NONBASICUPB;
					lp->x[leave] = lp->iupperbound[leave];
				}
			}
			else if (lp->varstat[leave]==ALPS_FIXED) {
				lp->colstat[leave]=ALPS_NONBASICLOWB;
				lp->x[leave] = lp->ilowerbound[leave];
			}    
			else if (lp->varstat[leave]==ALPS_FREE) {
				lp->colstat[leave]=ALPS_NONBASICFREE;
				newx = lp->x[leave];
				printf("\nNew nonbasic free variable with value %lf!\n",newx);
			}
		}
	}

	if ((!enr&&(pril==1))||(pril>=2)) printf(" %15.4lf *\n",-lp->objval);


	if (lp->objval>=-ALPS_EPS) {
		// Compute phase1 |x| and value from scratch
		/*
		*  After each factorization in phase~1, we compute the solution vector
		* |lp->x| and the objective function value from scratch.
		*/
		for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
		for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) {
			switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		lp->x[j] = lp->ilowerbound[j];
		break;
	case ALPS_NONBASICUPB:
		lp->x[j] = lp->iupperbound[j];
		break;
	case ALPS_NONBASICFREE:
		lp->x[j] = 0.0;
		break;
			}
			for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				i = lp->imatcolind[k];
				trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
			}
		}
#ifdef ALPS_TIMING_ENABLED
		starttime = cputime();
#endif
		status = alps_fsolveeqs(
			lp,
			lp->im,
			pivrow,
			pivcol,
			pivvalue,
			lp->rumatbeg,
			lp->rumatend,
			lp->rumatind,
			lp->rumatcoeff,
			lp->clmatbeg,
			lp->clmatend,
			lp->clmatind,
			lp->clmatcoeff,
			lp->etanr,
			lp->etacol,
			lp->etaval,
			lp->etaind,
			lp->etastart,
			lp->etacount,
			lp->eta,
			trhs,
			xbas
			);

#ifdef ALPS_TIMING_ENABLED
		stoptime = cputime();
		lp->fsolvetime += (stoptime - starttime);
#endif
		if (status==ALPS_RUNOUTOFMEMORY) {
			fprintf(stderr,"FSOLVE failed: run out of memory.\n");
			exit(1000);
		}
		lp->objval = 0.0;
		for (i=0; i<lp->im; i++) {
			j = lp->basisheader[i];
			lp->x[j] = xbas[i];
			if (lp->x[j]<lp->ilowerbound[j]) lp->x[j] = lp->ilowerbound[j];
			if (lp->x[j]>lp->iupperbound[j]) lp->x[j] = lp->iupperbound[j];
			if (lp->x[j]<lp->ioriglowerbound[j]-ALPS_ZEROEPS) {
				lp->objval -= lp->iobj[j]*(lp->ioriglowerbound[j]-lp->x[j]);
			}
			else if (lp->x[j]>lp->iorigupperbound[j]+ALPS_ZEROEPS) {
				lp->objval += lp->iobj[j]*(lp->x[j]-lp->iorigupperbound[j]);
			}
		}
		if (lp->objval>=-ALPS_EPS) {
			lp->problemstatus = ALPS_OPTIMAL;
			goto terminate_primal_phase1_simplex_loop;
		}


	}
	if (pril>=4) printprimalsol(lp);
	nriter++;
	if (lp->etanr>=lp->etaclear) {
		// Construct basis matrix
		/*
		*  In order to construct the basis matrix, all we have to do is counting
		* the number of nonzeroes in the columns corresponding to the variables
		* listed in the |lp->basisheader|, and then allocating the right amount
		* of space, and copying the data.
		*/
		ffree( (char **) &lp->basmatbeg);
		ffree( (char **) &lp->basmatcount);
		ffree( (char **) &lp->basmatind);
		ffree( (char **) &lp->basmatcoeff);
		basnonz = 0;
		for(k=0;k<lp->im;k++) basnonz += lp->imatcolcount[lp->basisheader[k]];
		lp->basmatbeg = (int *) malloc( lp->im * sizeof(int) );
		lp->basmatcount = (int *) malloc( lp->im * sizeof(int) );
		lp->basmatind = (int *) malloc( basnonz * sizeof(int) );
		lp->basmatcoeff = (double *) malloc( basnonz * sizeof(double) );
		if (!lp->basmatbeg || ! lp->basmatcount || !lp->basmatind 
			|| !lp->basmatcoeff )
			// run out of memory

		{
			fprintf (stderr, "run out of memory\n");
			return(ALPS_RUNOUTOFMEMORY);
		}



		ind=0;
		for(k=0;k<lp->im;k++) {
			j = lp->basisheader[k];
			lp->basmatbeg[k] = ind;
			lp->basmatcount[k] = lp->imatcolcount[j];
			for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) {
				lp->basmatind[ind] = lp->imatcolind[l];
				lp->basmatcoeff[ind] =  lp->imatcolcoeff[l];
				ind++;
			}
		}



		// Factor basis matrix
		/*
		*  Here we compute an $LU$-factorization of the basis matrix. Later, we
		* should add here a fresh computation of |lp->x|.
		*/
		ffree( (char **) &pivrow);
		ffree( (char **) &pivcol);
		ffree( (char **) &pivvalue);
		ffree( (char **) &lp->rumatbeg);
		ffree( (char **) &lp->rumatend);
		ffree( (char **) &lp->rumatind);
		ffree( (char **) &lp->rumatcoeff);
		ffree( (char **) &lp->clmatbeg);
		ffree( (char **) &lp->clmatend);
		ffree( (char **) &lp->clmatind);
		ffree( (char **) &lp->clmatcoeff);
		ffree( (char **) &lp->cumatbeg);
		ffree( (char **) &lp->cumatend);
		ffree( (char **) &lp->cumatind);
		ffree( (char **) &lp->cumatcoeff);
#ifdef ALPS_TIMING_ENABLED
		starttime = cputime();
#endif
		status= alps_lufac (
			pril,
			(int) 0,
			lp->im,
			lp->basmatbeg,
			lp->basmatcount,
			lp->basmatind,
			lp->basmatcoeff,
			&pivrow,
			&pivcol,
			&pivvalue,
			&lp->rumatbeg,
			&lp->rumatend,
			&lp->rumatind,
			&lp->rumatcoeff,
			&lp->cumatbeg,
			&lp->cumatend,
			&lp->cumatind,
			&lp->cumatcoeff,
			&lp->clmatbeg,
			&lp->clmatend,
			&lp->clmatind,
			&lp->clmatcoeff,
			&totalfill,
			&totalcancel
			);
#ifdef ALPS_TIMING_ENABLED
		stoptime = cputime();
		lp->factortime += (stoptime - starttime);
#endif
		if (status==ALPS_RUNOUTOFMEMORY) {
			fprintf(stderr,"LU factorization failed: run out of memory.\n");
			return ALPS_RUNOUTOFMEMORY;
		}
		else if (status) {
			fprintf(stderr,"LU factorization failed. Matrix singular.\n");
			return ALPS_LU_NOPIVOT;
		}
		lp->etanr = 0;
		lp->etafirstfree = 0;
		lp->numfactor++;
		lp->sumfill += ( (double) totalfill ) / ( (double) lp->im );


		if (pril>=3) 
			printf("=== Refactorization performed (%1d/%1d). ===\n",lp->im,totalfill);
		// Compute phase1 |x| and value from scratch
		/*
		*  After each factorization in phase~1, we compute the solution vector
		* |lp->x| and the objective function value from scratch.
		*/
		for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
		for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) {
			switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		lp->x[j] = lp->ilowerbound[j];
		break;
	case ALPS_NONBASICUPB:
		lp->x[j] = lp->iupperbound[j];
		break;
	case ALPS_NONBASICFREE:
		lp->x[j] = 0.0;
		break;
			}
			for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				i = lp->imatcolind[k];
				trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
			}
		}
#ifdef ALPS_TIMING_ENABLED
		starttime = cputime();
#endif
		status = alps_fsolveeqs(
			lp,
			lp->im,
			pivrow,
			pivcol,
			pivvalue,
			lp->rumatbeg,
			lp->rumatend,
			lp->rumatind,
			lp->rumatcoeff,
			lp->clmatbeg,
			lp->clmatend,
			lp->clmatind,
			lp->clmatcoeff,
			lp->etanr,
			lp->etacol,
			lp->etaval,
			lp->etaind,
			lp->etastart,
			lp->etacount,
			lp->eta,
			trhs,
			xbas
			);

#ifdef ALPS_TIMING_ENABLED
		stoptime = cputime();
		lp->fsolvetime += (stoptime - starttime);
#endif
		if (status==ALPS_RUNOUTOFMEMORY) {
			fprintf(stderr,"FSOLVE failed: run out of memory.\n");
			exit(1000);
		}
		lp->objval = 0.0;
		for (i=0; i<lp->im; i++) {
			j = lp->basisheader[i];
			lp->x[j] = xbas[i];
			if (lp->x[j]<lp->ilowerbound[j]) lp->x[j] = lp->ilowerbound[j];
			if (lp->x[j]>lp->iupperbound[j]) lp->x[j] = lp->iupperbound[j];
			if (lp->x[j]<lp->ioriglowerbound[j]-ALPS_ZEROEPS) {
				lp->objval -= lp->iobj[j]*(lp->ioriglowerbound[j]-lp->x[j]);
			}
			else if (lp->x[j]>lp->iorigupperbound[j]+ALPS_ZEROEPS) {
				lp->objval += lp->iobj[j]*(lp->x[j]-lp->iorigupperbound[j]);
			}
		}
		if (lp->objval>=-ALPS_EPS) {
			lp->problemstatus = ALPS_OPTIMAL;
			goto terminate_primal_phase1_simplex_loop;
		}



		if ( fabs(lp->objval)<ALPS_ZEROEPS && lp->objval<preobjval+ALPS_ZEROEPS) {
			ageobjval++;
			if ((ageobjval>degenage)&&(!degeneracy)) {
				degeneracy = ALPS_TRUE;
				if (pril) printf("Looks degenerate...\n");
			}
		}
		else if ( fabs(lp->objval)>ALPS_ZEROEPS 
			&& fabs((lp->objval-preobjval)/lp->objval)<ALPS_ZEROEPS) {
				ageobjval++;
				if ((ageobjval>degenage)&&(!degeneracy)) {
					degeneracy = ALPS_TRUE;
					if (pril) printf("Looks degenerate...\n");
				}
			}
		else {
			preobjval = lp->objval;
			ageobjval = 0;
			if (degeneracy) {
				degeneracy = ALPS_FALSE;
				if (pril) printf("Looks fine again...\n");
			}
		}
		if (pril>=4) printprimalsol(lp);
	}
	}
terminate_primal_phase1_simplex_loop:
	lp->nrphase1iter = nriter;
	if (pril>=3) {
		if (lp->problemstatus==ALPS_UNKNOWN) printf("\nStatus: ALPS_UNKNOWN\n");
		if (lp->problemstatus==ALPS_INFEASIBLE) printf("\nStatus: ALPS_INFEASIBLE\n");
		if (lp->problemstatus==ALPS_UNBOUNDED) printf("\nStatus: ALPS_UNBOUNDED\n");
		if (lp->problemstatus==ALPS_OPTIMAL) printf("\nStatus: ALPS_OPTIMAL\n");
	}


	/** Free space for simplex arrays */

	ffree( (char **) &pivrow);
	ffree( (char **) &pivcol);
	ffree( (char **) &pivvalue);
	ffree( (char **) &lp->rumatbeg);
	ffree( (char **) &lp->rumatend);
	ffree( (char **) &lp->rumatind);
	ffree( (char **) &lp->rumatcoeff);
	ffree( (char **) &lp->clmatbeg);
	ffree( (char **) &lp->clmatend);
	ffree( (char **) &lp->clmatind);
	ffree( (char **) &lp->clmatcoeff);
	ffree( (char **) &lp->cumatbeg);
	ffree( (char **) &lp->cumatend);
	ffree( (char **) &lp->cumatind);
	ffree( (char **) &lp->cumatcoeff);
	ffree( (char **) &cbas);
	ffree( (char **) &xbas);
	ffree( (char **) &trhs);
	ffree( (char **) &entcol);
	ffree( (char **) &lp->w);
	ffree( (char **) &lp->wsval);
	ffree( (char **) &lp->wsind);

	return 0;
}

int dualsimplex(
				lpstruct *lp,
				int pril
				)

{
	/** Declare simplex scalars */

	int totalfill;
	int totalcancel;
	int status;
	int basnonz;
	int ind;
	int enter;
	int leaveindex;
	int leave;
	int i,j,k,l;
	int nriter;
	int dirindl;
	int eff;
	int dsc;
	int enr;
	int dsindl;


	int starttime;
	int stoptime;
	int primalalgorithm;






	double dsvall;
	double dirvall;
	double redcost;
	double change;
	double yl;
	double oldobjcoeff;
	double newx;









	/** Declare simplex arrays */

	int *pivrow;
	int *pivcol;
	double *pivvalue;
	double *cbas;
	double *xbas;
	double *trhs;
	double *entcol;



	/** Allocate space for simplex arrays */

	pivrow = NULL;
	pivcol = NULL;
	pivvalue = NULL;
	cbas = (double *) malloc( lp->im * sizeof(double) );
	xbas = (double *) malloc( lp->im * sizeof(double) );
	trhs = (double *) malloc( lp->im * sizeof(double) );
	entcol = (double *) malloc( lp->im * sizeof(double) );
	lp->w = (double *) malloc( lp->in * sizeof(double) );
	lp->wsval = (double *) malloc( lp->in * sizeof(double) );
	lp->wsind = (int *) malloc( lp->in * sizeof(int) );
	if (!cbas ||!xbas ||!trhs || !entcol )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}





	primalalgorithm = ALPS_FALSE;
	/** Construct basis matrix */
	/*
	*  In order to construct the basis matrix, all we have to do is counting
	* the number of nonzeroes in the columns corresponding to the variables
	* listed in the |lp->basisheader|, and then allocating the right amount
	* of space, and copying the data.
	*/
	ffree( (char **) &lp->basmatbeg);
	ffree( (char **) &lp->basmatcount);
	ffree( (char **) &lp->basmatind);
	ffree( (char **) &lp->basmatcoeff);
	basnonz = 0;
	for(k=0;k<lp->im;k++) basnonz += lp->imatcolcount[lp->basisheader[k]];
	lp->basmatbeg = (int *) malloc( lp->im * sizeof(int) );
	lp->basmatcount = (int *) malloc( lp->im * sizeof(int) );
	lp->basmatind = (int *) malloc( basnonz * sizeof(int) );
	lp->basmatcoeff = (double *) malloc( basnonz * sizeof(double) );
	if (!lp->basmatbeg || ! lp->basmatcount || !lp->basmatind 
		|| !lp->basmatcoeff )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}



	ind=0;
	for(k=0;k<lp->im;k++) {
		j = lp->basisheader[k];
		lp->basmatbeg[k] = ind;
		lp->basmatcount[k] = lp->imatcolcount[j];
		for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) {
			lp->basmatind[ind] = lp->imatcolind[l];
			lp->basmatcoeff[ind] =  lp->imatcolcoeff[l];
			ind++;
		}
	}



	/** Factor basis matrix */
	/*
	*  Here we compute an $LU$-factorization of the basis matrix. Later, we
	* should add here a fresh computation of |lp->x|.
	*/
	ffree( (char **) &pivrow);
	ffree( (char **) &pivcol);
	ffree( (char **) &pivvalue);
	ffree( (char **) &lp->rumatbeg);
	ffree( (char **) &lp->rumatend);
	ffree( (char **) &lp->rumatind);
	ffree( (char **) &lp->rumatcoeff);
	ffree( (char **) &lp->clmatbeg);
	ffree( (char **) &lp->clmatend);
	ffree( (char **) &lp->clmatind);
	ffree( (char **) &lp->clmatcoeff);
	ffree( (char **) &lp->cumatbeg);
	ffree( (char **) &lp->cumatend);
	ffree( (char **) &lp->cumatind);
	ffree( (char **) &lp->cumatcoeff);
#ifdef ALPS_TIMING_ENABLED
	starttime = cputime();
#endif
	status= alps_lufac (
		pril,
		(int) 0,
		lp->im,
		lp->basmatbeg,
		lp->basmatcount,
		lp->basmatind,
		lp->basmatcoeff,
		&pivrow,
		&pivcol,
		&pivvalue,
		&lp->rumatbeg,
		&lp->rumatend,
		&lp->rumatind,
		&lp->rumatcoeff,
		&lp->cumatbeg,
		&lp->cumatend,
		&lp->cumatind,
		&lp->cumatcoeff,
		&lp->clmatbeg,
		&lp->clmatend,
		&lp->clmatind,
		&lp->clmatcoeff,
		&totalfill,
		&totalcancel
		);
#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->factortime += (stoptime - starttime);
#endif
	if (status==ALPS_RUNOUTOFMEMORY) {
		fprintf(stderr,"LU factorization failed: run out of memory.\n");
		return ALPS_RUNOUTOFMEMORY;
	}
	else if (status) {
		fprintf(stderr,"LU factorization failed. Matrix singular.\n");
		return ALPS_LU_NOPIVOT;
	}
	lp->etanr = 0;
	lp->etafirstfree = 0;
	lp->numfactor++;
	lp->sumfill += ( (double) totalfill ) / ( (double) lp->im );


	/** Compute |x| and value from scratch */
	/*
	*  After each factorization, we compute the solution vector |lp->x| and
	* the objective function value from scratch.
	*/
	for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
	lp->objval = 0.0;
	for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) {
		switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		lp->x[j] = lp->ilowerbound[j];
		lp->objval += lp->iobj[j]*lp->x[j];
		break;
	case ALPS_NONBASICUPB:
		lp->x[j] = lp->iupperbound[j];
		lp->objval += lp->iobj[j]*lp->x[j];
		break;
	case ALPS_NONBASICFREE:
		lp->x[j] = 0.0;
		break;
		}
		for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			i = lp->imatcolind[k];
			trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
		}
	}
#ifdef ALPS_TIMING_ENABLED
	starttime = cputime();
#endif
	status = alps_fsolveeqs(
		lp,
		lp->im,
		pivrow,
		pivcol,
		pivvalue,
		lp->rumatbeg,
		lp->rumatend,
		lp->rumatind,
		lp->rumatcoeff,
		lp->clmatbeg,
		lp->clmatend,
		lp->clmatind,
		lp->clmatcoeff,
		lp->etanr,
		lp->etacol,
		lp->etaval,
		lp->etaind,
		lp->etastart,
		lp->etacount,
		lp->eta,
		trhs,
		xbas
		);

#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->fsolvetime += (stoptime - starttime);
#endif
	if (status==ALPS_RUNOUTOFMEMORY) {
		fprintf(stderr,"FSOLVE failed: run out of memory.\n");
		exit(1000);
	}
	for (i=0; i<lp->im; i++) {
		j = lp->basisheader[i];
		lp->x[j] = xbas[i];
		if (primalalgorithm) {
			if (lp->x[j]<lp->ilowerbound[j]) lp->x[j] = lp->ilowerbound[j];
			if (lp->x[j]>lp->iupperbound[j]) lp->x[j] = lp->iupperbound[j];
		}
		lp->objval += lp->iobj[j]*lp->x[j];
	}


	/** Dual simplex loop */
	/*
	*  The dual simplex loop doesn't go forever (hopefully) since on various
	* termination condition we jump to |terminate_dual_simplex_loop|. The
	* layout is textbook-like. Currently, there is no dual phase 1 yet.
	*/
	if (lp->phase==ALPS_PHASE1) nriter = 0;
	else nriter = lp->nrphase1iter;
	while(ALPS_TRUE) {
start_dual_simplex_loop:
		if (lp->etanr==0 || lp->howtopisel!=ALPS_FULLEASYPISEL) {
			// Compute multipliers
			/*
			*  The multipliers $y$, here |lp->y|, are found by solving the system
			* $y^TB=c_B$.
			*/
			for (k=0;k<lp->im;k++) cbas[k] = lp->iobj[lp->basisheader[k]];
#ifdef ALPS_TIMING_ENABLED
			starttime = cputime();
#endif
			status = alps_bsolveeqs(
				lp,
				lp->im,
				pivrow,
				pivcol,
				pivvalue,
				lp->cumatbeg,
				lp->cumatend,
				lp->cumatind,
				lp->cumatcoeff,
				lp->clmatbeg,
				lp->clmatend,
				lp->clmatind,
				lp->clmatcoeff,
				lp->etanr,
				lp->etacol,
				lp->etaval,
				lp->etaind,
				lp->etastart,
				lp->etacount,
				lp->eta,
				cbas,
				lp->y
				);

#ifdef ALPS_TIMING_ENABLED
			stoptime = cputime();
			lp->bsolvetime += (stoptime - starttime);
#endif
			if (status==ALPS_RUNOUTOFMEMORY) {
				fprintf(stderr,"BSOLVE failed: run out of memory.\n");
				exit(1000);
			}
			k = 0;
			for (l=0;l<lp->im;l++) {
				yl = lp->y[l];
				if ((yl<-ALPS_ZEROEPS)||(yl>ALPS_ZEROEPS)) {
					lp->ysind[k] = l;
					lp->ysval[k] = yl;
					k++;
				}
			}
			lp->yscount = k;
			if (pril>=4) {
				printf("\ncbas: "); for (i=0;i<lp->im;i++) printf(" %.2lf",cbas[i]);printf("\n");
				printf("y   : "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->y[i]);printf("\n");
			}


		}
		// Make dual pivot selection
		/*
		*  For dual pivot selection, we call dualpisel.
		*/
#ifdef ALPS_TIMING_ENABLED
		starttime = cputime();
#endif
		status = alps_dualeasypisel(
			lp,
			pril,
			&enter,
			&leave,
			&leaveindex,
			&redcost,
			&change,
			pivrow,
			pivcol,
			pivvalue,
			&oldobjcoeff
			);
#ifdef ALPS_TIMING_ENABLED
		stoptime = cputime();
		lp->piseltime += (stoptime - starttime);
#endif
		if ((lp->problemstatus==ALPS_INFEASIBLE)||(lp->problemstatus==ALPS_OPTIMAL)) {
			if (!lp->etanr) goto terminate_dual_simplex_loop;
			else {
				// Construct basis matrix
				/*
				*  In order to construct the basis matrix, all we have to do is counting
				* the number of nonzeroes in the columns corresponding to the variables
				* listed in the |lp->basisheader|, and then allocating the right amount
				* of space, and copying the data.
				*/
				ffree( (char **) &lp->basmatbeg);
				ffree( (char **) &lp->basmatcount);
				ffree( (char **) &lp->basmatind);
				ffree( (char **) &lp->basmatcoeff);
				basnonz = 0;
				for(k=0;k<lp->im;k++) basnonz += lp->imatcolcount[lp->basisheader[k]];
				lp->basmatbeg = (int *) malloc( lp->im * sizeof(int) );
				lp->basmatcount = (int *) malloc( lp->im * sizeof(int) );
				lp->basmatind = (int *) malloc( basnonz * sizeof(int) );
				lp->basmatcoeff = (double *) malloc( basnonz * sizeof(double) );
				if (!lp->basmatbeg || ! lp->basmatcount || !lp->basmatind 
					|| !lp->basmatcoeff )
					// run out of memory

				{
					fprintf (stderr, "run out of memory\n");
					return(ALPS_RUNOUTOFMEMORY);
				}



				ind=0;
				for(k=0;k<lp->im;k++) {
					j = lp->basisheader[k];
					lp->basmatbeg[k] = ind;
					lp->basmatcount[k] = lp->imatcolcount[j];
					for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) {
						lp->basmatind[ind] = lp->imatcolind[l];
						lp->basmatcoeff[ind] =  lp->imatcolcoeff[l];
						ind++;
					}
				}



				// Factor basis matrix
				/*
				*  Here we compute an $LU$-factorization of the basis matrix. Later, we
				* should add here a fresh computation of |lp->x|.
				*/
				ffree( (char **) &pivrow);
				ffree( (char **) &pivcol);
				ffree( (char **) &pivvalue);
				ffree( (char **) &lp->rumatbeg);
				ffree( (char **) &lp->rumatend);
				ffree( (char **) &lp->rumatind);
				ffree( (char **) &lp->rumatcoeff);
				ffree( (char **) &lp->clmatbeg);
				ffree( (char **) &lp->clmatend);
				ffree( (char **) &lp->clmatind);
				ffree( (char **) &lp->clmatcoeff);
				ffree( (char **) &lp->cumatbeg);
				ffree( (char **) &lp->cumatend);
				ffree( (char **) &lp->cumatind);
				ffree( (char **) &lp->cumatcoeff);
#ifdef ALPS_TIMING_ENABLED
				starttime = cputime();
#endif
				status= alps_lufac (
					pril,
					(int) 0,
					lp->im,
					lp->basmatbeg,
					lp->basmatcount,
					lp->basmatind,
					lp->basmatcoeff,
					&pivrow,
					&pivcol,
					&pivvalue,
					&lp->rumatbeg,
					&lp->rumatend,
					&lp->rumatind,
					&lp->rumatcoeff,
					&lp->cumatbeg,
					&lp->cumatend,
					&lp->cumatind,
					&lp->cumatcoeff,
					&lp->clmatbeg,
					&lp->clmatend,
					&lp->clmatind,
					&lp->clmatcoeff,
					&totalfill,
					&totalcancel
					);
#ifdef ALPS_TIMING_ENABLED
				stoptime = cputime();
				lp->factortime += (stoptime - starttime);
#endif
				if (status==ALPS_RUNOUTOFMEMORY) {
					fprintf(stderr,"LU factorization failed: run out of memory.\n");
					return ALPS_RUNOUTOFMEMORY;
				}
				else if (status) {
					fprintf(stderr,"LU factorization failed. Matrix singular.\n");
					return ALPS_LU_NOPIVOT;
				}
				lp->etanr = 0;
				lp->etafirstfree = 0;
				lp->numfactor++;
				lp->sumfill += ( (double) totalfill ) / ( (double) lp->im );


				if (pril>=3) printf("=== Refactorization performed. ===\n");
				// Compute |x| and value from scratch
				/*
				*  After each factorization, we compute the solution vector |lp->x| and
				* the objective function value from scratch.
				*/
				for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
				lp->objval = 0.0;
				for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) {
					switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		lp->x[j] = lp->ilowerbound[j];
		lp->objval += lp->iobj[j]*lp->x[j];
		break;
	case ALPS_NONBASICUPB:
		lp->x[j] = lp->iupperbound[j];
		lp->objval += lp->iobj[j]*lp->x[j];
		break;
	case ALPS_NONBASICFREE:
		lp->x[j] = 0.0;
		break;
					}
					for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
						i = lp->imatcolind[k];
						trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
					}
				}
#ifdef ALPS_TIMING_ENABLED
				starttime = cputime();
#endif
				status = alps_fsolveeqs(
					lp,
					lp->im,
					pivrow,
					pivcol,
					pivvalue,
					lp->rumatbeg,
					lp->rumatend,
					lp->rumatind,
					lp->rumatcoeff,
					lp->clmatbeg,
					lp->clmatend,
					lp->clmatind,
					lp->clmatcoeff,
					lp->etanr,
					lp->etacol,
					lp->etaval,
					lp->etaind,
					lp->etastart,
					lp->etacount,
					lp->eta,
					trhs,
					xbas
					);

#ifdef ALPS_TIMING_ENABLED
				stoptime = cputime();
				lp->fsolvetime += (stoptime - starttime);
#endif
				if (status==ALPS_RUNOUTOFMEMORY) {
					fprintf(stderr,"FSOLVE failed: run out of memory.\n");
					exit(1000);
				}
				for (i=0; i<lp->im; i++) {
					j = lp->basisheader[i];
					lp->x[j] = xbas[i];
					if (primalalgorithm) {
						if (lp->x[j]<lp->ilowerbound[j]) lp->x[j] = lp->ilowerbound[j];
						if (lp->x[j]>lp->iupperbound[j]) lp->x[j] = lp->iupperbound[j];
					}
					lp->objval += lp->iobj[j]*lp->x[j];
				}


				if (pril>=4) printprimalsol(lp);
				goto start_dual_simplex_loop;
			}
		}
		else if (pril>=4) printf("Leaving variable: %d [change = %.2lf]\n",leave,change);

		if (pril>=4) {
			for (k=0;k<lp->im;k++) entcol[k] = 0.0;
			for (l=lp->imatcolbeg[enter];l<lp->imatcolbeg[enter]+lp->imatcolcount[enter];l++) 
				entcol[lp->imatcolind[l]] = lp->imatcolcoeff[l];
			printf("\nentcol: "); for (i=0;i<lp->im;i++) printf(" %.2lf",entcol[i]);printf("\n");
			printf("d     : "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->d[i]);printf("\n");
		}


		// Basis update (dual)
		/*
		*  In the dual case, change may be negative. There are no consistency
		* checks yet.
		*/
		enr = lp->etanr;
		if (((!enr)&&(pril==1))||(pril>=2)) printf("%6d:   ",nriter);
		for(l=0;l<lp->dscount;l++) {
			dirindl = lp->dsind[l];
			dirvall = lp->dsval[l];
			lp->x[lp->basisheader[dirindl]] -= change * dirvall;
			lp->objval -= change*dirvall*lp->iobj[lp->basisheader[dirindl]];
		}
		lp->x[enter] += change;
		lp->objval += change*dirvall*lp->iobj[enter];
		if (((!enr)&&(pril==1))||(pril>=2)) {
			if (leave<lp->inorig) printf("x");
			else if (leave<lp->inonartif) printf("s");
			else printf("a");
			printf("[%4d] -> ",leave);
			if (enter<lp->inorig) printf("x");
			else if (enter<lp->inonartif) printf("s");
			else printf("a");
			printf("[%4d] ",enter);
		}
		lp->basisheader[leaveindex] = enter;
		lp->colstat[enter]=ALPS_BASIC;

		eff = lp->etafirstfree;
		dsc = lp->dscount;
		enr = lp->etanr;
		lp->etacol[enr] = leaveindex;
		lp->etastart[enr] = eff;
		lp->etacount[enr] = dsc - 1;
		for (l=0; l<dsc; l++) {
			dsindl = lp->dsind[l];
			dsvall = lp->dsval[l];
			if (dsindl == leaveindex) {
				lp->eta[enr] = dsvall;
				if (((!enr)&&(pril==1))||(pril>=2)) printf(" %8.1le",dsvall);
			}
			else {
				lp->etaind[eff] = dsindl;
				lp->etaval[eff] = dsvall;
				eff++;
			}
		}
		lp->etanr = enr + 1;
		lp->etafirstfree = eff;
		if (leave>=lp->inonartif) { /* artificial */
			lp->ilowerbound[leave] = 0.0;
			lp->iupperbound[leave] = 0.0;
			lp->iphase1obj[leave] = 0.0;
			lp->colstat[leave]=ALPS_NONBASICLOWB;
		}
		else if (leave>=lp->inorig) { /* slack */
			lp->ilowerbound[leave] = 0.0;
			lp->iupperbound[leave] = ALPS_REAL_INFINITY;
			oldobjcoeff = lp->iphase1obj[leave];
			lp->iphase1obj[leave] = 0.0;
			lp->colstat[leave]=ALPS_NONBASICLOWB;
		}
		else { /* normal */
			if (lp->x[leave]>=lp->iupperbound[leave]-ALPS_ZEROEPS) {
				lp->colstat[leave]=ALPS_NONBASICUPB;
				lp->x[leave] = lp->iupperbound[leave];
			}
			else if (lp->x[leave]<=lp->ilowerbound[leave]+ALPS_ZEROEPS) {
				lp->colstat[leave]=ALPS_NONBASICLOWB;
				lp->x[leave] = lp->ilowerbound[leave];
			}
			else { 
				lp->colstat[leave]=ALPS_NONBASICFREE;
				newx = lp->x[leave];
				if ((newx<-0.001)||(newx>0.001))
					printf("\nNew nonbasic free variable with value %lf!\n",newx);
				lp->x[leave] = 0.0;
			} 
		}
		if (((!enr)&&(pril==1))||(pril>=2)) {
			if (lp->aobjminmax==ALPS_MAXIMIZE) 
				printf(" %15.4lf\n",lp->objval);
			else
				printf(" %15.4lf\n",-lp->objval);
		}


		if (pril>=4) printprimalsol(lp);
		nriter++;
		if (lp->etanr>=lp->etaclear) {
			// Construct basis matrix
			/*
			*  In order to construct the basis matrix, all we have to do is counting
			* the number of nonzeroes in the columns corresponding to the variables
			* listed in the |lp->basisheader|, and then allocating the right amount
			* of space, and copying the data.
			*/
			ffree( (char **) &lp->basmatbeg);
			ffree( (char **) &lp->basmatcount);
			ffree( (char **) &lp->basmatind);
			ffree( (char **) &lp->basmatcoeff);
			basnonz = 0;
			for(k=0;k<lp->im;k++) basnonz += lp->imatcolcount[lp->basisheader[k]];
			lp->basmatbeg = (int *) malloc( lp->im * sizeof(int) );
			lp->basmatcount = (int *) malloc( lp->im * sizeof(int) );
			lp->basmatind = (int *) malloc( basnonz * sizeof(int) );
			lp->basmatcoeff = (double *) malloc( basnonz * sizeof(double) );
			if (!lp->basmatbeg || ! lp->basmatcount || !lp->basmatind 
				|| !lp->basmatcoeff )
				// run out of memory

			{
				fprintf (stderr, "run out of memory\n");
				return(ALPS_RUNOUTOFMEMORY);
			}



			ind=0;
			for(k=0;k<lp->im;k++) {
				j = lp->basisheader[k];
				lp->basmatbeg[k] = ind;
				lp->basmatcount[k] = lp->imatcolcount[j];
				for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) {
					lp->basmatind[ind] = lp->imatcolind[l];
					lp->basmatcoeff[ind] =  lp->imatcolcoeff[l];
					ind++;
				}
			}



			// Factor basis matrix
			/*
			*  Here we compute an $LU$-factorization of the basis matrix. Later, we
			* should add here a fresh computation of |lp->x|.
			*/
			ffree( (char **) &pivrow);
			ffree( (char **) &pivcol);
			ffree( (char **) &pivvalue);
			ffree( (char **) &lp->rumatbeg);
			ffree( (char **) &lp->rumatend);
			ffree( (char **) &lp->rumatind);
			ffree( (char **) &lp->rumatcoeff);
			ffree( (char **) &lp->clmatbeg);
			ffree( (char **) &lp->clmatend);
			ffree( (char **) &lp->clmatind);
			ffree( (char **) &lp->clmatcoeff);
			ffree( (char **) &lp->cumatbeg);
			ffree( (char **) &lp->cumatend);
			ffree( (char **) &lp->cumatind);
			ffree( (char **) &lp->cumatcoeff);
#ifdef ALPS_TIMING_ENABLED
			starttime = cputime();
#endif
			status= alps_lufac (
				pril,
				(int) 0,
				lp->im,
				lp->basmatbeg,
				lp->basmatcount,
				lp->basmatind,
				lp->basmatcoeff,
				&pivrow,
				&pivcol,
				&pivvalue,
				&lp->rumatbeg,
				&lp->rumatend,
				&lp->rumatind,
				&lp->rumatcoeff,
				&lp->cumatbeg,
				&lp->cumatend,
				&lp->cumatind,
				&lp->cumatcoeff,
				&lp->clmatbeg,
				&lp->clmatend,
				&lp->clmatind,
				&lp->clmatcoeff,
				&totalfill,
				&totalcancel
				);
#ifdef ALPS_TIMING_ENABLED
			stoptime = cputime();
			lp->factortime += (stoptime - starttime);
#endif
			if (status==ALPS_RUNOUTOFMEMORY) {
				fprintf(stderr,"LU factorization failed: run out of memory.\n");
				return ALPS_RUNOUTOFMEMORY;
			}
			else if (status) {
				fprintf(stderr,"LU factorization failed. Matrix singular.\n");
				return ALPS_LU_NOPIVOT;
			}
			lp->etanr = 0;
			lp->etafirstfree = 0;
			lp->numfactor++;
			lp->sumfill += ( (double) totalfill ) / ( (double) lp->im );


			if (pril>=3) printf("=== Refactorization performed (%1d/%1d). ===\n",lp->im,totalfill);
			// Compute |x| and value from scratch
			/*
			*  After each factorization, we compute the solution vector |lp->x| and
			* the objective function value from scratch.
			*/
			for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
			lp->objval = 0.0;
			for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) {
				switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		lp->x[j] = lp->ilowerbound[j];
		lp->objval += lp->iobj[j]*lp->x[j];
		break;
	case ALPS_NONBASICUPB:
		lp->x[j] = lp->iupperbound[j];
		lp->objval += lp->iobj[j]*lp->x[j];
		break;
	case ALPS_NONBASICFREE:
		lp->x[j] = 0.0;
		break;
				}
				for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
					i = lp->imatcolind[k];
					trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
				}
			}
#ifdef ALPS_TIMING_ENABLED
			starttime = cputime();
#endif
			status = alps_fsolveeqs(
				lp,
				lp->im,
				pivrow,
				pivcol,
				pivvalue,
				lp->rumatbeg,
				lp->rumatend,
				lp->rumatind,
				lp->rumatcoeff,
				lp->clmatbeg,
				lp->clmatend,
				lp->clmatind,
				lp->clmatcoeff,
				lp->etanr,
				lp->etacol,
				lp->etaval,
				lp->etaind,
				lp->etastart,
				lp->etacount,
				lp->eta,
				trhs,
				xbas
				);

#ifdef ALPS_TIMING_ENABLED
			stoptime = cputime();
			lp->fsolvetime += (stoptime - starttime);
#endif
			if (status==ALPS_RUNOUTOFMEMORY) {
				fprintf(stderr,"FSOLVE failed: run out of memory.\n");
				exit(1000);
			}
			for (i=0; i<lp->im; i++) {
				j = lp->basisheader[i];
				lp->x[j] = xbas[i];
				if (primalalgorithm) {
					if (lp->x[j]<lp->ilowerbound[j]) lp->x[j] = lp->ilowerbound[j];
					if (lp->x[j]>lp->iupperbound[j]) lp->x[j] = lp->iupperbound[j];
				}
				lp->objval += lp->iobj[j]*lp->x[j];
			}


			if (pril>=4) printprimalsol(lp);
		}
	}
terminate_dual_simplex_loop:
	if (lp->phase==ALPS_PHASE1) lp->nrphase1iter = nriter;
	else lp->nrphase2iter = nriter - lp->nrphase1iter;
	if (pril>=3) {
		if (lp->problemstatus==ALPS_UNKNOWN) printf("\nStatus: ALPS_UNKNOWN\n");
		if (lp->problemstatus==ALPS_INFEASIBLE) printf("\nStatus: ALPS_INFEASIBLE\n");
		if (lp->problemstatus==ALPS_UNBOUNDED) printf("\nStatus: ALPS_UNBOUNDED\n");
		if (lp->problemstatus==ALPS_OPTIMAL) printf("\nStatus: ALPS_OPTIMAL\n");
	}


	/** Free space for simplex arrays */

	ffree( (char **) &pivrow);
	ffree( (char **) &pivcol);
	ffree( (char **) &pivvalue);
	ffree( (char **) &lp->rumatbeg);
	ffree( (char **) &lp->rumatend);
	ffree( (char **) &lp->rumatind);
	ffree( (char **) &lp->rumatcoeff);
	ffree( (char **) &lp->clmatbeg);
	ffree( (char **) &lp->clmatend);
	ffree( (char **) &lp->clmatind);
	ffree( (char **) &lp->clmatcoeff);
	ffree( (char **) &lp->cumatbeg);
	ffree( (char **) &lp->cumatend);
	ffree( (char **) &lp->cumatind);
	ffree( (char **) &lp->cumatcoeff);
	ffree( (char **) &cbas);
	ffree( (char **) &xbas);
	ffree( (char **) &trhs);
	ffree( (char **) &entcol);
	ffree( (char **) &lp->w);
	ffree( (char **) &lp->wsval);
	ffree( (char **) &lp->wsind);

	return 0;
}

} // end namespace ogdf
