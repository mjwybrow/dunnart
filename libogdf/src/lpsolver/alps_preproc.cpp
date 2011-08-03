/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Preprocessing.
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
// Preprocessing. Here we perform several preprocessing procedures in
// order to reduce the problem. Preprocessing modifies the LP and stores
// data such that the original representation can be reconstructed.
// Preprocessing requires that the external matrix is available both in
// column and in row format and that the internal problem will be a {\bf
// maximization} problem. In the present version the row format is
// generated here, but later this should be done in the input routine
// already. Preprocessing is performed by calling four functions in an
// appropriate way. First, the initialization routine |alps_prepinit()|
// has to be called. Then the preprocessing routine |alps_preprocess()|
// can be called as long as some actions are performed. Based on the
// information collected during preprocessing, the function
// |alps_prepgenprob()| generates the internal problem to be solved by
// the simplex algorithm. By calling |alps_prepundo()| an optimum
// solution for the original problem is retrieved. Preprocessing is
// currently under development.
//

/*
*  Include files and definitions. We have three print levels: |PL1| only
* gives initial and final statistics, |PL2| gives numbers of eliminated
* columns, changed bounds, etc., |PL3| gives all individual
* modifications of the LP. |PL3| should be equal to 3 in later versions.
*/


#include "alps_preproc.h"
#include "alps.h"
#include "alps_misc.h"
#include "math.h"

#define PL1 1
#define PL2 2
#define PL3 3

namespace ogdf {

int *org_colnr;     /* original column number of internal variable */
int *org_rownr;     /* original row number of internal row */
int *new_rownr;     /* new row number of external row */
int *new_colnr;     /* new col number of external col */
int *prep_rowclass; /* class of row G,L,E */
int *prep_rowstat;  /* elimination possible? */
int *prep_nrowvars; /* nr of nonfixed vars in row */
int *prep_colstat;  /* bound status */
double *prep_obj;   /* current objective */
double *prep_lowerbound; /* current lower bound */
double *prep_upperbound; /* current upper bound */
double *prep_rhs; /* current right hand side */

int prep_nrfixvars; /* total number of fixed variables */
int prep_nrredrows; /* total number of redundant rows */
int prep_probstat; /* problem status */

double prep_objcorr; /* obj constant */

int *prep_rowptr; /* for row format */
int *prep_amatrowcount;
int *prep_amatrowbeg;
int *prep_amatrowind;
double *prep_amatrowcoeff;

struct prep_struct *prep_stack;
int prep_stacksize;
int prep_stackptr;

int push_stack(
			   int a,
			   int r,
			   int c,
			   double v1,
			   double v2,
			   double v3) 
{
	prep_stack[prep_stackptr].action = a;
	prep_stack[prep_stackptr].rowind = r;
	prep_stack[prep_stackptr].colind = c;
	prep_stack[prep_stackptr].val1 = v1;
	prep_stack[prep_stackptr].val2 = v2;
	prep_stack[prep_stackptr].val3 = v3;
	prep_stackptr++;
	if (prep_stackptr>=prep_stacksize) {
		prep_stacksize *= 2;
		prep_stack = (struct prep_struct *)
			realloc (prep_stack,prep_stacksize * sizeof(struct prep_struct) );
		if (!prep_stack ) return -1;
	}
	return 0;
}

int pop_stack(
			  int *a,
			  int *r,
			  int *c,
			  double *v1,
			  double *v2,
			  double *v3) 
{
	prep_stackptr--;
	if (prep_stackptr<0) return -1;
	*a = prep_stack[prep_stackptr].action;
	*r = prep_stack[prep_stackptr].rowind;
	*c = prep_stack[prep_stackptr].colind;
	*v1 = prep_stack[prep_stackptr].val1;
	*v2 = prep_stack[prep_stackptr].val2;
	*v3 = prep_stack[prep_stackptr].val3;
	return 0;
}

/*
*  Initialization for preprocessing. The initialization routine
* classifies rows and columns of the original problem. The original
* problem will never be modified, new bounds and rhs are stored
* temporarily for setting up the internal problem later. The temporary
* objective function takes into account that the internal problem will
* be a maximization problem. The layout of the initialization routine
* for preprocessing is as follows.
*/


int alps_prepinit( int pril, lpstruct *lp  )
{
	int orgless,orggreater,orgequal,orgfixed,orgfree,orglowbd,orgupbd,orgbd;
	int nrinfeasrows,nrvoidcols,nrvoidrows;

	nrvoidrows = 0;

	if (!lp->preprocessing) return 0;
	if (pril>=PL1) printf("Preprocessing ...\n");
	/** Allocate memory for preprocessing */

	prep_rowclass = (int *) malloc( lp->am * sizeof(int) );
	prep_rowstat = (int *) malloc( lp->am * sizeof(int) );
	prep_nrowvars = (int *) malloc( lp->am * sizeof(int) );
	prep_colstat = (int *) malloc( lp->an * sizeof(int) );
	prep_obj = (double *) malloc( lp->an * sizeof(double) );
	prep_lowerbound = (double *) malloc( lp->an * sizeof(double) );
	prep_upperbound = (double *) malloc( lp->an * sizeof(double) );
	prep_rhs = (double *) malloc( lp->am * sizeof(double) );
	if (!prep_rowclass || !prep_rowstat || !prep_nrowvars || !prep_colstat 
		|| !prep_obj || !prep_lowerbound || !prep_upperbound || !prep_rhs )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	prep_stacksize = lp->am + lp->an;
	prep_stackptr = 0;
	prep_stack = (struct prep_struct *) 
		malloc (prep_stacksize * sizeof(struct prep_struct) );
	if (!prep_stack )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	/** Generate input matrix in row format */
	/*
	*  We need to have access to the input matrix in row format. This
	* section can be removed as soon as the input matrix is provided in row
	* format by |alps_initproblem()|.
	*/
	{
		int nonz,j,k,rowindex,index;
		prep_rowptr = (int *) malloc( lp->am * sizeof(int) );
		prep_amatrowcount = (int *) malloc( lp->am * sizeof(int) );
		if (!prep_rowptr || !prep_amatrowcount )
			// run out of memory

		{
			fprintf (stderr, "run out of memory\n");
			return(ALPS_RUNOUTOFMEMORY);
		}

		nonz = 0;
		for (k=0;k<lp->am;k++) prep_amatrowcount[k] = 0;
		for (k=0;k<lp->amatcolbeg[lp->an-1]+lp->amatcolcount[lp->an-1];k++) {
			rowindex = lp->amatcolind[k];
			prep_amatrowcount[rowindex]++;
			nonz++;
		}
		prep_amatrowbeg = (int *) malloc( lp->am * sizeof(int) );
		prep_amatrowind = (int *) malloc( nonz * sizeof(int) );
		prep_amatrowcoeff = (double *) malloc( nonz * sizeof(double) );
		if (!prep_amatrowbeg || !prep_amatrowind || !prep_amatrowcoeff )
			// run out of memory

		{
			fprintf (stderr, "run out of memory\n");
			return(ALPS_RUNOUTOFMEMORY);
		}

		prep_amatrowbeg[0] = 0;
		prep_rowptr[0] = 0;
		for (k=1;k<lp->am;k++) {
			prep_rowptr[k] = prep_amatrowbeg[k-1] + prep_amatrowcount[k-1];
			prep_amatrowbeg[k] = prep_rowptr[k];
		}
		for (j=0;j<lp->an;j++) {
			for (k=lp->amatcolbeg[j];k<lp->amatcolbeg[j]+lp->amatcolcount[j];k++) {
				rowindex = lp->amatcolind[k];
				index = prep_rowptr[rowindex];
				prep_amatrowind[index] = j;
				prep_amatrowcoeff[index] = lp->amatcolcoeff[k];
				prep_rowptr[rowindex] = index + 1;
			}
		}
	}


	/** Classify rows */
	/*
	*  We store the type of constraints in |prep_rowclass|. Note that the
	* type of a constraint may change, namely equations can be turned into
	* inequalities after preprocessing.
	*/
	{ 
		int i;
		orgless = orggreater = orgequal = 0;
		for (i=0;i<lp->am;i++) {
			if (lp->aconstrainttype[i]=='G') {
				prep_rowclass[i] = PREP_ROW_GREATER;
				orggreater++;
			}
			else if (lp->aconstrainttype[i]=='L') {
				prep_rowclass[i] = PREP_ROW_LESS;
				orgless++;
			}
			else {
				prep_rowclass[i] = PREP_ROW_EQUAL;
				orgequal++;
			}
		}
	}


	/** Classify columns */
	/*
	*  Variables are classified according to their bounds. Fixed variables
	* will be registered. Necessary correction of rows due to variable
	* fixing will be performed during final initialization.
	*/
	{ 
		int j;
		double newfix;
		prep_nrfixvars = 0;
		nrvoidcols = 0;
		orgfixed = orgfree = orglowbd = orgupbd = orgbd = 0;
		for (j=0;j<lp->an;j++) {
			double ilbj = prep_lowerbound[j] = lp->alowerbound[j];
			double iubj = prep_upperbound[j] = lp->aupperbound[j];
			if (ilbj>-ALPS_REAL_INFINITY) {
				if (iubj<ALPS_REAL_INFINITY) {
					if (ilbj<iubj) {
						prep_colstat[j] = PREP_VAR_BOUNDEDTWICE;
						orgbd++;
					}
					else {
						newfix = prep_lowerbound[j];
						// Register variable fixing

						{
							push_stack(FIX_VARIABLE,-1,j,prep_lowerbound[j],prep_upperbound[j],newfix);
							prep_colstat[j] = PREP_VAR_FIXED;
							nrvoidcols++;
							if (pril>=PL3) {
								printf("Variable %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
								if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
								else printf("-INFTY ");
								if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
								else printf("INFTY] ");
								printf("fixed to %.3lf\n",newfix);
							}
							prep_lowerbound[j] = prep_upperbound[j] = newfix;
						}


						orgfixed++;
					}
				}
				else{
					prep_colstat[j] = PREP_VAR_BOUNDEDBELOW;
					orglowbd++;
				}
			}
			else {
				if (iubj<ALPS_REAL_INFINITY) {
					prep_colstat[j] = PREP_VAR_BOUNDEDABOVE;
					orgupbd++;
				}
				else{ /* no finite upperbound */
					prep_colstat[j] = PREP_VAR_FREE;
					orgfree++;
				}
			}
		}
		prep_nrfixvars = nrvoidcols;
		prep_nrredrows = nrvoidrows;
	}


	/** Initialize further data */
	/*
	*  Some final initializations are performed. The objective function is
	* complemented if necessary. Fixed variables are eliminated from the
	* constraints and the right hand sides are adjusted. This can lead to
	* the detection of redundant or infeasible rows. The variable
	* |prep_objcorr| is needed for computing the correct objective function
	* value.
	*/
	nrinfeasrows = 0;
	{ int i,j;
	double lhs;
	if (lp->aobjminmax==ALPS_MINIMIZE) {
		for (j=0;j<lp->an;j++) prep_obj[j] = -lp->aobj[j];
	}
	else {
		for (j=0;j<lp->an;j++) prep_obj[j] = lp->aobj[j];
	}
	prep_objcorr = 0.0;
	nrvoidrows = 0;
	for (i=0;i<lp->am;i++) {
		prep_rowstat[i] = PREP_ROW_OK;
		prep_nrowvars[i] = prep_amatrowcount[i];
		prep_rhs[i] = lp->arhs[i];
	}
	for (j=0;j<lp->an;j++) if (prep_colstat[j]==PREP_VAR_FIXED) {
		// Adjust constraints and objective
		/*
		*  If variable $j$ is fixed then the constraints have to be corrected
		* and the objective function correction value updated.
		*/
		{ int ii,kk;
		for (kk=lp->amatcolbeg[j];kk<lp->amatcolbeg[j]+lp->amatcolcount[j];kk++) {
			ii = lp->amatcolind[kk];
			prep_rhs[ii] -= lp->amatcolcoeff[kk]*prep_upperbound[j];
			prep_nrowvars[ii]--;
		}
		prep_objcorr += prep_obj[j]*prep_upperbound[j];
		}


	}
	for (i=0;i<lp->am;i++) if (prep_nrowvars[i]==0) {
		lhs = lp->arhs[i] - prep_rhs[i];
		switch (prep_rowclass[i]) {
  case PREP_ROW_GREATER:
	  if (prep_rhs[i]>ALPS_ZEROEPS)
		  // Register infeasible row

	  {
		  push_stack(INFEASIBLE_ROW,i,-1,-1,-1,-1);
		  prep_rowstat[i] = PREP_ROW_INFEAS;
		  nrinfeasrows++;
		  if (pril>=PL2) printf("Infeasible row %d (%s): %.4lf < %.4lf!\n",
			  i,lp->arownamestr+lp->arownameind[i],lhs,lp->arhs[i]);
	  }


	  else
		  // Register redundant row
		  /*
		  *  Store preprocessing actions. Actions performed during preprocessing
		  * are stored in a stack. Indices of involved rows and columns have to be
		  * |i| and |j| for the respective actions. Variables are fixed to value
		  * |newfix|, bound changes are given by |newlb| and |newub|.
		  */
	  {
		  push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
		  prep_rowstat[i] = PREP_ROW_REDUND;
		  nrvoidrows++;
		  if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
			  i,lp->arownamestr+lp->arownameind[i]);
	  }


	  break;
  case PREP_ROW_LESS:
	  if (prep_rhs[i]<-ALPS_ZEROEPS)
		  // Register infeasible row

	  {
		  push_stack(INFEASIBLE_ROW,i,-1,-1,-1,-1);
		  prep_rowstat[i] = PREP_ROW_INFEAS;
		  nrinfeasrows++;
		  if (pril>=PL2) printf("Infeasible row %d (%s): %.4lf < %.4lf!\n",
			  i,lp->arownamestr+lp->arownameind[i],lhs,lp->arhs[i]);
	  }


	  else
		  // Register redundant row
		  /*
		  *  Store preprocessing actions. Actions performed during preprocessing
		  * are stored in a stack. Indices of involved rows and columns have to be
		  * |i| and |j| for the respective actions. Variables are fixed to value
		  * |newfix|, bound changes are given by |newlb| and |newub|.
		  */
	  {
		  push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
		  prep_rowstat[i] = PREP_ROW_REDUND;
		  nrvoidrows++;
		  if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
			  i,lp->arownamestr+lp->arownameind[i]);
	  }


	  break;
  case PREP_ROW_EQUAL:
	  if (fabs(prep_rhs[i])>ALPS_ZEROEPS)
		  // Register infeasible row

	  {
		  push_stack(INFEASIBLE_ROW,i,-1,-1,-1,-1);
		  prep_rowstat[i] = PREP_ROW_INFEAS;
		  nrinfeasrows++;
		  if (pril>=PL2) printf("Infeasible row %d (%s): %.4lf < %.4lf!\n",
			  i,lp->arownamestr+lp->arownameind[i],lhs,lp->arhs[i]);
	  }


	  else
		  // Register redundant row
		  /*
		  *  Store preprocessing actions. Actions performed during preprocessing
		  * are stored in a stack. Indices of involved rows and columns have to be
		  * |i| and |j| for the respective actions. Variables are fixed to value
		  * |newfix|, bound changes are given by |newlb| and |newub|.
		  */
	  {
		  push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
		  prep_rowstat[i] = PREP_ROW_REDUND;
		  nrvoidrows++;
		  if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
			  i,lp->arownamestr+lp->arownameind[i]);
	  }


	  break;
		}
	}
	}


	prep_nrredrows = nrvoidrows;
	if (pril>=PL2) {
		printf("Void rows:        %6d\n", prep_nrredrows);
		printf("Infeasible rows:  %6d\n", nrinfeasrows);
	}
	if (nrinfeasrows) {
		if (pril>=PL1) printf("Problem infeasible (bad row)!\n");
		return ALPS_INFEASIBLE;
	}
	else prep_probstat = PREP_PROB_OK;
	if (pril>=PL1) {
		printf("Number of rows:   %6d\n",orgless+orggreater+orgequal);
		printf("        'less'    %6d\n",orgless);
		printf("     'greater'    %6d\n",orggreater);
		printf("       'equal'    %6d\n",orgequal);
		printf("Number of vars:   %6d\n",orgfixed+orgfree+orglowbd+orgupbd+orgbd);
		printf("       'fixed'    %6d\n",orgfixed);
		printf("        'free'    %6d\n",orgfree);
		printf("      'low bd'    %6d\n",orglowbd);
		printf("       'up bd'    %6d\n",orgupbd);
		printf("     'bounded'    %6d\n",orgbd);
	}

	return 0;
}

