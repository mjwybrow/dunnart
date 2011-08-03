/*
 * $Revision: 1.2 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Scaling.
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
// Scaling. Scaling is done on the internal problem. Note, that scaling
// factors are computed using only data of this internal problem. There
// are various scaling options. At present it is not clear which one
// performs best. The layout of this routine is as follows. 
//

#include "alps.h"
#include "alps_preproc.h"
#include "alps_misc.h"
#include "math.h"
#include "alps_transport.h"

#define DEBUG 1
#define CHECK 1

#define SCALE_EPS (10.0*ALPS_ZEROEPS)

namespace ogdf {

int alps_scaling(
				 int pril,
				 lpstruct *lp
				 )

{
	/** Declare scaling arrays */

	double *saverowscale, *savecolscale;
	double *sccolmin, *sccolmax;
	double *scrowmin, *scrowmax;


	/** Declare scaling scalars */

	double rho,rhonew,ff;
	double maxrange,tmp;
	int i,j,k;
	int maxi;


	/** Declare scaling structs */

	BASIS *basis;

	lp->rowscale = (double *) malloc( lp->im * sizeof(double) );
	lp->colscale = (double *) malloc( lp->inorig * sizeof(double) );
	sccolmin = (double *) malloc( lp->inorig * sizeof(double) );
	sccolmax = (double *) malloc( lp->inorig * sizeof(double) );
	scrowmin = (double *) malloc( lp->im * sizeof(double) );
	scrowmax = (double *) malloc( lp->im * sizeof(double) );
	if (!lp->rowscale || !lp->colscale 
		|| !sccolmin || !sccolmax || !scrowmin || !scrowmax )
		/** run out of memory */
	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	switch (lp->scaling) {
  case 1:
  case 6:
	  /** Compute equilibration scaling factors */
	  /*
	  *  Equilibration scaling. The following is a simple row and column
	  * scaling in order that the maximum absolute value of a row or column
	  * coefficient is equal to 1.
	  */
	  { double ff;

	  for (i=0;i<lp->im;i++) lp->rowscale[i] = 0.0;
	  for (j=0;j<lp->inorig;j++) lp->colscale[j] = 0.0;

	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  i = lp->imatcolind[k];
			  ff = fabs(lp->imatcolcoeff[k]);
			  if (ff>lp->rowscale[i]) lp->rowscale[i] = ff;
		  }
	  }
#ifdef CHECK
	  for (i=0;i<lp->im;i++) {
		  if ( (fabs(lp->rowscale[i])<=SCALE_EPS) 
			  || (fabs(lp->rowscale[i])>=1.0/SCALE_EPS) )
			  lp->rowscale[i] = 1.0;
	  }
#endif
	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  ff = lp->imatcolcoeff[k] / lp->rowscale[lp->imatcolind[k]];
			  if (fabs(ff)>lp->colscale[j])
				  lp->colscale[j] = fabs(ff);
		  }
#ifdef CHECK
		  if ( (fabs(lp->colscale[j])<=SCALE_EPS)
			  || (fabs(lp->colscale[j])>=1.0/SCALE_EPS) )
			  lp->colscale[j] = 1.0;
#endif
	  }

	  if (pril>=1) printf("Problem has been scaled (Equilibration scaling).\n");
	  }


	  /** Scale the problem */

	  { int i,j,k,cancel;
	  cancel = ALPS_FALSE;
	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  i = lp->imatcolind[k];
			  lp->imatcolcoeff[k] /= lp->rowscale[i];
			  lp->imatcolcoeff[k] /= lp->colscale[j];
			  if (fabs(lp->imatcolcoeff[k])<ALPS_ZEROEPS) cancel = ALPS_TRUE;
		  }
	  }
	  for (i=0;i<lp->im;i++) lp->irhs[i] /= lp->rowscale[i];
	  for (j=0;j<lp->inorig;j++) {
		  if (lp->ilowerbound[j]>-ALPS_INT_INFINITY) {
			  lp->ilowerbound[j] *= lp->colscale[j];
			  if (lp->ilowerbound[j]<-ALPS_INT_INFINITY) lp->ilowerbound[j] = -ALPS_INT_INFINITY;
			  if (fabs(lp->ilowerbound[j])<ALPS_ZEROEPS)
				  lp->ilowerbound[j] = 0.0;
		  }
	  }
	  for (j=0;j<lp->inorig;j++) {
		  if (lp->iupperbound[j]<ALPS_INT_INFINITY) {
			  lp->iupperbound[j] *= lp->colscale[j];
			  if (lp->iupperbound[j]>ALPS_INT_INFINITY) lp->iupperbound[j] = ALPS_INT_INFINITY;
			  if (fabs(lp->iupperbound[j])<ALPS_ZEROEPS)
				  lp->iupperbound[j] = 0.0;
		  }
	  }
	  for (j=0;j<lp->inorig;j++) lp->iphase2obj[j] /= lp->colscale[j];
	  if (cancel) printf("WARNING: matrix elements may be cancelled!\n");
	  }


	  break;
  case 2:
  case 7:
	  /** Compute geometric scaling factors */
	  /*
	  *  Geometric scaling. The following is a more advanced scaling procedure
	  * according to Tomlin (1975) and Fourer (1982).
	  */
	  for (j=0;j<lp->inorig;j++) {
		  lp->colscale[j] = 1.0;
		  sccolmin[j] = ALPS_REAL_INFINITY;
		  sccolmax[j] = 0.0;
	  }
	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  ff = fabs(lp->imatcolcoeff[k]);
			  if (ff>sccolmax[j]) sccolmax[j] = ff;
			  if (ff<sccolmin[j]) sccolmin[j] = ff;
		  }
