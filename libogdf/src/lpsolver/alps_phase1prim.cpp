/*
 * $Revision: 1.2 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief 
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
// Purpose:
//
// Phase 1. Here we define the phase 1 problem and compute the starting
// basis for the primal simplex algorithm to solve the phase 1 problem. 
//

/*
*  We prepare phase 1 of the primal simplex algorithm by constructing an
* initial basis for the phase 1 problem. Since we may introduce original
* variables into this basis, this may lead to bound violations by
* original variables. Then we switch to the phase 1 objective function,
* so that the primal simplex algorithm can minimize the infeasibilities.
* This objective function also forces original variables to eventually
* attain values obeying their true bounds.
*/


#include "alps.h"
#include "alps_misc.h"

namespace ogdf {

int alps_phase1prim(
					lpstruct *lp,
					int pril
					)
{
	/** Declare alps_phase1prim scalars */
	int i,j,k,jj;
	int row_ind_of_slack_art;
	int nextbasisheader;
	int randseed;
	int basnonz,ind,l,starttime,status,totalfill,totalcancel,stoptime;
	int freecount,freepos;
	int oneboundcount,oneboundpos;
	int twoboundscount,twoboundspos;
	double maxabsobjcoeff,absobjcoeff;
	double colmax,colrmax,colimax;
	int colrind,coliind;
	double faij;
	double objcorr;
	int initbasisstructs,initbasisslacks,initbasisartifs;

	/** Declare alps\_phase1prim arrays */
	int *rowcovered;
	int *rowfill;
	double *rowpivot;
	int *pivrow;
	int *pivcol;
	double *pivvalue;
	double *xbas;
	double *trhs;
	double *sortkey;
	int *sortindex;

	/** Allocate space for alps\_phase1prim arrays */
	rowcovered = (int *) malloc( lp->im * sizeof(int) );
	rowfill = (int *) malloc( lp->im * sizeof(int) );
	rowpivot = (double *) malloc( lp->im * sizeof(double) );
	sortkey = (double *) malloc( lp->inorig * sizeof(double) );
	sortindex = (int *) malloc( lp->inorig * sizeof(int) );
	pivrow = NULL;
	pivcol = NULL;
	pivvalue = NULL;
	xbas = (double *) malloc( lp->im * sizeof(double) );
	trhs = (double *) malloc( lp->im * sizeof(double) );
	if (!rowcovered || !rowfill || !rowpivot
		|| !sortkey || !sortindex || !xbas || !trhs )
		// run out of memory
	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

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

	/** Construct initial basis */
	/*
	*  Here we set up the initial basis for phase~1. At present, there are
	* three options. The first is to proceed along the same lines as Bixby
	* in his {\sl ORSA Journal on Computing} publication. The initial basis
	* consists of all slacks, some structural variables and some
	* artificials. The second option is a simplified version where
	* structurals are chosen such that the basismatrix is triangular. The
	* third option does not introduce structural basic variables. Variables
	* not selected are nonbasic, i.e., either free or at their upper or at
	* their lower bound. Along with this, the corresponding solution vector
	* |lp->x| is set up for the nonbasic variables.
	*/
	// Initializations

	for (i=0; i<lp->im; i++) {
		rowcovered[i] = ALPS_FALSE;
		rowpivot[i] = ALPS_REAL_INFINITY;
		rowfill[i] = 0;
	}
	for (j=0; j<lp->in; j++) {
		lp->colstat[j] = ALPS_UNKNOWN;
		lp->iphase1obj[j] = 0.0;
		lp->ioriglowerbound[j] = lp->ilowerbound[j]; 
		lp->iorigupperbound[j] = lp->iupperbound[j]; 
		lp->origvarstat[j] = lp->varstat[j]; 
		lp->iartifbounds[j] = ALPS_FALSE; 
	}
	initbasisslacks = 0;
	initbasisstructs = 0;
	initbasisartifs = 0;


	switch (lp->initialbasis) {
  case ALPS_INITBASIS:
	  // Select all slack variables
	  /*
	  *  The following makes all slack variables basic.
	  */
	  for (j=lp->inorig;j<lp->inonartif;j++) {
		  row_ind_of_slack_art = lp->imatcolind[lp->imatcolbeg[j]];
		  lp->colstat[j] = ALPS_BASIC;
		  lp->basisheader[j-lp->inorig] = j;
		  rowcovered[row_ind_of_slack_art] = ALPS_TRUE;
		  rowfill[row_ind_of_slack_art] = 1;
	  }
	  nextbasisheader = lp->inonartif - lp->inorig;
	  initbasisslacks = lp->inonartif-lp->inorig;
	  if (nextbasisheader>=lp->im) goto done;


	  // Priorities for structural variables
	  /*
	  *  We will later scan the structural variables for checking if they are
	  * inserted into the initial basis. We scan these variables in a sequence
	  * according to priorities which are based primarily on their bounds and
	  * secondarily on their objective function coefficients.
	  */
	  freecount = 0;
	  oneboundcount = 0;
	  twoboundscount = 0;
	  maxabsobjcoeff = -ALPS_REAL_INFINITY;
	  for (j=0; j<lp->inorig; j++) {
		  absobjcoeff = fabs(lp->iphase2obj[j]);
		  if (absobjcoeff>maxabsobjcoeff)
			  maxabsobjcoeff = absobjcoeff;
		  if (lp->varstat[j]==ALPS_BOUNDEDTWICE)
			  twoboundscount++;
		  else if ((lp->varstat[j]==ALPS_BOUNDEDABOVE)
			  ||(lp->varstat[j]==ALPS_BOUNDEDBELOW)) 
			  oneboundcount++;
		  else if (lp->varstat[j]==ALPS_FREE) freecount++;
	  }
	  if (maxabsobjcoeff<ALPS_ZEROEPS) maxabsobjcoeff = 1.0;
	  else maxabsobjcoeff *= 1000.0;
	  freepos = 0;
	  oneboundpos = freecount;
	  twoboundspos = freecount + oneboundcount;
	  for (j=0; j<lp->inorig; j++) {
		  objcorr = - lp->iphase2obj[j]/maxabsobjcoeff;
		  if (lp->varstat[j]==ALPS_BOUNDEDTWICE) {
			  sortindex[twoboundspos] = j;
			  sortkey[twoboundspos++] = lp->ilowerbound[j] - lp->iupperbound[j] + objcorr;
		  }
		  else if (lp->varstat[j]==ALPS_BOUNDEDBELOW) {
			  sortindex[oneboundpos] = j;
			  sortkey[oneboundpos++] = lp->ilowerbound[j] + objcorr;
		  }
		  else if (lp->varstat[j]==ALPS_BOUNDEDABOVE) {
			  sortindex[oneboundpos] = j;
			  sortkey[oneboundpos++] = -lp->iupperbound[j] + objcorr;
		  }
		  else if (lp->varstat[j]==ALPS_FREE) {
			  sortindex[freepos] = j;
			  sortkey[freepos++] = objcorr;
		  }
	  }
	  quick_sort2(sortkey,sortindex,0,freepos-1);
	  quick_sort2(sortkey,sortindex,freepos,oneboundpos-1);
	  quick_sort2(sortkey,sortindex,oneboundpos,twoboundspos-1);


	  // Select structural basic variables
	  /*
	  *  The following procedure insert basic structural variables allowing
	  * for slight violations of triangularity.
	  */
	  for (jj=0; jj<twoboundspos; jj++)  {
		  j = sortindex[jj];
		  if (lp->imatcolcount[j]) {
			  colrmax = 0.0;
			  colimax = 0.0;
			  colmax = 0.0;
			  colrind = -1;
			  coliind = -1;
			  for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				  i = lp->imatcolind[k];
				  faij = fabs(lp->imatcolcoeff[k]);
				  if (faij>colmax) colmax = faij;
				  if (rowfill[i]==0) {
					  if (faij>colrmax) {
						  colrmax = faij;
						  colrind = i;
					  }
				  }
				  if (!rowcovered[i]) {
					  if (faij>colimax) {
						  colimax = faij;
						  coliind = i;
					  }
				  }
			  }
			  if (colmax<0.001) goto next;

			  if (colrmax>0.99*colmax) {
				  initbasisstructs++;
				  lp->colstat[j] = ALPS_BASIC;
				  lp->basisheader[nextbasisheader++] = j;
				  rowcovered[colrind] = ALPS_TRUE;
				  rowpivot[colrind] = colrmax;
				  for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
					  i = lp->imatcolind[k];
					  if (fabs(lp->imatcolcoeff[k])>ALPS_ZEROEPS) rowfill[i]++;
				  }
				  if (nextbasisheader>=lp->im) goto done;
				  goto next;
			  }

			  for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				  i = lp->imatcolind[k];
				  faij = fabs(lp->imatcolcoeff[k]);
				  if (faij>0.01*rowpivot[i]) goto next;
			  }

			  if (colimax<0.001) goto next;

			  initbasisstructs++;
			  lp->colstat[j] = ALPS_BASIC;
			  lp->basisheader[nextbasisheader++] = j;
			  rowcovered[coliind] = ALPS_TRUE;
			  rowpivot[coliind] = colimax;
			  for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				  i = lp->imatcolind[k];
				  if (fabs(lp->imatcolcoeff[k])>ALPS_ZEROEPS) rowfill[i]++;
			  }
			  if (nextbasisheader>=lp->im) goto done;
			  goto next;
		  }
