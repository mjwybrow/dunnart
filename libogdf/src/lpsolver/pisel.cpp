/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-11-27 03:02:28 +1100 (Thu, 27 Nov 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Pivot selection for the primal simplex algorithm.
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
*  The function |pisel| implements our heuristic for pivot selection in
* the primal simplex algorithm.
*/


#include "alps.h"
#include "alps_misc.h"

namespace ogdf {

int pisel(
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
		  double * /*oldobjcoeff*/
		  )

{
	/** Declare pisel scalars */

	int numconsider;
	int status;
	int j_is_candidate;
	int i,j,k,l;
	int mindegen;
	int jmindegen;
	int jmindegenleave;
	int degenj;
	int leavej;
	int jmaxobjchange;
	int jmaxobjchangeleave;
	int jmaxabsreco;
	int jmaxabssteep;
	int starttime;
	int stoptime;
	double jmindegenchange;
	double jmindegenreco;
	double jmaxabsrecoreco;
	double jmaxabssteepreco;
	double jmaxobjchangereco;
	double jmaxobjchangechange;
	double maxobjchange;
	double maxabsreco;
	double maxabssteep;
	double steepj;
	double changej;
	double objchangej;
	double ya;
	double tt;
	double recoj;
	double dirjl;
	double dirjsvall;
	double ilowerboundi;
	double iupperboundi;
	int dirjscount;
	int dirjsindl;


	/** Declare pisel arrays */

	double *dirj;
	double *dirjsval;
	int *dirjsind;
	double *entcol;
	double *reco;



	/** Allocate space for pisel arrays */

	dirj = (double *) malloc( lp->im * sizeof(double) );
	dirjsval = (double *) malloc( lp->im * sizeof(double) );
	dirjsind = (int *) malloc( lp->im * sizeof(int) );
	entcol = (double *) malloc( lp->im * sizeof(double) );
	reco = (double *) malloc( lp->maxconsider * sizeof(double) );
	if (!dirj || !entcol ||!reco)
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}





	/** Compute set of entering candidates */

	k = 0;
	numconsider = 0;
	while ((k<lp->in)&&(numconsider<lp->maxconsider)) {
		j = lp->cand[k];
		if (lp->colstat[j]!=ALPS_BASIC) {
			// Compute reduced cost |recoj| for variable |j|

			j_is_candidate = ALPS_FALSE;
			ya = 0.0;
			for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) 
				ya += lp->imatcolcoeff[l]*lp->y[lp->imatcolind[l]];
			recoj = lp->iobj[j]-ya;
			if (pril) {
				printf("  reco[%d] = %5.2lf\n",j,recoj);
			}
			switch (lp->colstat[j]) {
  case ALPS_NONBASICLOWB:
	  if ((lp->x[j]<lp->iupperbound[j]-ALPS_ZEROEPS)&&(recoj>ALPS_ZEROEPS)) {
		  j_is_candidate = ALPS_TRUE;
	  }
	  break;
  case ALPS_NONBASICUPB:
	  if ((lp->x[j]>lp->ilowerbound[j]+ALPS_ZEROEPS)&&(recoj<-ALPS_ZEROEPS)) {
		  j_is_candidate = ALPS_TRUE;
	  }
	  break;
  case ALPS_NONBASICFREE:
	  if ((recoj<-ALPS_ZEROEPS)||(recoj>ALPS_ZEROEPS)) {
		  j_is_candidate = ALPS_TRUE;
	  }
	  break;
			}


			if (j_is_candidate) {
				lp->cand[k] = lp->cand[numconsider];
				lp->cand[numconsider] = j;
				reco[numconsider] = recoj;
				numconsider++;
			}
		}
		k++;
	}


	/** Select entering and leaving variables */

	if (!numconsider) lp->problemstatus = ALPS_OPTIMAL;
	else {
		maxobjchange = -ALPS_REAL_INFINITY;
		mindegen = ALPS_INT_INFINITY;
		maxabsreco = -ALPS_REAL_INFINITY;
		maxabssteep = -ALPS_REAL_INFINITY;
		for (k=0; k<numconsider; k++) {
			j = lp->cand[k];
			recoj = reco[k];
			switch (lp->howtopisel) {
	  case ALPS_EASYPISEL:
		  if (fabs(recoj)>maxabsreco) {
			  maxabsreco = fabs(recoj);
			  jmaxabsreco = j;
			  jmaxabsrecoreco = recoj;
			  if (j>lp->an && lp->phase==ALPS_PHASE1) goto fastpivot;
		  }
		  break;
	  case ALPS_STEEPPISEL:
		  // Compute direction |dirj| for variable |j|
		  /*
		  *  The direction $d$, here |dirj|, is found by solving the system
		  * $Bd=a$, where $a$ is the candidate entering column |j|.
		  */
		  for (l=0;l<lp->im;l++) entcol[l] = 0.0;
		  for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) 
			  entcol[lp->imatcolind[l]] = lp->imatcolcoeff[l];
		  starttime = cputime();
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
			  dirj
			  );
		  stoptime = cputime();
		  lp->fsolvetime += (stoptime - starttime);
		  lp->piseltime -= (stoptime - starttime);
		  dirjscount=0;
		  for (l=0;l<lp->im;l++) {
			  dirjl = dirj[l];
			  if ((dirjl<-ALPS_ZEROEPS)||(dirjl>ALPS_ZEROEPS)) {
				  dirjsind[dirjscount] = l;
				  dirjsval[dirjscount] = dirjl;
				  dirjscount++;
			  }
		  }
#ifdef TTT
		  { double dmin = 10000000.0; double dmax = -1000000.0;
		  for (l=0;l<dirjscount;l++) {
			  if (dirjsval[l]<dmin) dmin = dirjsval[l];
			  if (dirjsval[l]>dmax) dmax = dirjsval[l];
		  }
		  printf("  d[%.2le,%.2le] ",dmin,dmax);
		  }
#endif


		  steepj = 1.0;
		  for (l=0; l<dirjscount; l++) {
			  dirjsvall = dirjsval[l];
			  steepj += dirjsvall*dirjsvall;
		  }
		  steepj = recoj*recoj / steepj;
		  if (steepj>maxabssteep) {
			  maxabssteep = steepj;
			  jmaxabssteep = j;
			  jmaxabssteepreco = recoj;
			  lp->dscount = dirjscount;
			  for (l=0; l<dirjscount; l++) {
				  lp->dsval[l] = dirjsval[l]; 
				  lp->dsind[l] = dirjsind[l]; 
			  }
		  }
		  break;
	  case ALPS_DEGENERATEPISEL:
		  // Compute direction |dirj| for variable |j|
		  /*
		  *  The direction $d$, here |dirj|, is found by solving the system
		  * $Bd=a$, where $a$ is the candidate entering column |j|.
		  */
		  for (l=0;l<lp->im;l++) entcol[l] = 0.0;
		  for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) 
			  entcol[lp->imatcolind[l]] = lp->imatcolcoeff[l];
		  starttime = cputime();
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
			  dirj
			  );
		  stoptime = cputime();
		  lp->fsolvetime += (stoptime - starttime);
		  lp->piseltime -= (stoptime - starttime);
		  dirjscount=0;
		  for (l=0;l<lp->im;l++) {
			  dirjl = dirj[l];
			  if ((dirjl<-ALPS_ZEROEPS)||(dirjl>ALPS_ZEROEPS)) {
				  dirjsind[dirjscount] = l;
				  dirjsval[dirjscount] = dirjl;
				  dirjscount++;
			  }
		  }