#ifdef CHECK
		  if (sccolmax[j]<SCALE_EPS || sccolmax[j]>1.0/SCALE_EPS) {
			  sccolmax[j] = 1.0;
			  sccolmin[j] = 1.0;
		  }
		  if (sccolmin[j]<SCALE_EPS || sccolmin[j]>1.0/SCALE_EPS) {
			  sccolmax[j] = 1.0;
			  sccolmin[j] = 1.0;
		  }
#endif
	  }

	  rhonew = 0.0;
	  for (j=0;j<lp->inorig;j++) {
		  if (sccolmax[j]/sccolmin[j]>rhonew) rhonew = sccolmax[j]/sccolmin[j];
	  }
	  rho = ALPS_REAL_INFINITY;

	  while (rhonew<ALPS_SCALINGCONDITION*rho) {
		  rho = rhonew;

		  for (i=0;i<lp->im;i++) {
			  scrowmin[i] = ALPS_REAL_INFINITY;
			  scrowmax[i] = 0.0;
		  }
		  for (j=0;j<lp->inorig;j++) {
			  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				  i = lp->imatcolind[k];
				  ff = fabs(lp->imatcolcoeff[k] / lp->colscale[j]);
				  if (ff>scrowmax[i]) scrowmax[i] = ff;
				  if (ff<scrowmin[i]) scrowmin[i] = ff;
			  }
		  }
		  for (i=0;i<lp->im;i++) {
#ifdef CHECK
			  if (scrowmax[i]<SCALE_EPS || scrowmax[i]>1.0/SCALE_EPS) {
				  scrowmax[i] = 1.0;
				  scrowmin[i] = 1.0;
			  }
			  if (scrowmin[i]<SCALE_EPS || scrowmin[i]>1.0/SCALE_EPS) {
				  scrowmax[i] = 1.0;
				  scrowmin[i] = 1.0;
			  }
#endif
			  ff = (double) sqrt(scrowmax[i]*scrowmin[i]);
			  lp->rowscale[i] = ff;
#ifdef CHECK
			  if ( (fabs(ff)<SCALE_EPS) || (fabs(ff)>1.0/SCALE_EPS) ) 
				  lp->rowscale[i] = 1.0;
#endif
		  }

		  for (j=0;j<lp->inorig;j++) {
			  sccolmin[j] = ALPS_REAL_INFINITY;
			  sccolmax[j] = 0.0;
		  }
		  for (j=0;j<lp->inorig;j++) {
			  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				  i = lp->imatcolind[k];
				  ff = fabs(lp->imatcolcoeff[k] / lp->rowscale[i]);
				  if (ff>sccolmax[j]) sccolmax[j] = ff;
				  if (ff<sccolmin[j]) sccolmin[j] = ff;
			  }
#ifdef CHECK
			  if (sccolmax[j]<SCALE_EPS || sccolmax[j]>1.0/SCALE_EPS) {
				  sccolmax[j] = 1.0;
				  sccolmin[j] = 1.0;
			  }
			  if (sccolmin[j]<SCALE_EPS || sccolmin[j]>1.0/SCALE_EPS) {
				  sccolmax[j] = 1.0;
				  sccolmin[j] = 1.0;
			  }
