/*
 * $Revision: 1.2 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-11-27 03:02:28 +1100 (Thu, 27 Nov 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Basic versions of primal and dual pivot selection.
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
*  The function |primaleasypisel| implements greatest increase pivot
* selection for the primal simplex algorithm with full pricing. New
* multipliers |y| are available as well as the old |z|. The function
* |dualeasypisel| implements a basic dual pivot selection algorithm.
* Recoheap timing can be disabled.
*/

#include "alps.h"
#include "alps_misc.h"

namespace ogdf {

int randseed = 333;

int alps_primaleasypisel(
						 lpstruct *lp,
						 int /* pril */,
						 int *enter,
						 int *leave,
						 int *leaveindex,
						 double *redcost,
						 double *change,
						 int *pivrow,
						 int *pivcol,
						 double *pivvalue,
						 double * /*oldobjcoeff*/
						 )
{
	/** Declare easypisel scalars */
	double yi;
	double changej;
	int status;
	int i,j,k,l;
	int leavej;
	int starttime;
	int stoptime;
	double tt;
	double recoj;
	double dl;
	double dsvall;
	double ilowerboundi;
	double iupperboundi;
	int varstati;
	int dirjcount;
	int dsindl;

	/** Declare easypisel arrays */
	double *entcol;

	/** Allocate space for easypisel arrays */
	entcol = (double *) malloc( lp->im * sizeof(double) );
	if ( !entcol )
		// run out of memory
	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	/** Find reduced cost (primal) */
	/*
	*  If the basis matrix is freshly factorized, we compute the reduced
	* cost from scratch, otherwise we update it. In either case, we'll have
	* a priority queue of the entering candidates by Dantzig's criterion
	* afterwards.
	*/
#ifdef ORIG
	if (!lp->etanr) {
		// Compute reduced cost from scratch (primal)
		/*
		*  In order to compute the reduced cost from scratch, we set
		* |lp->reducedcost[j]| to zero for basic variables |j| and to
		* $c_j-y^TA_{.j}$ for nonbasic variables |j|. In the primal version, the
		* reduced cost heap is initialized, in the dual version, this is
		* omitted.
		*/
		for (j=0; j<lp->in; j++) lp->reducedcost[j] = lp->iobj[j];
		for (l=0; l<lp->yscount; l++) {
			i = lp->ysind[l];
			yi = lp->ysval[l];
			for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
				lp->reducedcost[lp->imatrowind[k]] -= yi*lp->imatrowcoeff[k];
			}
		}
		k = 0;
		for (j=0; j<lp->in; j++) lp->recoheappindex[j] = -1;
		for (j=0; j<lp->in; j++) {
			if (lp->colstat[j]==ALPS_BASIC) lp->reducedcost[j] = 0.0;
			else if (j<lp->inonartif) { /* nonbasis structural or slack */
				recoj = lp->reducedcost[j];
				if (recoj>ALPS_ZEROEPS) { /* wants to increase */
					if ((lp->colstat[j]==ALPS_NONBASICFREE)
						||((lp->colstat[j]==ALPS_NONBASICLOWB)&&(lp->varstat[j]!=ALPS_FIXED))) {
							k++;
							lp->recoheapp[k] = j;
							lp->recoheappindex[j] = k;
						}
				}
				if (recoj<-ALPS_ZEROEPS) { /* wants to decrease */
					if ((lp->colstat[j]==ALPS_NONBASICFREE)
						||((lp->colstat[j]==ALPS_NONBASICUPB)&&(lp->varstat[j]!=ALPS_FIXED))) {
							k++;
							lp->recoheapp[k] = j;
							lp->recoheappindex[j] = k;
						}
				}
			}
		}
		lp->recoheapcard = k;
		status = makerecoheap(lp);
	} 
	else {
		// Update reduced cost (primal)
		/*
		*  In order to update the reduced cost $r_{\rm new}$ from the reduced
		* cost $r_{\rm old}$, we first compute the quantity $\alpha = r_{\rm
		* old}[{\it enter}]/d[{\it leaveindex}]$. Then we can update $r^T_{\rm
		* new} = r^T_{\rm old}-\alpha z^TA_N$. In the primal version, we
		* maintain the entering candidate heap, in the dual version, this step
		* is omitted.
		*/

		if ((*leaveindex)>=0) {
			lp->reducedcost[(*enter)] = 0.0;
			alpha = (*redcost)/lp->d[(*leaveindex)];
			for (l=0; l<lp->zscount; l++) {
				i = lp->zsind[l];
				zi = lp->zsval[l];
				for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
					j = lp->imatrowind[k];
					if (lp->colstat[j]!=ALPS_BASIC) {
						rcdecrease = alpha*zi*lp->imatrowcoeff[k];
						oldrc = lp->reducedcost[j];
						newrc = oldrc - rcdecrease;
						absoldrc = fabs(oldrc);
						absnewrc = fabs(newrc);
						lp->reducedcost[j] = newrc;
						// Update the entering candidate heap for variable |j|

						if (j<lp->inonartif) {
							if (lp->x[j]<lp->iupperbound[j]-ALPS_ZEROEPS) { /* increaseable */
								if (lp->recoheappindex[j]>0) { /* was candidate */
									if (newrc>ALPS_ZEROEPS) { /* remains candidate */
										if (absnewrc>absoldrc+ALPS_ZEROEPS) {
											status = increaserecoheap(lp,j);
										}
										if (absnewrc<absoldrc-ALPS_ZEROEPS) {
											status = decreaserecoheap(lp,j);
										}
									}
									else {
										status = removerecoheap(lp,j);
									}
								}
								else { /* was no candidate */
									if (newrc>ALPS_ZEROEPS) {
										status = insertrecoheap(lp,j);
									}
								}
							}
							if (lp->x[j]>lp->ilowerbound[j]+ALPS_ZEROEPS) { /* decreaseable */
								if (lp->recoheappindex[j]>0) { /* was candidate */
									if (newrc<ALPS_ZEROEPS) { /* remains candidate */
										if (absnewrc>absoldrc+ALPS_ZEROEPS) {
											status = increaserecoheap(lp,j);
										}
										if (absnewrc<absoldrc-ALPS_ZEROEPS) {
											status = decreaserecoheap(lp,j);
										}
									}
									else {
										status = removerecoheap(lp,j);
									}
								}
								else { /* was no candidate */
									if (newrc<ALPS_ZEROEPS) {
										status = insertrecoheap(lp,j);
									}
								}
							}
						}


					}
				}
			}
			j = *leave;
#ifdef TTT
			oldrc = 0.0;
			absoldrc = fabs(oldrc);
			lp->reducedcost[j] = -alpha;
			if (lp->phase==ALPS_PHASE1)
				lp->reducedcost[j] -= (*oldobjcoeff);
			newrc = lp->reducedcost[j];
			absnewrc = fabs(newrc);
			// Update the entering candidate heap for variable |j|

			if (j<lp->inonartif) {
				if (lp->x[j]<lp->iupperbound[j]-ALPS_ZEROEPS) { /* increaseable */
					if (lp->recoheappindex[j]>0) { /* was candidate */
						if (newrc>ALPS_ZEROEPS) { /* remains candidate */
							if (absnewrc>absoldrc+ALPS_ZEROEPS) {
								status = increaserecoheap(lp,j);
							}
							if (absnewrc<absoldrc-ALPS_ZEROEPS) {
								status = decreaserecoheap(lp,j);
							}
						}
						else {
							status = removerecoheap(lp,j);
						}
					}
					else { /* was no candidate */
						if (newrc>ALPS_ZEROEPS) {
							status = insertrecoheap(lp,j);
						}
					}
				}
				if (lp->x[j]>lp->ilowerbound[j]+ALPS_ZEROEPS) { /* decreaseable */
					if (lp->recoheappindex[j]>0) { /* was candidate */
						if (newrc<ALPS_ZEROEPS) { /* remains candidate */
							if (absnewrc>absoldrc+ALPS_ZEROEPS) {
								status = increaserecoheap(lp,j);
							}
							if (absnewrc<absoldrc-ALPS_ZEROEPS) {
								status = decreaserecoheap(lp,j);
							}
						}
						else {
							status = removerecoheap(lp,j);
						}
					}
					else { /* was no candidate */
						if (newrc<ALPS_ZEROEPS) {
							status = insertrecoheap(lp,j);
						}
					}
				}
			}


#endif
			if (lp->colstat[j]==ALPS_NONBASICUPB) lp->reducedcost[j] = 1000.0;
			else lp->reducedcost[j] = -1000.0;
		}
	}
	if (pril>=3) {
		for (j=0; j<lp->in; j++) printf("  reco[%d] = %5.2lf\n",j,lp->reducedcost[j]);
	}
