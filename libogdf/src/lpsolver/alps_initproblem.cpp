/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Initialization.
 * 
 * This routine is called after loading the problem and before
 * performing the primal or dual simplex algorithm. It sets up the
 * internal problem definition and does scaling and preprocessing. 
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
#include "alps_preproc.h"
#include "math.h"

namespace ogdf {

//int randseed;

int alps_initproblem( int pril, lpstruct *lp )
{
	/** Declare initproblem arrays */
	int *rowptr;

	/** Declare initproblem scalars */
	int i,j,k,ind1,ind2;
	int nrineq;
	int index,rowindex;
	double ilbj,iubj;

	if (lp->preprocessing) {
		/** Perform preprocessing */
		/*
		*  Preprocessing. Here we perform several preprocessing procedures in
		* order to reduce the problem. We assume that the internal matrix is
		* already available in row format. Preprocessing is currently being
		* developed. So far no real actions are performed. How to handle
		* elimination of rows or columns??
		*/{
		int prepstatus,prepcode,prepactions;
		alps_prepinit(pril,lp);
		prepactions = 0;
		do {
			prepstatus = alps_preprocess(pril,lp,&prepcode,&prepactions);
		} while (prepactions && prepstatus==0);
		if (prepstatus!=0) {
			printf("Preprocessing failed (Code=%1d, status=%1d)\n",prepcode,prepstatus);
			exit (1000);
		}
		alps_prepgenprob(pril,lp);
		}

		if (lp->preprocessing>1) return(ALPS_PREPROCESSONLY);
	}
	else {
		prep_objcorr = 0.0;
		/** Allocate space for internal LP data */
		/*
		*  Generate internal problem. Space for the internal problem is
		* allocated. We do not need a constraint type because all constraint
		* will be equality constraints. The problem is assumed to be a
		* maximization problem.
		*/
		lp->im = lp->am;
		lp->in = lp->an + lp->am;
		lp->inorig = lp->an;
		lp->inonz = lp->amatcolbeg[lp->an-1]+lp->amatcolcount[lp->an-1]+lp->im;
		lp->recoheapp = (int *) malloc( (1 + lp->in) * sizeof(int) );
		lp->recoheappindex = (int *) malloc( lp->in * sizeof(int) );
		lp->cand = (int *) malloc( lp->in * sizeof(int) );
		lp->imatcolbeg = (int *) malloc( lp->in * sizeof(int) );
		lp->imatcolcount = (int *) malloc( lp->in * sizeof(int) );
		lp->imatcolind = (int *) malloc( lp->inonz * sizeof(int) );
		lp->imatcolcoeff = (double *) malloc( lp->inonz * sizeof(double) );
		lp->imatrowcount = (int *) malloc( lp->im * sizeof(int) );
		lp->varstat = (int *) malloc( lp->in * sizeof(int) );
		lp->origvarstat = (int *) malloc( lp->in * sizeof(int) );
		lp->iupperbound = (double *) malloc( lp->in * sizeof(double) );
		lp->ilowerbound = (double *) malloc( lp->in * sizeof(double) );
		lp->iorigupperbound = (double *) malloc( lp->in * sizeof(double) );
		lp->ioriglowerbound = (double *) malloc( lp->in * sizeof(double) );
		lp->iartifbounds = (int *) malloc( lp->in * sizeof(int) );
		lp->irhs = (double *) malloc( lp->im * sizeof(double) );
		lp->iphase1obj = (double *) malloc( lp->in * sizeof(double) );
		lp->iphase2obj = (double *) malloc( lp->in * sizeof(double) );
		lp->reducedcost = (double *) malloc( lp->in * sizeof(double) );
		lp->u = (double *) malloc( lp->im * sizeof(double) );
		lp->z = (double *) malloc( lp->im * sizeof(double) );
		lp->zsval = (double *) malloc( lp->im * sizeof(double) );
		lp->zsind = (int *) malloc( lp->im * sizeof(int) );
		if (!lp->imatcolbeg || ! lp->imatcolcount || !lp->imatcolind 
			|| !lp->imatcolcoeff || !lp->imatrowcount
			|| !lp->iupperbound || !lp->ilowerbound || !lp->irhs 
			|| !lp->iorigupperbound || !lp->ioriglowerbound || !lp->iartifbounds 
			|| !lp->iphase1obj || ! lp->iphase2obj 
			|| !lp->reducedcost || !lp->u || !lp->z
			|| !lp->zsval || !lp->zsind )
			// run out of memory

		{
			fprintf (stderr, "run out of memory\n");
			return(ALPS_RUNOUTOFMEMORY);
		}

		lp->basmatbeg = NULL;
		lp->basmatcount = NULL;
		lp->basmatind = NULL;
		lp->basmatcoeff = NULL;

		/** Generate internal problem */
		/*
		*  The internal data structures for the optimization process are set up
		* here. First, we copy the objective function coefficients, possibly
		* multiplied by $-1$, if the optimization sense is minimization, because
		* internally, we always maximize. Then, the lower and upper bounds are
		* copied, followed by the problem matrix, where we make sure,
		* multiplying by $-1$, if necessary, that no ``$\ge$'' constraints are
		* left. Next, we introduce slacks for the inequality constraints and
		* artificials for the equality constraints. The slacks always come
		* first, so that the first |lp->an| variables are structural variables,
		* followed by nonnegative slack variables up to |lp->inonartif|,
		* followed by artificial variables up to |lp->in|. The objective
		* function is temporarily set to the original (phase2) objective
		* function.
		*/

		if (lp->aobjminmax==ALPS_MINIMIZE) {
			for (j=0;j<lp->an;j++) lp->iphase2obj[j] = -lp->aobj[j];
		}
		else {
			for (j=0;j<lp->an;j++) lp->iphase2obj[j] = lp->aobj[j];
		}
		for (j=0;j<lp->an;j++) {
			ilbj = lp->alowerbound[j];
			iubj = lp->aupperbound[j];
			lp->ilowerbound[j] = ilbj;
			lp->iupperbound[j] = iubj;
			if (ilbj>-ALPS_REAL_INFINITY) { /* finite lower bound */
				if (iubj<ALPS_REAL_INFINITY) { /* finite upper bound */
					if (ilbj<iubj) lp->varstat[j] = ALPS_BOUNDEDTWICE;
					else lp->varstat[j] = ALPS_FIXED;
				}
				else{ /* no finite upperbound */
					lp->varstat[j] = ALPS_BOUNDEDBELOW;
				}
			}
			else { /* no finite lower bound */
				if (iubj<ALPS_REAL_INFINITY) { /* finite upper bound */
					lp->varstat[j] = ALPS_BOUNDEDABOVE;
				}
				else{ /* no finite upperbound */
					lp->varstat[j] = ALPS_FREE;
				}
			}
		}
		for (j=0;j<lp->an;j++) {
			lp->imatcolbeg[j] = lp->amatcolbeg[j];
			lp->imatcolcount[j] = lp->amatcolcount[j];
		}
		for (k=0; k<lp->im; k++) lp->imatrowcount[k] = 1;
		for (k=0;k<lp->amatcolbeg[lp->an-1]+lp->amatcolcount[lp->an-1];k++) {
			rowindex = lp->amatcolind[k];
			lp->imatcolind[k] = rowindex;
			lp->imatrowcount[rowindex]++;
			if (lp->aconstrainttype[rowindex]=='G')
				lp->imatcolcoeff[k] = -lp->amatcolcoeff[k];
			else
				lp->imatcolcoeff[k] = lp->amatcolcoeff[k];
		}

		nrineq = 0;
		for (i=0;i<lp->im;i++) 
			if (lp->aconstrainttype[i]!='E') nrineq++;
		ind1 = lp->an;
		ind2 = ind1 + nrineq;
		k = lp->amatcolbeg[lp->an-1]+lp->amatcolcount[lp->an-1];
		for (i=0;i<lp->im;i++) {
			if (lp->aconstrainttype[i]=='G') {
				lp->irhs[i] = -lp->arhs[i];
				lp->ilowerbound[ind1] = 0.0;
				lp->iupperbound[ind1] = ALPS_REAL_INFINITY;
				lp->varstat[ind1] = ALPS_BOUNDEDBELOW;
				lp->imatcolbeg[ind1] = k;
				lp->imatcolcount[ind1] = 1;
				lp->imatcolind[k] = i;
				lp->imatcolcoeff[k] = 1.0;
				ind1++;
			}
			else if (lp->aconstrainttype[i]=='L') {
				lp->irhs[i] = lp->arhs[i];
				lp->ilowerbound[ind1] = 0.0;
				lp->iupperbound[ind1] = ALPS_REAL_INFINITY;
				lp->varstat[ind1] = ALPS_BOUNDEDBELOW;
				lp->imatcolbeg[ind1] = k;
				lp->imatcolcount[ind1] = 1;
				lp->imatcolind[k] = i;
				lp->imatcolcoeff[k] = 1.0;
				ind1++;
			}
			else {
				lp->irhs[i] = lp->arhs[i];
				lp->ilowerbound[ind2] = 0.0;
				lp->iupperbound[ind2] = 0.0;
				lp->varstat[ind2] = ALPS_FIXED;
				lp->imatcolbeg[ind2] = k;
				lp->imatcolcount[ind2] = 1;
				lp->imatcolind[k] = i;
				lp->imatcolcoeff[k] = 1.0;
				ind2++;
			}
			k++;
		}
		lp->inonartif = ind1;
		for (j=lp->an;j<lp->in;j++) lp->iphase2obj[j] = 0.0;
		lp->iobj = lp->iphase2obj;


	}
	alps_scaling(pril,lp);
	/** Compute transpose of internal matrix */
	/*
	*  Final initializations. Here we compute the internal matrix in sparse
	* row format.
	*/

	rowptr = (int *) malloc( lp->im * sizeof(int) );
	lp->imatrowbeg = (int *) malloc( lp->im * sizeof(int) );
	lp->imatrowind = (int *) malloc( lp->inonz * sizeof(int) );
	lp->imatrowcoeff = (double *) malloc( lp->inonz * sizeof(double) );
	if (!rowptr || !lp->imatrowbeg || !lp->imatrowind || !lp->imatrowcoeff )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}