#ifdef TTT
		  { double dmin = 10000000.0; double dmax = -1000000.0;
		  for (l=0;l<dirjscount;l++) {
			  if (dirjsval[l]<dmin) dmin = dirjsval[l];
			  if (dirjsval[l]>dmax) dmax = dirjsval[l];
		  }
		  printf("  d[%.2le,%.2le] ",dmin,dmax);
		  }
#endif


		  // Compute change |changej|, degeneracy |degenj|, and |leavej| for |j|
		  /*
		  *  The ratio test in its current form is is a messy case distinction,
		  * this should eventually be somewhat untangeled.
		  */
		  changej = ALPS_REAL_INFINITY;
		  leavej = -1000;
		  degenj = 0;
		  switch (lp->colstat[j]) {
	  case ALPS_NONBASICLOWB:
		  for(l=0;l<dirjscount;l++) {
			  tt = ALPS_REAL_INFINITY;
			  dirjsvall = dirjsval[l];
			  dirjsindl = dirjsind[l];
			  i = lp->basisheader[dirjsindl];
			  iupperboundi = lp->iupperbound[i];
			  ilowerboundi = lp->ilowerbound[i];
			  if ((dirjsvall<-ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY))
				  tt = (lp->x[i]-iupperboundi)/dirjsvall;
			  if ((dirjsvall>ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY))
				  tt = (lp->x[i]-ilowerboundi)/dirjsvall;
			  if (tt<changej) {
				  changej = tt;
				  leavej = dirjsindl;
				  degenj = 0;
			  }
			  else {
				  if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
			  }
		  }
		  break;
	  case ALPS_NONBASICUPB:
		  for(l=0;l<dirjscount;l++) {
			  tt = ALPS_REAL_INFINITY;
			  dirjsvall = dirjsval[l];
			  dirjsindl = dirjsind[l];
			  i = lp->basisheader[dirjsindl];
			  iupperboundi = lp->iupperbound[i];
			  ilowerboundi = lp->ilowerbound[i];
			  if ((dirjsvall<-ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY)) 
				  tt = (ilowerboundi-lp->x[i])/dirjsvall;
			  if ((dirjsvall>ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY)) 
				  tt = (iupperboundi-lp->x[i])/dirjsvall;
			  if (tt<changej) {
				  changej = tt;
				  leavej = dirjsindl;
				  degenj = 0;
			  }
			  else {
				  if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
			  }
		  }
		  break;
	  case ALPS_NONBASICFREE:
		  if (recoj<-ALPS_ZEROEPS) {
			  for(l=0;l<dirjscount;l++) {
				  tt = ALPS_REAL_INFINITY;
				  dirjsvall = dirjsval[l];
				  dirjsindl = dirjsind[l];
				  i = lp->basisheader[dirjsindl];
				  iupperboundi = lp->iupperbound[i];
				  ilowerboundi = lp->ilowerbound[i];
				  if ((dirjsvall<-ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY)) 
					  tt = (ilowerboundi-lp->x[i])/dirjsvall;
				  if ((dirjsvall>ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY)) 
					  tt = (iupperboundi-lp->x[i])/dirjsvall;
				  if (tt<changej) {
					  changej = tt;
					  leavej = dirjsindl;
					  degenj = 0;
				  }
				  else {
					  if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
				  }
			  }
		  }
		  else {
			  for(l=0;l<dirjscount;l++) {
				  tt = ALPS_REAL_INFINITY;
				  dirjsvall = dirjsval[l];
				  dirjsindl = dirjsind[l];
				  i = lp->basisheader[dirjsindl];
				  iupperboundi = lp->iupperbound[i];
				  ilowerboundi = lp->ilowerbound[i];
				  if ((dirjsvall<-ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY))
					  tt = (lp->x[i]-iupperboundi)/dirjsvall;
				  if ((dirjsvall>ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY))
					  tt = (lp->x[i]-ilowerboundi)/dirjsvall;
				  if (tt<changej) {
					  changej = tt;
					  leavej = dirjsindl;
					  degenj = 0;
				  }
				  else {
					  if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
				  }
			  }
		  }
		  break;
		  }
		  if ((lp->iupperbound[j]<ALPS_REAL_INFINITY)&&(lp->ilowerbound[j]>-ALPS_REAL_INFINITY)) {
			  tt = lp->iupperbound[j]-lp->ilowerbound[j];
			  if (tt<changej) {
				  changej = tt;
				  leavej = -1;
				  degenj = 0;
			  }
			  else {
				  if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
			  }
		  }
		  if (leavej==-1000) {
			  lp->problemstatus = ALPS_UNBOUNDED;
			  goto freeall;
		  }


		  if (degenj<mindegen) {
			  mindegen = degenj;
			  jmindegen = j;
			  jmindegenreco = recoj;
			  jmindegenleave = leavej;
			  jmindegenchange = changej;
			  lp->dscount = dirjscount;
			  for (l=0; l<dirjscount; l++) {
				  lp->dsval[l] = dirjsval[l]; 
				  lp->dsind[l] = dirjsind[l]; 
			  }
		  }
		  break;
	  case ALPS_GREATCHANGEPISEL:
		  // Compute direction |dirj| for variable |j|
		  /*
		  *  The direction $d$, here |dirj|, is found by solving the system
		  * $Bd=a$, where $a$ is the candidate entering column |j|.
		  */
		  for (l=0;l<lp->im;l++) entcol[l] = 0.0;
		  for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) 
			  entcol[lp->imatcolind[l]] = lp->imatcolcoeff[l];
		  starttime = cputime();
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
			  dirj
			  );
		  stoptime = cputime();
		  lp->fsolvetime += (stoptime - starttime);
		  lp->piseltime -= (stoptime - starttime);
		  dirjscount=0;
		  for (l=0;l<lp->im;l++) {
			  dirjl = dirj[l];
			  if ((dirjl<-ALPS_ZEROEPS)||(dirjl>ALPS_ZEROEPS)) {
				  dirjsind[dirjscount] = l;
				  dirjsval[dirjscount] = dirjl;
				  dirjscount++;
			  }
		  }