#endif
		  }
		  for (j=0;j<lp->inorig;j++) {
			  ff = (double) sqrt(sccolmax[j]*sccolmin[j]);
			  if ( (fabs(ff)<SCALE_EPS) || (fabs(ff)>1.0/SCALE_EPS) ) 
				  lp->colscale[j] = 1.0;
			  else lp->colscale[j] = ff;
		  }

		  rhonew = 0.0;
		  for (j=0;j<lp->inorig;j++) {
			  if (sccolmax[j]/sccolmin[j]>rhonew) rhonew = sccolmax[j]/sccolmin[j];
		  }

		  if (pril>=4) printf("Scaling condition %.4lf -> %.4lf\n",rho,rhonew);
	  }

	  if (pril>=1) printf("Problem has been scaled (Geometric scaling).\n");


	  /** Scale the problem */

	  { int i,j,k,cancel;
	  cancel = ALPS_FALSE;
	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  i = lp->imatcolind[k];
			  lp->imatcolcoeff[k] /= lp->rowscale[i];
			  lp->imatcolcoeff[k] /= lp->colscale[j];
			  if (fabs(lp->imatcolcoeff[k])<ALPS_ZEROEPS) cancel = ALPS_TRUE;
		  }
	  }
	  for (i=0;i<lp->im;i++) lp->irhs[i] /= lp->rowscale[i];
	  for (j=0;j<lp->inorig;j++) {
		  if (lp->ilowerbound[j]>-ALPS_INT_INFINITY) {
			  lp->ilowerbound[j] *= lp->colscale[j];
			  if (lp->ilowerbound[j]<-ALPS_INT_INFINITY) lp->ilowerbound[j] = -ALPS_INT_INFINITY;
			  if (fabs(lp->ilowerbound[j])<ALPS_ZEROEPS)
				  lp->ilowerbound[j] = 0.0;
		  }
	  }
	  for (j=0;j<lp->inorig;j++) {
		  if (lp->iupperbound[j]<ALPS_INT_INFINITY) {
			  lp->iupperbound[j] *= lp->colscale[j];
			  if (lp->iupperbound[j]>ALPS_INT_INFINITY) lp->iupperbound[j] = ALPS_INT_INFINITY;
			  if (fabs(lp->iupperbound[j])<ALPS_ZEROEPS)
				  lp->iupperbound[j] = 0.0;
		  }
	  }
	  for (j=0;j<lp->inorig;j++) lp->iphase2obj[j] /= lp->colscale[j];
	  if (cancel) printf("WARNING: matrix elements may be cancelled!\n");
	  }


	  break;
  case 3:
  case 8:
	  saverowscale = (double *) malloc( lp->im * sizeof(double) );
	  savecolscale = (double *) malloc( lp->inorig * sizeof(double) );
	  /** Compute geometric scaling factors */
	  /*
	  *  Geometric scaling. The following is a more advanced scaling procedure
	  * according to Tomlin (1975) and Fourer (1982).
	  */
	  for (j=0;j<lp->inorig;j++) {
		  lp->colscale[j] = 1.0;
		  sccolmin[j] = ALPS_REAL_INFINITY;
		  sccolmax[j] = 0.0;
	  }
	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  ff = fabs(lp->imatcolcoeff[k]);
			  if (ff>sccolmax[j]) sccolmax[j] = ff;
			  if (ff<sccolmin[j]) sccolmin[j] = ff;
		  }
#ifdef CHECK
		  if (sccolmax[j]<SCALE_EPS || sccolmax[j]>1.0/SCALE_EPS) {
			  sccolmax[j] = 1.0;
			  sccolmin[j] = 1.0;
		  }
		  if (sccolmin[j]<SCALE_EPS || sccolmin[j]>1.0/SCALE_EPS) {
			  sccolmax[j] = 1.0;
			  sccolmin[j] = 1.0;
		  }