next:;
	  }


	  break;
  case ALPS_TRIANGULAR_INITBASIS:
	  // Select all slack variables
	  /*
	  *  The following makes all slack variables basic.
	  */
	  for (j=lp->inorig;j<lp->inonartif;j++) {
		  row_ind_of_slack_art = lp->imatcolind[lp->imatcolbeg[j]];
		  lp->colstat[j] = ALPS_BASIC;
		  lp->basisheader[j-lp->inorig] = j;
		  rowcovered[row_ind_of_slack_art] = ALPS_TRUE;
		  rowfill[row_ind_of_slack_art] = 1;
	  }
	  nextbasisheader = lp->inonartif - lp->inorig;
	  initbasisslacks = lp->inonartif-lp->inorig;
	  if (nextbasisheader>=lp->im) goto done;


	  // Priorities for structural variables
	  /*
	  *  We will later scan the structural variables for checking if they are
	  * inserted into the initial basis. We scan these variables in a sequence
	  * according to priorities which are based primarily on their bounds and
	  * secondarily on their objective function coefficients.
	  */
	  freecount = 0;
	  oneboundcount = 0;
	  twoboundscount = 0;
	  maxabsobjcoeff = -ALPS_REAL_INFINITY;
	  for (j=0; j<lp->inorig; j++) {
		  absobjcoeff = fabs(lp->iphase2obj[j]);
		  if (absobjcoeff>maxabsobjcoeff)
			  maxabsobjcoeff = absobjcoeff;
		  if (lp->varstat[j]==ALPS_BOUNDEDTWICE)
			  twoboundscount++;
		  else if ((lp->varstat[j]==ALPS_BOUNDEDABOVE)
			  ||(lp->varstat[j]==ALPS_BOUNDEDBELOW)) 
			  oneboundcount++;
		  else if (lp->varstat[j]==ALPS_FREE) freecount++;
	  }
	  if (maxabsobjcoeff<ALPS_ZEROEPS) maxabsobjcoeff = 1.0;
	  else maxabsobjcoeff *= 1000.0;
	  freepos = 0;
	  oneboundpos = freecount;
	  twoboundspos = freecount + oneboundcount;
	  for (j=0; j<lp->inorig; j++) {
		  objcorr = - lp->iphase2obj[j]/maxabsobjcoeff;
		  if (lp->varstat[j]==ALPS_BOUNDEDTWICE) {
			  sortindex[twoboundspos] = j;
			  sortkey[twoboundspos++] = lp->ilowerbound[j] - lp->iupperbound[j] + objcorr;
		  }
		  else if (lp->varstat[j]==ALPS_BOUNDEDBELOW) {
			  sortindex[oneboundpos] = j;
			  sortkey[oneboundpos++] = lp->ilowerbound[j] + objcorr;
		  }
		  else if (lp->varstat[j]==ALPS_BOUNDEDABOVE) {
			  sortindex[oneboundpos] = j;
			  sortkey[oneboundpos++] = -lp->iupperbound[j] + objcorr;
		  }
		  else if (lp->varstat[j]==ALPS_FREE) {
			  sortindex[freepos] = j;
			  sortkey[freepos++] = objcorr;
		  }
	  }
	  quick_sort2(sortkey,sortindex,0,freepos-1);
	  quick_sort2(sortkey,sortindex,freepos,oneboundpos-1);
	  quick_sort2(sortkey,sortindex,oneboundpos,twoboundspos-1);


	  // Simple selection of structural basic variables
	  /*
	  *  We insert structural basic variables as long as the basis matrix
	  * stays triangular.
	  */
	  for (jj=0; jj<twoboundspos; jj++)  {
		  j = sortindex[jj];
		  if (lp->imatcolcount[j]) {
			  for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				  i = lp->imatcolind[k];
				  if (rowcovered[i]) goto nextvar;
			  }
			  initbasisstructs++;
			  lp->colstat[j] = ALPS_BASIC;
			  lp->basisheader[nextbasisheader++] = j;
			  if (nextbasisheader>=lp->im) goto done;
			  k = lp->imatcolbeg[j]; 
			  i = lp->imatcolind[k];
			  rowcovered[i] = ALPS_TRUE;
		  }