#ifdef TTT
		  { double dmin = 10000000.0; double dmax = -1000000.0;
		  for (l=0;l<dirjscount;l++) {
			  if (dirjsval[l]<dmin) dmin = dirjsval[l];
			  if (dirjsval[l]>dmax) dmax = dirjsval[l];
		  }
		  printf("  d[%.2le,%.2le] ",dmin,dmax);
		  }
#endif


		  // Compute change |changej|, degeneracy |degenj|, and |leavej| for |j|
		  /*
		  *  The ratio test in its current form is is a messy case distinction,
		  * this should eventually be somewhat untangeled.
		  */
		  changej = ALPS_REAL_INFINITY;
		  leavej = -1000;
		  degenj = 0;
		  switch (lp->colstat[j]) {
	  case ALPS_NONBASICLOWB:
		  for(l=0;l<dirjscount;l++) {
			  tt = ALPS_REAL_INFINITY;
			  dirjsvall = dirjsval[l];
			  dirjsindl = dirjsind[l];
			  i = lp->basisheader[dirjsindl];
			  iupperboundi = lp->iupperbound[i];
			  ilowerboundi = lp->ilowerbound[i];
			  if ((dirjsvall<-ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY))
				  tt = (lp->x[i]-iupperboundi)/dirjsvall;
			  if ((dirjsvall>ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY))
				  tt = (lp->x[i]-ilowerboundi)/dirjsvall;
			  if (tt<changej) {
				  changej = tt;
				  leavej = dirjsindl;
				  degenj = 0;
			  }
			  else {
				  if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
			  }
		  }
		  break;
	  case ALPS_NONBASICUPB:
		  for(l=0;l<dirjscount;l++) {
			  tt = ALPS_REAL_INFINITY;
			  dirjsvall = dirjsval[l];
			  dirjsindl = dirjsind[l];
			  i = lp->basisheader[dirjsindl];
			  iupperboundi = lp->iupperbound[i];
			  ilowerboundi = lp->ilowerbound[i];
			  if ((dirjsvall<-ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY)) 
				  tt = (ilowerboundi-lp->x[i])/dirjsvall;
			  if ((dirjsvall>ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY)) 
				  tt = (iupperboundi-lp->x[i])/dirjsvall;
			  if (tt<changej) {
				  changej = tt;
				  leavej = dirjsindl;
				  degenj = 0;
			  }
			  else {
				  if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
			  }
		  }
		  break;
	  case ALPS_NONBASICFREE:
		  if (recoj<-ALPS_ZEROEPS) {
			  for(l=0;l<dirjscount;l++) {
				  tt = ALPS_REAL_INFINITY;
				  dirjsvall = dirjsval[l];
				  dirjsindl = dirjsind[l];
				  i = lp->basisheader[dirjsindl];
				  iupperboundi = lp->iupperbound[i];
				  ilowerboundi = lp->ilowerbound[i];
				  if ((dirjsvall<-ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY)) 
					  tt = (ilowerboundi-lp->x[i])/dirjsvall;
				  if ((dirjsvall>ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY)) 
					  tt = (iupperboundi-lp->x[i])/dirjsvall;
				  if (tt<changej) {
					  changej = tt;
					  leavej = dirjsindl;
					  degenj = 0;
				  }
				  else {
					  if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
				  }
			  }
		  }
		  else {
			  for(l=0;l<dirjscount;l++) {
				  tt = ALPS_REAL_INFINITY;
				  dirjsvall = dirjsval[l];
				  dirjsindl = dirjsind[l];
				  i = lp->basisheader[dirjsindl];
				  iupperboundi = lp->iupperbound[i];
				  ilowerboundi = lp->ilowerbound[i];
				  if ((dirjsvall<-ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY))
					  tt = (lp->x[i]-iupperboundi)/dirjsvall;
				  if ((dirjsvall>ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY))
					  tt = (lp->x[i]-ilowerboundi)/dirjsvall;
				  if (tt<changej) {
					  changej = tt;
					  leavej = dirjsindl;
					  degenj = 0;
				  }
				  else {
					  if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
				  }
			  }
		  }
		  break;
		  }
		  if ((lp->iupperbound[j]<ALPS_REAL_INFINITY)&&(lp->ilowerbound[j]>-ALPS_REAL_INFINITY)) {
			  tt = lp->iupperbound[j]-lp->ilowerbound[j];
			  if (tt<changej) {
				  changej = tt;
				  leavej = -1;
				  degenj = 0;
			  }
			  else {
				  if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
			  }
		  }
		  if (leavej==-1000) {
			  lp->problemstatus = ALPS_UNBOUNDED;
			  goto freeall;
		  }


		  // Compute objective function increase |objchangej| if variable |j| enters
		  /*
		  *  Here we compute the change in the objective function value if the
		  * possible pivot with |j| entering and |lp->basisheader[leavej]| leaving
		  * is carried out. Depending on the nonbasis status of the entering
		  * variable, different cases are considered in order to determine the
		  * resulting changes.
		  */
		  objchangej = 0.0;
		  switch (lp->colstat[j]) {
	  case ALPS_NONBASICLOWB:
		  for(l=0;l<dirjscount;l++) {
			  dirjsvall = dirjsval[l];
			  objchangej -= changej*dirjsvall*lp->iobj[lp->basisheader[dirjsind[l]]];
		  }
		  objchangej += changej*lp->iobj[j];
		  break;
	  case ALPS_NONBASICUPB:
		  for(l=0;l<dirjscount;l++) {
			  dirjsvall = dirjsval[l];
			  objchangej += changej*dirjsvall*lp->iobj[lp->basisheader[dirjsind[l]]];
		  }
		  objchangej -= changej*lp->iobj[j];
		  break;
	  case ALPS_NONBASICFREE:
		  if (recoj<-ALPS_ZEROEPS) {
			  for(l=0;l<dirjscount;l++) {
				  dirjsvall = dirjsval[l];
				  objchangej += changej*dirjsvall*lp->iobj[lp->basisheader[dirjsind[l]]];
			  }
			  objchangej -= changej*lp->iobj[j];
		  }
		  else {
			  for(l=0;l<dirjscount;l++) {
				  dirjsvall = dirjsval[l];
				  objchangej -= changej*dirjsvall*lp->iobj[lp->basisheader[dirjsind[l]]];
			  }
			  objchangej += changej*lp->iobj[j];
		  }
		  break;
		  }


		  if (objchangej>maxobjchange) {
			  maxobjchange = objchangej;
			  jmaxobjchange = j;
			  jmaxobjchangereco = recoj;
			  jmaxobjchangeleave = leavej;
			  jmaxobjchangechange = changej;
			  lp->dscount = dirjscount;
			  for (l=0; l<dirjscount; l++) {
				  lp->dsval[l] = dirjsval[l]; 
				  lp->dsind[l] = dirjsind[l]; 
			  }
		  }
		  break;
			}
		}