#endif
	// Compute reduced cost from scratch (primal)
	/*
	*  In order to compute the reduced cost from scratch, we set
	* |lp->reducedcost[j]| to zero for basic variables |j| and to
	* $c_j-y^TA_{.j}$ for nonbasic variables |j|. In the primal version, the
	* reduced cost heap is initialized, in the dual version, this is
	* omitted.
	*/
	for (j=0; j<lp->in; j++) lp->reducedcost[j] = lp->iobj[j];
	for (l=0; l<lp->yscount; l++) {
		i = lp->ysind[l];
		yi = lp->ysval[l];
		for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
			lp->reducedcost[lp->imatrowind[k]] -= yi*lp->imatrowcoeff[k];
		}
	}
	k = 0;
	for (j=0; j<lp->in; j++) lp->recoheappindex[j] = -1;
	for (j=0; j<lp->in; j++) {
		if (lp->colstat[j]==ALPS_BASIC) lp->reducedcost[j] = 0.0;
		else if (j<lp->inonartif) { /* nonbasis structural or slack */
			recoj = lp->reducedcost[j];
			if (recoj>ALPS_ZEROEPS) { /* wants to increase */
				if ((lp->colstat[j]==ALPS_NONBASICFREE)
					||((lp->colstat[j]==ALPS_NONBASICLOWB)&&(lp->varstat[j]!=ALPS_FIXED))) {
						k++;
						lp->recoheapp[k] = j;
						lp->recoheappindex[j] = k;
					}
			}
			if (recoj<-ALPS_ZEROEPS) { /* wants to decrease */
				if ((lp->colstat[j]==ALPS_NONBASICFREE)
					||((lp->colstat[j]==ALPS_NONBASICUPB)&&(lp->varstat[j]!=ALPS_FIXED))) {
						k++;
						lp->recoheapp[k] = j;
						lp->recoheappindex[j] = k;
					}
			}
		}
	}
	lp->recoheapcard = k;
	makerecoheap(lp);

	/** Find entering variable (primal) */
	/*
	*  We take the first variable from the priority queue, which must be the
	* best candidate. If the priority queue is empty, we have reached
	* optimality.
	*/
	if (lp->recoheapcard) {
		j = removemaxrecoheap(lp);
		recoj = lp->reducedcost[j];
	}
	else {
		lp->problemstatus = ALPS_OPTIMAL;
		goto freeall;
	}


	/** Find leaving variable (primal) */

	// Compute direction |dj| for variable |j|
	/*
	*  The direction $d$ is found by solving the system $A_Bd=a$, where $a$
	* is the candidate entering column |j|.
	*/
	for (l=0;l<lp->im;l++) entcol[l] = 0.0;
	for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) 
		entcol[lp->imatcolind[l]] = lp->imatcolcoeff[l];
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
		entcol,
		lp->d
		);

#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->fsolvetime += (stoptime - starttime);
	lp->piseltime -= (stoptime - starttime);
#endif
	dirjcount=0;
	for (l=0;l<lp->im;l++) {
		dl = lp->d[l];
		if ((dl<-ALPS_ZEROEPS)||(dl>ALPS_ZEROEPS)) { 
			lp->dsind[dirjcount] = l;
			lp->dsval[dirjcount] = dl;
			dirjcount++; 
		}
	}
	lp->dscount = dirjcount;

	// Compute change |changej| and |leavej| for |j| (phase2)
	/*
	*  The ratio test for phase 2.
	*/
#ifdef DEBUG
	printf("\n\nx[%d] = %lf [%lf,%lf] r = %lf\n\n",
		j,lp->x[j],lp->ilowerbound[j],lp->iupperbound[j],lp->reducedcost[j]); /***/
#endif
	leavej = -1000;
	if (recoj>ALPS_ZEROEPS) {
		changej = ALPS_REAL_INFINITY;
		for(l=0;l<dirjcount;l++) {
			tt = ALPS_REAL_INFINITY;
			dsvall = lp->dsval[l];
			dsindl = lp->dsind[l];
			i = lp->basisheader[dsindl];
			iupperboundi = lp->iupperbound[i];
			ilowerboundi = lp->ilowerbound[i];
			varstati = lp->varstat[i];
#ifdef DEBUG
			printf("x[%d] = %lf [%lf,%lf] d = %lf\n",
				i,lp->x[i],ilowerboundi,iupperboundi,dsvall); /***/
#endif
			if (varstati==ALPS_FIXED) tt = 0.0;
			if (dsvall<-ALPS_EPS) { /* wants to increase */
				if ((varstati==ALPS_BOUNDEDABOVE)||(varstati==ALPS_BOUNDEDTWICE))
					tt = (lp->x[i]-iupperboundi)/dsvall;
			}
			if (dsvall>ALPS_EPS) { /* wants to decreae */
				if ((varstati==ALPS_BOUNDEDBELOW)||(varstati==ALPS_BOUNDEDTWICE))
					tt = (lp->x[i]-ilowerboundi)/dsvall;
			}
#ifdef DEBUG
			printf("tt = %lf\n",tt); /***/
#endif
			if (tt<changej) {
				changej = tt;
				leavej = dsindl;
			}
		}
		if (lp->varstat[j]==ALPS_FIXED) tt = 0.0;
		if ((lp->varstat[j]==ALPS_BOUNDEDABOVE)||(lp->varstat[j]==ALPS_BOUNDEDTWICE)) {
			tt = lp->iupperbound[j]-lp->ilowerbound[j];
			if (tt<changej) {
				changej = tt;
				leavej = -1;
			}
		}
		if (changej<0.0) changej = 0.0;
	}
	else {
		changej = -ALPS_REAL_INFINITY;
		for(l=0;l<dirjcount;l++) {
			tt = -ALPS_REAL_INFINITY;
			dsvall = lp->dsval[l];
			dsindl = lp->dsind[l];
			i = lp->basisheader[dsindl];
			iupperboundi = lp->iupperbound[i];
			ilowerboundi = lp->ilowerbound[i];
			varstati = lp->varstat[i];
			if (varstati==ALPS_FIXED) tt = 0.0;
			if (dsvall<-ALPS_EPS) { /* wants to decrease */
				if ((varstati==ALPS_BOUNDEDBELOW)||(varstati==ALPS_BOUNDEDTWICE))
					tt = (lp->x[i]-ilowerboundi)/dsvall;
			}
			if (dsvall>ALPS_EPS) { /* wants to increase */
				if ((varstati==ALPS_BOUNDEDABOVE)||(varstati==ALPS_BOUNDEDTWICE))
					tt = (lp->x[i]-iupperboundi)/dsvall;
			}
			if (tt>changej) {
				changej = tt;
				leavej = dsindl;
			}
		}
		if (lp->varstat[j]==ALPS_FIXED) tt = 0.0;
		if ((lp->varstat[j]==ALPS_BOUNDEDBELOW)||(lp->varstat[j]==ALPS_BOUNDEDTWICE)) {
			tt = lp->ilowerbound[j]-lp->iupperbound[j];
			if (tt>changej) {
				changej = tt;
				leavej = -1;
			}
		}
		if (changej>0.0) changej = 0.0;
	}
	if (leavej==-1000) {
		lp->problemstatus = ALPS_UNBOUNDED;
		goto freeall;
	}


	(*enter) = j;
	(*leaveindex) = leavej;
	if (leavej>=0) (*leave) = lp->basisheader[leavej];
	(*redcost) = recoj;
	(*change) = changej;


freeall:
	/** Free space for easypisel arrays */

	ffree( (char **) &entcol);

	return 0;
}

int alps_primalphase1easypisel(
							   lpstruct *lp,
							   int /* pril */,
							   int *enter,
							   int *leave,
							   int *leaveindex,
							   double *redcost,
							   double *change,
							   int *pivrow,
							   int *pivcol,
							   double *pivvalue,
							   double * /* oldobjcoeff */,
							   int *wentfurther
							   )