	rowptr[0] = 0;
	lp->imatrowbeg[0] = 0;
	for (k=1; k<lp->im; k++) {
		rowptr[k] = lp->imatrowbeg[k-1] + lp->imatrowcount[k-1];
		lp->imatrowbeg[k] = rowptr[k];
	}
	for (j=0; j<lp->in; j++) {

		for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j]; k++) {
			rowindex = lp->imatcolind[k];
			index = rowptr[rowindex];
			lp->imatrowind[index] = j;
			lp->imatrowcoeff[index] = lp->imatcolcoeff[k];
			rowptr[rowindex] = index + 1;
		}
	}
	ffree( (char **) &rowptr);
	if (pril>=4) printintlpdata(lp);


	/** Do some final initializations */
	/*
	*  Initially, we neither have a basis, nor know anything meaningful
	* about the problem status.
	*/
	lp->rumatbeg = NULL;
	lp->rumatend = NULL;
	lp->rumatind = NULL;
	lp->rumatcoeff = NULL;
	lp->clmatbeg = NULL;
	lp->clmatend = NULL;
	lp->clmatind = NULL;
	lp->clmatcoeff = NULL;
	lp->cumatbeg = NULL;
	lp->cumatend = NULL;
	lp->cumatind = NULL;
	lp->cumatcoeff = NULL;
	lp->problemstatus = ALPS_UNKNOWN;
	lp->etanr = 0;
	lp->etafirstfree = 0;
	for (j=0; j<lp->in; j++) lp->cand[j] = lp->in - j - 1;
	for (i=0; i<lp->im; i++) lp->u[i] = 0.0;

	if (pril>=4) printlpdata(lp);
	if (pril>=4) printintlpdata(lp);

	return 0;
}

} // end namespace ogdf