#endif
	  }

	  rhonew = 0.0;
	  for (j=0;j<lp->inorig;j++) {
		  if (sccolmax[j]/sccolmin[j]>rhonew) rhonew = sccolmax[j]/sccolmin[j];
	  }
	  rho = ALPS_REAL_INFINITY;

	  while (rhonew<ALPS_SCALINGCONDITION*rho) {
		  rho = rhonew;

		  for (i=0;i<lp->im;i++) {
			  scrowmin[i] = ALPS_REAL_INFINITY;
			  scrowmax[i] = 0.0;
		  }
		  for (j=0;j<lp->inorig;j++) {
			  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				  i = lp->imatcolind[k];
				  ff = fabs(lp->imatcolcoeff[k] / lp->colscale[j]);
				  if (ff>scrowmax[i]) scrowmax[i] = ff;
				  if (ff<scrowmin[i]) scrowmin[i] = ff;
			  }
		  }
		  for (i=0;i<lp->im;i++) {
#ifdef CHECK
			  if (scrowmax[i]<SCALE_EPS || scrowmax[i]>1.0/SCALE_EPS) {
				  scrowmax[i] = 1.0;
				  scrowmin[i] = 1.0;
			  }
			  if (scrowmin[i]<SCALE_EPS || scrowmin[i]>1.0/SCALE_EPS) {
				  scrowmax[i] = 1.0;
				  scrowmin[i] = 1.0;
			  }
#endif
			  ff = (double) sqrt(scrowmax[i]*scrowmin[i]);
			  lp->rowscale[i] = ff;
#ifdef CHECK
			  if ( (fabs(ff)<SCALE_EPS) || (fabs(ff)>1.0/SCALE_EPS) ) 
				  lp->rowscale[i] = 1.0;
#endif
		  }

		  for (j=0;j<lp->inorig;j++) {
			  sccolmin[j] = ALPS_REAL_INFINITY;
			  sccolmax[j] = 0.0;
		  }
		  for (j=0;j<lp->inorig;j++) {
			  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
				  i = lp->imatcolind[k];
				  ff = fabs(lp->imatcolcoeff[k] / lp->rowscale[i]);
				  if (ff>sccolmax[j]) sccolmax[j] = ff;
				  if (ff<sccolmin[j]) sccolmin[j] = ff;
			  }
#ifdef CHECK
			  if (sccolmax[j]<SCALE_EPS || sccolmax[j]>1.0/SCALE_EPS) {
				  sccolmax[j] = 1.0;
				  sccolmin[j] = 1.0;
			  }
			  if (sccolmin[j]<SCALE_EPS || sccolmin[j]>1.0/SCALE_EPS) {
				  sccolmax[j] = 1.0;
				  sccolmin[j] = 1.0;
			  }
#endif
		  }
		  for (j=0;j<lp->inorig;j++) {
			  ff = (double) sqrt(sccolmax[j]*sccolmin[j]);
			  if ( (fabs(ff)<SCALE_EPS) || (fabs(ff)>1.0/SCALE_EPS) ) 
				  lp->colscale[j] = 1.0;
			  else lp->colscale[j] = ff;
		  }

		  rhonew = 0.0;
		  for (j=0;j<lp->inorig;j++) {
			  if (sccolmax[j]/sccolmin[j]>rhonew) rhonew = sccolmax[j]/sccolmin[j];
		  }

		  if (pril>=4) printf("Scaling condition %.4lf -> %.4lf\n",rho,rhonew);
	  }

	  if (pril>=1) printf("Problem has been scaled (Geometric scaling).\n");


	  /** Perturbation of scale factors */
	  /*
	  *  Scale the problem. We now scale the problem using the computed row
	  * and column factors. It turned out that a random perturbation of the
	  * scaling factors before performing the scaling sometimes helps.
	  */
	  { 
		  int randseed,i,j;
		  double ff;
		  randseed = 12345;
		  for (j=0;j<lp->inorig;j++) {
			  ff = lp->colscale[j] * (1.0 + 0.0002*(rand01(&randseed)-0.5));
			  if (ff>0.0) lp->colscale[j] = ff;
		  }
		  for (i=0;i<lp->im;i++) {
			  ff = lp->rowscale[i] * (1.0 + 0.0002*(rand01(&randseed)-0.5));
			  if (ff>0.0) lp->rowscale[i] = ff;
		  }
		  if (pril>=1) printf("Random perturbation of scaling factors.\n");
	  }


	  /** Scale the problem */

	  { int i,j,k,cancel;
	  cancel = ALPS_FALSE;
	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  i = lp->imatcolind[k];
			  lp->imatcolcoeff[k] /= lp->rowscale[i];
			  lp->imatcolcoeff[k] /= lp->colscale[j];
			  if (fabs(lp->imatcolcoeff[k])<ALPS_ZEROEPS) cancel = ALPS_TRUE;
		  }
	  }
	  for (i=0;i<lp->im;i++) lp->irhs[i] /= lp->rowscale[i];
	  for (j=0;j<lp->inorig;j++) {
		  if (lp->ilowerbound[j]>-ALPS_INT_INFINITY) {
			  lp->ilowerbound[j] *= lp->colscale[j];
			  if (lp->ilowerbound[j]<-ALPS_INT_INFINITY) lp->ilowerbound[j] = -ALPS_INT_INFINITY;
			  if (fabs(lp->ilowerbound[j])<ALPS_ZEROEPS)
				  lp->ilowerbound[j] = 0.0;
		  }
	  }
	  for (j=0;j<lp->inorig;j++) {
		  if (lp->iupperbound[j]<ALPS_INT_INFINITY) {
			  lp->iupperbound[j] *= lp->colscale[j];
			  if (lp->iupperbound[j]>ALPS_INT_INFINITY) lp->iupperbound[j] = ALPS_INT_INFINITY;
			  if (fabs(lp->iupperbound[j])<ALPS_ZEROEPS)
				  lp->iupperbound[j] = 0.0;
		  }
	  }
	  for (j=0;j<lp->inorig;j++) lp->iphase2obj[j] /= lp->colscale[j];
	  if (cancel) printf("WARNING: matrix elements may be cancelled!\n");
	  }


	  for (i=0;i<lp->im;i++) saverowscale[i] = lp->rowscale[i];
	  for (j=0;j<lp->inorig;j++) savecolscale[j] = lp->colscale[j];
	  /** Compute equilibration scaling factors */
	  /*
	  *  Equilibration scaling. The following is a simple row and column
	  * scaling in order that the maximum absolute value of a row or column
	  * coefficient is equal to 1.
	  */
	  { double ff;

	  for (i=0;i<lp->im;i++) lp->rowscale[i] = 0.0;
	  for (j=0;j<lp->inorig;j++) lp->colscale[j] = 0.0;

	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  i = lp->imatcolind[k];
			  ff = fabs(lp->imatcolcoeff[k]);
			  if (ff>lp->rowscale[i]) lp->rowscale[i] = ff;
		  }
	  }