{
	/** Declare easypisel scalars */

	double yi;
	double changej,changej1;
	int status;
	int i,j,k,l;
	int leavej,leavej1;
	int starttime;
	int stoptime;
	double tt,tt1;
	double recoj;
	double dl;
	double dsvall;
	double ilowerboundi;
	double iupperboundi;
	double ioriglowerboundi;
	double iorigupperboundi;
	int varstati;
	int origvarstati;
	int dirjcount;
	int dsindl;


	/** Declare easypisel arrays */
	double *entcol;

	/** Allocate space for easypisel arrays */
	entcol = (double *) malloc( lp->im * sizeof(double) );
	if ( !entcol )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	/** Find reduced cost (primalphase1) */
	/*
	*  If the basis matrix is freshly factorized, we compute the reduced
	* cost from scratch, otherwise we update it. In either case, we'll have
	* a priority queue of the entering candidates by Dantzig's criterion
	* afterwards.
	*/
#ifdef ORIG
	if ((!lp->etanr)||(wentfurther)) {
		// Compute reduced cost from scratch (primal)
		/*
		*  In order to compute the reduced cost from scratch, we set
		* |lp->reducedcost[j]| to zero for basic variables |j| and to
		* $c_j-y^TA_{.j}$ for nonbasic variables |j|. In the primal version, the
		* reduced cost heap is initialized, in the dual version, this is
		* omitted.
		*/
		for (j=0; j<lp->in; j++) lp->reducedcost[j] = lp->iobj[j];
		for (l=0; l<lp->yscount; l++) {
			i = lp->ysind[l];
			yi = lp->ysval[l];
			for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
				lp->reducedcost[lp->imatrowind[k]] -= yi*lp->imatrowcoeff[k];
			}
		}
		k = 0;
		for (j=0; j<lp->in; j++) lp->recoheappindex[j] = -1;
		for (j=0; j<lp->in; j++) {
			if (lp->colstat[j]==ALPS_BASIC) lp->reducedcost[j] = 0.0;
			else if (j<lp->inonartif) { /* nonbasis structural or slack */
				recoj = lp->reducedcost[j];
				if (recoj>ALPS_ZEROEPS) { /* wants to increase */
					if ((lp->colstat[j]==ALPS_NONBASICFREE)
						||((lp->colstat[j]==ALPS_NONBASICLOWB)&&(lp->varstat[j]!=ALPS_FIXED))) {
							k++;
							lp->recoheapp[k] = j;
							lp->recoheappindex[j] = k;
						}
				}
				if (recoj<-ALPS_ZEROEPS) { /* wants to decrease */
					if ((lp->colstat[j]==ALPS_NONBASICFREE)
						||((lp->colstat[j]==ALPS_NONBASICUPB)&&(lp->varstat[j]!=ALPS_FIXED))) {
							k++;
							lp->recoheapp[k] = j;
							lp->recoheappindex[j] = k;
						}
				}
			}
		}
		lp->recoheapcard = k;
		status = makerecoheap(lp);
	} 
	else {
		// Update reduced cost (primal)
		/*
		*  In order to update the reduced cost $r_{\rm new}$ from the reduced
		* cost $r_{\rm old}$, we first compute the quantity $\alpha = r_{\rm
		* old}[{\it enter}]/d[{\it leaveindex}]$. Then we can update $r^T_{\rm
		* new} = r^T_{\rm old}-\alpha z^TA_N$. In the primal version, we
		* maintain the entering candidate heap, in the dual version, this step
		* is omitted.
		*/

		if ((*leaveindex)>=0) {
			lp->reducedcost[(*enter)] = 0.0;
			alpha = (*redcost)/lp->d[(*leaveindex)];
			for (l=0; l<lp->zscount; l++) {
				i = lp->zsind[l];
				zi = lp->zsval[l];
				for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
					j = lp->imatrowind[k];
					if (lp->colstat[j]!=ALPS_BASIC) {
						rcdecrease = alpha*zi*lp->imatrowcoeff[k];
						oldrc = lp->reducedcost[j];
						newrc = oldrc - rcdecrease;
						absoldrc = fabs(oldrc);
						absnewrc = fabs(newrc);
						lp->reducedcost[j] = newrc;
						// Update the entering candidate heap for variable |j|

						if (j<lp->inonartif) {
							if (lp->x[j]<lp->iupperbound[j]-ALPS_ZEROEPS) { /* increaseable */
								if (lp->recoheappindex[j]>0) { /* was candidate */
									if (newrc>ALPS_ZEROEPS) { /* remains candidate */
										if (absnewrc>absoldrc+ALPS_ZEROEPS) {
											status = increaserecoheap(lp,j);
										}
										if (absnewrc<absoldrc-ALPS_ZEROEPS) {
											status = decreaserecoheap(lp,j);
										}
									}
									else {
										status = removerecoheap(lp,j);
									}
								}
								else { /* was no candidate */
									if (newrc>ALPS_ZEROEPS) {
										status = insertrecoheap(lp,j);
									}
								}
							}
							if (lp->x[j]>lp->ilowerbound[j]+ALPS_ZEROEPS) { /* decreaseable */
								if (lp->recoheappindex[j]>0) { /* was candidate */
									if (newrc<ALPS_ZEROEPS) { /* remains candidate */
										if (absnewrc>absoldrc+ALPS_ZEROEPS) {
											status = increaserecoheap(lp,j);
										}
										if (absnewrc<absoldrc-ALPS_ZEROEPS) {
											status = decreaserecoheap(lp,j);
										}
									}
									else {
										status = removerecoheap(lp,j);
									}
								}
								else { /* was no candidate */
									if (newrc<ALPS_ZEROEPS) {
										status = insertrecoheap(lp,j);
									}
								}
							}
						}


					}
				}
			}
			j = *leave;
#ifdef TTT
			oldrc = 0.0;
			absoldrc = fabs(oldrc);
			lp->reducedcost[j] = -alpha;
			if (lp->phase==ALPS_PHASE1)
				lp->reducedcost[j] -= (*oldobjcoeff);
			newrc = lp->reducedcost[j];
			absnewrc = fabs(newrc);
			// Update the entering candidate heap for variable |j|

			if (j<lp->inonartif) {
				if (lp->x[j]<lp->iupperbound[j]-ALPS_ZEROEPS) { /* increaseable */
					if (lp->recoheappindex[j]>0) { /* was candidate */
						if (newrc>ALPS_ZEROEPS) { /* remains candidate */
							if (absnewrc>absoldrc+ALPS_ZEROEPS) {
								status = increaserecoheap(lp,j);
							}
							if (absnewrc<absoldrc-ALPS_ZEROEPS) {
								status = decreaserecoheap(lp,j);
							}
						}
						else {
							status = removerecoheap(lp,j);
						}
					}
					else { /* was no candidate */
						if (newrc>ALPS_ZEROEPS) {
							status = insertrecoheap(lp,j);
						}
					}
				}
				if (lp->x[j]>lp->ilowerbound[j]+ALPS_ZEROEPS) { /* decreaseable */
					if (lp->recoheappindex[j]>0) { /* was candidate */
						if (newrc<ALPS_ZEROEPS) { /* remains candidate */
							if (absnewrc>absoldrc+ALPS_ZEROEPS) {
								status = increaserecoheap(lp,j);
							}
							if (absnewrc<absoldrc-ALPS_ZEROEPS) {
								status = decreaserecoheap(lp,j);
							}
						}
						else {
							status = removerecoheap(lp,j);
						}
					}
					else { /* was no candidate */
						if (newrc<ALPS_ZEROEPS) {
							status = insertrecoheap(lp,j);
						}
					}
				}
			}

#endif
			if (lp->colstat[j]==ALPS_NONBASICUPB) lp->reducedcost[j] = 1000.0;
			else lp->reducedcost[j] = -1000.0;
		}
	}
	if (pril>=3) {
		for (j=0; j<lp->in; j++) printf("  reco[%d] = %5.2lf\n",j,lp->reducedcost[j]);
	}
#endif
	// Compute reduced cost from scratch (primal)
	/*
	*  In order to compute the reduced cost from scratch, we set
	* |lp->reducedcost[j]| to zero for basic variables |j| and to
	* $c_j-y^TA_{.j}$ for nonbasic variables |j|. In the primal version, the
	* reduced cost heap is initialized, in the dual version, this is
	* omitted.
	*/
	for (j=0; j<lp->in; j++) lp->reducedcost[j] = lp->iobj[j];
	for (l=0; l<lp->yscount; l++) {
		i = lp->ysind[l];
		yi = lp->ysval[l];
		for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
			lp->reducedcost[lp->imatrowind[k]] -= yi*lp->imatrowcoeff[k];
		}
	}
	k = 0;
	for (j=0; j<lp->in; j++) lp->recoheappindex[j] = -1;
	for (j=0; j<lp->in; j++) {
		if (lp->colstat[j]==ALPS_BASIC) lp->reducedcost[j] = 0.0;
		else if (j<lp->inonartif) { /* nonbasis structural or slack */
			recoj = lp->reducedcost[j];
			if (recoj>ALPS_ZEROEPS) { /* wants to increase */
				if ((lp->colstat[j]==ALPS_NONBASICFREE)
					||((lp->colstat[j]==ALPS_NONBASICLOWB)&&(lp->varstat[j]!=ALPS_FIXED))) {
						k++;
						lp->recoheapp[k] = j;
						lp->recoheappindex[j] = k;
					}
			}
			if (recoj<-ALPS_ZEROEPS) { /* wants to decrease */
				if ((lp->colstat[j]==ALPS_NONBASICFREE)
					||((lp->colstat[j]==ALPS_NONBASICUPB)&&(lp->varstat[j]!=ALPS_FIXED))) {
						k++;
						lp->recoheapp[k] = j;
						lp->recoheappindex[j] = k;
					}
			}
		}
	}
	lp->recoheapcard = k;
	makerecoheap(lp);




	/** Find entering variable (primal) */
	/*
	*  We take the first variable from the priority queue, which must be the
	* best candidate. If the priority queue is empty, we have reached
	* optimality.
	*/
	if (lp->recoheapcard) {
		j = removemaxrecoheap(lp);
		recoj = lp->reducedcost[j];
	}
	else {
		lp->problemstatus = ALPS_OPTIMAL;
		goto freeall;
	}


	/** Find leaving variable (primalphase1) */

	// Compute direction |dj| for variable |j|
	/*
	*  The direction $d$ is found by solving the system $A_Bd=a$, where $a$
	* is the candidate entering column |j|.
	*/
	for (l=0;l<lp->im;l++) entcol[l] = 0.0;
	for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) 
		entcol[lp->imatcolind[l]] = lp->imatcolcoeff[l];
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
		entcol,
		lp->d
		);

#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->fsolvetime += (stoptime - starttime);
	lp->piseltime -= (stoptime - starttime);
