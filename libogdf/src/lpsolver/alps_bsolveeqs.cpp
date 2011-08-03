/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-11-27 03:02:28 +1100 (Thu, 27 Nov 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief The BTRAN algorithm.
 * 
 * Using the LU factorization computed by \c alps_lufac, a 
 * system of equations $yB=c$ is solved by backward transformation. 
 * Here, only the column oriented format of the $U$ matrix is 
 * needed. Remark: see description of |alps_fsolveeqs|
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
* The layout of the function implementing the backward
* transformation algorithm is as follows.
*/

#include "alps.h"
#include "alps_misc.h"

namespace ogdf {

int alps_bsolveeqs(lpstruct *lp,
				   int dim,
				   int *pivrow,
				   int * /* pivcol */,
				   double *pivvalue,
				   int * /* cumatbeg */,
				   int * /* cumatend */,
				   int *cumatind,
				   double *cumatcoeff,
				   int *clmatbeg,
				   int *clmatend,
				   int *clmatind,
				   double *clmatcoeff,
				   int etanr,
				   int *etacol,
				   double *etaval,
				   int *etaind,
				   int *etastart,
				   int *etacount,
				   double *eta,
				   double *c,
				   double *y)
{
	/** Declare alps_bsolveeqs scalars */
	int i,l,iter,row,col,lindex;
	int cureta,k;
	int starttime,stoptime;
	double sumeiyi;

	/** Declare alps_bsolveeqs arrays */
	double *w; 


	/** Allocate space for the alps_bsolveeqs arrays */
	w   = (double *) malloc(dim * sizeof(double));
	if (!w)// run out of memory
	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}
	for (i=0; i<dim; i++) w[i] = c[i];
	if (etanr>0) {
		/** Perform multiplication with eta matrices */
		/*
		*  The eta-loop comes first. The eta matrices are scanned backward.
		*/
		k = etacol[etanr-1];
		sumeiyi = w[k];
		for (l=etastart[etanr-1]; l<etastart[etanr-1]+etacount[etanr-1]; l++) {
			i = etaind[l];
			sumeiyi -= etaval[l]*w[i];
		}
		w[k] = sumeiyi/eta[etanr-1];
		for (cureta=etanr-2; cureta>=0; cureta--) {
			k = etacol[cureta];
			sumeiyi = w[k];
			for (l=etastart[cureta]; l<etastart[cureta+1]; l++) {
				i = etaind[l];
				sumeiyi -= etaval[l]*w[i];
			}
			w[k] = sumeiyi/eta[cureta];
		}


	}
#ifdef ALPS_TIMING_ENABLED
	starttime = cputime();
#endif
	/** Perform multiplication with U matrix */
	/*
	*  Note that the U matrix is basically in column oriented format, but
	* that we have transformed indices already and stored additional
	* pivoting information.
	*/
	k = 0;
	while (cumatind[k]!=-2) {
		row = cumatind[k++];
		col = cumatind[k++];
		y[row] = w[col];
		while (cumatind[k]!=-1) {
			y[row] -= y[cumatind[k]] * cumatcoeff[k];
			k++;
		}
		k++;
	}

	/** Perform multiplication with L matrix */
	/*
	*  Here we need a backwards scan of the L matrix in order to perform the
	* multiplication.
	*/
	for (iter=dim-1; iter>=0; iter--) {
		col = pivrow[iter];
		y[col] = y[col]/pivvalue[iter];
		for (lindex = clmatbeg[iter]; lindex<clmatend[iter]; lindex++) {
			row = clmatind[lindex];
			y[col] += y[row]*clmatcoeff[lindex];
		}
	}

	/** Free space for the alps_bsolveeqs arrays */

	ffree( (char **) &w);

#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->btrantime += (stoptime - starttime);
#endif

	return 0;
}

/*
* We also include an iterated version of FTRAN to achieve better
* accuracy. This version needs the basis matrix computing residuals.
*/

int alps_bisolveeqs(lpstruct *lp,
					int dim,
					int *basmatbeg,
					int *basmatcount,
					int *basmatind,
					double *basmatcoeff,
					int *pivrow,
					int *pivcol,
					double *pivvalue,
					int *cumatbeg,
					int *cumatend,
					int *cumatind,
					double *cumatcoeff,
					int *clmatbeg,
					int *clmatend,
					int *clmatind,
					double *clmatcoeff,
					int etanr,
					int *etacol,
					double *etaval,
					int *etaind,
					int *etastart,
					int *etacount,
					double *eta,
					double *c,
					double *y
					)

{
	double *cc,*res,*z;
	double err,maxerror;
	int i,ii,j,additer;

	alps_bsolveeqs(
		lp,dim,pivrow,pivcol,pivvalue,
		cumatbeg,cumatend,cumatind,cumatcoeff,
		clmatbeg,clmatend,clmatind,clmatcoeff,
		etanr,etacol,etaval,etaind,etastart,etacount,eta,
		c,y
		);

	cc = (double *) malloc(dim * sizeof(double));
	res = (double *) malloc(dim * sizeof(double));
	z = (double *) malloc(dim * sizeof(double));
	if (!cc || !res || !z) /** run out of memory */
	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	additer = 0;
	maxerror = 0.0;
	for (i=0; i<dim; i++) {
		res[i] = c[i] - cc[i];
		err = fabs(res[i]);
		if (err>maxerror) maxerror = err; 
	}

	while (additer<5 && maxerror>1.0E-9) {
		printf("B Maxerror = %.6le\n",maxerror);

		alps_bsolveeqs(
			lp,dim,pivrow,pivcol,pivvalue,
			cumatbeg,cumatend,cumatind,cumatcoeff,
			clmatbeg,clmatend,clmatind,clmatcoeff,
			etanr,etacol,etaval,etaind,etastart,etacount,eta,
			res,z
			);

		for (i=0; i<dim; i++) {
			cc[i] = 0.0;
			y[i] += z[i]; 
		}

		for (j=0; j<dim; j++) {
			cc[j] = 0.0;
			for (ii=basmatbeg[j]; ii<basmatbeg[j]+basmatcount[j]; ii++) {
				i = basmatind[ii];
				cc[j] += y[i]*basmatcoeff[ii];
			}
		}

		maxerror = 0.0;
		for (i=0; i<dim; i++) {
			res[i] = c[i] - cc[i];
			err = fabs(res[i]);
			if (err>maxerror) maxerror = err; 
		}

		additer++;
	}
	printf("B Maxerror = %.6le\n",maxerror);

	ffree( (char **) &cc);
	ffree( (char **) &res);
	ffree( (char **) &z);

	return 0;
}

} // end namespace ogdf