#ifdef CHECK
	  for (i=0;i<lp->im;i++) {
		  if ( (fabs(lp->rowscale[i])<=SCALE_EPS) 
			  || (fabs(lp->rowscale[i])>=1.0/SCALE_EPS) )
			  lp->rowscale[i] = 1.0;
	  }
#endif
	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  ff = lp->imatcolcoeff[k] / lp->rowscale[lp->imatcolind[k]];
			  if (fabs(ff)>lp->colscale[j])
				  lp->colscale[j] = fabs(ff);
		  }
#ifdef CHECK
		  if ( (fabs(lp->colscale[j])<=SCALE_EPS)
			  || (fabs(lp->colscale[j])>=1.0/SCALE_EPS) )
			  lp->colscale[j] = 1.0;
#endif
	  }

	  if (pril>=1) printf("Problem has been scaled (Equilibration scaling).\n");
	  }


	  /** Scale the problem */

	  { int i,j,k,cancel;
	  cancel = ALPS_FALSE;
	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  i = lp->imatcolind[k];
			  lp->imatcolcoeff[k] /= lp->rowscale[i];
			  lp->imatcolcoeff[k] /= lp->colscale[j];
			  if (fabs(lp->imatcolcoeff[k])<ALPS_ZEROEPS) cancel = ALPS_TRUE;
		  }
	  }
	  for (i=0;i<lp->im;i++) lp->irhs[i] /= lp->rowscale[i];
	  for (j=0;j<lp->inorig;j++) {
		  if (lp->ilowerbound[j]>-ALPS_INT_INFINITY) {
			  lp->ilowerbound[j] *= lp->colscale[j];
			  if (lp->ilowerbound[j]<-ALPS_INT_INFINITY) lp->ilowerbound[j] = -ALPS_INT_INFINITY;
			  if (fabs(lp->ilowerbound[j])<ALPS_ZEROEPS)
				  lp->ilowerbound[j] = 0.0;
		  }
	  }
	  for (j=0;j<lp->inorig;j++) {
		  if (lp->iupperbound[j]<ALPS_INT_INFINITY) {
			  lp->iupperbound[j] *= lp->colscale[j];
			  if (lp->iupperbound[j]>ALPS_INT_INFINITY) lp->iupperbound[j] = ALPS_INT_INFINITY;
			  if (fabs(lp->iupperbound[j])<ALPS_ZEROEPS)
				  lp->iupperbound[j] = 0.0;
		  }
	  }
	  for (j=0;j<lp->inorig;j++) lp->iphase2obj[j] /= lp->colscale[j];
	  if (cancel) printf("WARNING: matrix elements may be cancelled!\n");
	  }


	  for (i=0;i<lp->im;i++) lp->rowscale[i] *= saverowscale[i];
	  for (j=0;j<lp->inorig;j++) lp->colscale[j] *= savecolscale[j];
	  ffree( (char **) &saverowscale);
	  ffree( (char **) &savecolscale);
	  break;
  case 4:
  case 9:
	  /** Compute optimal scale factors */
	  /*
	  *  Optimal scaling. The following is a scaling procedure using an
	  * algorithm for transportation problems.
	  */

	  basis = (BASIS *) malloc(sizeof(BASIS));
	  if (!lp->preprocessing) {
		  basis->n = lp->an;
		  basis->m = lp->am;
	  }
	  else {
		  basis->n = lp->inorig;
		  basis->m = lp->im;
	  }
	  basis->colbeg = lp->imatcolbeg;
	  basis->colcount = lp->imatcolcount;
	  basis->colind = lp->imatcolind;
	  if (!lp->preprocessing) {
		  basis->no_arcs = lp->imatcolbeg[lp->an - 1]+lp->imatcolcount[lp->an - 1];
	  }
	  else {
		  basis->no_arcs = lp->imatcolbeg[lp->inorig-1]+lp->imatcolcount[lp->inorig-1];
	  }
	  basis->cost = (double *) malloc( (basis->no_arcs+1) * sizeof(double) );
	  basis->demand = (int *) malloc ( (basis->n + basis->m) *sizeof(int));
	  if(!basis->cost || !basis->demand) 
		  // run out of memory

	  {
		  fprintf (stderr, "run out of memory\n");
		  return(ALPS_RUNOUTOFMEMORY);
	  }




	  for(k=0;k<basis->m;k++) basis->demand[k]=0;

	  basis->maxcost=-ALPS_REAL_INFINITY;
	  for (j=0;j<basis->n;j++) {
		  basis->demand[basis->m+j]=basis->colcount[j];
		  for (k=basis->colbeg[j];k<basis->colbeg[j]+basis->colcount[j];k++) {
			  i = basis->colind[k];
			  basis->demand[i]-=1;
			  basis->cost[k]=tmp=-log(fabs(lp->imatcolcoeff[k]));
			  if (fabs(tmp)>basis->maxcost) basis->maxcost=fabs(tmp);
		  } 
	  }      

	  optimize_transport(basis);  

	  if (pril>=4) {
		  printf("some statistics of the transportation problem:\n");
		  printf("        bigM : %20.10lf\n",basis->bigM);
		  printf("# arcs       : %d\n",basis->no_arcs);
		  printf("# art. arcs  : %d\n",basis->no_artificial);
		  printf("# pivots     : %d\n",basis->pivot);
		  printf("# deg. pivots: %d\n",basis->degpivot);
		  printf("     rerooted: %d\n",basis->rerooted);
	  }
	  free(basis->pred);
	  free(basis->brother);
	  free(basis->son);
	  free(basis->family);
	  free(basis->reverse);
	  free(basis->upward);
	  free(basis->flow);
	  free(basis->demand);
	  free(basis->cost);

	  if (!lp->preprocessing) {
		  for(i=0;i<lp->am;i++) lp->rowscale[i]=exp(-basis->dual[i]);
		  for(j=0;j<lp->an;j++) lp->colscale[j]=exp(basis->dual[lp->am+j]);
	  }
	  else {
		  for(i=0;i<lp->im;i++) lp->rowscale[i]=exp(-basis->dual[i]);
		  for(j=0;j<lp->inorig;j++) lp->colscale[j]=exp(basis->dual[lp->im+j]);
	  }
	  free(basis->dual);
	  free(basis);

	  if (pril>=1) printf("Problem has been scaled (Optimal scaling).\n");


	  /** Scale the problem */

	  { int i,j,k,cancel;
	  cancel = ALPS_FALSE;
	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  i = lp->imatcolind[k];
			  lp->imatcolcoeff[k] /= lp->rowscale[i];
			  lp->imatcolcoeff[k] /= lp->colscale[j];
			  if (fabs(lp->imatcolcoeff[k])<ALPS_ZEROEPS) cancel = ALPS_TRUE;
		  }
	  }
	  for (i=0;i<lp->im;i++) lp->irhs[i] /= lp->rowscale[i];
	  for (j=0;j<lp->inorig;j++) {
		  if (lp->ilowerbound[j]>-ALPS_INT_INFINITY) {
			  lp->ilowerbound[j] *= lp->colscale[j];
			  if (lp->ilowerbound[j]<-ALPS_INT_INFINITY) lp->ilowerbound[j] = -ALPS_INT_INFINITY;
			  if (fabs(lp->ilowerbound[j])<ALPS_ZEROEPS)
				  lp->ilowerbound[j] = 0.0;
		  }
	  }
	  for (j=0;j<lp->inorig;j++) {
		  if (lp->iupperbound[j]<ALPS_INT_INFINITY) {
			  lp->iupperbound[j] *= lp->colscale[j];
			  if (lp->iupperbound[j]>ALPS_INT_INFINITY) lp->iupperbound[j] = ALPS_INT_INFINITY;
			  if (fabs(lp->iupperbound[j])<ALPS_ZEROEPS)
				  lp->iupperbound[j] = 0.0;
		  }
	  }
	  for (j=0;j<lp->inorig;j++) lp->iphase2obj[j] /= lp->colscale[j];
	  if (cancel) printf("WARNING: matrix elements may be cancelled!\n");
	  }


	  break;
  case 5:
  case 10:
	  /** Compute equilibration scaling factors */
	  /*
	  *  Equilibration scaling. The following is a simple row and column
	  * scaling in order that the maximum absolute value of a row or column
	  * coefficient is equal to 1.
	  */
	  { double ff;

	  for (i=0;i<lp->im;i++) lp->rowscale[i] = 0.0;
	  for (j=0;j<lp->inorig;j++) lp->colscale[j] = 0.0;

	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  i = lp->imatcolind[k];
			  ff = fabs(lp->imatcolcoeff[k]);
			  if (ff>lp->rowscale[i]) lp->rowscale[i] = ff;
		  }
	  }