nextvar:;
	  }


	  break;
  case ALPS_ARTIFICIAL_INITBASIS:
	  // Select all slack variables
	  /*
	  *  The following makes all slack variables basic.
	  */
	  for (j=lp->inorig;j<lp->inonartif;j++) {
		  row_ind_of_slack_art = lp->imatcolind[lp->imatcolbeg[j]];
		  lp->colstat[j] = ALPS_BASIC;
		  lp->basisheader[j-lp->inorig] = j;
		  rowcovered[row_ind_of_slack_art] = ALPS_TRUE;
		  rowfill[row_ind_of_slack_art] = 1;
	  }
	  nextbasisheader = lp->inonartif - lp->inorig;
	  initbasisslacks = lp->inonartif-lp->inorig;
	  if (nextbasisheader>=lp->im) goto done;


	  break;
  default:
	  // Select all slack variables
	  /*
	  *  The following makes all slack variables basic.
	  */
	  for (j=lp->inorig;j<lp->inonartif;j++) {
		  row_ind_of_slack_art = lp->imatcolind[lp->imatcolbeg[j]];
		  lp->colstat[j] = ALPS_BASIC;
		  lp->basisheader[j-lp->inorig] = j;
		  rowcovered[row_ind_of_slack_art] = ALPS_TRUE;
		  rowfill[row_ind_of_slack_art] = 1;
	  }
	  nextbasisheader = lp->inonartif - lp->inorig;
	  initbasisslacks = lp->inonartif-lp->inorig;
	  if (nextbasisheader>=lp->im) goto done;


	  break;
	}
	// Complete with artificial variables
	/*
	*  For uncovered rows we insert the corresponding artificial variables
	* into the basis.
	*/
	for (j=lp->inonartif;j<lp->in;j++) {
		row_ind_of_slack_art = lp->imatcolind[lp->imatcolbeg[j]];
		if (!rowcovered[row_ind_of_slack_art]) {
			initbasisartifs++;
			lp->colstat[j] = ALPS_BASIC;
			lp->basisheader[nextbasisheader++] = j;
			if (nextbasisheader>=lp->im) goto done;
		}
	}