fastpivot:
		switch (lp->howtopisel) {
	case ALPS_EASYPISEL:
		j = jmaxabsreco;
		recoj = jmaxabsrecoreco;
		// Compute direction |dirj| for variable |j|
		/*
		*  The direction $d$, here |dirj|, is found by solving the system
		* $Bd=a$, where $a$ is the candidate entering column |j|.
		*/
		for (l=0;l<lp->im;l++) entcol[l] = 0.0;
		for (l=lp->imatcolbeg[j];l<lp->imatcolbeg[j]+lp->imatcolcount[j];l++) 
			entcol[lp->imatcolind[l]] = lp->imatcolcoeff[l];
		starttime = cputime();
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
			dirj
			);
		stoptime = cputime();
		lp->fsolvetime += (stoptime - starttime);
		lp->piseltime -= (stoptime - starttime);
		dirjscount=0;
		for (l=0;l<lp->im;l++) {
			dirjl = dirj[l];
			if ((dirjl<-ALPS_ZEROEPS)||(dirjl>ALPS_ZEROEPS)) {
				dirjsind[dirjscount] = l;
				dirjsval[dirjscount] = dirjl;
				dirjscount++;
			}
		}
#ifdef TTT
		{ double dmin = 10000000.0; double dmax = -1000000.0;
		for (l=0;l<dirjscount;l++) {
			if (dirjsval[l]<dmin) dmin = dirjsval[l];
			if (dirjsval[l]>dmax) dmax = dirjsval[l];
		}
		printf("  d[%.2le,%.2le] ",dmin,dmax);
		}