#ifdef CHECK
	  for (i=0;i<lp->im;i++) {
		  if ( (fabs(lp->rowscale[i])<=SCALE_EPS) 
			  || (fabs(lp->rowscale[i])>=1.0/SCALE_EPS) )
			  lp->rowscale[i] = 1.0;
	  }
#endif
	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  ff = lp->imatcolcoeff[k] / lp->rowscale[lp->imatcolind[k]];
			  if (fabs(ff)>lp->colscale[j])
				  lp->colscale[j] = fabs(ff);
		  }
#ifdef CHECK
		  if ( (fabs(lp->colscale[j])<=SCALE_EPS)
			  || (fabs(lp->colscale[j])>=1.0/SCALE_EPS) )
			  lp->colscale[j] = 1.0;
#endif
	  }

	  if (pril>=1) printf("Problem has been scaled (Equilibration scaling).\n");
	  }


	  /** Perturbation of scale factors */
	  /*
	  *  Scale the problem. We now scale the problem using the computed row
	  * and column factors. It turned out that a random perturbation of the
	  * scaling factors before performing the scaling sometimes helps.
	  */
	  { 
		  int randseed,i,j;
		  double ff;
		  randseed = 12345;
		  for (j=0;j<lp->inorig;j++) {
			  ff = lp->colscale[j] * (1.0 + 0.0002*(rand01(&randseed)-0.5));
			  if (ff>0.0) lp->colscale[j] = ff;
		  }
		  for (i=0;i<lp->im;i++) {
			  ff = lp->rowscale[i] * (1.0 + 0.0002*(rand01(&randseed)-0.5));
			  if (ff>0.0) lp->rowscale[i] = ff;
		  }
		  if (pril>=1) printf("Random perturbation of scaling factors.\n");
	  }


	  /** Scale the problem */

	  { int i,j,k,cancel;
	  cancel = ALPS_FALSE;
	  for (j=0;j<lp->inorig;j++) {
		  for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			  i = lp->imatcolind[k];
			  lp->imatcolcoeff[k] /= lp->rowscale[i];
			  lp->imatcolcoeff[k] /= lp->colscale[j];
			  if (fabs(lp->imatcolcoeff[k])<ALPS_ZEROEPS) cancel = ALPS_TRUE;
		  }
	  }
	  for (i=0;i<lp->im;i++) lp->irhs[i] /= lp->rowscale[i];
	  for (j=0;j<lp->inorig;j++) {
		  if (lp->ilowerbound[j]>-ALPS_INT_INFINITY) {
			  lp->ilowerbound[j] *= lp->colscale[j];
			  if (lp->ilowerbound[j]<-ALPS_INT_INFINITY) lp->ilowerbound[j] = -ALPS_INT_INFINITY;
			  if (fabs(lp->ilowerbound[j])<ALPS_ZEROEPS)
				  lp->ilowerbound[j] = 0.0;
		  }
	  }
	  for (j=0;j<lp->inorig;j++) {
		  if (lp->iupperbound[j]<ALPS_INT_INFINITY) {
			  lp->iupperbound[j] *= lp->colscale[j];
			  if (lp->iupperbound[j]>ALPS_INT_INFINITY) lp->iupperbound[j] = ALPS_INT_INFINITY;
			  if (fabs(lp->iupperbound[j])<ALPS_ZEROEPS)
				  lp->iupperbound[j] = 0.0;
		  }
	  }
	  for (j=0;j<lp->inorig;j++) lp->iphase2obj[j] /= lp->colscale[j];
	  if (cancel) printf("WARNING: matrix elements may be cancelled!\n");
	  }


	  break;
  default:
	  break;
	}
	/** Compute row and column ranges */
	/*
	*  For debugging purposes, we can obtain some information about the
	* scaled problem.
	*/