/*
*  The preprocessing routine. It is advisable to call the preprocessing
* routine several times as long as some modifications of the problem
* were performed (|actions>0|) and no error occured (|retcode=0|). The
* layout of the preprocessing package is as follows.
*/


int alps_preprocess(
					int pril,
					lpstruct *lp,
					int *retcode,
					int *actions
					)

{
	/** Declare preprocessing scalars */

	int prep_actions;
	int nrvoidrows,nrinfeasrows,nrvoidcols,nrembounds,nremslacks;



	if (!lp->preprocessing) return 0;
	prep_actions = 0;
	/** Search for void rows and columns */
	/*
	*  Eliminate void rows and columns. We check if there are rows or
	* columns that can be eliminated from the problem. This can either be
	* due to zero rows/columns or due to the fact that all variables of a
	* row are fixed.
	*/
	{
		int i,j,k;
		double lhs,newfix;

		nrvoidrows = 0;
		nrinfeasrows = 0;
		for (i=0;i<lp->am;i++) {
			if (prep_rowstat[i]==PREP_ROW_REDUND
				|| prep_nrowvars[i]>0) goto nextrow;
			lhs = 0.0;
			for (k=prep_amatrowbeg[i];k<prep_amatrowbeg[i]+prep_amatrowcount[i];k++) {
				j = prep_amatrowind[k];
				lhs += prep_amatrowcoeff[k]*prep_lowerbound[j];
			}
			switch (prep_rowclass[i]) {
  case PREP_ROW_GREATER:
	  if (lhs<lp->arhs[i]-ALPS_ZEROEPS)
		  // Register infeasible row

	  {
		  push_stack(INFEASIBLE_ROW,i,-1,-1,-1,-1);
		  prep_rowstat[i] = PREP_ROW_INFEAS;
		  nrinfeasrows++;
		  if (pril>=PL2) printf("Infeasible row %d (%s): %.4lf < %.4lf!\n",
			  i,lp->arownamestr+lp->arownameind[i],lhs,lp->arhs[i]);
	  }


	  else
		  // Register redundant row
		  /*
		  *  Store preprocessing actions. Actions performed during preprocessing
		  * are stored in a stack. Indices of involved rows and columns have to be
		  * |i| and |j| for the respective actions. Variables are fixed to value
		  * |newfix|, bound changes are given by |newlb| and |newub|.
		  */
	  {
		  push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
		  prep_rowstat[i] = PREP_ROW_REDUND;
		  nrvoidrows++;
		  if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
			  i,lp->arownamestr+lp->arownameind[i]);
	  }


	  break;
  case PREP_ROW_LESS:
	  if (lhs>lp->arhs[i]+ALPS_ZEROEPS)
		  // Register infeasible row

	  {
		  push_stack(INFEASIBLE_ROW,i,-1,-1,-1,-1);
		  prep_rowstat[i] = PREP_ROW_INFEAS;
		  nrinfeasrows++;
		  if (pril>=PL2) printf("Infeasible row %d (%s): %.4lf < %.4lf!\n",
			  i,lp->arownamestr+lp->arownameind[i],lhs,lp->arhs[i]);
	  }


	  else
		  // Register redundant row
		  /*
		  *  Store preprocessing actions. Actions performed during preprocessing
		  * are stored in a stack. Indices of involved rows and columns have to be
		  * |i| and |j| for the respective actions. Variables are fixed to value
		  * |newfix|, bound changes are given by |newlb| and |newub|.
		  */
	  {
		  push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
		  prep_rowstat[i] = PREP_ROW_REDUND;
		  nrvoidrows++;
		  if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
			  i,lp->arownamestr+lp->arownameind[i]);
	  }


	  break;
  case PREP_ROW_EQUAL:
	  if (fabs(lhs-lp->arhs[i])>ALPS_ZEROEPS)
		  // Register infeasible row

	  {
		  push_stack(INFEASIBLE_ROW,i,-1,-1,-1,-1);
		  prep_rowstat[i] = PREP_ROW_INFEAS;
		  nrinfeasrows++;
		  if (pril>=PL2) printf("Infeasible row %d (%s): %.4lf < %.4lf!\n",
			  i,lp->arownamestr+lp->arownameind[i],lhs,lp->arhs[i]);
	  }


	  else
		  // Register redundant row
		  /*
		  *  Store preprocessing actions. Actions performed during preprocessing
		  * are stored in a stack. Indices of involved rows and columns have to be
		  * |i| and |j| for the respective actions. Variables are fixed to value
		  * |newfix|, bound changes are given by |newlb| and |newub|.
		  */
	  {
		  push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
		  prep_rowstat[i] = PREP_ROW_REDUND;
		  nrvoidrows++;
		  if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
			  i,lp->arownamestr+lp->arownameind[i]);
	  }


	  break;
			}
			prep_actions = 1;