#endif


		lp->dscount = dirjscount;
		for (l=0; l<dirjscount; l++) {
			lp->dsval[l] = dirjsval[l]; 
			lp->dsind[l] = dirjsind[l]; 
		}
		// Compute change |changej|, degeneracy |degenj|, and |leavej| for |j|
		/*
		*  The ratio test in its current form is is a messy case distinction,
		* this should eventually be somewhat untangeled.
		*/
		changej = ALPS_REAL_INFINITY;
		leavej = -1000;
		degenj = 0;
		switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		for(l=0;l<dirjscount;l++) {
			tt = ALPS_REAL_INFINITY;
			dirjsvall = dirjsval[l];
			dirjsindl = dirjsind[l];
			i = lp->basisheader[dirjsindl];
			iupperboundi = lp->iupperbound[i];
			ilowerboundi = lp->ilowerbound[i];
			if ((dirjsvall<-ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY))
				tt = (lp->x[i]-iupperboundi)/dirjsvall;
			if ((dirjsvall>ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY))
				tt = (lp->x[i]-ilowerboundi)/dirjsvall;
			if (tt<changej) {
				changej = tt;
				leavej = dirjsindl;
				degenj = 0;
			}
			else {
				if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
			}
		}
		break;
	case ALPS_NONBASICUPB:
		for(l=0;l<dirjscount;l++) {
			tt = ALPS_REAL_INFINITY;
			dirjsvall = dirjsval[l];
			dirjsindl = dirjsind[l];
			i = lp->basisheader[dirjsindl];
			iupperboundi = lp->iupperbound[i];
			ilowerboundi = lp->ilowerbound[i];
			if ((dirjsvall<-ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY)) 
				tt = (ilowerboundi-lp->x[i])/dirjsvall;
			if ((dirjsvall>ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY)) 
				tt = (iupperboundi-lp->x[i])/dirjsvall;
			if (tt<changej) {
				changej = tt;
				leavej = dirjsindl;
				degenj = 0;
			}
			else {
				if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
			}
		}
		break;
	case ALPS_NONBASICFREE:
		if (recoj<-ALPS_ZEROEPS) {
			for(l=0;l<dirjscount;l++) {
				tt = ALPS_REAL_INFINITY;
				dirjsvall = dirjsval[l];
				dirjsindl = dirjsind[l];
				i = lp->basisheader[dirjsindl];
				iupperboundi = lp->iupperbound[i];
				ilowerboundi = lp->ilowerbound[i];
				if ((dirjsvall<-ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY)) 
					tt = (ilowerboundi-lp->x[i])/dirjsvall;
				if ((dirjsvall>ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY)) 
					tt = (iupperboundi-lp->x[i])/dirjsvall;
				if (tt<changej) {
					changej = tt;
					leavej = dirjsindl;
					degenj = 0;
				}
				else {
					if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
				}
			}
		}
		else {
			for(l=0;l<dirjscount;l++) {
				tt = ALPS_REAL_INFINITY;
				dirjsvall = dirjsval[l];
				dirjsindl = dirjsind[l];
				i = lp->basisheader[dirjsindl];
				iupperboundi = lp->iupperbound[i];
				ilowerboundi = lp->ilowerbound[i];
				if ((dirjsvall<-ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY))
					tt = (lp->x[i]-iupperboundi)/dirjsvall;
				if ((dirjsvall>ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY))
					tt = (lp->x[i]-ilowerboundi)/dirjsvall;
				if (tt<changej) {
					changej = tt;
					leavej = dirjsindl;
					degenj = 0;
				}
				else {
					if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
				}
			}
		}
		break;
		}
		if ((lp->iupperbound[j]<ALPS_REAL_INFINITY)&&(lp->ilowerbound[j]>-ALPS_REAL_INFINITY)) {
			tt = lp->iupperbound[j]-lp->ilowerbound[j];
			if (tt<changej) {
				changej = tt;
				leavej = -1;
				degenj = 0;
			}
			else {
				if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
			}
		}
		if (leavej==-1000) {
			lp->problemstatus = ALPS_UNBOUNDED;
			goto freeall;
		}


		break;
	case ALPS_STEEPPISEL:
		j = jmaxabssteep;
		recoj = jmaxabssteepreco;
		dirjscount = lp->dscount;
		for (l=0;l<dirjscount;l++) {
			dirjsind[l] = lp->dsind[l];
			dirjsval[l] = lp->dsval[l];
		}
		// Compute change |changej|, degeneracy |degenj|, and |leavej| for |j|
		/*
		*  The ratio test in its current form is is a messy case distinction,
		* this should eventually be somewhat untangeled.
		*/
		changej = ALPS_REAL_INFINITY;
		leavej = -1000;
		degenj = 0;
		switch (lp->colstat[j]) {
	case ALPS_NONBASICLOWB:
		for(l=0;l<dirjscount;l++) {
			tt = ALPS_REAL_INFINITY;
			dirjsvall = dirjsval[l];
			dirjsindl = dirjsind[l];
			i = lp->basisheader[dirjsindl];
			iupperboundi = lp->iupperbound[i];
			ilowerboundi = lp->ilowerbound[i];
			if ((dirjsvall<-ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY))
				tt = (lp->x[i]-iupperboundi)/dirjsvall;
			if ((dirjsvall>ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY))
				tt = (lp->x[i]-ilowerboundi)/dirjsvall;
			if (tt<changej) {
				changej = tt;
				leavej = dirjsindl;
				degenj = 0;
			}
			else {
				if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
			}
		}
		break;
	case ALPS_NONBASICUPB:
		for(l=0;l<dirjscount;l++) {
			tt = ALPS_REAL_INFINITY;
			dirjsvall = dirjsval[l];
			dirjsindl = dirjsind[l];
			i = lp->basisheader[dirjsindl];
			iupperboundi = lp->iupperbound[i];
			ilowerboundi = lp->ilowerbound[i];
			if ((dirjsvall<-ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY)) 
				tt = (ilowerboundi-lp->x[i])/dirjsvall;
			if ((dirjsvall>ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY)) 
				tt = (iupperboundi-lp->x[i])/dirjsvall;
			if (tt<changej) {
				changej = tt;
				leavej = dirjsindl;
				degenj = 0;
			}
			else {
				if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
			}
		}
		break;
	case ALPS_NONBASICFREE:
		if (recoj<-ALPS_ZEROEPS) {
			for(l=0;l<dirjscount;l++) {
				tt = ALPS_REAL_INFINITY;
				dirjsvall = dirjsval[l];
				dirjsindl = dirjsind[l];
				i = lp->basisheader[dirjsindl];
				iupperboundi = lp->iupperbound[i];
				ilowerboundi = lp->ilowerbound[i];
				if ((dirjsvall<-ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY)) 
					tt = (ilowerboundi-lp->x[i])/dirjsvall;
				if ((dirjsvall>ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY)) 
					tt = (iupperboundi-lp->x[i])/dirjsvall;
				if (tt<changej) {
					changej = tt;
					leavej = dirjsindl;
					degenj = 0;
				}
				else {
					if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
				}
			}
		}
		else {
			for(l=0;l<dirjscount;l++) {
				tt = ALPS_REAL_INFINITY;
				dirjsvall = dirjsval[l];
				dirjsindl = dirjsind[l];
				i = lp->basisheader[dirjsindl];
				iupperboundi = lp->iupperbound[i];
				ilowerboundi = lp->ilowerbound[i];
				if ((dirjsvall<-ALPS_EPS)&&(iupperboundi<ALPS_REAL_INFINITY))
					tt = (lp->x[i]-iupperboundi)/dirjsvall;
				if ((dirjsvall>ALPS_EPS)&&(ilowerboundi>-ALPS_REAL_INFINITY))
					tt = (lp->x[i]-ilowerboundi)/dirjsvall;
				if (tt<changej) {
					changej = tt;
					leavej = dirjsindl;
					degenj = 0;
				}
				else {
					if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
				}
			}
		}
		break;
		}
		if ((lp->iupperbound[j]<ALPS_REAL_INFINITY)&&(lp->ilowerbound[j]>-ALPS_REAL_INFINITY)) {
			tt = lp->iupperbound[j]-lp->ilowerbound[j];
			if (tt<changej) {
				changej = tt;
				leavej = -1;
				degenj = 0;
			}
			else {
				if (((tt-changej)<=ALPS_EPS)&&((tt-changej)>=-ALPS_EPS)) degenj++;
			}
		}
		if (leavej==-1000) {
			lp->problemstatus = ALPS_UNBOUNDED;
			goto freeall;
		}


		break;
	case ALPS_DEGENERATEPISEL:
		j = jmindegen;
		recoj = jmindegenreco;
		leavej = jmindegenleave;
		changej = jmindegenchange;
		break;
	case ALPS_GREATCHANGEPISEL:
		j = jmaxobjchange;
		recoj = jmaxobjchangereco;
		leavej = jmaxobjchangeleave;
		changej = jmaxobjchangechange;
		break;
		}
		(*enter) = j;
		(*leaveindex) = leavej;
		if ((*leaveindex)>=0) (*leave) = lp->basisheader[(*leaveindex)];
		(*redcost) = recoj;
		(*change) = changej;
		for (l=0;l<lp->im;l++) lp->d[l] = 0.0;
		for (l=0;l<lp->dscount;l++) lp->d[lp->dsind[l]] = lp->dsval[l];
	}



freeall:
	/** Free space for pisel arrays */

	ffree( (char **) &dirj);
	ffree( (char **) &dirjsval);
	ffree( (char **) &dirjsind);
	ffree( (char **) &entcol);
	ffree( (char **) &reco);


	return 0;
}

} // end namespace ogdf