#endif
	dirjcount=0;
	for (l=0;l<lp->im;l++) {
		dl = lp->d[l];
		if ((dl<-ALPS_ZEROEPS)||(dl>ALPS_ZEROEPS)) { 
			lp->dsind[dirjcount] = l;
			lp->dsval[dirjcount] = dl;
			dirjcount++; 
		}
	}
	lp->dscount = dirjcount;


	// Compute change |changej| and |leavej| for |j| (phase1)
	/*
	*  The ratio test for phase 1. Here we have introduced an option to go
	* further as long as infeasibilities decrease.
	*/
	leavej = -1000;
	leavej1 = -1000;
	if (recoj>ALPS_ZEROEPS) {
		changej = ALPS_REAL_INFINITY;
		changej1 = ALPS_REAL_INFINITY;
		for(l=0;l<dirjcount;l++) {
			tt = ALPS_REAL_INFINITY;
			tt1 = ALPS_REAL_INFINITY;
			dsvall = lp->dsval[l];
			dsindl = lp->dsind[l];
			i = lp->basisheader[dsindl];

			iupperboundi = lp->iupperbound[i];
			ilowerboundi = lp->ilowerbound[i];
			iorigupperboundi = lp->iorigupperbound[i];
			ioriglowerboundi = lp->ioriglowerbound[i];
			varstati = lp->varstat[i];
			origvarstati = lp->origvarstat[i];

			if (varstati==ALPS_FIXED) tt = 0.0;
			if (dsvall<-ALPS_EPS) { /* wants to increase */
				if ((varstati==ALPS_BOUNDEDABOVE)||(varstati==ALPS_BOUNDEDTWICE)) {
					tt = (lp->x[i]-iupperboundi)/dsvall;
					if (lp->iartifbounds[i]) {
						if ((origvarstati==ALPS_BOUNDEDABOVE)
							||(origvarstati==ALPS_BOUNDEDTWICE)
							||(origvarstati==ALPS_FIXED))
							tt1 = (lp->x[i]-iorigupperboundi)/dsvall;
					}
					else tt1 = tt;
				}
			}
			if (dsvall>ALPS_EPS) { /* wants to decrease */
				if ((varstati==ALPS_BOUNDEDBELOW)||(varstati==ALPS_BOUNDEDTWICE)) {
					tt = (lp->x[i]-ilowerboundi)/dsvall;
					if (lp->iartifbounds[i]) {
						if ((origvarstati==ALPS_BOUNDEDBELOW)
							||(origvarstati==ALPS_BOUNDEDTWICE)
							||(origvarstati==ALPS_FIXED)) 
							tt1 = (lp->x[i]-ioriglowerboundi)/dsvall;
					}
					else tt1 = tt;
				}
			}

			if (tt<changej) {
				changej = tt;
				leavej = dsindl;
			}

			if (tt1<changej1) {
				changej1 = tt1;
				leavej1 = dsindl;
			}
		}
		if ((lp->varstat[j]==ALPS_BOUNDEDABOVE)||(lp->varstat[j]==ALPS_BOUNDEDTWICE)) {
			tt = lp->iupperbound[j]-lp->ilowerbound[j];
			if (tt<changej) {
				changej = tt;
				leavej = -1;
			}
			if (tt<changej1) {
				changej1 = tt;
				leavej1 = -1;
			}
		}
		if (changej<0.0) changej = 0.0;
		if (changej1<0.0) changej1 = 0.0;
	}
	else {
		changej = -ALPS_REAL_INFINITY;
		changej1 = -ALPS_REAL_INFINITY;
		for(l=0;l<dirjcount;l++) {
			tt = -ALPS_REAL_INFINITY;
			tt1 = -ALPS_REAL_INFINITY;
			dsvall = lp->dsval[l];
			dsindl = lp->dsind[l];
			i = lp->basisheader[dsindl];

			iupperboundi = lp->iupperbound[i];
			ilowerboundi = lp->ilowerbound[i];
			iorigupperboundi = lp->iorigupperbound[i];
			ioriglowerboundi = lp->ioriglowerbound[i];
			varstati = lp->varstat[i];
			origvarstati = lp->origvarstat[i];

			if (varstati==ALPS_FIXED) tt = 0.0;
			if (dsvall<-ALPS_EPS) { /* wants to decrease */
				if ((varstati==ALPS_BOUNDEDBELOW)||(varstati==ALPS_BOUNDEDTWICE)) {
					tt = (lp->x[i]-ilowerboundi)/dsvall;
					if (lp->iartifbounds[i]) {
						if ((origvarstati==ALPS_BOUNDEDBELOW)
							||(origvarstati==ALPS_BOUNDEDTWICE)
							||(origvarstati==ALPS_FIXED)) 
							tt1 = (lp->x[i]-ioriglowerboundi)/dsvall;   
					}
					else tt1 = tt;    
				} 
			}

			if (dsvall>ALPS_EPS) { /* wants to increase */
				if ((varstati==ALPS_BOUNDEDABOVE)||(varstati==ALPS_BOUNDEDTWICE)) {
					tt = (lp->x[i]-iupperboundi)/dsvall;
					if (lp->iartifbounds[i]) {
						if ((origvarstati==ALPS_BOUNDEDABOVE)
							||(origvarstati==ALPS_BOUNDEDTWICE)
							||(origvarstati==ALPS_FIXED)) 
							tt1 = (lp->x[i]-iorigupperboundi)/dsvall;
					}
					else tt1 = tt;   
				}  
			}

			if (tt>changej) {
				changej = tt;
				leavej = dsindl;
			}

			if (tt1>changej1) {
				changej1 = tt1;
				leavej1 = dsindl;
			}
		}
		if ((lp->varstat[j]==ALPS_BOUNDEDBELOW)||(lp->varstat[j]==ALPS_BOUNDEDTWICE)) {
			tt = lp->ilowerbound[j]-lp->iupperbound[j];
			if (tt>changej) {
				changej = tt;
				leavej = -1;
			}
			if (tt>changej1) {
				changej1 = tt;
				leavej1 = -1;
			}
		}
		if (changej>0.0) changej = 0.0;
		if (changej1>0.0) changej1 = 0.0;
	}
	*wentfurther = ALPS_FALSE;

#ifdef TTT
	if (changej1>changej) { 
		double simobjval,oldx,newx,dirvall; 
		int l,dirindl,basisvariable;
		// Basis update simulator (primal phase1)

		simobjval = 0.0;
		for(l=0;l<lp->dscount;l++) {
			dirindl = lp->dsind[l];
			dirvall = lp->dsval[l];
			basisvariable = lp->basisheader[dirindl];
			oldx = lp->x[basisvariable];
			newx = oldx - changej1*dirvall;
			simobjval -= (oldx-newx)*lp->iobj[basisvariable];
			if (lp->iartifbounds[basisvariable]) {
				if ((newx<=lp->iorigupperbound[basisvariable]+ALPS_ZEROEPS)&&
					(newx>=lp->ioriglowerbound[basisvariable]-ALPS_ZEROEPS)) {
						simobjval -= lp->iphase1obj[basisvariable]*lp->x[basisvariable];
					}
			}
		}
		oldx = lp->x[j];
		newx = oldx + changej1;
		simobjval -= (oldx-newx)*lp->iobj[j];


		if (simobjval>=0.001) {
			printf("changej = %lf, changej1 = %lf\n",changej,changej1);
			changej = changej1;
			leavej = leavej1;
			*wentfurther = ALPS_TRUE;
		}
	}

	oder so?
		if (changej1>changej && changej1<10000.0) { 
			if (pril>=2) printf("changej = %lf, changej1 = %lf\n",changej,changej1);
			changej = changej1;
			leavej = leavej1;
			*wentfurther = ALPS_TRUE;
		}
#endif

		if (leavej==-1000) {
			lp->problemstatus = ALPS_UNBOUNDED;
			goto freeall;
		}


		(*enter) = j;
		(*leaveindex) = leavej;
		if (leavej>=0) (*leave) = lp->basisheader[leavej];
		(*redcost) = recoj;
		(*change) = changej;


freeall:
		/** Free space for easypisel arrays */

		ffree( (char **) &entcol);

		return 0;
}

int alps_primalrandompisel(
						   lpstruct *lp,
						   int /* pril */,
						   int *enter,
						   int *leave,
						   int *leaveindex,
						   double *redcost,
						   double *change,
						   int *pivrow,
						   int *pivcol,
						   double *pivvalue,
						   double * /* oldobjcoeff */
						   )

{
	/** Declare easypisel scalars */

	double yi;
	double changej;
	int status;
	int i,j,k,l;
	int leavej;
	int starttime;
	int stoptime;
	double tt;
	double recoj;
	double dl;
	double dsvall;
	double ilowerboundi;
	double iupperboundi;
	int varstati;
	int dirjcount;
	int dsindl;
	int jpos;


	/** Declare easypisel arrays */

	double *entcol;


	/** Allocate space for easypisel arrays */

	entcol = (double *) malloc( lp->im * sizeof(double) );
	if ( !entcol )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}





	/** Find reduced cost (primal) */
	/*
	*  If the basis matrix is freshly factorized, we compute the reduced
	* cost from scratch, otherwise we update it. In either case, we'll have
	* a priority queue of the entering candidates by Dantzig's criterion
	* afterwards.
	*/