nextrow:;
		}
		prep_nrredrows += nrvoidrows;
		if (pril>=PL2) {
			printf("Void rows:        %6d\n", nrvoidrows);
			printf("Infeasible rows:  %6d\n", nrinfeasrows);
		}
		if (nrinfeasrows) {
			if (pril>=PL1) printf("Problem infeasible (bad row)!\n");
			*retcode = PREP_PROB_INFEAS;
			return ALPS_INFEASIBLE;
		}

		nrvoidcols = 0;
		for (j=0;j<lp->an;j++) {
			if (prep_colstat[j]==PREP_VAR_FIXED) goto nextcol;
			for (k=lp->amatcolbeg[j];k<lp->amatcolbeg[j]+lp->amatcolcount[j];k++) {
				i = lp->amatcolind[k];
				if (prep_rowstat[i]!=PREP_ROW_REDUND) goto nextcol;
			}
			if (prep_obj[j]>ALPS_ZEROEPS) {
				if (prep_colstat[j]==PREP_VAR_BOUNDEDABOVE) {
					newfix = prep_upperbound[j];
					// Register variable fixing

					{
						push_stack(FIX_VARIABLE,-1,j,prep_lowerbound[j],prep_upperbound[j],newfix);
						prep_colstat[j] = PREP_VAR_FIXED;
						nrvoidcols++;
						if (pril>=PL3) {
							printf("Variable %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
							if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
							else printf("-INFTY ");
							if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
							else printf("INFTY] ");
							printf("fixed to %.3lf\n",newfix);
						}
						prep_lowerbound[j] = prep_upperbound[j] = newfix;
					}


				}
				else {
					*retcode = PREP_PROB_UNSOLVABLE;
					return ALPS_UNKNOWN;
				}
			}
			else if (prep_obj[j]<-ALPS_ZEROEPS) {
				if (prep_colstat[j]==PREP_VAR_BOUNDEDBELOW) {
					newfix = prep_lowerbound[j];
					// Register variable fixing

					{
						push_stack(FIX_VARIABLE,-1,j,prep_lowerbound[j],prep_upperbound[j],newfix);
						prep_colstat[j] = PREP_VAR_FIXED;
						nrvoidcols++;
						if (pril>=PL3) {
							printf("Variable %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
							if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
							else printf("-INFTY ");
							if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
							else printf("INFTY] ");
							printf("fixed to %.3lf\n",newfix);
						}
						prep_lowerbound[j] = prep_upperbound[j] = newfix;
					}


				}
				else {
					*retcode = PREP_PROB_UNSOLVABLE;
					return ALPS_UNKNOWN;
				}
			}
			else {
				newfix = 0.0;
				// Register variable fixing

				{
					push_stack(FIX_VARIABLE,-1,j,prep_lowerbound[j],prep_upperbound[j],newfix);
					prep_colstat[j] = PREP_VAR_FIXED;
					nrvoidcols++;
					if (pril>=PL3) {
						printf("Variable %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
						if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
						else printf("-INFTY ");
						if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
						else printf("INFTY] ");
						printf("fixed to %.3lf\n",newfix);
					}
					prep_lowerbound[j] = prep_upperbound[j] = newfix;
				}


			}
			// Adjust constraints and objective
			/*
			*  If variable $j$ is fixed then the constraints have to be corrected
			* and the objective function correction value updated.
			*/
			{ int ii,kk;
			for (kk=lp->amatcolbeg[j];kk<lp->amatcolbeg[j]+lp->amatcolcount[j];kk++) {
				ii = lp->amatcolind[kk];
				prep_rhs[ii] -= lp->amatcolcoeff[kk]*prep_upperbound[j];
				prep_nrowvars[ii]--;
			}
			prep_objcorr += prep_obj[j]*prep_upperbound[j];
			}


			prep_actions = 1;

nextcol:;
		}
		if (pril>=PL2) printf("Void columns:     %6d\n", nrvoidcols);
		prep_nrfixvars += nrvoidcols;
	}


	/** Search for embedded bounds */
	/*
	*  Take embedded bounds into account. Constraints containing only one
	* nonfixed structural variable can lead to a strengthening of the bounds
	* of this variable. If the constraint is an equation then the variable
	* can be fixed.
	*/
	{ int i,j,k,nrvoidcols;
	double aij,newupper,newlower,newfix;
	nrembounds = 0;
	nrvoidcols = 0;
	for (i=0;i<lp->am;i++) 
		if (prep_nrowvars[i]==1 && prep_rowstat[i]!=PREP_ROW_REDUND) {
			k = prep_amatrowbeg[i];
			j = prep_amatrowind[k];
			while (prep_colstat[j]==PREP_VAR_FIXED) {
				k++;
				j = prep_amatrowind[k];
			}
			aij = prep_amatrowcoeff[k];
			switch (prep_rowclass[i]) {
  case PREP_ROW_GREATER:
	  if (aij<-ALPS_ZEROEPS) {
		  newupper = prep_rhs[i] / aij;
		  if (fabs(newupper)<ALPS_ZEROEPS) newupper = 0.0;
		  if (newupper<prep_upperbound[j]) {
			  // Register upper bound change

			  {
				  push_stack(UPPER_BOUND_CHANGE,i,j,prep_lowerbound[j],prep_upperbound[j],newupper);
				  if (pril>=PL3) {
					  printf("Upper bound of %1d (%s) :",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_colstat[j]==PREP_VAR_BOUNDEDABOVE 
						  || prep_colstat[j]==PREP_VAR_BOUNDEDTWICE)
						  printf("%.3lf ",prep_upperbound[j]);
					  else printf("INFTY ");
					  printf(" -> %.3lf\n",newupper);
				  }
				  if (prep_colstat[j]==PREP_VAR_BOUNDEDBELOW) prep_colstat[j] = PREP_VAR_BOUNDEDTWICE;
				  if (prep_colstat[j]==PREP_VAR_FREE) prep_colstat[j] = PREP_VAR_BOUNDEDABOVE;
				  prep_upperbound[j] = newupper;
			  }


			  nrembounds++;
		  }
	  }
	  else if (aij>ALPS_ZEROEPS) {
		  newlower = prep_rhs[i] / aij;
		  if (fabs(newlower)<ALPS_ZEROEPS) newlower = 0.0;
		  if (newlower>prep_lowerbound[j]) {
			  // Register lower bound change

			  {
				  push_stack(LOWER_BOUND_CHANGE,i,j,prep_lowerbound[j],prep_upperbound[j],newlower);
				  if (pril>=PL3) {
					  printf("Lower bound of %1d (%s) :",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_colstat[j]==PREP_VAR_BOUNDEDBELOW 
						  || prep_colstat[j]==PREP_VAR_BOUNDEDTWICE)
						  printf("%.3lf ",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  printf(" -> %.3lf\n",newlower);
				  }
				  if (prep_colstat[j]==PREP_VAR_BOUNDEDABOVE) prep_colstat[j] = PREP_VAR_BOUNDEDTWICE;
				  if (prep_colstat[j]==PREP_VAR_FREE) prep_colstat[j] = PREP_VAR_BOUNDEDBELOW;
				  prep_lowerbound[j] = newlower;
			  }


			  nrembounds++;
		  }
	  }
	  break;
  case PREP_ROW_LESS:
	  if (aij>ALPS_ZEROEPS) {
		  newupper = prep_rhs[i] / aij;
		  if (fabs(newupper)<ALPS_ZEROEPS) newupper = 0.0;
		  if (newupper<prep_upperbound[j]) {
			  // Register upper bound change

			  {
				  push_stack(UPPER_BOUND_CHANGE,i,j,prep_lowerbound[j],prep_upperbound[j],newupper);
				  if (pril>=PL3) {
					  printf("Upper bound of %1d (%s) :",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_colstat[j]==PREP_VAR_BOUNDEDABOVE 
						  || prep_colstat[j]==PREP_VAR_BOUNDEDTWICE)
						  printf("%.3lf ",prep_upperbound[j]);
					  else printf("INFTY ");
					  printf(" -> %.3lf\n",newupper);
				  }
				  if (prep_colstat[j]==PREP_VAR_BOUNDEDBELOW) prep_colstat[j] = PREP_VAR_BOUNDEDTWICE;
				  if (prep_colstat[j]==PREP_VAR_FREE) prep_colstat[j] = PREP_VAR_BOUNDEDABOVE;
				  prep_upperbound[j] = newupper;
			  }


			  nrembounds++;
		  }
	  }
	  else if (aij<-ALPS_ZEROEPS) {
		  newlower = prep_rhs[i] / aij;
		  if (fabs(newlower)<ALPS_ZEROEPS) newlower = 0.0;
		  if (newlower>prep_lowerbound[j]) {
			  // Register lower bound change

			  {
				  push_stack(LOWER_BOUND_CHANGE,i,j,prep_lowerbound[j],prep_upperbound[j],newlower);
				  if (pril>=PL3) {
					  printf("Lower bound of %1d (%s) :",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_colstat[j]==PREP_VAR_BOUNDEDBELOW 
						  || prep_colstat[j]==PREP_VAR_BOUNDEDTWICE)
						  printf("%.3lf ",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  printf(" -> %.3lf\n",newlower);
				  }
				  if (prep_colstat[j]==PREP_VAR_BOUNDEDABOVE) prep_colstat[j] = PREP_VAR_BOUNDEDTWICE;
				  if (prep_colstat[j]==PREP_VAR_FREE) prep_colstat[j] = PREP_VAR_BOUNDEDBELOW;
				  prep_lowerbound[j] = newlower;
			  }


			  nrembounds++;
		  }
	  }
	  break;
  case PREP_ROW_EQUAL:
	  if (fabs(aij)>ALPS_ZEROEPS) {
		  newfix = prep_rhs[i] / aij;
		  if (fabs(newfix)<ALPS_ZEROEPS) newfix = 0.0;
		  // Register variable fixing

		  {
			  push_stack(FIX_VARIABLE,-1,j,prep_lowerbound[j],prep_upperbound[j],newfix);
			  prep_colstat[j] = PREP_VAR_FIXED;
			  nrvoidcols++;
			  if (pril>=PL3) {
				  printf("Variable %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
				  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
				  else printf("-INFTY ");
				  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
				  else printf("INFTY] ");
				  printf("fixed to %.3lf\n",newfix);
			  }
			  prep_lowerbound[j] = prep_upperbound[j] = newfix;
		  }


		  // Adjust constraints and objective
		  /*
		  *  If variable $j$ is fixed then the constraints have to be corrected
		  * and the objective function correction value updated.
		  */
		  { int ii,kk;
		  for (kk=lp->amatcolbeg[j];kk<lp->amatcolbeg[j]+lp->amatcolcount[j];kk++) {
			  ii = lp->amatcolind[kk];
			  prep_rhs[ii] -= lp->amatcolcoeff[kk]*prep_upperbound[j];
			  prep_nrowvars[ii]--;
		  }
		  prep_objcorr += prep_obj[j]*prep_upperbound[j];
		  }


		  nrembounds++;
	  }
	  break;
			}
			if (prep_upperbound[j]<prep_lowerbound[j]-ALPS_ZEROEPS) {
				if (pril>=PL1) printf("Conflicting bounds for var %1s!\n",
					lp->acolnamestr+lp->acolnameind[j]);
				*retcode = PREP_PROB_INFEAS;
				return ALPS_INFEASIBLE;
			}

			if (fabs(prep_upperbound[j]-prep_lowerbound[j])<ALPS_ZEROEPS
				&& prep_colstat[j]!=PREP_VAR_FIXED) {
					newfix = prep_upperbound[j];
					// Register variable fixing

					{
						push_stack(FIX_VARIABLE,-1,j,prep_lowerbound[j],prep_upperbound[j],newfix);
						prep_colstat[j] = PREP_VAR_FIXED;
						nrvoidcols++;
						if (pril>=PL3) {
							printf("Variable %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
							if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
							else printf("-INFTY ");
							if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
							else printf("INFTY] ");
							printf("fixed to %.3lf\n",newfix);
						}
						prep_lowerbound[j] = prep_upperbound[j] = newfix;
					}


					// Adjust constraints and objective
					/*
					*  If variable $j$ is fixed then the constraints have to be corrected
					* and the objective function correction value updated.
					*/
					{ int ii,kk;
					for (kk=lp->amatcolbeg[j];kk<lp->amatcolbeg[j]+lp->amatcolcount[j];kk++) {
						ii = lp->amatcolind[kk];
						prep_rhs[ii] -= lp->amatcolcoeff[kk]*prep_upperbound[j];
						prep_nrowvars[ii]--;
					}
					prep_objcorr += prep_obj[j]*prep_upperbound[j];
					}


				}

		}
		if (pril>=PL2) printf("Embedded bounds:  %6d\n", nrembounds);
		if (nrembounds) prep_actions = 1;
		prep_nrfixvars += nrvoidcols;
	}


	/** Search for embedded slacks */
	/*
	*  Identify embedded slacks. We serach for variables that occur only in
	* one constraint.
	*/
	{ int i,j,k,nrel,row;
	double aij;
	nremslacks = 0;
	nrvoidrows = 0;
	for (j=0;j<lp->an;j++) if (prep_colstat[j]==PREP_VAR_BOUNDEDBELOW
		|| prep_colstat[j]==PREP_VAR_BOUNDEDABOVE) {
			nrel = 0;
			for (k=lp->amatcolbeg[j];k<lp->amatcolbeg[j]+lp->amatcolcount[j];k++) {
				i = lp->amatcolind[k];
				if (prep_rowstat[i]!=PREP_ROW_REDUND) {
					if (nrel>0) goto nextslack;
					nrel++;
					row = i;
					aij = lp->amatcolcoeff[k];
				}
			}
			if (nrel!=1 || fabs(prep_obj[j])>ALPS_ZEROEPS) goto nextslack;
			i = row;
			switch (prep_rowclass[i]) {
  case PREP_ROW_GREATER:
	  if (aij>ALPS_ZEROEPS) {
		  if (prep_colstat[j]==PREP_VAR_BOUNDEDABOVE) {
			  prep_nrowvars[i]--;
			  // Register embedded slack elimination

			  {
				  push_stack(EMBEDDED_SLACK_VAR,i,j,0.0,prep_rhs[i],aij);
				  prep_colstat[j] = PREP_VAR_FIXED;
				  nremslacks++;
				  prep_nrfixvars++;
				  if (pril>=PL3) {
					  printf("Embedded slack %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
					  else printf("INFTY] ");
					  printf("eliminated.\n");
				  }
			  }


			  prep_rhs[i] -= aij*prep_upperbound[j];
		  }
		  else {
			  // Register embedded slack elimination

			  {
				  push_stack(EMBEDDED_SLACK_VAR,i,j,0.0,prep_rhs[i],aij);
				  prep_colstat[j] = PREP_VAR_FIXED;
				  nremslacks++;
				  prep_nrfixvars++;
				  if (pril>=PL3) {
					  printf("Embedded slack %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
					  else printf("INFTY] ");
					  printf("eliminated.\n");
				  }
			  }


			  // Register redundant row
			  /*
			  *  Store preprocessing actions. Actions performed during preprocessing
			  * are stored in a stack. Indices of involved rows and columns have to be
			  * |i| and |j| for the respective actions. Variables are fixed to value
			  * |newfix|, bound changes are given by |newlb| and |newub|.
			  */
			  {
				  push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
				  prep_rowstat[i] = PREP_ROW_REDUND;
				  nrvoidrows++;
				  if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
					  i,lp->arownamestr+lp->arownameind[i]);
			  }


		  }
	  }
	  else if (aij<-ALPS_ZEROEPS) {
		  if (prep_colstat[j]==PREP_VAR_BOUNDEDABOVE) {
			  // Register embedded slack elimination

			  {
				  push_stack(EMBEDDED_SLACK_VAR,i,j,0.0,prep_rhs[i],aij);
				  prep_colstat[j] = PREP_VAR_FIXED;
				  nremslacks++;
				  prep_nrfixvars++;
				  if (pril>=PL3) {
					  printf("Embedded slack %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
					  else printf("INFTY] ");
					  printf("eliminated.\n");
				  }
			  }


			  // Register redundant row
			  /*
			  *  Store preprocessing actions. Actions performed during preprocessing
			  * are stored in a stack. Indices of involved rows and columns have to be
			  * |i| and |j| for the respective actions. Variables are fixed to value
			  * |newfix|, bound changes are given by |newlb| and |newub|.
			  */
			  {
				  push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
				  prep_rowstat[i] = PREP_ROW_REDUND;
				  nrvoidrows++;
				  if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
					  i,lp->arownamestr+lp->arownameind[i]);
			  }


		  }
		  else {
			  prep_nrowvars[i]--;
			  // Register embedded slack elimination

			  {
				  push_stack(EMBEDDED_SLACK_VAR,i,j,0.0,prep_rhs[i],aij);
				  prep_colstat[j] = PREP_VAR_FIXED;
				  nremslacks++;
				  prep_nrfixvars++;
				  if (pril>=PL3) {
					  printf("Embedded slack %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
					  else printf("INFTY] ");
					  printf("eliminated.\n");
				  }
			  }


			  prep_rhs[i] -= aij*prep_lowerbound[j];
		  }
	  }
	  break;
  case PREP_ROW_LESS:
	  if (aij>ALPS_ZEROEPS) {
		  if (prep_colstat[j]==PREP_VAR_BOUNDEDABOVE) {
			  // Register embedded slack elimination

			  {
				  push_stack(EMBEDDED_SLACK_VAR,i,j,0.0,prep_rhs[i],aij);
				  prep_colstat[j] = PREP_VAR_FIXED;
				  nremslacks++;
				  prep_nrfixvars++;
				  if (pril>=PL3) {
					  printf("Embedded slack %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
					  else printf("INFTY] ");
					  printf("eliminated.\n");
				  }
			  }


			  // Register redundant row
			  /*
			  *  Store preprocessing actions. Actions performed during preprocessing
			  * are stored in a stack. Indices of involved rows and columns have to be
			  * |i| and |j| for the respective actions. Variables are fixed to value
			  * |newfix|, bound changes are given by |newlb| and |newub|.
			  */
			  {
				  push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
				  prep_rowstat[i] = PREP_ROW_REDUND;
				  nrvoidrows++;
				  if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
					  i,lp->arownamestr+lp->arownameind[i]);
			  }


		  }
		  else {
			  prep_nrowvars[i]--;
			  // Register embedded slack elimination

			  {
				  push_stack(EMBEDDED_SLACK_VAR,i,j,0.0,prep_rhs[i],aij);
				  prep_colstat[j] = PREP_VAR_FIXED;
				  nremslacks++;
				  prep_nrfixvars++;
				  if (pril>=PL3) {
					  printf("Embedded slack %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
					  else printf("INFTY] ");
					  printf("eliminated.\n");
				  }
			  }


			  prep_rhs[i] -= aij*prep_lowerbound[j];
		  }
	  }
	  else if (aij<-ALPS_ZEROEPS) {
		  if (prep_colstat[j]==PREP_VAR_BOUNDEDABOVE) {
			  prep_nrowvars[i]--;
			  // Register embedded slack elimination

			  {
				  push_stack(EMBEDDED_SLACK_VAR,i,j,0.0,prep_rhs[i],aij);
				  prep_colstat[j] = PREP_VAR_FIXED;
				  nremslacks++;
				  prep_nrfixvars++;
				  if (pril>=PL3) {
					  printf("Embedded slack %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
					  else printf("INFTY] ");
					  printf("eliminated.\n");
				  }
			  }


			  prep_rhs[i] -= aij*prep_upperbound[j];
		  }
		  else {
			  // Register embedded slack elimination

			  {
				  push_stack(EMBEDDED_SLACK_VAR,i,j,0.0,prep_rhs[i],aij);
				  prep_colstat[j] = PREP_VAR_FIXED;
				  nremslacks++;
				  prep_nrfixvars++;
				  if (pril>=PL3) {
					  printf("Embedded slack %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
					  else printf("INFTY] ");
					  printf("eliminated.\n");
				  }
			  }


			  // Register redundant row
			  /*
			  *  Store preprocessing actions. Actions performed during preprocessing
			  * are stored in a stack. Indices of involved rows and columns have to be
			  * |i| and |j| for the respective actions. Variables are fixed to value
			  * |newfix|, bound changes are given by |newlb| and |newub|.
			  */
			  {
				  push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
				  prep_rowstat[i] = PREP_ROW_REDUND;
				  nrvoidrows++;
				  if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
					  i,lp->arownamestr+lp->arownameind[i]);
			  }


		  }
	  }
	  break;
  case PREP_ROW_EQUAL:
	  if (aij>ALPS_ZEROEPS) {
		  if (prep_colstat[j]==PREP_VAR_BOUNDEDBELOW) {
			  prep_nrowvars[i]--;
			  // Register embedded slack elimination

			  {
				  push_stack(EMBEDDED_SLACK_VAR,i,j,0.0,prep_rhs[i],aij);
				  prep_colstat[j] = PREP_VAR_FIXED;
				  nremslacks++;
				  prep_nrfixvars++;
				  if (pril>=PL3) {
					  printf("Embedded slack %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
					  else printf("INFTY] ");
					  printf("eliminated.\n");
				  }
			  }


			  prep_rhs[i] -= aij*prep_lowerbound[j];
			  // Register row sense to less

			  {
				  push_stack(ROW_SENSE_TO_LESS,i,-1,-1,-1,-1);
				  prep_rowclass[i] = PREP_ROW_LESS;
				  if (pril>=PL3) printf("Row %d (%s) now '<'.\n",
					  i,lp->arownamestr+lp->arownameind[i]);
			  }


		  }
		  else {
			  prep_nrowvars[i]--;
			  // Register embedded slack elimination

			  {
				  push_stack(EMBEDDED_SLACK_VAR,i,j,0.0,prep_rhs[i],aij);
				  prep_colstat[j] = PREP_VAR_FIXED;
				  nremslacks++;
				  prep_nrfixvars++;
				  if (pril>=PL3) {
					  printf("Embedded slack %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
					  else printf("INFTY] ");
					  printf("eliminated.\n");
				  }
			  }


			  prep_rhs[i] -= aij*prep_upperbound[j];
			  // Register row sense to greater

			  {
				  push_stack(ROW_SENSE_TO_GREATER,i,-1,-1,-1,-1);
				  prep_rowclass[i] = PREP_ROW_GREATER;
				  if (pril>=PL3) printf("Row %d (%s) now '>'.\n",
					  i,lp->arownamestr+lp->arownameind[i]);
			  }


		  }
	  }
	  else if (aij<-ALPS_ZEROEPS) {
		  if (prep_colstat[j]==PREP_VAR_BOUNDEDBELOW) {
			  prep_nrowvars[i]--;
			  // Register embedded slack elimination

			  {
				  push_stack(EMBEDDED_SLACK_VAR,i,j,0.0,prep_rhs[i],aij);
				  prep_colstat[j] = PREP_VAR_FIXED;
				  nremslacks++;
				  prep_nrfixvars++;
				  if (pril>=PL3) {
					  printf("Embedded slack %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
					  else printf("INFTY] ");
					  printf("eliminated.\n");
				  }
			  }


			  prep_rhs[i] -= aij*prep_lowerbound[j];
			  // Register row sense to greater

			  {
				  push_stack(ROW_SENSE_TO_GREATER,i,-1,-1,-1,-1);
				  prep_rowclass[i] = PREP_ROW_GREATER;
				  if (pril>=PL3) printf("Row %d (%s) now '>'.\n",
					  i,lp->arownamestr+lp->arownameind[i]);
			  }


		  }
		  else {
			  prep_nrowvars[i]--;
			  // Register embedded slack elimination

			  {
				  push_stack(EMBEDDED_SLACK_VAR,i,j,0.0,prep_rhs[i],aij);
				  prep_colstat[j] = PREP_VAR_FIXED;
				  nremslacks++;
				  prep_nrfixvars++;
				  if (pril>=PL3) {
					  printf("Embedded slack %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
					  if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
					  else printf("-INFTY ");
					  if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
					  else printf("INFTY] ");
					  printf("eliminated.\n");
				  }
			  }


			  prep_rhs[i] -= aij*prep_upperbound[j];
			  // Register row sense to less

			  {
				  push_stack(ROW_SENSE_TO_LESS,i,-1,-1,-1,-1);
				  prep_rowclass[i] = PREP_ROW_LESS;
				  if (pril>=PL3) printf("Row %d (%s) now '<'.\n",
					  i,lp->arownamestr+lp->arownameind[i]);
			  }


		  }
	  }
	  break;
			}
nextslack:;
		}
		if (nremslacks) prep_actions = 1;
		prep_nrredrows += nrvoidrows;
		if (pril>=PL2) {
			printf("Embedded slacks:  %6d\n", nremslacks);
			if (nrvoidrows) printf("Void rows:        %6d\n", nrvoidrows);
		}
	}


	/** Row analysis */
	/*
	*  Analysis of rows. Depending on the bounds of the variables we compute
	* lower and upper bounds on the left hand sides of the constraints.
	* These bounds are then compared with the right hand side and, depending
	* on the type of constraint, this can lead to fixing of variables or the
	* detection of redundancy or infeasibility.
	*/
	{ int i,j,k;
	double rowlb,rowub,aij;
	double rowrhs;
	int lbvalid,ubvalid;
	nrvoidrows = 0;
	nrvoidcols = 0;
	nrinfeasrows = 0;
	for (i=0;i<lp->am;i++) if (prep_rowstat[i]!=PREP_ROW_REDUND) {
		lbvalid = ALPS_TRUE;
		ubvalid = ALPS_TRUE;
		rowlb = 0.0;
		rowub = 0.0;
		for (k=prep_amatrowbeg[i];k<prep_amatrowbeg[i]+prep_amatrowcount[i];k++) {
			j = prep_amatrowind[k];
			aij = prep_amatrowcoeff[k];
			if (prep_colstat[j]==PREP_VAR_BOUNDEDTWICE) {
				if (aij<0.0) {
					rowlb += aij*prep_upperbound[j];
					rowub += aij*prep_lowerbound[j];
				}
				else {
					rowlb += aij*prep_lowerbound[j];
					rowub += aij*prep_upperbound[j];
				}
			}
			else if (prep_colstat[j]==PREP_VAR_BOUNDEDBELOW) {
				if (aij<0.0) {
					rowub += aij*prep_lowerbound[j];
					lbvalid = ALPS_FALSE;
				}
				else {
					rowlb += aij*prep_lowerbound[j];
					ubvalid = ALPS_FALSE;
				}
			}
			else if (prep_colstat[j]==PREP_VAR_BOUNDEDABOVE) {
				if (aij<0.0) {
					rowlb += aij*prep_upperbound[j];
					ubvalid = ALPS_FALSE;
				}
				else {
					rowub += aij*prep_upperbound[j];
					lbvalid = ALPS_FALSE;
				}
			}
			else goto nextrow2;
		}

		rowrhs = prep_rhs[i];
		if (lbvalid) {
			switch (prep_rowclass[i]) {
	case PREP_ROW_GREATER:
		if (rowlb > rowrhs-ALPS_ZEROEPS)
			// Register redundant row
			/*
			*  Store preprocessing actions. Actions performed during preprocessing
			* are stored in a stack. Indices of involved rows and columns have to be
			* |i| and |j| for the respective actions. Variables are fixed to value
			* |newfix|, bound changes are given by |newlb| and |newub|.
			*/
		{
			push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
			prep_rowstat[i] = PREP_ROW_REDUND;
			nrvoidrows++;
			if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
				i,lp->arownamestr+lp->arownameind[i]);
		}


		break;
	case PREP_ROW_LESS:
		if (rowlb > rowrhs+ALPS_ZEROEPS)
			// Register infeasible row

		{
			push_stack(INFEASIBLE_ROW,i,-1,-1,-1,-1);
			prep_rowstat[i] = PREP_ROW_INFEAS;
			nrinfeasrows++;
			if (pril>=PL2) printf("Infeasible row %d (%s): %.4lf < %.4lf!\n",
				i,lp->arownamestr+lp->arownameind[i],rowlb,lp->arhs[i]);
		}


		else if (fabs(rowlb-rowrhs)<ALPS_ZEROEPS) {
			// Fix variables of row |i| according to lhs lower bound
			/*
			*  All (unfixed) variables of row |i| are fixed to their bounds which
			* were used for computing the lower bound on the left hand side. The row
			* is declared redundant.
			*/
			{ int k,j;
			double newfix;
			if (prep_rowstat[i]!=PREP_ROW_REDUND) {
				if (pril>=PL3) printf("Fix vars of row %d (%1s) w.r.t. lhs lower bound\n",
					i,lp->arownamestr+lp->arownameind[i]);
				for (k=prep_amatrowbeg[i];k<prep_amatrowbeg[i]+prep_amatrowcount[i];k++) {
					j = prep_amatrowind[k];
					if (prep_colstat[j]!=PREP_VAR_FIXED) {
						if (prep_amatrowcoeff[k]<0.0)
							newfix = prep_upperbound[j];
						else
							newfix = prep_lowerbound[j];
						if (fabs(newfix)<ALPS_ZEROEPS) newfix = 0.0;
						// Register variable fixing

						{
							push_stack(FIX_VARIABLE,-1,j,prep_lowerbound[j],prep_upperbound[j],newfix);
							prep_colstat[j] = PREP_VAR_FIXED;
							nrvoidcols++;
							if (pril>=PL3) {
								printf("Variable %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
								if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
								else printf("-INFTY ");
								if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
								else printf("INFTY] ");
								printf("fixed to %.3lf\n",newfix);
							}
							prep_lowerbound[j] = prep_upperbound[j] = newfix;
						}


						// Adjust constraints and objective
						/*
						*  If variable $j$ is fixed then the constraints have to be corrected
						* and the objective function correction value updated.
						*/
						{ int ii,kk;
						for (kk=lp->amatcolbeg[j];kk<lp->amatcolbeg[j]+lp->amatcolcount[j];kk++) {
							ii = lp->amatcolind[kk];
							prep_rhs[ii] -= lp->amatcolcoeff[kk]*prep_upperbound[j];
							prep_nrowvars[ii]--;
						}
						prep_objcorr += prep_obj[j]*prep_upperbound[j];
						}


					}
				}
				// Register redundant row
				/*
				*  Store preprocessing actions. Actions performed during preprocessing
				* are stored in a stack. Indices of involved rows and columns have to be
				* |i| and |j| for the respective actions. Variables are fixed to value
				* |newfix|, bound changes are given by |newlb| and |newub|.
				*/
				{
					push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
					prep_rowstat[i] = PREP_ROW_REDUND;
					nrvoidrows++;
					if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
						i,lp->arownamestr+lp->arownameind[i]);
				}


			}
			}
		}
		break;
	case PREP_ROW_EQUAL:
		if (rowlb > rowrhs+ALPS_ZEROEPS)
			// Register infeasible row

		{
			push_stack(INFEASIBLE_ROW,i,-1,-1,-1,-1);
			prep_rowstat[i] = PREP_ROW_INFEAS;
			nrinfeasrows++;
			if (pril>=PL2) printf("Infeasible row %d (%s): %.4lf < %.4lf!\n",
				i,lp->arownamestr+lp->arownameind[i],rowlb,lp->arhs[i]);
		}


		else if (fabs(rowlb-rowrhs)<ALPS_ZEROEPS) {
			// Fix variables of row |i| according to lhs lower bound
			/*
			*  All (unfixed) variables of row |i| are fixed to their bounds which
			* were used for computing the lower bound on the left hand side. The row
			* is declared redundant.
			*/
			{ int k,j;
			double newfix;
			if (prep_rowstat[i]!=PREP_ROW_REDUND) {
				if (pril>=PL3) printf("Fix vars of row %d (%1s) w.r.t. lhs lower bound\n",
					i,lp->arownamestr+lp->arownameind[i]);
				for (k=prep_amatrowbeg[i];k<prep_amatrowbeg[i]+prep_amatrowcount[i];k++) {
					j = prep_amatrowind[k];
					if (prep_colstat[j]!=PREP_VAR_FIXED) {
						if (prep_amatrowcoeff[k]<0.0)
							newfix = prep_upperbound[j];
						else
							newfix = prep_lowerbound[j];
						if (fabs(newfix)<ALPS_ZEROEPS) newfix = 0.0;
						// Register variable fixing

						{
							push_stack(FIX_VARIABLE,-1,j,prep_lowerbound[j],prep_upperbound[j],newfix);
							prep_colstat[j] = PREP_VAR_FIXED;
							nrvoidcols++;
							if (pril>=PL3) {
								printf("Variable %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
								if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
								else printf("-INFTY ");
								if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
								else printf("INFTY] ");
								printf("fixed to %.3lf\n",newfix);
							}
							prep_lowerbound[j] = prep_upperbound[j] = newfix;
						}


						// Adjust constraints and objective
						/*
						*  If variable $j$ is fixed then the constraints have to be corrected
						* and the objective function correction value updated.
						*/
						{ int ii,kk;
						for (kk=lp->amatcolbeg[j];kk<lp->amatcolbeg[j]+lp->amatcolcount[j];kk++) {
							ii = lp->amatcolind[kk];
							prep_rhs[ii] -= lp->amatcolcoeff[kk]*prep_upperbound[j];
							prep_nrowvars[ii]--;
						}
						prep_objcorr += prep_obj[j]*prep_upperbound[j];
						}


					}
				}
				// Register redundant row
				/*
				*  Store preprocessing actions. Actions performed during preprocessing
				* are stored in a stack. Indices of involved rows and columns have to be
				* |i| and |j| for the respective actions. Variables are fixed to value
				* |newfix|, bound changes are given by |newlb| and |newub|.
				*/
				{
					push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
					prep_rowstat[i] = PREP_ROW_REDUND;
					nrvoidrows++;
					if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
						i,lp->arownamestr+lp->arownameind[i]);
				}
			}
			}
		}
		break;
			}
		}
		if (ubvalid) {
			switch (prep_rowclass[i]) {
	case PREP_ROW_GREATER:
		if (rowub < rowrhs-ALPS_ZEROEPS)
			// Register infeasible row

		{
			push_stack(INFEASIBLE_ROW,i,-1,-1,-1,-1);
			prep_rowstat[i] = PREP_ROW_INFEAS;
			nrinfeasrows++;
			if (pril>=PL2) printf("Infeasible row %d (%s): %.4lf < %.4lf!\n",
				i,lp->arownamestr+lp->arownameind[i],rowub,lp->arhs[i]);
		}


		else if (fabs(rowub-rowrhs)<ALPS_ZEROEPS) {
			// Fix variables of row |i| according to lhs upper bound
			/*
			*  All (unfixed) variables of row |i| are fixed to their bounds which
			* were used for computing the upper bound on the left hand side. The row
			* is declared redundant.
			*/
			{ int k,j;
			double newfix;
			if (prep_rowstat[i]!=PREP_ROW_REDUND) {
				if (pril>=PL3) printf("Fix vars of row %d (%1s) w.r.t. lhs upper bound\n",
					i,lp->arownamestr+lp->arownameind[i]);
				for (k=prep_amatrowbeg[i];k<prep_amatrowbeg[i]+prep_amatrowcount[i];k++) {
					j = prep_amatrowind[k];
					if (prep_colstat[j]!=PREP_VAR_FIXED) {
						if (prep_amatrowcoeff[k]<0.0)
							newfix = prep_lowerbound[j];
						else
							newfix = prep_upperbound[j];
						if (fabs(newfix)<ALPS_ZEROEPS) newfix = 0.0;
						// Register variable fixing

						{
							push_stack(FIX_VARIABLE,-1,j,prep_lowerbound[j],prep_upperbound[j],newfix);
							prep_colstat[j] = PREP_VAR_FIXED;
							nrvoidcols++;
							if (pril>=PL3) {
								printf("Variable %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
								if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
								else printf("-INFTY ");
								if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
								else printf("INFTY] ");
								printf("fixed to %.3lf\n",newfix);
							}
							prep_lowerbound[j] = prep_upperbound[j] = newfix;
						}


						// Adjust constraints and objective
						/*
						*  If variable $j$ is fixed then the constraints have to be corrected
						* and the objective function correction value updated.
						*/
						{ int ii,kk;
						for (kk=lp->amatcolbeg[j];kk<lp->amatcolbeg[j]+lp->amatcolcount[j];kk++) {
							ii = lp->amatcolind[kk];
							prep_rhs[ii] -= lp->amatcolcoeff[kk]*prep_upperbound[j];
							prep_nrowvars[ii]--;
						}
						prep_objcorr += prep_obj[j]*prep_upperbound[j];
						}


					}
				}
				// Register redundant row
				/*
				*  Store preprocessing actions. Actions performed during preprocessing
				* are stored in a stack. Indices of involved rows and columns have to be
				* |i| and |j| for the respective actions. Variables are fixed to value
				* |newfix|, bound changes are given by |newlb| and |newub|.
				*/
				{
					push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
					prep_rowstat[i] = PREP_ROW_REDUND;
					nrvoidrows++;
					if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
						i,lp->arownamestr+lp->arownameind[i]);
				}


			}
			}


		}
		break;
	case PREP_ROW_LESS:
		if (rowub < rowrhs+ALPS_ZEROEPS)
			// Register redundant row
			/*
			*  Store preprocessing actions. Actions performed during preprocessing
			* are stored in a stack. Indices of involved rows and columns have to be
			* |i| and |j| for the respective actions. Variables are fixed to value
			* |newfix|, bound changes are given by |newlb| and |newub|.
			*/
		{
			push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
			prep_rowstat[i] = PREP_ROW_REDUND;
			nrvoidrows++;
			if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
				i,lp->arownamestr+lp->arownameind[i]);
		}


		break;
	case PREP_ROW_EQUAL:
		if (rowub < rowrhs-ALPS_ZEROEPS)
			// Register infeasible row

		{
			push_stack(INFEASIBLE_ROW,i,-1,-1,-1,-1);
			prep_rowstat[i] = PREP_ROW_INFEAS;
			nrinfeasrows++;
			if (pril>=PL2) printf("Infeasible row %d (%s): %.4lf < %.4lf!\n",
				i,lp->arownamestr+lp->arownameind[i],rowub,lp->arhs[i]);
		}


		else if (fabs(rowub-rowrhs)<ALPS_ZEROEPS) {
			// Fix variables of row |i| according to lhs upper bound
			/*
			*  All (unfixed) variables of row |i| are fixed to their bounds which
			* were used for computing the upper bound on the left hand side. The row
			* is declared redundant.
			*/
			{ int k,j;
			double newfix;
			if (prep_rowstat[i]!=PREP_ROW_REDUND) {
				if (pril>=PL3) printf("Fix vars of row %d (%1s) w.r.t. lhs upper bound\n",
					i,lp->arownamestr+lp->arownameind[i]);
				for (k=prep_amatrowbeg[i];k<prep_amatrowbeg[i]+prep_amatrowcount[i];k++) {
					j = prep_amatrowind[k];
					if (prep_colstat[j]!=PREP_VAR_FIXED) {
						if (prep_amatrowcoeff[k]<0.0)
							newfix = prep_lowerbound[j];
						else
							newfix = prep_upperbound[j];
						if (fabs(newfix)<ALPS_ZEROEPS) newfix = 0.0;
						// Register variable fixing

						{
							push_stack(FIX_VARIABLE,-1,j,prep_lowerbound[j],prep_upperbound[j],newfix);
							prep_colstat[j] = PREP_VAR_FIXED;
							nrvoidcols++;
							if (pril>=PL3) {
								printf("Variable %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
								if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
								else printf("-INFTY ");
								if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
								else printf("INFTY] ");
								printf("fixed to %.3lf\n",newfix);
							}
							prep_lowerbound[j] = prep_upperbound[j] = newfix;
						}


						// Adjust constraints and objective
						/*
						*  If variable $j$ is fixed then the constraints have to be corrected
						* and the objective function correction value updated.
						*/
						{ int ii,kk;
						for (kk=lp->amatcolbeg[j];kk<lp->amatcolbeg[j]+lp->amatcolcount[j];kk++) {
							ii = lp->amatcolind[kk];
							prep_rhs[ii] -= lp->amatcolcoeff[kk]*prep_upperbound[j];
							prep_nrowvars[ii]--;
						}
						prep_objcorr += prep_obj[j]*prep_upperbound[j];
						}


					}
				}
				// Register redundant row
				/*
				*  Store preprocessing actions. Actions performed during preprocessing
				* are stored in a stack. Indices of involved rows and columns have to be
				* |i| and |j| for the respective actions. Variables are fixed to value
				* |newfix|, bound changes are given by |newlb| and |newub|.
				*/
				{
					push_stack(REDUNDANT_ROW,i,-1,-1,-1,-1);
					prep_rowstat[i] = PREP_ROW_REDUND;
					nrvoidrows++;
					if (pril>=PL3) printf("Row %d (%s) eliminated.\n",
						i,lp->arownamestr+lp->arownameind[i]);
				}


			}
			}


		}
		break;
			}
		}
nextrow2:;
	}
	if (nrvoidrows || nrvoidcols) prep_actions = 1;
	prep_nrredrows += nrvoidrows;
	prep_nrfixvars += nrvoidcols;
	if (pril>=PL2) {
		printf("Void rows (RA):   %6d\n", nrvoidrows);
		printf("Void cols (RA):   %6d\n", nrvoidcols);
	}
	if (nrinfeasrows) {
		printf("Infeas rows (RA): %6d\n", nrinfeasrows);
		if (pril>=PL1) printf("Problem infeasible (bad row)!\n");
		*retcode = PREP_PROB_INFEAS;
		return ALPS_INFEASIBLE;
	}
	}


	/** Column analysis */
	/*
	*  Analysis of columns.
	*/
	{ int i,j,k;
	double newfix;
	nrvoidcols = 0;
	for (j=0;j<lp->an;j++) if (prep_colstat[j]!=PREP_VAR_FIXED) {
		if (prep_obj[j]<-ALPS_ZEROEPS 
			&& ( prep_colstat[j]==PREP_VAR_BOUNDEDBELOW 
			|| prep_colstat[j]==PREP_VAR_BOUNDEDTWICE) ) {
				for (k=lp->amatcolbeg[j];k<lp->amatcolbeg[j]+lp->amatcolcount[j];k++) {
					i = lp->amatcolind[k];
					if (prep_rowstat[i]!=PREP_ROW_REDUND) {
						if (lp->amatcolcoeff[k]>ALPS_ZEROEPS
							&& prep_rowclass[i]!=PREP_ROW_LESS) goto nextcol2;
						if (lp->amatcolcoeff[k]<-ALPS_ZEROEPS
							&& prep_rowclass[i]!=PREP_ROW_GREATER) goto nextcol2;
					}
				}
				newfix = prep_lowerbound[j];
				if (fabs(newfix)<ALPS_ZEROEPS) newfix = 0.0;
				// Register variable fixing

				{
					push_stack(FIX_VARIABLE,-1,j,prep_lowerbound[j],prep_upperbound[j],newfix);
					prep_colstat[j] = PREP_VAR_FIXED;
					nrvoidcols++;
					if (pril>=PL3) {
						printf("Variable %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
						if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
						else printf("-INFTY ");
						if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
						else printf("INFTY] ");
						printf("fixed to %.3lf\n",newfix);
					}
					prep_lowerbound[j] = prep_upperbound[j] = newfix;
				}


				// Adjust constraints and objective
				/*
				*  If variable $j$ is fixed then the constraints have to be corrected
				* and the objective function correction value updated.
				*/
				{ int ii,kk;
				for (kk=lp->amatcolbeg[j];kk<lp->amatcolbeg[j]+lp->amatcolcount[j];kk++) {
					ii = lp->amatcolind[kk];
					prep_rhs[ii] -= lp->amatcolcoeff[kk]*prep_upperbound[j];
					prep_nrowvars[ii]--;
				}
				prep_objcorr += prep_obj[j]*prep_upperbound[j];
				}


			}
		else  if (prep_obj[j]>ALPS_ZEROEPS 
			&& ( prep_colstat[j]==PREP_VAR_BOUNDEDABOVE 
			|| prep_colstat[j]==PREP_VAR_BOUNDEDTWICE) ) {
				for (k=lp->amatcolbeg[j];k<lp->amatcolbeg[j]+lp->amatcolcount[j];k++) {
					i = lp->amatcolind[k];
					if (prep_rowstat[i]!=PREP_ROW_REDUND) {
						if (lp->amatcolcoeff[k]>ALPS_ZEROEPS
							&& prep_rowclass[i]!=PREP_ROW_GREATER) goto nextcol2;
						if (lp->amatcolcoeff[k]<-ALPS_ZEROEPS
							&& prep_rowclass[i]!=PREP_ROW_LESS) goto nextcol2;
					}
				}
				newfix = prep_upperbound[j];
				if (fabs(newfix)<ALPS_ZEROEPS) newfix = 0.0;
				// Register variable fixing

				{
					push_stack(FIX_VARIABLE,-1,j,prep_lowerbound[j],prep_upperbound[j],newfix);
					prep_colstat[j] = PREP_VAR_FIXED;
					nrvoidcols++;
					if (pril>=PL3) {
						printf("Variable %1d (%s) : [",j,lp->acolnamestr+lp->acolnameind[j]);
						if (prep_lowerbound[j]>-ALPS_REAL_INFINITY) printf("%.3lf,",prep_lowerbound[j]);
						else printf("-INFTY ");
						if (prep_upperbound[j]<ALPS_REAL_INFINITY) printf("%.3lf] ",prep_upperbound[j]);
						else printf("INFTY] ");
						printf("fixed to %.3lf\n",newfix);
					}
					prep_lowerbound[j] = prep_upperbound[j] = newfix;
				}


				// Adjust constraints and objective
				/*
				*  If variable $j$ is fixed then the constraints have to be corrected
				* and the objective function correction value updated.
				*/
				{ int ii,kk;
				for (kk=lp->amatcolbeg[j];kk<lp->amatcolbeg[j]+lp->amatcolcount[j];kk++) {
					ii = lp->amatcolind[kk];
					prep_rhs[ii] -= lp->amatcolcoeff[kk]*prep_upperbound[j];
					prep_nrowvars[ii]--;
				}
				prep_objcorr += prep_obj[j]*prep_upperbound[j];
				}


			}
nextcol2:;
	}
	if (nrvoidcols) prep_actions = 1;
	prep_nrfixvars += nrvoidcols;
	if (pril>=PL2) printf("Fixed columns (CA):%5d\n", nrvoidcols);
	}


	*actions = prep_actions;
	*retcode = prep_probstat;
	return 0;
}

/*
*  Generate internal preprocessed problem. Finally, the internal problem
* has to be generated. This means that in contrast to the standard
* generation of the internal problem all redundant rows and void/fixed
* columns are eliminated, and possibly some bounds are tightened. Right
* hand sides may change and there is a constant |prep_objcorr| which has
* to be added to the objective function.
*/


int alps_prepgenprob(
					 int pril,
					 lpstruct *lp
					 )

{
	if (pril>=PL1) {
		printf("Total eliminated rows        %6d\n", prep_nrredrows);
		printf("Total eliminated columns     %6d\n", prep_nrfixvars);
		if (pril>=PL3) printf("Objective constant      %8.5le\n", prep_objcorr);
	}
	/** Allocate space for internal preprocessed LP data */
	/*
	*  For the time being we just allocate space for nonzeros as if the
	* problem had not been preprocessed. This has to be changed later.
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


	/** Generate internal preprocessed problem */
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
	{ int i,j,k,nrineq,ind,ind1,ind2;
	int orgi,orgj;
	double ilbj,iubj;

	lp->im = lp->am - prep_nrredrows;
	lp->inorig = lp->an - prep_nrfixvars;
	lp->in = lp->inorig + lp->im;

	org_colnr = (int *) malloc( lp->in * sizeof(int) );
	new_colnr = (int *) malloc( lp->an * sizeof(int) );
	org_rownr = (int *) malloc( lp->im * sizeof(int) );
	new_rownr = (int *) malloc( lp->am * sizeof(int) );
	if (!org_colnr || !new_colnr || !org_rownr || !new_rownr )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}



	k = 0;
	for (j=0;j<lp->an;j++) {
		if (prep_colstat[j]!=PREP_VAR_FIXED) {
			org_colnr[k] = j;
			new_colnr[j] = k;
			k++;
		}
		else {
			new_colnr[j] = -1;
		}
	}
	if (pril>=PL1) printf("Number of structural vars    %6d\n", lp->inorig);
	if (k!=lp->inorig) { printf("ERROR 1: %1d <> %1d \n",k,lp->inorig); exit(999); }
	k = 0;
	for (i=0;i<lp->am;i++) {
		if (prep_rowstat[i]!=PREP_ROW_REDUND) {
			org_rownr[k] = i;
			new_rownr[i] = k;
			k++;
		}
		else 
			new_rownr[i] = -1;
	}
	if (pril>=PL1) printf("Number of rows               %6d\n", lp->im);
	if (k!=lp->im) { printf("ERROR 2: %1d <> %1d \n",k,lp->im); exit(999); }

	if (lp->aobjminmax==ALPS_MINIMIZE) {
		for (j=0;j<lp->inorig;j++) lp->iphase2obj[j] = -lp->aobj[org_colnr[j]];
	}
	else {
		for (j=0;j<lp->inorig;j++) lp->iphase2obj[j] = lp->aobj[org_colnr[j]];
	}

	for (j=0;j<lp->inorig;j++) {
		ilbj = prep_lowerbound[org_colnr[j]];
		iubj = prep_upperbound[org_colnr[j]];
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
		if (lp->varstat[j]==ALPS_FIXED) { 
			printf("ERROR 3: %1d %1d (%.3le,%.3le)\n",j,org_colnr[j],ilbj,iubj); exit(999); }
	}

	for (i=0; i<lp->im; i++) lp->imatrowcount[i] = 1;
	ind = 0;
	for (j=0;j<lp->inorig;j++) {
		lp->imatcolbeg[j] = ind;
		lp->imatcolcount[j] = 0;
		orgj = org_colnr[j];
		for (k=lp->amatcolbeg[orgj];k<lp->amatcolbeg[orgj]+lp->amatcolcount[orgj];k++) {
			orgi = lp->amatcolind[k];
			i = new_rownr[orgi];
			if (prep_rowstat[orgi]!=PREP_ROW_REDUND) {
				if (i<0) { printf("ERROR 4\n"); exit(999); }
				if (prep_rowclass[orgi]==PREP_ROW_GREATER)
					lp->imatcolcoeff[ind] = -lp->amatcolcoeff[k];
				else
					lp->imatcolcoeff[ind] = lp->amatcolcoeff[k];
				lp->imatcolind[ind] = i;
				lp->imatrowcount[i]++;
				lp->imatcolcount[j]++;
				ind++;
			}
		}
	}

	nrineq = 0;
	for (i=0;i<lp->im;i++) 
		if (prep_rowclass[org_rownr[i]]!=PREP_ROW_EQUAL) nrineq++;
	if (pril>=PL1) printf("Number of inequalities       %6d\n", nrineq);

	ind1 = lp->inorig;
	ind2 = ind1 + nrineq;
	k = lp->imatcolbeg[lp->inorig-1]+lp->imatcolcount[lp->inorig-1];

	for (i=0;i<lp->im;i++) {
		orgi = org_rownr[i];
		if (prep_rowclass[orgi]==PREP_ROW_GREATER) {
			lp->irhs[i] = -prep_rhs[orgi];
			lp->ilowerbound[ind1] = 0.0;
			lp->iupperbound[ind1] = ALPS_REAL_INFINITY;
			lp->varstat[ind1] = ALPS_BOUNDEDBELOW;
			lp->imatcolbeg[ind1] = k;
			lp->imatcolcount[ind1] = 1;
			lp->imatcolind[k] = i;
			lp->imatcolcoeff[k] = 1.0;
			ind1++;
		}
		else if (prep_rowclass[orgi]==PREP_ROW_LESS) {
			lp->irhs[i] = prep_rhs[orgi];
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
			lp->irhs[i] = prep_rhs[orgi];
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
	for (j=lp->inorig;j<lp->in;j++) lp->iphase2obj[j] = 0.0;
	lp->iobj = lp->iphase2obj;
	}


	/** Free preprocessing memory */
	/*
	*  Here we free the space that was only needed during preprocessing.
	* Note that arrays containing information for reconstructing the
	* original problem are still needed.
	*/
	ffree( (char **) &prep_rowstat);
	ffree( (char **) &prep_nrowvars);
	ffree( (char **) &prep_obj);
	ffree( (char **) &prep_lowerbound);
	ffree( (char **) &prep_upperbound);
	ffree( (char **) &prep_rhs);
	ffree( (char **) &prep_rowptr);


	if (pril>=PL1) printf("End of preprocessing.\n\n");

	return 0;
}

/*
*  Reconstruct original data. The following routine undoes all
* preprocessing actions and reconstructs the original internal
* representation. There might be cases where we cannot produce an
* optimal basis for the original problem from an optimal basis for the
* internal problem. The basis status will then be changed accordingly.
* To extend the internal solution we have to reinsert fixed variables
* (with corresponding update of left hand sides) and to compute the left
* hand side of constraints that were eliminated. Here it is convenient
* to have the external matrix in row format. This routine is not yet
* fully implemented!
*/


#define UNDEFINED 0
#define DEFINED 1

int alps_prepundo(
				  int pril,
				  lpstruct *lp
				  )

{
	int i,j,k,a,r,c,nrbasvar;
	double v1,v2,v3,lhs,xx;

	lp->abasisstatus = lp->basisstatus;
	nrbasvar = 0;
	for (j=0;j<lp->an;j++) {
		prep_colstat[j] = UNDEFINED;
		lp->acolstat[j] = ALPS_UNKNOWN;
	}
	for (i=0;i<lp->am;i++) {
		lp->aslack[i] = lp->arhs[i];
		lp->acolstat[lp->an+i] = ALPS_UNKNOWN;
	}
	for (j=0;j<lp->inorig;j++) {
		lp->ax[org_colnr[j]] = lp->x[j];
		lp->acolstat[org_colnr[j]] = lp->colstat[j];
		if (lp->colstat[j]==ALPS_BASIC) nrbasvar++;
		prep_colstat[org_colnr[j]] = DEFINED;
	}
	for (j=lp->inorig;j<lp->in;j++) {
		lp->aslack[org_rownr[j-lp->inorig]] = lp->x[j];
		lp->acolstat[lp->an+org_rownr[j-lp->inorig]] = lp->colstat[j];
	}

	if (prep_stackptr>1) {
		while (pop_stack(&a,&r,&c,&v1,&v2,&v3)==0) {
			switch (a) {
	  case INFEASIBLE_ROW:
		  if (pril>=PL1) printf("Infeasible row %1d\n",r);
		  break;
	  case REDUNDANT_ROW:
		  if (pril>=PL3) printf("Redundant row %1d\n",r);
		  for (k=prep_amatrowbeg[r];k<prep_amatrowbeg[r]+prep_amatrowcount[r];k++) {
			  j = new_colnr[prep_amatrowind[k]];
			  if (j>0) lp->aslack[r] -= prep_amatrowcoeff[k]*lp->x[j];
		  }
		  if (lp->acolstat[lp->an+r]==ALPS_UNKNOWN) {
			  lp->acolstat[lp->an+r] = ALPS_BASIC;
			  nrbasvar++;
		  }
		  break;
	  case ROW_SENSE_TO_LESS:
		  if (pril>=PL3) printf("Row %1d changed to '<'\n",r);
		  prep_rowclass[r] = PREP_ROW_EQUAL;
		  break;
	  case ROW_SENSE_TO_GREATER:
		  if (pril>=PL3) printf("Row %1d changed to '>'\n",r);
		  prep_rowclass[r] = PREP_ROW_EQUAL;
		  break;
	  case FIX_VARIABLE:
		  if (pril>=PL3) printf("Fixed variable %1d\n",c);
		  lp->ax[c] = v3;
		  for (k=lp->amatcolbeg[c];k<lp->amatcolbeg[c]+lp->amatcolcount[c];k++) {
			  i = lp->amatcolind[k];
			  lp->aslack[i] -= lp->amatcolcoeff[k]*v3;
		  }
		  prep_colstat[c] = DEFINED;
		  if (v3<=lp->alowerbound[c]+ALPS_EPS) 
			  lp->acolstat[c] = ALPS_NONBASICLOWB;
		  else if (v3>=lp->aupperbound[c]-ALPS_EPS) 
			  lp->acolstat[c] = ALPS_NONBASICUPB;
		  else lp->abasisstatus = ALPS_NOBASIS;
		  break;
	  case EMBEDDED_SLACK_VAR:
		  if (pril>=PL3) printf("Eliminated embedded slack %1d from row %d\n",c,r);
		  lhs = 0.0;
		  for (k=prep_amatrowbeg[r];k<prep_amatrowbeg[r]+prep_amatrowcount[r];k++) {
			  j = prep_amatrowind[k];
			  if (prep_colstat[j]!=UNDEFINED) lhs += prep_amatrowcoeff[k]*lp->ax[j];
		  }
		  xx = (v2 - lhs)/v3;
		  switch (prep_rowclass[r]) {
	  case PREP_ROW_LESS:
		  if (v3>0.0) {
			  if (xx<lp->alowerbound[c]-ALPS_EPS) {
				  printf("SEVERE ERROR in prepundo()\n");
				  exit(ALPS_SEVERE_ERROR);
			  }
			  if (xx>lp->aupperbound[c]-ALPS_EPS) {
				  xx = lp->aupperbound[c];
				  lp->acolstat[c] = ALPS_NONBASICUPB;
			  }
			  if (lp->acolstat[lp->an+r]==ALPS_UNKNOWN) {
				  lp->acolstat[c] = ALPS_BASIC;
				  nrbasvar++;
				  lp->aslack[r] = 0.0;
				  lp->acolstat[lp->an+r] = ALPS_NONBASICLOWB;
			  }
			  else lp->abasisstatus = ALPS_NOBASIS;
		  }
		  else {
			  if (xx>lp->aupperbound[c]+ALPS_EPS) {
				  printf("SEVERE ERROR in prepundo()\n");
				  exit(ALPS_SEVERE_ERROR);
			  }
			  if (xx<lp->alowerbound[c]+ALPS_EPS) {
				  xx = lp->alowerbound[c];
				  lp->acolstat[c] = ALPS_NONBASICLOWB;
			  }
			  if (lp->acolstat[lp->an+r]==ALPS_UNKNOWN) {
				  lp->acolstat[c] = ALPS_BASIC;
				  nrbasvar++;
				  lp->aslack[r] = 0.0;
				  lp->acolstat[lp->an+r] = ALPS_NONBASICLOWB;
			  }
			  else lp->abasisstatus = ALPS_NOBASIS;
		  }
		  break;
	  case PREP_ROW_GREATER:
		  if (v3<0.0) {
			  if (xx<lp->alowerbound[c]-ALPS_EPS) {
				  printf("SEVERE ERROR in prepundo()\n");
				  exit(ALPS_SEVERE_ERROR);
			  }
			  if (xx>lp->aupperbound[c]-ALPS_EPS) {
				  xx = lp->aupperbound[c];
				  lp->acolstat[c] = ALPS_NONBASICUPB;
			  }
			  if (lp->acolstat[lp->an+r]==ALPS_UNKNOWN) {
				  lp->acolstat[c] = ALPS_BASIC;
				  nrbasvar++;
				  lp->aslack[r] = 0.0;
				  lp->acolstat[lp->an+r] = ALPS_NONBASICLOWB;
			  }
			  else lp->abasisstatus = ALPS_NOBASIS;
		  }
		  else {
			  if (xx>lp->aupperbound[c]+ALPS_EPS) {
				  printf("SEVERE ERROR in prepundo()\n");
				  exit(ALPS_SEVERE_ERROR);
			  }
			  if (xx<lp->alowerbound[c]+ALPS_EPS) {
				  xx = lp->alowerbound[c];
				  lp->acolstat[c] = ALPS_NONBASICLOWB;
			  }
			  if (lp->acolstat[lp->an+r]==ALPS_UNKNOWN) {
				  lp->acolstat[c] = ALPS_BASIC;
				  nrbasvar++;
				  lp->aslack[r] = 0.0;
				  lp->acolstat[lp->an+r] = ALPS_NONBASICLOWB;
			  }
			  else lp->abasisstatus = ALPS_NOBASIS;
		  }
		  break;
	  case PREP_ROW_EQUAL:
		  if (xx<=lp->alowerbound[c]+ALPS_EPS) 
			  lp->acolstat[c] = ALPS_NONBASICLOWB;
		  else if (xx>=lp->aupperbound[c]-ALPS_EPS) 
			  lp->acolstat[c] = ALPS_NONBASICUPB;
		  if (lp->acolstat[lp->an+r]==ALPS_UNKNOWN) {
			  lp->acolstat[c] = ALPS_BASIC;
			  nrbasvar++;
			  lp->aslack[r] = 0.0;
			  lp->acolstat[lp->an+r] = ALPS_NONBASICLOWB;
		  }
		  else lp->abasisstatus = ALPS_NOBASIS;
		  break;
		  }
		  lp->ax[c] = xx;
		  prep_colstat[c] = DEFINED;
		  break;
	  case UPPER_BOUND_CHANGE:
		  if (pril>=PL3) printf("Upper bound change variable %1d\n",c);
		  break;
	  case LOWER_BOUND_CHANGE:
		  if (pril>=PL3) printf("Lower bound change variable %1d\n",c);
		  break;
	  default:
		  if (pril>=PL1) printf("Invalid preprocessing action! Stack not ok.\n");
		  break;
			}
		}
	}
	if (lp->aobjminmax==ALPS_MINIMIZE) lp->objval += prep_objcorr;  
	else lp->objval -= prep_objcorr;

	if (pril>=PL3) {
		for (j=0;j<lp->an;j++) {
			if (prep_colstat[j]==UNDEFINED) {
				printf("Var %3d (%10s) undefined\n",j,lp->acolnamestr+lp->acolnameind[j]);
				exit(ALPS_SEVERE_ERROR);
			}
			else
				printf("Col %3d (%10s) %10.3lf\n",
				j,lp->acolnamestr+lp->acolnameind[j],lp->ax[j]);
		}
		for (i=0;i<lp->am;i++) {
			printf("Row %3d (%10s) %10.3lf\n",
				i,lp->arownamestr+lp->arownameind[i],lp->aslack[i]);
		}
	}
	else {
		for (j=0;j<lp->an;j++) {
			if (prep_colstat[j]==UNDEFINED) {
				printf("Var %3d (%10s) undefined\n",j,lp->acolnamestr+lp->acolnameind[j]);
				exit(ALPS_SEVERE_ERROR);
			}
		}
	}

	ffree( (char **) &org_colnr);
	ffree( (char **) &new_colnr);
	ffree( (char **) &org_rownr);
	ffree( (char **) &new_rownr);
	ffree( (char **) &prep_colstat);
	ffree( (char **) &prep_rowclass);
	ffree( (char **) &prep_stack);
	ffree( (char **) &prep_amatrowcount);
	ffree( (char **) &prep_amatrowbeg);
	ffree( (char **) &prep_amatrowind);
	ffree( (char **) &prep_amatrowcoeff);
	if (pril>=0) { 
		printf("Solution for original LP constructed (");
		if (lp->abasisstatus==ALPS_NOBASIS) 
			printf("partial basis, %1d + %1d)\n",nrbasvar,lp->am-nrbasvar);
		else printf("including basis)\n");
	}

	return 0;
}

} // end namespace ogdf