#ifdef DEBUG
	if (pril>=4) {
		printf("\nRow scale factors:\n");
		for (i=0;i<lp->im;i++) printf(" %.2lf",lp->rowscale[i]);
		printf("\n");
		printf("\nColumn scale factors:\n");
		for (j=0;j<lp->inorig;j++) printf(" %.2lf",lp->colscale[j]);
		printf("\n");
	}

	for (j=0;j<lp->inorig;j++) {
		sccolmin[j] =  ALPS_REAL_INFINITY;
		sccolmax[j] = -ALPS_REAL_INFINITY;
	}
	for (i=0;i<lp->im;i++) {
		scrowmin[i] =  ALPS_REAL_INFINITY;
		scrowmax[i] = -ALPS_REAL_INFINITY;
	}
	for (j=0;j<lp->inorig;j++) {
		for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++) {
			i = lp->imatcolind[k];
			ff = lp->imatcolcoeff[k];
			if (ff>sccolmax[j]) sccolmax[j] = ff;
			if (ff<sccolmin[j]) sccolmin[j] = ff;
			if (ff>scrowmax[i]) scrowmax[i] = ff;
			if (ff<scrowmin[i]) scrowmin[i] = ff;
		}
	}
	if (pril>=4) {
		maxrange = 0.0;
		maxi = 0;
		for (i=0;i<lp->im;i++) {
			if (scrowmax[i]-scrowmin[i]>maxrange) {
				maxrange = scrowmax[i]-scrowmin[i];
				maxi = i;
			}
		}
		printf("Maximum row range: [%16.8lf %16.8lf]\n",
			scrowmin[maxi],scrowmax[maxi]);
		maxrange = 0.0;
		maxi = 0;
		for (j=0;j<lp->inorig;j++) {
			if (sccolmax[j]-sccolmin[j]>maxrange) {
				maxrange = sccolmax[j]-sccolmin[j];
				maxi = j;
			}
		}
		printf("Maximum col range: [%16.8lf %16.8lf]\n",
			sccolmin[maxi],sccolmax[maxi]);
	}
	if (pril>=4) {
		printf("Range of row entries:\n");
		for (i=0;i<lp->im;i++) {
			printf("%-6d: [%16.8lf %16.8lf]\n",i,scrowmin[i],scrowmax[i]);
		}
		printf("Range of column entries:\n");
		for (j=0;j<lp->inorig;j++) {
			printf("%-6d: [%16.8lf %16.8lf]\n",j,sccolmin[j],sccolmax[j]);
		}
	}
#endif


	ffree( (char **) &sccolmin);
	ffree( (char **) &sccolmax);
	ffree( (char **) &scrowmin);
	ffree( (char **) &scrowmax);
	if (lp->scaling>5) {
		/** Perturb right hand side */

		{ 
			double rand01 (int *randseed);
			int randseed,i;
			randseed = 43857;
			for (i=0;i<lp->im;i++) {
				if ((lp->preprocessing && lp->aconstrainttype[org_rownr[i]]!='E')
					|| (!lp->preprocessing && lp->aconstrainttype[i]!='E') ) {
#ifdef TT
						lp->irhs[i] *= ( 0.999999 + 0.000001 * rand01(&randseed) );
#endif
						lp->irhs[i] -= ( 0.0000005 * rand01(&randseed) );
					}
				else {
#ifdef TT
					lp->irhs[i] *= ( 0.9999999 + 0.0000001 * rand01(&randseed) );
					lp->irhs[i] += 0.0000005 - ( 0.000001 * rand01(&randseed) );
#endif
				}
			}
			if (pril>=1) printf("Random perturbation of right hand side.\n");
		}


	}

	return 0;
}

} // end namespace ogdf