#ifdef ORIG
	if (!lp->etanr) {
		// Compute reduced cost from scratch (primal)
		/*
		*  In order to compute the reduced cost from scratch, we set
		* |lp->reducedcost[j]| to zero for basic variables |j| and to
		* $c_j-y^TA_{.j}$ for nonbasic variables |j|. In the primal version, the
		* reduced cost heap is initialized, in the dual version, this is
		* omitted.
		*/
		for (j=0; j<lp->in; j++) lp->reducedcost[j] = lp->iobj[j];
		for (l=0; l<lp->yscount; l++) {
			i = lp->ysind[l];
			yi = lp->ysval[l];
			for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
				lp->reducedcost[lp->imatrowind[k]] -= yi*lp->imatrowcoeff[k];
			}
		}
		k = 0;
		for (j=0; j<lp->in; j++) lp->recoheappindex[j] = -1;
		for (j=0; j<lp->in; j++) {
			if (lp->colstat[j]==ALPS_BASIC) lp->reducedcost[j] = 0.0;
			else if (j<lp->inonartif) { /* nonbasis structural or slack */
				recoj = lp->reducedcost[j];
				if (recoj>ALPS_ZEROEPS) { /* wants to increase */
					if ((lp->colstat[j]==ALPS_NONBASICFREE)
						||((lp->colstat[j]==ALPS_NONBASICLOWB)&&(lp->varstat[j]!=ALPS_FIXED))) {
							k++;
							lp->recoheapp[k] = j;
							lp->recoheappindex[j] = k;
						}
				}
				if (recoj<-ALPS_ZEROEPS) { /* wants to decrease */
					if ((lp->colstat[j]==ALPS_NONBASICFREE)
						||((lp->colstat[j]==ALPS_NONBASICUPB)&&(lp->varstat[j]!=ALPS_FIXED))) {
							k++;
							lp->recoheapp[k] = j;
							lp->recoheappindex[j] = k;
						}
				}
			}
		}
		lp->recoheapcard = k;
		status = makerecoheap(lp);


	} 
	else {
		// Update reduced cost (primal)
		/*
		*  In order to update the reduced cost $r_{\rm new}$ from the reduced
		* cost $r_{\rm old}$, we first compute the quantity $\alpha = r_{\rm
		* old}[{\it enter}]/d[{\it leaveindex}]$. Then we can update $r^T_{\rm
		* new} = r^T_{\rm old}-\alpha z^TA_N$. In the primal version, we
		* maintain the entering candidate heap, in the dual version, this step
		* is omitted.
		*/

		if ((*leaveindex)>=0) {
			lp->reducedcost[(*enter)] = 0.0;
			alpha = (*redcost)/lp->d[(*leaveindex)];
			for (l=0; l<lp->zscount; l++) {
				i = lp->zsind[l];
				zi = lp->zsval[l];
				for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
					j = lp->imatrowind[k];
					if (lp->colstat[j]!=ALPS_BASIC) {
						rcdecrease = alpha*zi*lp->imatrowcoeff[k];
						oldrc = lp->reducedcost[j];
						newrc = oldrc - rcdecrease;
						absoldrc = fabs(oldrc);
						absnewrc = fabs(newrc);
						lp->reducedcost[j] = newrc;
						// Update the entering candidate heap for variable |j|

						if (j<lp->inonartif) {
							if (lp->x[j]<lp->iupperbound[j]-ALPS_ZEROEPS) { /* increaseable */
								if (lp->recoheappindex[j]>0) { /* was candidate */
									if (newrc>ALPS_ZEROEPS) { /* remains candidate */
										if (absnewrc>absoldrc+ALPS_ZEROEPS) {
											status = increaserecoheap(lp,j);
										}
										if (absnewrc<absoldrc-ALPS_ZEROEPS) {
											status = decreaserecoheap(lp,j);
										}
									}
									else {
										status = removerecoheap(lp,j);
									}
								}
								else { /* was no candidate */
									if (newrc>ALPS_ZEROEPS) {
										status = insertrecoheap(lp,j);
									}
								}
							}
							if (lp->x[j]>lp->ilowerbound[j]+ALPS_ZEROEPS) { /* decreaseable */
								if (lp->recoheappindex[j]>0) { /* was candidate */
									if (newrc<ALPS_ZEROEPS) { /* remains candidate */
										if (absnewrc>absoldrc+ALPS_ZEROEPS) {
											status = increaserecoheap(lp,j);
										}
										if (absnewrc<absoldrc-ALPS_ZEROEPS) {
											status = decreaserecoheap(lp,j);
										}
									}
									else {
										status = removerecoheap(lp,j);
									}
								}
								else { /* was no candidate */
									if (newrc<ALPS_ZEROEPS) {
										status = insertrecoheap(lp,j);
									}
								}
							}
						}


					}
				}
			}
			j = *leave;
#ifdef TTT
			oldrc = 0.0;
			absoldrc = fabs(oldrc);
			lp->reducedcost[j] = -alpha;
			if (lp->phase==ALPS_PHASE1)
				lp->reducedcost[j] -= (*oldobjcoeff);
			newrc = lp->reducedcost[j];
			absnewrc = fabs(newrc);
			// Update the entering candidate heap for variable |j|

			if (j<lp->inonartif) {
				if (lp->x[j]<lp->iupperbound[j]-ALPS_ZEROEPS) { /* increaseable */
					if (lp->recoheappindex[j]>0) { /* was candidate */
						if (newrc>ALPS_ZEROEPS) { /* remains candidate */
							if (absnewrc>absoldrc+ALPS_ZEROEPS) {
								status = increaserecoheap(lp,j);
							}
							if (absnewrc<absoldrc-ALPS_ZEROEPS) {
								status = decreaserecoheap(lp,j);
							}
						}
						else {
							status = removerecoheap(lp,j);
						}
					}
					else { /* was no candidate */
						if (newrc>ALPS_ZEROEPS) {
							status = insertrecoheap(lp,j);
						}
					}
				}
				if (lp->x[j]>lp->ilowerbound[j]+ALPS_ZEROEPS) { /* decreaseable */
					if (lp->recoheappindex[j]>0) { /* was candidate */
						if (newrc<ALPS_ZEROEPS) { /* remains candidate */
							if (absnewrc>absoldrc+ALPS_ZEROEPS) {
								status = increaserecoheap(lp,j);
							}
							if (absnewrc<absoldrc-ALPS_ZEROEPS) {
								status = decreaserecoheap(lp,j);
							}
						}
						else {
							status = removerecoheap(lp,j);
						}
					}
					else { /* was no candidate */
						if (newrc<ALPS_ZEROEPS) {
							status = insertrecoheap(lp,j);
						}
					}
				}
			}


#endif
			if (lp->colstat[j]==ALPS_NONBASICUPB) lp->reducedcost[j] = 1000.0;
			else lp->reducedcost[j] = -1000.0;
		}


	}
	if (pril>=3) {
		for (j=0; j<lp->in; j++) printf("  reco[%d] = %5.2lf\n",j,lp->reducedcost[j]);
	}
#endif
	// Compute reduced cost from scratch (primal)
	/*
	*  In order to compute the reduced cost from scratch, we set
	* |lp->reducedcost[j]| to zero for basic variables |j| and to
	* $c_j-y^TA_{.j}$ for nonbasic variables |j|. In the primal version, the
	* reduced cost heap is initialized, in the dual version, this is
	* omitted.
	*/
	for (j=0; j<lp->in; j++) lp->reducedcost[j] = lp->iobj[j];
	for (l=0; l<lp->yscount; l++) {
		i = lp->ysind[l];
		yi = lp->ysval[l];
		for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
			lp->reducedcost[lp->imatrowind[k]] -= yi*lp->imatrowcoeff[k];
		}
	}
	k = 0;
	for (j=0; j<lp->in; j++) lp->recoheappindex[j] = -1;
	for (j=0; j<lp->in; j++) {
		if (lp->colstat[j]==ALPS_BASIC) lp->reducedcost[j] = 0.0;
		else if (j<lp->inonartif) { /* nonbasis structural or slack */
			recoj = lp->reducedcost[j];
			if (recoj>ALPS_ZEROEPS) { /* wants to increase */
				if ((lp->colstat[j]==ALPS_NONBASICFREE)
					||((lp->colstat[j]==ALPS_NONBASICLOWB)&&(lp->varstat[j]!=ALPS_FIXED))) {
						k++;
						lp->recoheapp[k] = j;
						lp->recoheappindex[j] = k;
					}
			}
			if (recoj<-ALPS_ZEROEPS) { /* wants to decrease */
				if ((lp->colstat[j]==ALPS_NONBASICFREE)
					||((lp->colstat[j]==ALPS_NONBASICUPB)&&(lp->varstat[j]!=ALPS_FIXED))) {
						k++;
						lp->recoheapp[k] = j;
						lp->recoheappindex[j] = k;
					}
			}
		}
	}
	lp->recoheapcard = k;
	makerecoheap(lp);





	/** Find random entering variable (primal) */
	/*
	*  We take a random variable from the priority queue. If the priority
	* queue is empty, we have reached optimality.
	*/
	{
		if (lp->recoheapcard) {
			if (lp->recoheapcard==1) jpos = 1;
			else {
				jpos = 1 + ( (int) ( rand01(&randseed)* ((double)lp->recoheapcard-1) ) );
				if ((jpos<1)||(jpos>lp->recoheapcard)) printf("Random choice out of range!!!\n");
			}
			j = lp->recoheapp[jpos];
			recoj = lp->reducedcost[j];
			removerecoheap(lp,j);
		}
		else {
			lp->problemstatus = ALPS_OPTIMAL;
			goto freeall;
		}
	}


	/** Find leaving variable (primal) */

	// Compute direction |dj| for variable |j|
	/*
	*  The direction $d$ is found by solving the system $A_Bd=a$, where $a$
	* is the candidate entering column |j|.
	*/
	for (l=0;l<lp->im;l++) entcol[l] = 0.0;
	for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) 
		entcol[lp->imatcolind[l]] = lp->imatcolcoeff[l];
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
		entcol,
		lp->d
		);

#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->fsolvetime += (stoptime - starttime);
	lp->piseltime -= (stoptime - starttime);
#endif
	dirjcount=0;
	for (l=0;l<lp->im;l++) {
		dl = lp->d[l];
		if ((dl<-ALPS_ZEROEPS)||(dl>ALPS_ZEROEPS)) { 
			lp->dsind[dirjcount] = l;
			lp->dsval[dirjcount] = dl;
			dirjcount++; 
		}
	}
	lp->dscount = dirjcount;


	// Compute change |changej| and |leavej| for |j| (phase2)
	/*
	*  The ratio test for phase 2.
	*/
#ifdef DEBUG
	printf("\n\nx[%d] = %lf [%lf,%lf] r = %lf\n\n",
		j,lp->x[j],lp->ilowerbound[j],lp->iupperbound[j],lp->reducedcost[j]); /***/
#endif
	leavej = -1000;
	if (recoj>ALPS_ZEROEPS) {
		changej = ALPS_REAL_INFINITY;
		for(l=0;l<dirjcount;l++) {
			tt = ALPS_REAL_INFINITY;
			dsvall = lp->dsval[l];
			dsindl = lp->dsind[l];
			i = lp->basisheader[dsindl];
			iupperboundi = lp->iupperbound[i];
			ilowerboundi = lp->ilowerbound[i];
			varstati = lp->varstat[i];
#ifdef DEBUG
			printf("x[%d] = %lf [%lf,%lf] d = %lf\n",
				i,lp->x[i],ilowerboundi,iupperboundi,dsvall); /***/
#endif
			if (varstati==ALPS_FIXED) tt = 0.0;
			if (dsvall<-ALPS_EPS) { /* wants to increase */
				if ((varstati==ALPS_BOUNDEDABOVE)||(varstati==ALPS_BOUNDEDTWICE))
					tt = (lp->x[i]-iupperboundi)/dsvall;
			}
			if (dsvall>ALPS_EPS) { /* wants to decreae */
				if ((varstati==ALPS_BOUNDEDBELOW)||(varstati==ALPS_BOUNDEDTWICE))
					tt = (lp->x[i]-ilowerboundi)/dsvall;
			}
#ifdef DEBUG
			printf("tt = %lf\n",tt); /***/
#endif
			if (tt<changej) {
				changej = tt;
				leavej = dsindl;
			}
		}
		if (lp->varstat[j]==ALPS_FIXED) tt = 0.0;
		if ((lp->varstat[j]==ALPS_BOUNDEDABOVE)||(lp->varstat[j]==ALPS_BOUNDEDTWICE)) {
			tt = lp->iupperbound[j]-lp->ilowerbound[j];
			if (tt<changej) {
				changej = tt;
				leavej = -1;
			}
		}
		if (changej<0.0) changej = 0.0;
	}
	else {
		changej = -ALPS_REAL_INFINITY;
		for(l=0;l<dirjcount;l++) {
			tt = -ALPS_REAL_INFINITY;
			dsvall = lp->dsval[l];
			dsindl = lp->dsind[l];
			i = lp->basisheader[dsindl];
			iupperboundi = lp->iupperbound[i];
			ilowerboundi = lp->ilowerbound[i];
			varstati = lp->varstat[i];
			if (varstati==ALPS_FIXED) tt = 0.0;
			if (dsvall<-ALPS_EPS) { /* wants to decrease */
				if ((varstati==ALPS_BOUNDEDBELOW)||(varstati==ALPS_BOUNDEDTWICE))
					tt = (lp->x[i]-ilowerboundi)/dsvall;
			}
			if (dsvall>ALPS_EPS) { /* wants to increase */
				if ((varstati==ALPS_BOUNDEDABOVE)||(varstati==ALPS_BOUNDEDTWICE))
					tt = (lp->x[i]-iupperboundi)/dsvall;
			}
			if (tt>changej) {
				changej = tt;
				leavej = dsindl;
			}
		}
		if (lp->varstat[j]==ALPS_FIXED) tt = 0.0;
		if ((lp->varstat[j]==ALPS_BOUNDEDBELOW)||(lp->varstat[j]==ALPS_BOUNDEDTWICE)) {
			tt = lp->ilowerbound[j]-lp->iupperbound[j];
			if (tt>changej) {
				changej = tt;
				leavej = -1;
			}
		}
		if (changej>0.0) changej = 0.0;
	}
	if (leavej==-1000) {
		lp->problemstatus = ALPS_UNBOUNDED;
		goto freeall;
	}


	(*enter) = j;
	(*leaveindex) = leavej;
	if (leavej>=0) (*leave) = lp->basisheader[leavej];
	(*redcost) = recoj;
	(*change) = changej;


