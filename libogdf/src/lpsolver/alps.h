/*
 * $Revision: 1.4 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief The common header.
 * 
 * \author Michael Juenger, Gerhard Reinelt
 * 
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005
 * 
 * \par
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * Version 2 as published by the Free Software Foundation.
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
//   Essential declarations for the LP solver "alps"
//   The common header. The following sections define a common 
//   header file for all implemented functions.
//

#ifdef _MSC_VER
#pragma once
#endif

#undef ALPS_TIMING_ENABLED
#define ALPS_TIMING_ENABLED


//! Ignore Case Comparision of strings.
#ifdef _MSC_VER
#define OGDF_STRICMP _stricmp
#elif defined(__BORLANDC__)
#define OGDF_STRICMP stricmp
#else
#define OGDF_STRICMP strcasecmp
#endif



/** common include files */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include <ogdf/basic/basic.h>

#ifdef __cplusplus
extern "C"
{
#endif

namespace ogdf {

/** some constants */
/*
*  We can disable all timing statistics (except total time) by
* undefining |ALPS_TIMING_ENABLED|.
*/


/*
*  The following constants are convenient.
*/
#define ALPS_FALSE 0
#define ALPS_TRUE 1
#define ALPS_INT_INFINITY 2000000000
#define ALPS_REAL_INFINITY 1.0E20
#define ALPS_CANCELLATIONEPS 1.0E-7
#define ALPS_ZEROEPS 1.0E-7
#define ALPS_EPS 1.0E-7
#define ALPS_SCALINGCONDITION 0.9
#define ALPS_RUNOUTOFMEMORY 8001
#define ALPS_LU_ZEROROW     8002
#define ALPS_LU_ZEROCOL     8003
#define ALPS_LU_NOPIVOT     8004


/** The LP structure */
/*
*  Here is the definition of the LP structure.
*/
// constants

// program status

#define ALPS_SEVERE_ERROR 9999999


// problem status

#define ALPS_UNKNOWN 1040
#define ALPS_INFEASIBLE 1041
#define ALPS_UNBOUNDED 1042
#define ALPS_OPTIMAL 1043
#define ALPS_PREPROCESSONLY 1044


// row status

#define ALPS_ARTIFICIALBASIC 1000
#define ALPS_SLACKBASIC 1001
#define ALPS_SLACKNONBASIC 1002
#define ALPS_ARTIFICIALNONBASIC 1003


// column status

#define ALPS_BASIC 1010
#define ALPS_NONBASICFREE 1011
#define ALPS_NONBASICLOWB 1012
#define ALPS_NONBASICUPB 1013


// variable status

#define ALPS_BOUNDEDBELOW 1015
#define ALPS_BOUNDEDABOVE 1016
#define ALPS_BOUNDEDTWICE 1017
#define ALPS_FIXED 1018
#define ALPS_FREE 1019


// basis status

#define ALPS_NOBASIS 1020
#define ALPS_PRIMALFEASIBLEBASIS 1021
#define ALPS_DUALFEASIBLEBASIS 1022
#define ALPS_OPTIMALBASIS 1023
#define ALPS_PHASE1BASIS 1024
#define ALPS_NOEXTBASISAVAILABLE 1025
#define ALPS_EXTBASISAVAILABLE 1026
#define ALPS_INITBASIS 1027
#define ALPS_TRIANGULAR_INITBASIS 1028
#define ALPS_ARTIFICIAL_INITBASIS 1029


// objective sense

#define ALPS_MINIMIZE 1030
#define ALPS_MAXIMIZE 1031
#define ALPS_NONSENSE 1032


// phaseinfo

#define ALPS_PHASE1 1050
#define ALPS_PHASE2 1051


// boundviolationtypes

#define ALPS_LOWERBOUNDVIOLATION 1
#define ALPS_UPPERBOUNDVIOLATION 2


// pivot selection rules

#define ALPS_EASYPISEL 1060
#define ALPS_STEEPPISEL 1061
#define ALPS_DEGENERATEPISEL 1062
#define ALPS_GREATCHANGEPISEL 1063
#define ALPS_FULLEASYPISEL 1064


// choice of algorithm

#define ALPS_PRIMALSIMPLEX 1070
#define ALPS_DUALSIMPLEX 1071





typedef struct lp_struct{
	// original problem data

	int an; /* number of variables */
	int am; /* number of constraints */
	int *amatcolbeg; /* input matrix columnwise */
	int *amatcolcount; /* input matrix columnwise */
	int *amatcolind; /* input matrix columnwise */
	double *amatcolcoeff; /* input matrix columnwise */
	double *aupperbound; /* upper bounds */
	double *alowerbound; /* lower bounds */
	double *arhs; /* right hand side */
	double *aobj; /* objective function */
	int aobjminmax; /* sense of objective function 1=max, -1=min */
	char *aconstrainttype; /* L,G,E */
	char *acolnamestr; /* column names */
	int *acolnameind; /* column names */
	char *arownamestr; /* row names */
	int *arownameind; /* row names */


	// original problem results

	double *ax; /* primal solution */
	double *aslack; /* slack values */
	int *acolstat; /* column statuses */
	int abasisstatus; /* basis for original problem available?*/


	// scaling information

	int scaling; /* Has the problem been scaled? */
	double *rowscale; /* scaling factors for the rows */
	double *colscale; /* scaling factors for the columns */


	// internal problem data

	int in; /* number of variables */
	int inorig; /* original variables */
	int inonartif; /* original variables + slacks */
	int im; /* number of constraints */
	int inonz; /* number of nonzeros in matrix */
	int *imatcolbeg; /* input matrix columnwise */
	int *imatcolcount; /* input matrix columnwise */
	int *imatcolind; /* input matrix columnwise */
	double *imatcolcoeff; /* input matrix columnwise */
	int *imatrowbeg; /* input matrix row */
	int *imatrowcount; /* input matrix rowwise */
	int *imatrowind; /* input matrix rowwise */
	double *imatrowcoeff; /* input matrix rowwise */
	double *iupperbound; /* current upper bounds */
	double *ilowerbound; /* current lower bounds */
	double *iorigupperbound; /* true upper bounds */
	double *ioriglowerbound; /* true lower bounds */
	int *varstat; /* boundedbelow,boundedabove,boundedtwice,fixed,free */
	int *origvarstat; /* boundedbelow,boundedabove,boundedtwice,fixed,free */
	int *iartifbounds; /* there are temporary artificial bounds */
	double *irhs; /* right hand side */
	double *iobj; /* objective function */
	double *iphase1obj; /* objective function for phase 1*/
	double *iphase2obj; /* objective function for phase 2*/
	double *reducedcost; /* reduced cost*/
	double *u; /* unit vector */
	double *z; /* row of the basis inverse */
	double *zsval; /* row of the basis inverse */
	int *zsind; /* row of the basis inverse */
	int zscount; /* row of the basis inverse */
	double *w; /* row of the transformed matrix */
	double *wsval; /* row of the transformed matrix */
	int *wsind; /* row of the transformed matrix */
	int wscount; /* row of the transformed matrix */


	// row and column status

	int *colstat; /* column status */


	// basis data

	int basisstatus; /* current status of basis */
	int *basisheader; /* <=in : column index / >in : slack/artificial */
	int *basmatbeg; /* basis matrix (columnwise) */
	int *basmatcount; /* basis matrix (columnwise) */
	int *basmatind; /* basis matrix (columnwise) */
	double *basmatcoeff; /* basis matrix (columnwise) */
	int *rumatbeg; /* U matrix (rowwise) */
	int *rumatend; /* U matrix (rowwise) */
	int *rumatind; /* U matrix (rowwise) */
	double *rumatcoeff; /* U matrix (rowwise) */
	int *cumatbeg; /* U matrix (columnwise) */
	int *cumatend; /* U matrix (columnwise) */
	int *cumatind; /* U matrix (columnwise, transformed indices) */
	double *cumatcoeff; /* U matrix (columnwise) */
	int *clmatbeg; /* L matrix (columnwise) */
	int *clmatend; /* L matrix (columnwise) */
	int *clmatind; /* L matrix (columnwise) */
	double *clmatcoeff; /* L matrix (columnwise) */
	int etamax; /* maximum number of eta vectors */
	int *etacol; /* column index */
	int etanr; /* current number of eta vectors */
	int etaclear; /* refactorization frequency */
	int etafirstfree; /* first free index in eta vectors */
	double *etaval; /* eta column entries */
	int *etaind; /* row indices */
	int *etastart; /* as usual */
	int *etacount; /* as usual */
	double *eta; /* special nonzero */
	int *extcolstat; /* column statuses in external basis */
	int *extrowstat; /* row statuses in external basis */


	// primal and dual vectors

	double *x; /* primal solution (internal problem) */
	double *redcost; /* reduced cost */
	double *y; /* dual multipliers */
	double *ysval; /* dual multipliers */
	int *ysind; /* dual multipliers */
	int yscount; /* dual multipliers */
	double *d; /* direction */
	double *dsval; /* direction */
	int *dsind; /* direction */
	int dscount; /* direction */


	// miscellaneous data

	double objval; /* objective value */
	double truephase1objval;
	int problemstatus;
	int phase;
	int nrphase1iter;
	int nrphase2iter;
	int howtopisel;
	int maxconsider;
	int *cand;
	int recoheapcard;
	int *recoheapp;
	int *recoheappindex;
	int numfactor;
	double sumfill;
	int initialbasis;
	int preprocessing;


	// timing data

	int totaltime;
	int factortime;
	int piseltime;
	int recoheaptime;
	int fsolvetime;
	int bsolvetime;
	int ftrantime;
	int btrantime;


} lpstruct;

void alps_readbasis( char *abfilename, int ncols,  int nrows, int *basiccolstat, int *basicrowstat );
int alps_writebasis(lpstruct *lp, char *abfilename );
int alps_loadproblem(lpstruct *lp,int nrows, int ncols, int nonz, int minmax,  double *obj, int *matbeg, int *matcount, 
					 int *matind, double *matcoeff, double *rhs, char *csense, double *lb,
					 double *ub, char *colnamestr, int *colnameind, char *rownamestr, int *rownameind,
					 int ncolstrchars, int nrowstrchars);
int alps_writeproblem(lpstruct *lp, char *lpfilename);

int alps_scaling( int pril, lpstruct *lp);
void printintlpdata(lpstruct *lp);
void printlpdata(lpstruct *lp);
int primalphase1simplex(lpstruct *lp, int pril );
int alps_phase1prim( lpstruct *lp, int pril);
int dualsimplex( lpstruct *lp, int pril);
int alps_prepinit(int pril,lpstruct *lp );
int alps_prepgenprob( int pril, lpstruct *lp);
int alps_preprocess(int pril, lpstruct *lp, int *retcode, int *actions);
int alps_getsolution(lpstruct *lp, int pril, int *stat, double *obj, double *xx, double *yy, double *slack, double *rc);
int alps_lufac ( int pril, int luoption, int dim, int *cmatbeg, int *cmatcount, 
				int *cmatind, double *cmatcoeff, int **pivrow, int **pivcol, 
				double **pivvalue, int **rumatbeg, int **rumatend, int **rumatind, 
				double **rumatcoeff, int **cumatbeg, int **cumatend, int **cumatind,
				double **cumatcoeff, int **clmatbeg, int **clmatend, int **clmatind,
				double **clmatcoeff, int *totalfill, int *totalcancel);
int alps_dualeasypisel( lpstruct *lp, int pril, int *enter, int *leave, int *leaveindex,
					   double *redcost, double *change, int *pivrow, int *pivcol,
					   double *pivvalue, double *oldobjcoeff);
int alps_fsolveeqs( lpstruct *lp, int dim, int *pivrow, int *pivcol, double *pivvalue,
				   int *rumatbeg, int *rumatend, int *rumatind, double *rumatcoeff,
				   int *clmatbeg, int *clmatend, int *clmatind, double *clmatcoeff,
				   int etanr, int *etacol, double *etaval, int *etaind, int *etastart,
				   int *etacount, double *eta, double *b, double *x);
int alps_bsolveeqs( lpstruct *lp, int dim, int *pivrow, int *pivcol, double *pivvalue,
				   int *cumatbeg, int *cumatend, int *cumatind, double *cumatcoeff,
				   int *clmatbeg, int *clmatend, int *clmatind, double *clmatcoeff,
				   int etanr, int *etacol, double *etaval, int *etaind, int *etastart,
				   int *etacount, double *eta, double *c, double *y);
int alps_primalrandompisel( lpstruct *lp, int pril, int *enter, int *leave, 
						   int *leaveindex, double *redcost, double *change,
						   int *pivrow, int *pivcol, double *pivvalue, 
						   double *oldobjcoeff);
int alps_primaleasypisel( lpstruct *lp, int pril, int *enter, int *leave,
						 int *leaveindex, double *redcost, double *change,
						 int *pivrow, int *pivcol, double *pivvalue, 
						 double *oldobjcoeff);
void printprimalsol( lpstruct *lp );
int alps_initproblem( int pril, lpstruct *lp);
int alps_installextbasis( lpstruct *lp, int pril );
int alps_loadbasis( lpstruct *lp, int pril, int *basiccolstat, int *basicrowstat);
int alps_lpinput( int pril, int ignorevoidrows, char *infilename, char **problemname,
				 int *ncols,  int *nrows,  int *minmax, int *nonz,  int **matrowbeg, 
				 int **matrowcount,  int **matrowind,  double **matrowcoeff, 
				 int **matcolbeg,  int **matcolcount,  int **matcolind, 
				 double **matcolcoeff, double **rhs,  char **csense,
				 double **obj,  double **lb,  double **ub, char **colnamestr,
				 int **colnameind,  char **rownamestr,  int **rownameind,
				 int *nignoredrows,  int *ncolstrchars,  int *nrowstrchars);
int alps_optimize(  lpstruct *lp,  int algorithm,  int pril);
int alps_prepundo(  int pril,  lpstruct *lp);
int alps_primalphase1easypisel(  lpstruct *lp,  int pril,  int *enter,
							   int *leave,  int *leaveindex,  double *redcost,
							   double *change,  int *pivrow,  int *pivcol,
							   double *pivvalue,  double *oldobjcoeff,  int *wentfurther);
int alps_primalphase1randompisel(  lpstruct *lp,  int pril,  int *enter,
								 int *leave,  int *leaveindex,  double *redcost,
								 double *change,  int *pivrow,  int *pivcol,
								 double *pivvalue,  double *oldobjcoeff,  int *wentfurther);
int alps_unloadproblem(  lpstruct *lp );
int checkopt(  int pril,  char *probname,  double optval);
void makerecoheap (  lpstruct *lp);
int primalsimplex( lpstruct *lp, int pril);
int removemaxrecoheap ( lpstruct *lp);
void removerecoheap ( lpstruct *lp,  int element);
int cputime();

} // end namespace ogdf


#ifdef __cplusplus
} // extern "C"
#endif
