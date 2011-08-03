/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-11-27 03:02:28 +1100 (Thu, 27 Nov 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief The LU factorization algorithm.
 * 
 * The function |alps_lufac| implements an LU factorization 
 * algorithm for sparse matrices. The input matrix is assumed 
 * to be in column format. The L matrix will be returned in
 * column format, the U matrix in both column and row format.
 * In contrast to previous versions we do not use |...count| 
 * arrays for L and U, but have pointers to the last element of 
 * a row/column plus~1. Note that (omitting permutations) we 
 * compute $L$ and $U$ such that $LA=U$, i.e., $A=L^{-1}U$. 
 * Moreover, in the arrays for storing |U| in column format, we 
 * store as first two elements in |cumatind| for every column its
 * corresponding pivot row and pivot column. The pointers |cumatbeg|
 * and |cumatend| nevertheless correctly point to the nonzero
 * column elements. We hope to avoid some cache misses in BTRAN 
 * this way. If debugging is necessary then the file 
 * |alps_ludebug.w| has to be included here.
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
*  The current implementation of the |alps_lufac| function is in
* traditional \CEE/ style. Input parameters are the following: |pril|
* controls the output level, |dim| is the dimension of the (quadratic)
* input matrix, |cmatbeg| ... |cmatcoeff| contain the input matrix in
* column format. All other parameters are output parameters. Space for
* arrays is allocated by |alps_lufac| itself. In detail, output
* parameters are: |pivrow| ... |pivvalue| contain the pivot sequence,
* |rumatbeg| ... |rumatcoeff| contain the U matrix in row format,
* |cumatbeg| ... |cumatcoeff| contain the U matrix in column format,
* |clmatbeg| ... |clmatcoeff| contain the L matrix in column format,
* |totalfill| gives the total fill-in, |totalcancel| gives the number of
* cancelled elements.
*/

#include "alps.h"
#include "alps_misc.h"

namespace ogdf {

int alps_lufac (
				int pril,
				int /* luoption */,
				int dim,
				int *cmatbeg,
				int *cmatcount,
				int *cmatind,
				double *cmatcoeff,
				int **pivrow,
				int **pivcol,
				double **pivvalue,
				int **rumatbeg,
				int **rumatend,
				int **rumatind,
				double **rumatcoeff,
				int **cumatbeg,
				int **cumatend,
				int **cumatind,
				double **cumatcoeff,
				int **clmatbeg,
				int **clmatend,
				int **clmatind,
				double **clmatcoeff,
				int *totalfill,
				int *totalcancel
				)
{
	/** Declare alps_lufac structures */
	/*
	*  The matrix to be factorized is stored as a set of matrixelements
	* which are doubly linked per row and per column. Each element contains
	* its row and column index, the matrix entry, and pointers to the
	* previous and next entries in its row and column. The matrix is stored
	* in a huge array, so the pointers are in fact array indices.
	*/
	struct matrixelement{
		int row;
		int col;
		int prevrowofcol;
		int nextrowofcol;
		int prevcolofrow;
		int nextcolofrow;
		double coeff;
	};

	/** Declare alps_lufac scalars */
	/*
	*  Scalar variables of interest are: |unindex| and |lindex| point to the
	* next free element of the U and L matrix to be constructed,
	* |fillfactor| is an estimation for fill-in, |freememind| points to the
	* next free element in the array |mem|. |freememnolink| points to the
	* free element of this array which is not yet linked to the next one.
	* This element is always in the initial contiguous free area. So the
	* links are clear, but we can avoid to initialize them. Only in the case
	* of a fill-in we have to check if we have to introduce new links. {\sl
	* Many other variables are local to some section and should be declared
	* there in a later version.}
	*/
	int fillfactor;
	int maxmatrixentries;
	int freememind;
	int freememnolink;
	int i,j,k,r,s,ixx;
	int iter;
	int nelts;
	int marko;
	double ars;
	int uindex,lindex,meind;
	int upcol,downcol;
	int uind,uuind,ucol,currow;
	int leftrow,rightrow;
	double ais;
	int currowind;
	int pivcolind;
	int pivind;
	int nonzeroes;
	int ufrontind;
	int minind;
	int freeind;
	double canceleps;
	double xx;
	int rr,ss,rowpos,colpos,rowcnt,colcnt,lastind;
	int nzcurno,nzcurpos,nzfound;


	/** Declare alps_lufac arrays */
	/*
	*  The working matrix is stored as an array of |matrixelement|
	* structure. For every row or column we have a dummy first element which
	* is addressed by |rowfirstindex| and |colfirstindex|. For every row and
	* column we keep the number of nonzeroes and the sum ab absolute values
	* of its entries. Note that rows and columns are numbered starting with
	* 0.
	*/
	struct matrixelement *mem = NULL;
	int    *rowfirstindex = NULL; 
	int    *colfirstindex = NULL; 
	int    *rownonzeroes = NULL;
	int    *colnonzeroes = NULL;
	double *rowsum = NULL;   
	double *colsum = NULL;  

	/*
	*  We have to have access to the rows/columns with few nonzeroes. Since
	* the number of nonzeroes per row/column is between zero and |dim|, we
	* use a simple sorting mechanism. We have an array |nz| of row/column
	* indices sorted according to the number of nonzeroes. For |i| between 1
	* and |dim|, the number |nzbeg[i]| gives the first row/column index in
	* |nz| with |i| nonzeroes. The next array element to be used to write a
	* new index with |i| nonzeroes is given by |nznext[i]|. For every
	* row/column its position in |nz| is given by |nzpos[]|. Note that this
	* simple sorting is possible only because the number of nonzeroes is
	* nothing but incremented or decremented by~1 during pivoting.
	*/
	int   *nzind = NULL;
	int   *nzpos = NULL;
	int   *nzbeg = NULL;
	int   *nznext = NULL;

	/*
	*  Furthermore we need some multipurpose row and column indices and
	* indices for keeping track of the U front.
	*/
	int    *rowind = NULL;
	int    *colind = NULL; 
	int    *ufront = NULL; 

	/** Scalars for pivot search */
	/*
	*  For finding a suitable pivot we proceed according to the following
	* strategy. We give priority to rows/columns with least number of
	* nonzeroes in order to have a good chance to find a pivot with low
	* Markowitz count. Among possible pivots we give first priority to those
	* meeting the row and the column threshold criterion and second to those
	* meeting either of the two. If we cannot find pivots meeting at least
	* one criterion, we select one with low Markowitz count. We search for a
	* pivot meeting the threshold criterion only among the first |maxtries|
	* rows/columns with smallest number of nonzeroes. First tests seem to
	* suggest that small values of |maxtries|, |threshold| and |maxfound|
	* already suffice to obtain an LU factorization with small fill.
	*/
	int maxtries,maxfound,nretries;
	int minmarkowitz;
	int minmark1,minmark2,minmark3;
	int tries,found;
	int pivind1,pivind2,pivind3;
	double threshold;
	double fars,rowavg,colavg;

	nretries = 0;
retry:
	/** Set parameters for pivot search */
	/*
	*  Here we set the parameters for pivot search. If the first attempt to
	* factorize the matrix fails then we set parameters (depending on the
	* number of retries) with which there is a higher chance to obtain a
	* factorization.
	*/
	switch (nretries) {
case 0:
	maxtries     =   2;
	maxfound     =   2;
	threshold    =   0.02;
	minmarkowitz =   1;
	canceleps    =   1.0E-9;
	break;
case 1:
	// Free space for the alps_lufac arrays
	/*
	*  All local space is freed on normal return. If the function terminates
	* prematurely we do not always care about freeing arrays.
	*/
	ffree( (char **) &mem);
	ffree( (char **) &rownonzeroes);
	ffree( (char **) &colnonzeroes);
	ffree( (char **) &rowfirstindex);
	ffree( (char **) &colfirstindex);
	ffree( (char **) &rowsum);
	ffree( (char **) &colsum);
	ffree( (char **) &rowind);
	ffree( (char **) &colind);
	ffree( (char **) &ufront);
	ffree( (char **) &nzind);
	ffree( (char **) &nzpos);
	ffree( (char **) &nzbeg);
	ffree( (char **) &nznext);


	// Free space for the gauss elimination arrays

	ffree( (char **) &(*pivrow));
	ffree( (char **) &(*pivcol));
	ffree( (char **) &(*pivvalue));
	ffree( (char **) &(*rumatbeg));
	ffree( (char **) &(*rumatend));
	ffree( (char **) &(*rumatind));
	ffree( (char **) &(*rumatcoeff));
	ffree( (char **) &(*clmatbeg));
	ffree( (char **) &(*clmatend));
	ffree( (char **) &(*clmatind));
	ffree( (char **) &(*clmatcoeff));
	ffree( (char **) &(*cumatbeg));
	ffree( (char **) &(*cumatend));

	maxtries     =  10;
	maxfound     =  10;
	threshold    =   0.005;
	minmarkowitz =   4;
	canceleps    =   1.0E-12;
	break;
case 2:
	// Free space for the alps_lufac arrays
	/*
	*  All local space is freed on normal return. If the function terminates
	* prematurely we do not always care about freeing arrays.
	*/
	ffree( (char **) &mem);
	ffree( (char **) &rownonzeroes);
	ffree( (char **) &colnonzeroes);
	ffree( (char **) &rowfirstindex);
	ffree( (char **) &colfirstindex);
	ffree( (char **) &rowsum);
	ffree( (char **) &colsum);
	ffree( (char **) &rowind);
	ffree( (char **) &colind);
	ffree( (char **) &ufront);
	ffree( (char **) &nzind);
	ffree( (char **) &nzpos);
	ffree( (char **) &nzbeg);
	ffree( (char **) &nznext);


	// Free space for the gauss elimination arrays

	ffree( (char **) &(*pivrow));
	ffree( (char **) &(*pivcol));
	ffree( (char **) &(*pivvalue));
	ffree( (char **) &(*rumatbeg));
	ffree( (char **) &(*rumatend));
	ffree( (char **) &(*rumatind));
	ffree( (char **) &(*rumatcoeff));
	ffree( (char **) &(*clmatbeg));
	ffree( (char **) &(*clmatend));
	ffree( (char **) &(*clmatind));
	ffree( (char **) &(*clmatcoeff));
	ffree( (char **) &(*cumatbeg));
	ffree( (char **) &(*cumatend));

	maxtries     =   20;
	maxfound     =   20;
	threshold    =    0.001;
	minmarkowitz =    9;
	canceleps    =    0.0;
	break;
default:
	if (pril) printf("LU-ERROR: no pivot at iter %1d (of %1d)\n",iter+1,dim);
	// Free space for the alps_lufac arrays
	/*
	*  All local space is freed on normal return. If the function terminates
	* prematurely we do not always care about freeing arrays.
	*/
	ffree( (char **) &mem);
	ffree( (char **) &rownonzeroes);
	ffree( (char **) &colnonzeroes);
	ffree( (char **) &rowfirstindex);
	ffree( (char **) &colfirstindex);
	ffree( (char **) &rowsum);
	ffree( (char **) &colsum);
	ffree( (char **) &rowind);
	ffree( (char **) &colind);
	ffree( (char **) &ufront);
	ffree( (char **) &nzind);
	ffree( (char **) &nzpos);
	ffree( (char **) &nzbeg);
	ffree( (char **) &nznext);


	// Free space for the gauss elimination arrays

	ffree( (char **) &(*pivrow));
	ffree( (char **) &(*pivcol));
	ffree( (char **) &(*pivvalue));
	ffree( (char **) &(*rumatbeg));
	ffree( (char **) &(*rumatend));
	ffree( (char **) &(*rumatind));
	ffree( (char **) &(*rumatcoeff));
	ffree( (char **) &(*clmatbeg));
	ffree( (char **) &(*clmatend));
	ffree( (char **) &(*clmatind));
	ffree( (char **) &(*clmatcoeff));
	ffree( (char **) &(*cumatbeg));
	ffree( (char **) &(*cumatend));

	return ALPS_LU_NOPIVOT;
	break;
	}

	/** Initializations */
	/*
	*  We initialize a few variables. We estimate the number of matrix
	* elements needed by assuming some fillfactor. If the matrix grows
	* larger we will perform a reallocation.
	*/
	*totalfill = 0;
	*totalcancel = 0;
	fillfactor = 5; /* this one will be calculated somehow */
	maxmatrixentries = 2*dim + fillfactor * (cmatbeg[dim-1] + cmatcount[dim-1]);

	/** Allocate space for the alps_lufac arrays */
	/*
	*  The |mem| array is mainly responsible for the memory space needed.
	* E.g., for a $1000\times 1000$ matrix with 10000 nonzeroes and
	* fillfactor of~5 the array needs about 3MB.
	*/
	mem   = (struct matrixelement *) malloc(maxmatrixentries * sizeof(struct matrixelement));
	rownonzeroes  = (int *) malloc(dim * sizeof(int));
	colnonzeroes  = (int *) malloc(dim * sizeof(int));
	rowfirstindex = (int *) malloc(dim * sizeof(int));
	colfirstindex = (int *) malloc(dim * sizeof(int));
	rowsum        = (double *) malloc(dim * sizeof(double));
	colsum        = (double *) malloc(dim * sizeof(double));
	rowind        = (int *) malloc(dim * sizeof(int));
	colind        = (int *) malloc(dim * sizeof(int));
	ufront        = (int *) malloc(dim * sizeof(int));
	nzind         = (int *) malloc((2*dim+1) * sizeof(int));
	nzpos         = (int *) malloc((2*dim+1) * sizeof(int));
	nzbeg         = (int *) malloc((dim+1) * sizeof(int));
	nznext        = (int *) malloc((dim+1) * sizeof(int));

	if (!mem || !rownonzeroes || !colnonzeroes || !rowfirstindex 
		|| !colfirstindex || !rowsum || !colsum
		|| !rowind || !colind || !ufront 
		|| !nzind || !nzpos || !nzbeg || !nznext)
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	/** Store input matrix in doubly linked format */
	/*
	*  We assume that the matrix is given in true column oriented sparse
	* format, i.e., the nonzeroes are really nonzeroes. The calling program
	* is assumed to make sure that this is the case. This is not necessary
	* for correctness, only for efficiency. Here we set up the initial
	* sparse matrix without checking if the nonzeroes are really nonzeroes.
	* For each row or column we store the number of entries and the sum of
	* its entries in absolute value. Each doubly linked list contains a
	* dummy element, whose row, column entry respectively, is set to
	* |ALPS_INT_INFINITY|. These matrix elements are pointed to by
	* |rowfirstindex[]|, |colfirstindex[]| respectively.
	*/
	// Initialize nonzeroes, sums and dummy elements

	nonzeroes = 0;
	for (r=0; r<dim; r++) {
		rownonzeroes[r]  = 0;
		rowsum[r]   = 0.0;
	}
	for (s=0; s<dim; s++) {
		colsum[s]   = 0.0;
	}
	for (s=0; s<dim; s++) {
		colnonzeroes[s] = cmatcount[s];
		nonzeroes += colnonzeroes[s];
		for(j=cmatbeg[s]; j<cmatbeg[s]+cmatcount[s]; j++) rownonzeroes[cmatind[j]]++;
	}
	rowfirstindex[0] = 0;
	for (r=1; r<dim; r++) rowfirstindex[r] = rowfirstindex[r-1] + rownonzeroes[r-1] + 1; 
	for (r=0; r<dim; r++) {
		mem[rowfirstindex[r]].row = r;
		mem[rowfirstindex[r]].col = ALPS_INT_INFINITY;
		mem[rowfirstindex[r]].nextcolofrow = rowfirstindex[r] + 1;
		mem[rowfirstindex[r]].prevcolofrow = rowfirstindex[r] + rownonzeroes[r];
		rowind[r] = rowfirstindex[r];
	}
	for (s=0; s<dim; s++) {
		colfirstindex[s] = nonzeroes + dim + s;
		mem[colfirstindex[s]].row = ALPS_INT_INFINITY;
		mem[colfirstindex[s]].col = s;
		colind[s] = colfirstindex[s];
	}

	// Initialize sorted list of rows/columns
	/*
	*  Note that we misuse |nzpos| as an intermediate counter here when we
	* initialize the sorted lists of nonzero counts.
	*/
	for (i=0;i<=dim;i++) nzpos[i] = 0;
	for (r=0;r<dim;r++) nzpos[rownonzeroes[r]]++;
	for (s=0;s<dim;s++) nzpos[colnonzeroes[s]]++;
	nzbeg[0] = 0;
	for (i=1;i<=dim;i++) nzbeg[i] = nzbeg[i-1] + nzpos[i-1];
	for (i=0;i<=dim;i++) nznext[i] = nzbeg[i];
	for (i=0;i<dim;i++) {
		nzpos[i+dim] = nznext[colnonzeroes[i]];
		nzind[nzpos[i+dim]] = i+dim;
		nznext[colnonzeroes[i]]++;
		nzpos[i] = nznext[rownonzeroes[i]];
		nzind[nzpos[i]] = i;
		nznext[rownonzeroes[i]]++;
	}

	// Construct a doubly linked matrix from the input matrix
	for (s=0; s<dim; s++) {
		for(j=cmatbeg[s]; j<cmatbeg[s]+cmatcount[s]; j++) {
			r = cmatind[j];
			currowind = rowind[r] + 1;
			mem[currowind].row = r;
			mem[currowind].col = s;
			mem[currowind].coeff = cmatcoeff[j];
			rowsum[r] += fabs(cmatcoeff[j]);
			colsum[s] += fabs(cmatcoeff[j]);
			mem[currowind].prevcolofrow = currowind - 1;
			mem[currowind].nextcolofrow = currowind + 1;
			mem[currowind].prevrowofcol = colind[s];
			mem[colind[s]].nextrowofcol = currowind;
			colind[s] = currowind;
			rowind[r]++;
		}
	}
	for (r=0; r<dim; r++) mem[rowind[r]].nextcolofrow = rowfirstindex[r];
	for (s=0; s<dim; s++) {
		mem[colind[s]].nextrowofcol = colfirstindex[s];
		mem[colfirstindex[s]].prevrowofcol = colind[s];
	}

	// Link the free |mem| elements
	/*
	*  The free elements are not yet linked, because this turned out to be
	* rather time consuming. The linking is done as needed using the
	* pointers |nextcolofrow|. The index |freememnolink| points to the area
	* of unlinked elements.
	*/
	freememind = colfirstindex[dim-1] + 1;
	freememnolink = freememind;
	mem[freememnolink].nextcolofrow = 0;

	/** Gauss elimination */
	/*
	*  This is the central loop of the LU factorization. Basically, we
	* perform just Gau\ss\ elimination. The prime task here is to find a
	* good pivot that leads to little fill-in but does not lead to numerical
	* instabilities. So far, we did not care too much about numerics.
	*/
	// Gauss initializations
	/*
	*  Here we allocate the space for the triangular factors $L$ and $U$,
	* and initialize a few variables. Since we have no idea about their
	* expected size, we allocate |maxmatrixentries| elements.
	*/
	*rumatbeg   = (int *) malloc(dim * sizeof(int)); 
	*rumatend   = (int *) malloc(dim * sizeof(int)); 
	*rumatind   = (int *) malloc(maxmatrixentries * sizeof(int)); 
	*rumatcoeff = (double *) malloc(maxmatrixentries * sizeof(double));
	*clmatbeg   = (int *) malloc(dim * sizeof(int)); 
	*clmatend   = (int *) malloc(dim * sizeof(int)); 
	*clmatind   = (int *) malloc(maxmatrixentries * sizeof(int)); 
	*clmatcoeff = (double *) malloc(maxmatrixentries* sizeof(double));
	*cumatbeg   = (int *) malloc(dim * sizeof(int)); 
	*cumatend   = (int *) malloc(dim * sizeof(int)); 
	*pivrow     = (int *) malloc(dim * sizeof(int));
	*pivcol     = (int *) malloc(dim * sizeof(int));
	*pivvalue   = (double *) malloc(dim * sizeof(double));

	if (!(*pivrow) || !(*pivcol) || !(*pivvalue) 
		|| !(*rumatbeg) || !(*rumatend) || !(*rumatind) || !(*rumatcoeff)
		|| !(*clmatbeg) || !(*clmatend) || !(*clmatind) || !(*clmatcoeff) 
		|| !(*cumatbeg) || !(*cumatend))
		// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	uindex = 0;
	lindex = 0;
	for (j=0; j<dim; j++) (*cumatend)[j] = 0;

	for (iter=0; iter<dim; iter++) {
		// Search for stable pivot
		/*
		*  Depending on the parameter settings we may scan several row/columns
		* to find pivots meeting the threshold criterion.
		*/
		minmark1 = ALPS_INT_INFINITY;
		minmark2 = ALPS_INT_INFINITY;
		minmark3 = ALPS_INT_INFINITY;
		pivind1 = pivind2 = pivind3 = -1;
		tries = 0;
		found = 0;
		nzcurno = 1;
		nzcurpos = nzbeg[1];

		while (found<maxfound 
			&& tries<maxtries  
			&& tries<dim-iter
			&& minmark1>minmarkowitz
			&& minmark2>minmarkowitz) {

				nzfound = 0;
				while (!nzfound) {
					if (nzcurpos<nznext[nzcurno]) {
						nzfound = 1;
						minind = nzind[nzcurpos];
						nzcurpos++;
					}
					else {
						nzcurno++;
						if (nzcurno>dim) { 
							if (pril) 
								printf("LU-ERROR: no pivot at iter %1d (of %1d). Retry...\n",
								iter+1,dim);

							nretries++;
							goto retry;
						}
						nzcurpos = nzbeg[nzcurno];
					}
				}

				if (minind<dim) {
					// Pivot search in row |minind|

					r = minind;
					if (rownonzeroes[r]<=0) {
						if (pril) printf("LU-ERROR: zero row %1d at pivot %1d\n",r,iter);
						return ALPS_LU_ZEROROW;
					}
					rowavg = rowsum[r]/rownonzeroes[r];
					meind = mem[rowfirstindex[r]].nextcolofrow;
					while (meind!=rowfirstindex[r]) {
						j = mem[meind].col;
						marko = (rownonzeroes[r]-1)*(colnonzeroes[j]-1);
						colavg = colsum[j]/colnonzeroes[j];
						fars = fabs(mem[meind].coeff);
						if ( fars>threshold*rowavg && fars>threshold*colavg
							&& fars<rowavg/threshold && fars<colavg/threshold ) {
								if (marko<minmark1) {
									minmark1 = marko;
									pivind1 = meind;
								}
								found++;
							}
						else if ( (fars>threshold*rowavg && fars<rowavg/threshold)
							|| (fars>threshold*colavg && fars<colavg/threshold) ) {
								if (marko<minmark2) {
									minmark2 = marko;
									pivind2 = meind;
								}
							}
						else if (marko<minmark3) {
							minmark3 = marko;
							pivind3 = meind;
						}
						meind = mem[meind].nextcolofrow;
					}  


				}
				else {
					// Pivot search in column |minind-dim|

					s = minind-dim;
					if (colnonzeroes[s]<=0) {
						if (pril) printf("LU-ERROR: zero col %1d at pivot %1d\n",s,iter);
						return ALPS_LU_ZEROCOL;
					}
					colavg = colsum[s]/colnonzeroes[s];
					meind = mem[colfirstindex[s]].nextrowofcol;
					while (meind!=colfirstindex[s]) {
						i = mem[meind].row;
						rowavg = rowsum[i]/rownonzeroes[i];
						marko = (rownonzeroes[i]-1)*(colnonzeroes[s]-1);
						fars = fabs(mem[meind].coeff);
						if ( fars>threshold*rowavg && fars>threshold*colavg
							&& fars<rowavg/threshold && fars<colavg/threshold ) {
								if (marko<minmark1) {
									minmark1 = marko;
									pivind1 = meind;
								}
								found++;
							}
						else if ( (fars>threshold*rowavg && fars<rowavg/threshold)
							|| (fars>threshold*colavg && fars<colavg/threshold) ) {
								if (marko<minmark2) {
									minmark2 = marko;
									pivind2 = meind;
								}
							}
						else if (marko<minmark3) {
							minmark3 = marko;
							pivind3 = meind;
						}
						meind = mem[meind].nextrowofcol;
					}  

				}
				tries++;
			}

			if (found) {
				pivind = pivind1;
			}
			else if (minmark2<ALPS_INT_INFINITY) pivind = pivind2;
			else if (minmark3<ALPS_INT_INFINITY) pivind = pivind3;
			else {
				if (pril) 
					printf("LU iter %1d (of %1d): Pivoting problems. Retry...\n",iter,dim);
				nretries++;
				goto retry;
			}
			r = mem[pivind].row;
			s = mem[pivind].col;
			ars = mem[pivind].coeff;
			(*pivrow)[iter] = r;
			(*pivcol)[iter] = s;
			(*pivvalue)[iter] = ars;
			if (pril>=4) printf(" pivot %4d: %4d %4d %10.3lf  [%5d]\n",
				iter,r,s,ars,*totalfill);

			// Process the pivot row
			/*
			*  We scan the pivot row and divide nonzero elements by the pivot
			* element, storing the results and the corresponding columns in a row of
			* the $U$ matrix. Note that |uindex| points to the next free space in
			* the U matrix, and that we already update the column counts for the
			* transposition if U to be performed at the end of |alps_lufac|. The
			* pivot row is unlinked from the working matrix, and the freed space is
			* made available for fill-in. Namely, all freed elements are linked and
			* the first free element is addressed by |freememind|. In contrast to
			* previous versions, the |ufront| is already initialized here. We misuse
			* |cumatend| here as counter of the number of nonzeroes per column.
			*/
			(*rumatbeg)[iter] = uindex;
			meind = mem[rowfirstindex[r]].nextcolofrow;
			j = mem[meind].col;
			i = 0;
			while (j<ALPS_INT_INFINITY) {
				colsum[j] -= fabs(mem[meind].coeff);
				if (j!=s) {
					(*rumatcoeff)[uindex] = mem[meind].coeff/ars;
					(*rumatind)[uindex] = j;
					(*cumatend)[j]++;
					uindex++;
				}
				ss = j;
				// Decrement column count

				if ((colcnt = colnonzeroes[ss]) > 0) {
					colpos = nzpos[ss+dim];
					nzind[colpos] = nzind[nzbeg[colcnt]];
					nzpos[nzind[colpos]] = colpos;
					nzbeg[colcnt]++;
					if (colcnt>=1) {
						nzind[nznext[colcnt-1]] = ss+dim;
						nzpos[ss+dim] = nznext[colcnt-1];
						nznext[colcnt-1]++;
					}
					colnonzeroes[ss]--;
				}

				upcol   = mem[meind].prevrowofcol;
				downcol = mem[meind].nextrowofcol;
				mem[upcol].nextrowofcol = downcol;
				mem[downcol].prevrowofcol = upcol;
				freeind = meind; 
				meind = mem[meind].nextcolofrow;
				mem[freeind].nextcolofrow = freememind; 
				freememind = freeind; 
				if (j!=s) {
					ufront[i] = mem[colfirstindex[j]].nextrowofcol;
					i++;
				}
				j = mem[meind].col;
			}
			(*rumatend)[iter] = uindex;
			mem[rowfirstindex[r]].nextcolofrow = rowfirstindex[r];
			mem[rowfirstindex[r]].prevcolofrow = rowfirstindex[r];
			rr = r;
			// Remove row count

			if ((rowcnt = rownonzeroes[rr]) != 0) {
				rowpos = nzpos[rr];
				nzind[rowpos] = nzind[nzbeg[rowcnt]];
				nzpos[nzind[rowpos]] = rowpos;
				nzbeg[rowcnt]++;
				rownonzeroes[rr] = 0;
			}

			// Process the pivot column
			/*
			*  We now scan the pivot column to identify rows with a nonzero entry in
			* the pivot column and perform the pivot operations. Note that the pivot
			* row is already deleted and the U matrix is used instead. Fill-ins are
			* stored in |mem|. Freed space for pivot row and pivot column entries,
			* as well as cancellations, are appended to the list of free matrix
			* elements. When processing the pivot column, the most difficult part is
			* handling the fill as efficiently as possible. For this, we have a
			* ``downwave'' of column indices called |ufront|, that was initialized
			* while scanning the pivot row and is maintained during the following
			* computation. The pivot column is in the doubly linked structure until
			* the end of its processing, when it is unlinked and the space is made
			* available. For each pivot column element, we make the according $L$
			* matrix entry and transform the row. In the first case, a nonzero is
			* updated and possibly cancelled, in the second case, a fill is
			* produced.
			*/
			(*clmatbeg)[iter] = lindex;
			pivcolind = mem[colfirstindex[s]].nextrowofcol;
			while (pivcolind!=colfirstindex[s]) {
				ais = mem[pivcolind].coeff;
				currow = mem[pivcolind].row;
				rowsum[currow] -= fabs(ais);
				currowind = mem[rowfirstindex[currow]].nextcolofrow;
				(*clmatcoeff)[lindex] = -ais/ars;
				(*clmatind)[lindex] = currow;
				lindex++;
				rr = currow;
				// Decrement row count

				if ((rowcnt = rownonzeroes[rr]) > 0) {
					rowpos = nzpos[rr];
					nzind[rowpos] = nzind[nzbeg[rowcnt]];
					nzpos[nzind[rowpos]] = rowpos;
					nzbeg[rowcnt]++;
					if (rowcnt>=1) {
						nzind[nznext[rowcnt-1]] = rr;
						nzpos[rr] = nznext[rowcnt-1];
						nznext[rowcnt-1]++;
					}
					rownonzeroes[rr]--;
				}

				j = mem[currowind].col;
				for (uuind=(*rumatbeg)[iter],uind=0;uuind<(*rumatend)[iter];uuind++,uind++) {
					uind = uuind - (*rumatbeg)[iter];
					ucol = (*rumatind)[uuind];
					while (j<ucol) {
						currowind = mem[currowind].nextcolofrow;
						j = mem[currowind].col;
					}
					if (j==ucol) {
						// Update existing matrix element, take care of cancellations
						/*
						*  The update of an existing matrix element is trivial, a cancelled
						* element is unlinked and freed.
						*/
						rowsum[currow] -= fabs(mem[currowind].coeff);
						colsum[j] -= fabs(mem[currowind].coeff);
						mem[currowind].coeff -= ais*(*rumatcoeff)[uuind];
						rowsum[currow] += fabs(mem[currowind].coeff);
						colsum[j] += fabs(mem[currowind].coeff);
						if (fabs(mem[currowind].coeff)<=canceleps) {
							freeind = currowind; 
							upcol   = mem[currowind].prevrowofcol;
							downcol = mem[currowind].nextrowofcol;
							ufront[uind] = downcol; 
							mem[upcol].nextrowofcol = downcol;
							mem[downcol].prevrowofcol = upcol;
							leftrow  = mem[currowind].prevcolofrow;
							rightrow = mem[currowind].nextcolofrow;
							mem[leftrow].nextcolofrow = rightrow;
							mem[rightrow].prevcolofrow = leftrow;
							currowind = leftrow;
							rr = currow;
							// Decrement row count

							if ((rowcnt = rownonzeroes[rr]) > 0) {
								rowpos = nzpos[rr];
								nzind[rowpos] = nzind[nzbeg[rowcnt]];
								nzpos[nzind[rowpos]] = rowpos;
								nzbeg[rowcnt]++;
								if (rowcnt>=1) {
									nzind[nznext[rowcnt-1]] = rr;
									nzpos[rr] = nznext[rowcnt-1];
									nznext[rowcnt-1]++;
								}
								rownonzeroes[rr]--;
							}


							ss = ucol;
							// Decrement column count

							if ((colcnt = colnonzeroes[ss]) > 0) {
								colpos = nzpos[ss+dim];
								nzind[colpos] = nzind[nzbeg[colcnt]];
								nzpos[nzind[colpos]] = colpos;
								nzbeg[colcnt]++;
								if (colcnt>=1) {
									nzind[nznext[colcnt-1]] = ss+dim;
									nzpos[ss+dim] = nznext[colcnt-1];
									nznext[colcnt-1]++;
								}
								colnonzeroes[ss]--;
							}


							(*totalcancel)++;
							mem[freeind].nextcolofrow = freememind; 
							freememind = freeind; 
						}
						currowind = mem[currowind].nextcolofrow;
						j = mem[currowind].col;
					}
					else {    
						// Produce a fill
						/*
						*  If the memory space for matrix elements is exhausted, we make a
						* reallocation. Then we insert an appropriate new matrix element into
						* the doubly linked structure. Here the |ufront| comes in. Note that
						* |freememind==0| can also mean that we are in the area of free elements
						* not linked so far.
						*/
						if (freememind<freememnolink) {
							meind = freememind;
							freememind = mem[freememind].nextcolofrow;
						}
						else if (freememnolink<maxmatrixentries-5) {
							meind = freememind;
							freememnolink++;
							mem[meind].nextcolofrow = freememnolink;
							freememind = freememnolink;
							mem[freememnolink].nextcolofrow = 0;
						}
						else {
							meind = freememind;
							printf("\nReallocating: old size %d\n",maxmatrixentries);
							maxmatrixentries *= 2;
							printf("              new size %d\n",maxmatrixentries);
							mem = (struct matrixelement *) realloc(mem,maxmatrixentries*sizeof(struct matrixelement));
							if (!mem)// run out of memory

							{
								fprintf (stderr, "run out of memory\n");
								return(ALPS_RUNOUTOFMEMORY);
							}


							;
							freememind = mem[meind].nextcolofrow;
							freememnolink = freememind;
							mem[freememnolink].nextcolofrow = 0;
						}
						mem[meind].row = currow;
						mem[meind].col = ucol;
						xx = -ais*(*rumatcoeff)[uuind];
						mem[meind].coeff = xx;
						if (xx<0.0) xx = -xx;;
						rowsum[currow] += xx;
						colsum[ucol] += xx;
						mem[meind].nextcolofrow = currowind;
						ixx = mem[currowind].prevcolofrow;
						mem[meind].prevcolofrow = ixx;
						mem[ixx].nextcolofrow = meind;
						mem[currowind].prevcolofrow = meind;
						ufrontind = ufront[uind];
						while (mem[ufrontind].row<currow) 
							ufrontind = mem[ufrontind].nextrowofcol;
						mem[meind].nextrowofcol = ufrontind;
						ixx = mem[ufrontind].prevrowofcol;
						mem[meind].prevrowofcol = ixx;
						mem[ixx].nextrowofcol = meind;
						mem[ufrontind].prevrowofcol = meind;
						ufront[uind] = meind;
						rr = currow;
						// Increment row count
						/*
						*  For the management of the list containing row/column indices sorted
						* according to the number of nonzeroes we define a few macros for
						* decrementing and incrementing a count or for removing a row/column.
						* The row (column) index involved has to be given by |rr| (|ss|). Note
						* that |rownonzeroes| and |colnonzeroes| are updated here, and that
						* rows/columns which are zero must not be removed.
						*/
						rowpos = nzpos[rr];
						rowcnt = rownonzeroes[rr];
						lastind = nzind[nznext[rowcnt]-1];
						nzind[rowpos] = lastind;
						nzpos[lastind] = rowpos;
						nznext[rowcnt]--;
						nzbeg[rowcnt+1]--;
						nzind[nzbeg[rowcnt+1]] = rr;
						nzpos[rr] = nzbeg[rowcnt+1];
						rownonzeroes[rr]++;

						ss = ucol;
						// Increment column count

						colpos = nzpos[ss+dim];
						colcnt = colnonzeroes[ss];
						lastind = nzind[nznext[colcnt]-1];
						nzind[colpos] = lastind;
						nzpos[lastind] = colpos;
						nznext[colcnt]--;
						nzbeg[colcnt+1]--;
						nzind[nzbeg[colcnt+1]] = ss+dim;
						nzpos[ss+dim] = nzbeg[colcnt+1];
						colnonzeroes[ss]++;

						(*totalfill)++;
					}
				}
				pivcolind = mem[pivcolind].nextrowofcol;
			}
			pivcolind = mem[colfirstindex[s]].nextrowofcol;
			while (pivcolind!=colfirstindex[s]) {
				leftrow  = mem[pivcolind].prevcolofrow;
				rightrow = mem[pivcolind].nextcolofrow;
				mem[leftrow].nextcolofrow = rightrow;
				mem[rightrow].prevcolofrow = leftrow;
				mem[pivcolind].nextcolofrow = freememind; 
				freememind = pivcolind; 
				pivcolind = mem[pivcolind].nextrowofcol;
			}
			mem[colfirstindex[s]].nextrowofcol = colfirstindex[s];
			mem[colfirstindex[s]].prevrowofcol = colfirstindex[s];
			ss = s;
			// Remove column count

			if ((colcnt = colnonzeroes[ss]) != 0) {
				colpos = nzpos[ss+dim];
				nzind[colpos] = nzind[nzbeg[colcnt]];
				nzpos[nzind[colpos]] = colpos;
				nzbeg[colcnt]++;
				colnonzeroes[ss] = 0;
			}
			(*clmatend)[iter] = lindex;
	}

	/** Generate U-matrix in column format */
	/*
	*  Now we calculate the transpose of the $U$ matrix. The columns of the
	* transpose are stored consecutively in the sequence |pivcol[1]|,
	* |pivcol[2]|, etc. for easier access in BTRAN (not much faster,
	* though). Also for a speed-up of BTRAN we convert the row indices
	* according to the pivot sequence and store the result in |cumatind|.
	* Note that when we arrive here |cumatend| gives the number of nonzeroes
	* for each column. Note also that we store additional pivot information
	* in |cumatind|.
	*/
	nelts = (*rumatend)[dim-1] + 3*dim + 1;
	*cumatind   = (int *) malloc( (nelts) * sizeof(int)); 
	*cumatcoeff = (double *) malloc( (nelts) * sizeof(double)); 
	if (!(*cumatind) || !(*cumatcoeff))// run out of memory

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	(*cumatbeg)[(*pivcol)[0]] = 0;
	rowind[(*pivcol)[0]] = (*cumatbeg)[(*pivcol)[0]];
	(*cumatind)[rowind[(*pivcol)[0]]] = (*pivrow)[0];
	rowind[(*pivcol)[0]]++;
	(*cumatind)[rowind[(*pivcol)[0]]] = (*pivcol)[0];
	rowind[(*pivcol)[0]]++;
	for (j=1; j<dim; j++) {
		int pj = (*pivcol)[j];
		(*cumatbeg)[pj] = (*cumatbeg)[(*pivcol)[j-1]] 
		+ (*cumatend)[(*pivcol)[j-1]] + 3;
		rowind[pj] = (*cumatbeg)[pj];
		(*cumatind)[rowind[pj]] = (*pivrow)[j];
		rowind[pj]++;
		(*cumatind)[rowind[pj]] = pj;
		rowind[pj]++;
	}
	for (r=0; r<dim; r++) {
		for (k=(*rumatbeg)[r];k<(*rumatend)[r];k++) {
			j = (*rumatind)[k];
			(*cumatind)[rowind[j]] = (*pivrow)[r];
			(*cumatcoeff)[rowind[j]] = (*rumatcoeff)[k];
			rowind[j]++;
		}
	}
	for (j=0; j<dim; j++) {
		(*cumatind)[rowind[j]] = -1;
		rowind[j]++;
	}
	(*cumatind)[rowind[(*pivcol)[dim-1]]] = -2;
	for (j=0; j<dim; j++) (*cumatend)[j] = rowind[j] - 1;


	/** Free space for the alps_lufac arrays */
	/*
	*  All local space is freed on normal return. If the function terminates
	* prematurely we do not always care about freeing arrays.
	*/
	ffree( (char **) &mem);
	ffree( (char **) &rownonzeroes);
	ffree( (char **) &colnonzeroes);
	ffree( (char **) &rowfirstindex);
	ffree( (char **) &colfirstindex);
	ffree( (char **) &rowsum);
	ffree( (char **) &colsum);
	ffree( (char **) &rowind);
	ffree( (char **) &colind);
	ffree( (char **) &ufront);
	ffree( (char **) &nzind);
	ffree( (char **) &nzpos);
	ffree( (char **) &nzbeg);
	ffree( (char **) &nznext);

	return 0;
}

} // end namespace ogdf