freeall:
	/** Free space for easypisel arrays */

	ffree( (char **) &entcol);


	return 0;
}

int alps_primalphase1randompisel(
								 lpstruct *lp,
								 int /* pril */,
								 int *enter,
								 int *leave,
								 int *leaveindex,
								 double *redcost,
								 double *change,
								 int *pivrow,
								 int *pivcol,
								 double *pivvalue,
								 double * /* oldobjcoeff */,
								 int *wentfurther
								 )

{
	/** Declare easypisel scalars */

	double yi;
	double changej,changej1;
	int status;
	int i,j,k,l;
	int leavej,leavej1;
	int starttime;
	int stoptime;
	double tt,tt1;
	double recoj;
	double dl;
	double dsvall;
	double ilowerboundi;
	double iupperboundi;
	double ioriglowerboundi;
	double iorigupperboundi;
	int varstati;
	int origvarstati;
	int dirjcount;
	int dsindl;
	int jpos;


	/** Declare easypisel arrays */

	double *entcol;


	/** Allocate space for easypisel arrays */

	entcol = (double *) malloc( lp->im * sizeof(double) );
	if ( !entcol )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}





	/** Find reduced cost (primal) */
	/*
	*  If the basis matrix is freshly factorized, we compute the reduced
	* cost from scratch, otherwise we update it. In either case, we'll have
	* a priority queue of the entering candidates by Dantzig's criterion
	* afterwards.
	*/
#ifdef ORIG
	if (!lp->etanr) {
		// Compute reduced cost from scratch (primal)
		/*
		*  In order to compute the reduced cost from scratch, we set
		* |lp->reducedcost[j]| to zero for basic variables |j| and to
		* $c_j-y^TA_{.j}$ for nonbasic variables |j|. In the primal version, the
		* reduced cost heap is initialized, in the dual version, this is
		* omitted.
		*/
		for (j=0; j<lp->in; j++) lp->reducedcost[j] = lp->iobj[j];
		for (l=0; l<lp->yscount; l++) {
			i = lp->ysind[l];
			yi = lp->ysval[l];
			for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
				lp->reducedcost[lp->imatrowind[k]] -= yi*lp->imatrowcoeff[k];
			}
		}
		k = 0;
		for (j=0; j<lp->in; j++) lp->recoheappindex[j] = -1;
		for (j=0; j<lp->in; j++) {
			if (lp->colstat[j]==ALPS_BASIC) lp->reducedcost[j] = 0.0;
			else if (j<lp->inonartif) { /* nonbasis structural or slack */
				recoj = lp->reducedcost[j];
				if (recoj>ALPS_ZEROEPS) { /* wants to increase */
					if ((lp->colstat[j]==ALPS_NONBASICFREE)
						||((lp->colstat[j]==ALPS_NONBASICLOWB)&&(lp->varstat[j]!=ALPS_FIXED))) {
							k++;
							lp->recoheapp[k] = j;
							lp->recoheappindex[j] = k;
						}
				}
				if (recoj<-ALPS_ZEROEPS) { /* wants to decrease */
					if ((lp->colstat[j]==ALPS_NONBASICFREE)
						||((lp->colstat[j]==ALPS_NONBASICUPB)&&(lp->varstat[j]!=ALPS_FIXED))) {
							k++;
							lp->recoheapp[k] = j;
							lp->recoheappindex[j] = k;
						}
				}
			}
		}
		lp->recoheapcard = k;
		status = makerecoheap(lp);


	} 
	else {
		// Update reduced cost (primal)
		/*
		*  In order to update the reduced cost $r_{\rm new}$ from the reduced
		* cost $r_{\rm old}$, we first compute the quantity $\alpha = r_{\rm
		* old}[{\it enter}]/d[{\it leaveindex}]$. Then we can update $r^T_{\rm
		* new} = r^T_{\rm old}-\alpha z^TA_N$. In the primal version, we
		* maintain the entering candidate heap, in the dual version, this step
		* is omitted.
		*/

		if ((*leaveindex)>=0) {
			lp->reducedcost[(*enter)] = 0.0;
			alpha = (*redcost)/lp->d[(*leaveindex)];
			for (l=0; l<lp->zscount; l++) {
				i = lp->zsind[l];
				zi = lp->zsval[l];
				for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
					j = lp->imatrowind[k];
					if (lp->colstat[j]!=ALPS_BASIC) {
						rcdecrease = alpha*zi*lp->imatrowcoeff[k];
						oldrc = lp->reducedcost[j];
						newrc = oldrc - rcdecrease;
						absoldrc = fabs(oldrc);
						absnewrc = fabs(newrc);
						lp->reducedcost[j] = newrc;
						// Update the entering candidate heap for variable |j|

						if (j<lp->inonartif) {
							if (lp->x[j]<lp->iupperbound[j]-ALPS_ZEROEPS) { /* increaseable */
								if (lp->recoheappindex[j]>0) { /* was candidate */
									if (newrc>ALPS_ZEROEPS) { /* remains candidate */
										if (absnewrc>absoldrc+ALPS_ZEROEPS) {
											status = increaserecoheap(lp,j);
										}
										if (absnewrc<absoldrc-ALPS_ZEROEPS) {
											status = decreaserecoheap(lp,j);
										}
									}
									else {
										status = removerecoheap(lp,j);
									}
								}
								else { /* was no candidate */
									if (newrc>ALPS_ZEROEPS) {
										status = insertrecoheap(lp,j);
									}
								}
							}
							if (lp->x[j]>lp->ilowerbound[j]+ALPS_ZEROEPS) { /* decreaseable */
								if (lp->recoheappindex[j]>0) { /* was candidate */
									if (newrc<ALPS_ZEROEPS) { /* remains candidate */
										if (absnewrc>absoldrc+ALPS_ZEROEPS) {
											status = increaserecoheap(lp,j);
										}
										if (absnewrc<absoldrc-ALPS_ZEROEPS) {
											status = decreaserecoheap(lp,j);
										}
									}
									else {
										status = removerecoheap(lp,j);
									}
								}
								else { /* was no candidate */
									if (newrc<ALPS_ZEROEPS) {
										status = insertrecoheap(lp,j);
									}
								}
							}
						}


					}
				}
			}
			j = *leave;
#ifdef TTT
			oldrc = 0.0;
			absoldrc = fabs(oldrc);
			lp->reducedcost[j] = -alpha;
			if (lp->phase==ALPS_PHASE1)
				lp->reducedcost[j] -= (*oldobjcoeff);
			newrc = lp->reducedcost[j];
			absnewrc = fabs(newrc);
			// Update the entering candidate heap for variable |j|

			if (j<lp->inonartif) {
				if (lp->x[j]<lp->iupperbound[j]-ALPS_ZEROEPS) { /* increaseable */
					if (lp->recoheappindex[j]>0) { /* was candidate */
						if (newrc>ALPS_ZEROEPS) { /* remains candidate */
							if (absnewrc>absoldrc+ALPS_ZEROEPS) {
								status = increaserecoheap(lp,j);
							}
							if (absnewrc<absoldrc-ALPS_ZEROEPS) {
								status = decreaserecoheap(lp,j);
							}
						}
						else {
							status = removerecoheap(lp,j);
						}
					}
					else { /* was no candidate */
						if (newrc>ALPS_ZEROEPS) {
							status = insertrecoheap(lp,j);
						}
					}
				}
				if (lp->x[j]>lp->ilowerbound[j]+ALPS_ZEROEPS) { /* decreaseable */
					if (lp->recoheappindex[j]>0) { /* was candidate */
						if (newrc<ALPS_ZEROEPS) { /* remains candidate */
							if (absnewrc>absoldrc+ALPS_ZEROEPS) {
								status = increaserecoheap(lp,j);
							}
							if (absnewrc<absoldrc-ALPS_ZEROEPS) {
								status = decreaserecoheap(lp,j);
							}
						}
						else {
							status = removerecoheap(lp,j);
						}
					}
					else { /* was no candidate */
						if (newrc<ALPS_ZEROEPS) {
							status = insertrecoheap(lp,j);
						}
					}
				}
			}


#endif
			if (lp->colstat[j]==ALPS_NONBASICUPB) lp->reducedcost[j] = 1000.0;
			else lp->reducedcost[j] = -1000.0;
		}


	}
	if (pril>=3) {
		for (j=0; j<lp->in; j++) printf("  reco[%d] = %5.2lf\n",j,lp->reducedcost[j]);
	}