done:


	// Set nonbasic x-vector
	/*
	*  We now set the remaining structural and artificial nonbasic
	* variables. Depending on their coefficient in the phase~2 objective
	* function setting to lower or upper bound is preferred if possible.
	* Nonbasic variables are always set to feasible values. We already
	* compute the right hand side with which to compute the values of the
	* basic variables.
	*/
	for (i=0; i<lp->im; i++) trhs[i] = lp->irhs[i];
	for (j=0;j<lp->inorig;j++) {
		if (lp->colstat[j]==ALPS_UNKNOWN) {
			if (lp->varstat[j]==ALPS_BOUNDEDTWICE) {
				if (lp->iphase2obj[j]>0.0) {
					lp->colstat[j] = ALPS_NONBASICUPB;
					lp->x[j] = lp->iupperbound[j];
				}
				else {
					lp->colstat[j] = ALPS_NONBASICLOWB;
					lp->x[j] = lp->ilowerbound[j];
				}
			}
			else if (lp->varstat[j]==ALPS_BOUNDEDBELOW) {
				lp->colstat[j] = ALPS_NONBASICLOWB;
				lp->x[j] = lp->ilowerbound[j];
			}
			else if (lp->varstat[j]==ALPS_BOUNDEDABOVE) {
				lp->colstat[j] = ALPS_NONBASICUPB;
				lp->x[j] = lp->iupperbound[j];
			}
			else if (lp->varstat[j]==ALPS_FIXED) {
				lp->colstat[j] = ALPS_NONBASICLOWB;
				lp->x[j] = lp->ilowerbound[j];
			}
			else if (lp->varstat[j]==ALPS_FREE) {
				lp->colstat[j] = ALPS_NONBASICFREE;
				lp->x[j] = 0.0;
			}
			for (k=lp->imatcolbeg[j]; k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				i = lp->imatcolind[k];
				trhs[i] -= lp->imatcolcoeff[k]*lp->x[j];
			}
		}
	}
	for (j=lp->inonartif;j<lp->in;j++) {
		if (lp->colstat[j]==ALPS_UNKNOWN) {
			lp->colstat[j] = ALPS_NONBASICLOWB;
			lp->x[j] = 0.0;
		}
	}
	for (j=0;j<lp->in;j++) if (lp->colstat[j]!=ALPS_BASIC) { double viol;
	viol = lp->x[j] - lp->iupperbound[j];
	if (viol>0.0001) {
		printf("\n0Strong upper bound violation of variable %d: %lf > %lf!\n",
			j,lp->x[j],lp->iupperbound[j]);
		exit(1000);
	}
	viol = lp->ilowerbound[j] - lp->x[j];
	if (viol>0.0001) {
		printf("\n0Strong lower bound violation of variable %d: %lf < %lf!\n",
			j,lp->x[j],lp->ilowerbound[j]);
		exit(1000);
	}
	}


	if (pril>=1) {
		printf("Initial basis: %1d original vars, %1d slacks, %1d artificials.\n",
			initbasisstructs,initbasisslacks,initbasisartifs);
	}


	/** Compute basic solution */

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
		exit(ALPS_RUNOUTOFMEMORY);
	}


	/** Set bounds and construct phase1 objective function */
	/*
	*  We temporarily set the bounds of the basic variables so as to
	* accomodate their current values. The phase 1 objective function
	* punishes infeasibilities. A slack variable either receives objective
	* function coefficient |0.0|, if it is already nonnegative, or else
	* |1.0| to count this infeasibility. A basic structural or artificial
	* variable variable receives |-1.0|, |0.0|, or |1.0|, depending on
	* whether it is above its upperbound, is within its bounds, or below its
	* lower bound, respectively. To avoid degeneracies the true values are
	* perturbed slightly at random. We switch to this temporary objective
	* function, and compute the initial objective function value.
	*/

	if (pril>=4) printprimalsol(lp);
	randseed = 333333;
	lp->objval = 0.0;
	for (i=0; i<lp->im; i++) {
		j = lp->basisheader[i];
		lp->x[j] = xbas[i];
		if (lp->x[j]<lp->ilowerbound[j]-ALPS_ZEROEPS) {
			lp->iupperbound[j] = lp->ilowerbound[j];
			lp->ilowerbound[j] = -ALPS_REAL_INFINITY;
			lp->iartifbounds[j] = ALPS_TRUE;
			lp->varstat[j] = ALPS_BOUNDEDABOVE;
			lp->iphase1obj[j] = 1.0 + 0.2*rand01(&randseed);
			lp->objval -= lp->iphase1obj[j]*(lp->ioriglowerbound[j]-lp->x[j]);
		}
		else if (lp->x[j]>lp->iupperbound[j]+ALPS_ZEROEPS) {
			lp->ilowerbound[j] = lp->iupperbound[j];
			lp->iupperbound[j] = ALPS_REAL_INFINITY;
			lp->iartifbounds[j] = ALPS_TRUE;
			lp->varstat[j] = ALPS_BOUNDEDBELOW;
			lp->iphase1obj[j] = -1.0 - 0.2*rand01(&randseed);
			lp->objval += lp->iphase1obj[j]*(lp->x[j]-lp->iorigupperbound[j]);
		}
	}
	lp->iobj = lp->iphase1obj;

	lp->basisstatus=ALPS_PHASE1BASIS; 
	if (pril>=4) printprimalsol(lp);


	/** Free space for alps\_phase1prim arrays */

	ffree( (char **) &rowcovered);
	ffree( (char **) &rowfill);
	ffree( (char **) &rowpivot);
	ffree( (char **) &sortkey);
	ffree( (char **) &sortindex);
	ffree( (char **) &pivrow);
	ffree( (char **) &pivcol);
	ffree( (char **) &pivvalue);
	ffree( (char **) &xbas);
	ffree( (char **) &trhs);

	return 0;
}

} // end namespace ogdf
