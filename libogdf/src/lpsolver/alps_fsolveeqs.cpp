/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-11-27 03:02:28 +1100 (Thu, 27 Nov 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief The FTRAN algorithm.
 * 
 * Using the LU factorization computed by {\tt alps_lufac}, 
 * a system of equations $Bx=b$ is solved by forward transformation. 
 * Here, only the row oriented format of the $U$ matrix is needed. 
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
*  The layout of the \CEE/ function implementing the forward
* transformation algorithm is as follows.
*/


#include "alps.h"
#include "alps_misc.h"

namespace ogdf {

int alps_fsolveeqs(
				   lpstruct *lp,
				   int dim,
				   int *pivrow,
				   int *pivcol,
				   double *pivvalue,
				   int *rumatbeg,
				   int *rumatend,
				   int *rumatind,
				   double *rumatcoeff,
				   int * /* clmatbeg */,
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
				   double *b,
				   double *x
				   )
{
	/** Declare alps_fsolveeqs scalars */
	int i,l,iter,row,col;
	register int lindex,uindex;
	int cureta,k;
	int starttime,stoptime;
	double xk,wc;


	/** Declare alps_fsolveeqs arrays */
	double *w; 

#ifdef ALPS_TIMING_ENABLED
	starttime = cputime();
#endif
	/** Allocate space for the alps_fsolveeqs arrays */

	w = (double *) malloc(dim * sizeof(double));
	if (!w)// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	/** Multiplication with L matrix */
	/*
	*  Note that we assume that the rows of L are stored consecutively in
	* ascending order starting with row~0.
	*/
	for (i=0; i<dim; i++) w[i] = b[i];
	lindex = 0;
	for (iter=0; iter<dim; iter++) {
		col = pivrow[iter];
		wc = w[col];
		for (; lindex<clmatend[iter]; lindex++) {
			row = clmatind[lindex];
			w[row] += wc*clmatcoeff[lindex];
		}
		w[col] /= pivvalue[iter];
	}


	/** Multiplication with U matrix */
	/*
	*  Having $U$ in row oriented format, this is quite straightforward.
	*/
	for (iter=dim-1; iter>=0; iter--) {
		row = pivrow[iter];
		col = pivcol[iter];
		x[col] = w[row];
		for (uindex=rumatbeg[iter]; uindex<rumatend[iter]; uindex++) 
			x[col] -= x[rumatind[uindex]]*rumatcoeff[uindex];
	}


#ifdef ALPS_TIMING_ENABLED
	stoptime = cputime();
	lp->ftrantime += (stoptime - starttime);
#endif
	if (etanr>0) {
		/** Multiplication with eta matrices */
		/*
		*  The eta loop follows. Eta columns are assumed to be stored
		* consecutively.
		*/
		l = etastart[0];
		for (cureta=0; cureta<etanr-1; cureta++) {
			k = etacol[cureta];
			xk = x[k]/eta[cureta];
			x[k] = xk;
			for (; l<etastart[cureta+1]; l++) {
				i = etaind[l];
				x[i] -= etaval[l]*xk;
			}
		}
		k = etacol[etanr-1];
		xk = x[k]/eta[etanr-1];
		x[k] = xk;
		for (; l<etastart[etanr-1]+etacount[etanr-1]; l++) {
			i = etaind[l];
			x[i] -= etaval[l]*xk;
		}


	}
	/** Free space for the alps_fsolveeqs arrays */

	ffree( (char **) &w);

	return 0;
}

/*
*  We also include an iterated version of FTRAN to achieve better
* accuracy. This version needs the basis matrix computing residuals.
*/


int alps_fisolveeqs(
					lpstruct *lp,
					int dim,
					int *basmatbeg,
					int *basmatcount,
					int *basmatind,
					double *basmatcoeff,
					int *pivrow,
					int *pivcol,
					double *pivvalue,
					int *rumatbeg,
					int *rumatend,
					int *rumatind,
					double *rumatcoeff,
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
					double *b,
					double *x
					)

{
	double *bb,*res,*y;
	double err,maxerror;
	int i,ii,j,additer;

	alps_fsolveeqs(
		lp,dim,pivrow,pivcol,pivvalue,
		rumatbeg,rumatend,rumatind,rumatcoeff,
		clmatbeg,clmatend,clmatind,clmatcoeff,
		etanr,etacol,etaval,etaind,etastart,etacount,eta,
		b,x
		);

	bb = (double *) malloc(dim * sizeof(double));
	res = (double *) malloc(dim * sizeof(double));
	y = (double *) malloc(dim * sizeof(double));
	if (!bb || !res || !y) /** run out of memory */

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}


	;

	additer = 0;
	for (i=0; i<dim; i++) bb[i] = 0.0;
	for (j=0; j<dim; j++) {
		for (ii=basmatbeg[j]; ii<basmatbeg[j]+basmatcount[j]; ii++) {
			i = basmatind[ii];
			bb[i] += x[j]*basmatcoeff[ii];
		}
	}
	maxerror = 0.0;
	for (i=0; i<dim; i++) {
		res[i] = b[i] - bb[i];
		err = fabs(res[i]);
		if (err>maxerror) maxerror = err; 
	}

	while (additer<5 && maxerror>1.0E-9) {
		printf("F Maxerror = %.6le\n",maxerror);

		alps_fsolveeqs(
			lp,dim,pivrow,pivcol,pivvalue,
			rumatbeg,rumatend,rumatind,rumatcoeff,
			clmatbeg,clmatend,clmatind,clmatcoeff,
			etanr,etacol,etaval,etaind,etastart,etacount,eta,
			res,y
			);

		for (i=0; i<dim; i++) {
			bb[i] = 0.0;
			x[i] += y[i]; 
		}

		for (i=0; i<dim; i++) bb[i] = 0.0;
		for (j=0; j<dim; j++) {
			for (ii=basmatbeg[j]; ii<basmatbeg[j]+basmatcount[j]; ii++) {
				i = basmatind[ii];
				bb[i] += x[j]*basmatcoeff[ii];
			}
		}

		maxerror = 0.0;
		for (i=0; i<dim; i++) {
			res[i] = b[i] - bb[i];
			err = fabs(res[i]);
			if (err>maxerror) maxerror = err; 
		}

		additer++;
	}
	printf("F Maxerror = %.6le\n",maxerror);

	ffree( (char **) &bb);
	ffree( (char **) &res);
	ffree( (char **) &y);

	return 0;
}

} // end namespace ogdf