#endif
	// Compute reduced cost from scratch (primal)
	/*
	*  In order to compute the reduced cost from scratch, we set
	* |lp->reducedcost[j]| to zero for basic variables |j| and to
	* $c_j-y^TA_{.j}$ for nonbasic variables |j|. In the primal version, the
	* reduced cost heap is initialized, in the dual version, this is
	* omitted.
	*/
	for (j=0; j<lp->in; j++) lp->reducedcost[j] = lp->iobj[j];
	for (l=0; l<lp->yscount; l++) {
		i = lp->ysind[l];
		yi = lp->ysval[l];
		for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
			lp->reducedcost[lp->imatrowind[k]] -= yi*lp->imatrowcoeff[k];
		}
	}
	k = 0;
	for (j=0; j<lp->in; j++) lp->recoheappindex[j] = -1;
	for (j=0; j<lp->in; j++) {
		if (lp->colstat[j]==ALPS_BASIC) lp->reducedcost[j] = 0.0;
		else if (j<lp->inonartif) { /* nonbasis structural or slack */
			recoj = lp->reducedcost[j];
			if (recoj>ALPS_ZEROEPS) { /* wants to increase */
				if ((lp->colstat[j]==ALPS_NONBASICFREE)
					||((lp->colstat[j]==ALPS_NONBASICLOWB)&&(lp->varstat[j]!=ALPS_FIXED))) {
						k++;
						lp->recoheapp[k] = j;
						lp->recoheappindex[j] = k;
					}
			}
			if (recoj<-ALPS_ZEROEPS) { /* wants to decrease */
				if ((lp->colstat[j]==ALPS_NONBASICFREE)
					||((lp->colstat[j]==ALPS_NONBASICUPB)&&(lp->varstat[j]!=ALPS_FIXED))) {
						k++;
						lp->recoheapp[k] = j;
						lp->recoheappindex[j] = k;
					}
			}
		}
	}
	lp->recoheapcard = k;
	makerecoheap(lp);





	/** Find random entering variable (primal) */
	/*
	*  We take a random variable from the priority queue. If the priority
	* queue is empty, we have reached optimality.
	*/
	{
		if (lp->recoheapcard) {
			if (lp->recoheapcard==1) jpos = 1;
			else {
				jpos = 1 + ( (int) ( rand01(&randseed)* ((double)lp->recoheapcard-1) ) );
				if ((jpos<1)||(jpos>lp->recoheapcard)) printf("Random choice out of range!!!\n");
			}
			j = lp->recoheapp[jpos];
			recoj = lp->reducedcost[j];
			removerecoheap(lp,j);
		}
		else {
			lp->problemstatus = ALPS_OPTIMAL;
			goto freeall;
		}
	}


	/** Find leaving variable (primalphase1) */

	// Compute direction |dj| for variable |j|
	/*
	*  The direction $d$ is found by solving the system $A_Bd=a$, where $a$
	* is the candidate entering column |j|.
	*/
	for (l=0;l<lp->im;l++) entcol[l] = 0.0;
	for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) 
		entcol[lp->imatcolind[l]] = lp->imatcolcoeff[l];
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
		entcol,
		lp->d
		);

#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->fsolvetime += (stoptime - starttime);
	lp->piseltime -= (stoptime - starttime);
#endif
	dirjcount=0;
	for (l=0;l<lp->im;l++) {
		dl = lp->d[l];
		if ((dl<-ALPS_ZEROEPS)||(dl>ALPS_ZEROEPS)) { 
			lp->dsind[dirjcount] = l;
			lp->dsval[dirjcount] = dl;
			dirjcount++; 
		}
	}
	lp->dscount = dirjcount;


	// Compute change |changej| and |leavej| for |j| (phase1)
	/*
	*  The ratio test for phase 1. Here we have introduced an option to go
	* further as long as infeasibilities decrease.
	*/
	leavej = -1000;
	leavej1 = -1000;
	if (recoj>ALPS_ZEROEPS) {
		changej = ALPS_REAL_INFINITY;
		changej1 = ALPS_REAL_INFINITY;
		for(l=0;l<dirjcount;l++) {
			tt = ALPS_REAL_INFINITY;
			tt1 = ALPS_REAL_INFINITY;
			dsvall = lp->dsval[l];
			dsindl = lp->dsind[l];
			i = lp->basisheader[dsindl];

			iupperboundi = lp->iupperbound[i];
			ilowerboundi = lp->ilowerbound[i];
			iorigupperboundi = lp->iorigupperbound[i];
			ioriglowerboundi = lp->ioriglowerbound[i];
			varstati = lp->varstat[i];
			origvarstati = lp->origvarstat[i];

			if (varstati==ALPS_FIXED) tt = 0.0;
			if (dsvall<-ALPS_EPS) { /* wants to increase */
				if ((varstati==ALPS_BOUNDEDABOVE)||(varstati==ALPS_BOUNDEDTWICE)) {
					tt = (lp->x[i]-iupperboundi)/dsvall;
					if (lp->iartifbounds[i]) {
						if ((origvarstati==ALPS_BOUNDEDABOVE)
							||(origvarstati==ALPS_BOUNDEDTWICE)
							||(origvarstati==ALPS_FIXED))
							tt1 = (lp->x[i]-iorigupperboundi)/dsvall;
					}
					else tt1 = tt;
				}
			}
			if (dsvall>ALPS_EPS) { /* wants to decrease */
				if ((varstati==ALPS_BOUNDEDBELOW)||(varstati==ALPS_BOUNDEDTWICE)) {
					tt = (lp->x[i]-ilowerboundi)/dsvall;
					if (lp->iartifbounds[i]) {
						if ((origvarstati==ALPS_BOUNDEDBELOW)
							||(origvarstati==ALPS_BOUNDEDTWICE)
							||(origvarstati==ALPS_FIXED)) 
							tt1 = (lp->x[i]-ioriglowerboundi)/dsvall;
					}
					else tt1 = tt;
				}
			}

			if (tt<changej) {
				changej = tt;
				leavej = dsindl;
			}

			if (tt1<changej1) {
				changej1 = tt1;
				leavej1 = dsindl;
			}
		}
		if ((lp->varstat[j]==ALPS_BOUNDEDABOVE)||(lp->varstat[j]==ALPS_BOUNDEDTWICE)) {
			tt = lp->iupperbound[j]-lp->ilowerbound[j];
			if (tt<changej) {
				changej = tt;
				leavej = -1;
			}
			if (tt<changej1) {
				changej1 = tt;
				leavej1 = -1;
			}
		}
		if (changej<0.0) changej = 0.0;
		if (changej1<0.0) changej1 = 0.0;
	}
	else {
		changej = -ALPS_REAL_INFINITY;
		changej1 = -ALPS_REAL_INFINITY;
		for(l=0;l<dirjcount;l++) {
			tt = -ALPS_REAL_INFINITY;
			tt1 = -ALPS_REAL_INFINITY;
			dsvall = lp->dsval[l];
			dsindl = lp->dsind[l];
			i = lp->basisheader[dsindl];

			iupperboundi = lp->iupperbound[i];
			ilowerboundi = lp->ilowerbound[i];
			iorigupperboundi = lp->iorigupperbound[i];
			ioriglowerboundi = lp->ioriglowerbound[i];
			varstati = lp->varstat[i];
			origvarstati = lp->origvarstat[i];

			if (varstati==ALPS_FIXED) tt = 0.0;
			if (dsvall<-ALPS_EPS) { /* wants to decrease */
				if ((varstati==ALPS_BOUNDEDBELOW)||(varstati==ALPS_BOUNDEDTWICE)) {
					tt = (lp->x[i]-ilowerboundi)/dsvall;
					if (lp->iartifbounds[i]) {
						if ((origvarstati==ALPS_BOUNDEDBELOW)
							||(origvarstati==ALPS_BOUNDEDTWICE)
							||(origvarstati==ALPS_FIXED)) 
							tt1 = (lp->x[i]-ioriglowerboundi)/dsvall;   
					}
					else tt1 = tt;    
				} 
			}

			if (dsvall>ALPS_EPS) { /* wants to increase */
				if ((varstati==ALPS_BOUNDEDABOVE)||(varstati==ALPS_BOUNDEDTWICE)) {
					tt = (lp->x[i]-iupperboundi)/dsvall;
					if (lp->iartifbounds[i]) {
						if ((origvarstati==ALPS_BOUNDEDABOVE)
							||(origvarstati==ALPS_BOUNDEDTWICE)
							||(origvarstati==ALPS_FIXED)) 
							tt1 = (lp->x[i]-iorigupperboundi)/dsvall;
					}
					else tt1 = tt;   
				}  
			}

			if (tt>changej) {
				changej = tt;
				leavej = dsindl;
			}

			if (tt1>changej1) {
				changej1 = tt1;
				leavej1 = dsindl;
			}
		}
		if ((lp->varstat[j]==ALPS_BOUNDEDBELOW)||(lp->varstat[j]==ALPS_BOUNDEDTWICE)) {
			tt = lp->ilowerbound[j]-lp->iupperbound[j];
			if (tt>changej) {
				changej = tt;
				leavej = -1;
			}
			if (tt>changej1) {
				changej1 = tt;
				leavej1 = -1;
			}
		}
		if (changej>0.0) changej = 0.0;
		if (changej1>0.0) changej1 = 0.0;
	}
	*wentfurther = ALPS_FALSE;

#ifdef TTT
	if (changej1>changej) { 
		double simobjval,oldx,newx,dirvall; 
		int l,dirindl,basisvariable;
		// Basis update simulator (primal phase1)

		simobjval = 0.0;
		for(l=0;l<lp->dscount;l++) {
			dirindl = lp->dsind[l];
			dirvall = lp->dsval[l];
			basisvariable = lp->basisheader[dirindl];
			oldx = lp->x[basisvariable];
			newx = oldx - changej1*dirvall;
			simobjval -= (oldx-newx)*lp->iobj[basisvariable];
			if (lp->iartifbounds[basisvariable]) {
				if ((newx<=lp->iorigupperbound[basisvariable]+ALPS_ZEROEPS)&&
					(newx>=lp->ioriglowerbound[basisvariable]-ALPS_ZEROEPS)) {
						simobjval -= lp->iphase1obj[basisvariable]*lp->x[basisvariable];
					}
			}
		}
		oldx = lp->x[j];
		newx = oldx + changej1;
		simobjval -= (oldx-newx)*lp->iobj[j];


		if (simobjval>=0.001) {
			printf("changej = %lf, changej1 = %lf\n",changej,changej1);
			changej = changej1;
			leavej = leavej1;
			*wentfurther = ALPS_TRUE;
		}
	}

	oder so?
		if (changej1>changej && changej1<10000.0) { 
			if (pril>=2) printf("changej = %lf, changej1 = %lf\n",changej,changej1);
			changej = changej1;
			leavej = leavej1;
			*wentfurther = ALPS_TRUE;
		}
#endif

		if (leavej==-1000) {
			lp->problemstatus = ALPS_UNBOUNDED;
			goto freeall;
		}


		(*enter) = j;
		(*leaveindex) = leavej;
		if (leavej>=0) (*leave) = lp->basisheader[leavej];
		(*redcost) = recoj;
		(*change) = changej;


freeall:
		/** Free space for easypisel arrays */

		ffree( (char **) &entcol);

		return 0;
}

int alps_dualeasypisel(
					   lpstruct *lp,
					   int pril,
					   int *enter,
					   int *leave,
					   int *leaveindex,
					   double *redcost,
					   double *change,
					   int *pivrow,
					   int *pivcol,
					   double *pivvalue,
					   double * /* oldobjcoeff */
					   )

{
	/** Declare easypisel scalars */

	double yi;
	double zi;
	double zl;
	double alpha;
	double rcdecrease;
	double oldrc;
	double newrc;
	int status;
	int i,j,k,l;
	int starttime;
	int stoptime;
	double dl;
	int dirjcount;
	double maxviolation,violation;
	double wj,enterwj,ratio,minratio;
	int reason;


	/** Declare easypisel arrays */

	double *entcol;


	/** Allocate space for easypisel arrays */

	entcol = (double *) malloc( lp->im * sizeof(double) );
	if ( !entcol )
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}





	/** Find reduced cost (dual) */

	if (!lp->etanr) {
		// Compute reduced cost from scratch (dual)

		for (j=0; j<lp->in; j++) lp->reducedcost[j] = lp->iobj[j];
		for (l=0; l<lp->yscount; l++) {
			i = lp->ysind[l];
			yi = lp->ysval[l];
			for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
				lp->reducedcost[lp->imatrowind[k]] -= yi*lp->imatrowcoeff[k];
			}
		}
		for (j=0; j<lp->in; j++) {
			if (lp->colstat[j]==ALPS_BASIC) lp->reducedcost[j] = 0.0;
		}


	} 
	else {
		// Update reduced cost (dual)


		lp->reducedcost[(*enter)] = 0.0;
		alpha = (*redcost)/lp->d[(*leaveindex)];
		for (l=0; l<lp->zscount; l++) {
			i = lp->zsind[l];
			zi = lp->zsval[l];
			for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
				j = lp->imatrowind[k];
				if (lp->colstat[j]!=ALPS_BASIC) {
					rcdecrease = alpha*zi*lp->imatrowcoeff[k];
					oldrc = lp->reducedcost[j];
					newrc = oldrc - rcdecrease;
					lp->reducedcost[j] = newrc;
				}
			}
		}
		j = *leave;
		lp->reducedcost[j] = -alpha;


	}
	if (pril>=3) {
		for (j=0; j<lp->in; j++) printf("  reco[%d] = %5.2lf\n",j,lp->reducedcost[j]);
	}
	for (j=0; j<lp->in; j++) {
		if ((lp->colstat[j]==ALPS_NONBASICLOWB)&&(lp->reducedcost[j]>ALPS_ZEROEPS))
			printf("\n!!! Basis is not dual feasible !!!\n\n");
		if ((lp->colstat[j]==ALPS_NONBASICUPB)&&(lp->reducedcost[j]<-ALPS_ZEROEPS))
			printf("\n!!! Basis is not dual feasible !!!\n\n");
	}


	/** Find leaving variable (dual) */
	/*
	*  The leaving variable is one with maximum bound violation.
	*/
	maxviolation = ALPS_ZEROEPS;
	for (k=0; k<lp->im; k++) {
		i = lp->basisheader[k];
		violation = lp->ilowerbound[i] - lp->x[i]; 
		if (violation>maxviolation) {
			maxviolation = violation;
			(*leaveindex) = k;
			(*leave) = i;
			reason = ALPS_LOWERBOUNDVIOLATION;
		}
		else {
			violation = lp->x[i] - lp->iupperbound[i]; 
			if (violation>maxviolation) {
				maxviolation = violation;
				(*leaveindex) = k;
				(*leave) = i;
				reason = ALPS_UPPERBOUNDVIOLATION;
			}
		}
	}
	if (maxviolation<=ALPS_ZEROEPS) {
		lp->problemstatus = ALPS_OPTIMAL;
		goto freeall;
	}



	/** Find entering variable (dual) */
	/*
	*  First, we compute $z$ and $w$. Then, depending on the type of
	* violation of the leaving variable, we perform the tests for finding
	* the entering variable.
	*/
	// Compute |z|
	/*
	*  We compute $z$ by solving $z^TA_B=u^T$ for the unit vector $u$ with a
	* $1$ in the |leaveindex| position.
	*/
	lp->u[*leaveindex] = 1.0;
	starttime = cputime();
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

	stoptime = cputime();
	lp->bsolvetime += (stoptime - starttime);
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
	if (pril>=3) {
		printf("\nu: "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->u[i]);printf("\n");
		printf("z: "); for (i=0;i<lp->im;i++) printf(" %.2lf",lp->z[i]);printf("\n");
	}

	lp->u[*leaveindex] = 0.0;


	// Compute |w|
	/*
	*  We compute $w^T = z^TA_N$.
	*/
	for (j=0; j<lp->in; j++) lp->w[j] = 0.0;
	for (l=0; l<lp->zscount; l++) {
		i = lp->zsind[l];
		zi = lp->zsval[l];
		for (k=lp->imatrowbeg[i]; k<lp->imatrowbeg[i]+lp->imatrowcount[i]; k++) {
			j = lp->imatrowind[k];
			if (lp->colstat[j]!=ALPS_BASIC) {
				lp->w[j] += zi*lp->imatrowcoeff[k];
			}
		}
	}
	k = 0;
	for (j=0;j<lp->in;j++) {
		wj = lp->w[j];
		if ((wj<-ALPS_ZEROEPS)||(wj>ALPS_ZEROEPS)) {
			lp->wsind[k] = j;
			lp->wsval[k] = wj;
			k++;
		}
	}
	lp->wscount = k;



	minratio = ALPS_REAL_INFINITY;
	(*enter) = -1;
	if (reason==ALPS_LOWERBOUNDVIOLATION) {
		for (l=0; l<lp->wscount; l++) {
			j = lp->wsind[l];
			if (lp->colstat[j]!=ALPS_BASIC) {
				wj = lp->wsval[l];
				if (wj<-ALPS_ZEROEPS) {
					if (lp->x[j] < lp->iupperbound[j]-ALPS_ZEROEPS) {
						ratio = lp->reducedcost[j]/wj;
						if (ratio<0.0) ratio = -ratio;
						if (ratio<minratio) {
							minratio = ratio;
							(*enter) = j;
							enterwj = wj;
						}
					}
				}
				else if (wj>ALPS_ZEROEPS) {
					if (lp->x[j] > lp->ilowerbound[j]+ALPS_ZEROEPS) {
						ratio = lp->reducedcost[j]/wj;
						if (ratio<0.0) ratio = -ratio;
						if (ratio<minratio) {
							minratio = ratio;
							(*enter) = j;
							enterwj = wj;
						}
					}
				}
			}
		}
	}
	else {
		for (l=0; l<lp->wscount; l++) {
			j = lp->wsind[l];
			if (lp->colstat[j]!=ALPS_BASIC) {
				wj = lp->wsval[l];
				if (wj<-ALPS_ZEROEPS) {
					if (lp->x[j] > lp->ilowerbound[j]+ALPS_ZEROEPS) {
						ratio = lp->reducedcost[j]/wj;
						if (ratio<0.0) ratio = -ratio;
						if (ratio<minratio) {
							minratio = ratio;
							(*enter) = j;
							enterwj = wj;
						}
					}
				}
				else if (wj>ALPS_ZEROEPS) {
					if (lp->x[j] < lp->iupperbound[j]-ALPS_ZEROEPS) {
						ratio = lp->reducedcost[j]/wj;
						if (ratio<0.0) ratio = -ratio;
						if (ratio<minratio) {
							minratio = ratio;
							(*enter) = j;
							enterwj = wj;
						}
					}
				}
			}
		}
	}
	if ((*enter)==-1) {
		lp->problemstatus = ALPS_INFEASIBLE;
		goto freeall;
	}
	j = (*enter);
	(*redcost) = lp->reducedcost[j];
	// Compute direction |dj| for variable |j|
	/*
	*  The direction $d$ is found by solving the system $A_Bd=a$, where $a$
	* is the candidate entering column |j|.
	*/
	for (l=0;l<lp->im;l++) entcol[l] = 0.0;
	for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) 
		entcol[lp->imatcolind[l]] = lp->imatcolcoeff[l];
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
		entcol,
		lp->d
		);

#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->fsolvetime += (stoptime - starttime);
	lp->piseltime -= (stoptime - starttime);
#endif
	dirjcount=0;
	for (l=0;l<lp->im;l++) {
		dl = lp->d[l];
		if ((dl<-ALPS_ZEROEPS)||(dl>ALPS_ZEROEPS)) { 
			lp->dsind[dirjcount] = l;
			lp->dsval[dirjcount] = dl;
			dirjcount++; 
		}
	}
	lp->dscount = dirjcount;


	if (reason==ALPS_LOWERBOUNDVIOLATION) (*change) = -maxviolation/enterwj;
	else                             (*change) =  maxviolation/enterwj;


freeall:
	/** Free space for easypisel arrays */

	ffree( (char **) &entcol);

	return 0;
}

} // end namespace ogdf
