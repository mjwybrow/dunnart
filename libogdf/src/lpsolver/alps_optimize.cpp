/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of Optimize.
 * 
 * The layout of this routine is as follows. First, 
 * |alps_initproblem| is called in order to set up the internal
 * problem definition and do scaling and preprocessing. Then 
 * |alps_phase1prim| prepares the alps_auxiliary LP that is 
 * subsequently solved by |primalsimplex| in order to obtain a 
 * primally feasible basis. Then the original objective function 
 * is restored and |primalsimplex| is called again to find the
 * optimum solution.
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
*  Here we perform the simplex algorithm.
*/

#include "alps.h"
#include "alps_misc.h"
#include "alps_preproc.h"

#define ALPS_INFEAS_EPS (1000.0 * ALPS_EPS)

namespace ogdf {

int alps_optimize(
				  lpstruct *lp,
				  int algorithm,
				  int pril
				  )
{
	int maxviolindex;
	int status;
	int truepisel;
	int i,j,k;
	double maxviol,viol;
	double *trhs;

	if (lp->etaclear>lp->etamax) {
		lp->etaclear = lp->etamax;
	}
	if (lp->etaclear<1) {
		lp->etaclear = 1;
	}
	if (lp->maxconsider<1) {
		lp->maxconsider = 1;
	}

	if (pril>=4) printlpdata(lp);

	status = alps_initproblem(pril,lp);
	if (status==ALPS_PREPROCESSONLY) {
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
		return ALPS_PREPROCESSONLY;
	}

	if (pril>=4) printintlpdata(lp);

	if (lp->basisstatus==ALPS_EXTBASISAVAILABLE) {
		status = alps_installextbasis(lp,pril);
		lp->problemstatus = ALPS_UNKNOWN;
		lp->phase = ALPS_PHASE2;
		// WATCH OUT (Matthias Elf) :
		// up to now truepisel hasn't been set.
		// Thus, I commented out setting the pisel method
		// and hope that it had been set earlier.
		//
		// lp->howtopisel = truepisel;
		if (algorithm==ALPS_PRIMALSIMPLEX) {
			if (pril>=1) printf("\nPhase 2 (primal)\n\n");
			status = primalsimplex(lp,pril);
			if (status==ALPS_LU_NOPIVOT) {
				lp->problemstatus = ALPS_UNKNOWN;
				return status;
			}
			if (pril>=1) {
				if (lp->problemstatus==ALPS_UNBOUNDED) 
					if (pril>=1) printf("\n\nUNBOUNDED\n\n");
				else {
					printf("\n\nOPTIMAL\n\n");
					if (pril>=4) printprimalsol(lp);
				}
			}
		}
		else {
			if (pril>=1) printf("\nPhase 2 dual\n\n");
			status = dualsimplex(lp,pril);
			if (status==ALPS_LU_NOPIVOT) {
				lp->problemstatus = ALPS_UNKNOWN;
				return status;
			}
			if (pril>=1) {
				if (lp->problemstatus==ALPS_INFEASIBLE) printf("\n\nINFEASIBLE\n\n");
				else {
					printf("\n\nOPTIMAL\n\n");
					if (pril>=4) printprimalsol(lp);
				}
			}
		}
	}
	else {
		if (pril>=1) printf("\nPhase 1 (primal)\n\n");
		lp->phase = ALPS_PHASE1;
		if (pril>=3) printf("\n==> Phase 1: Initialization\n\n");
		truepisel = lp->howtopisel;
		lp->howtopisel = ALPS_FULLEASYPISEL;
		status = alps_phase1prim(lp,pril);

		if (pril>=3) printf("\n==> Phase 1: Primal Simplex\n\n");
		status = primalphase1simplex(lp,pril);

		if (status==ALPS_LU_NOPIVOT) {
			lp->problemstatus = ALPS_UNKNOWN;
			return status;
		}

		if (lp->problemstatus==ALPS_UNBOUNDED) {
			if (pril>=1) printf("\n\nUNBOUNDED\n\n");
			goto hell;
		}
		else if (lp->objval<-ALPS_INFEAS_EPS) {

			lp->problemstatus=ALPS_INFEASIBLE;

			if (pril>=1) {
				printf("\n\nINFEASIBLE\n");
				printf("Objective = %.8le\n\n",lp->objval);
			}
			if (pril>=4) {
				printf("im = %d, inorig = %d, inonartif = %d, in = %d\n\n",
					lp->im,lp->inorig,lp->inonartif,lp->in);
				for (j=0; j<lp->in; j++) {
					if (lp->iphase1obj[j]!=0.0)
						printf("j: %d, x: %lf, objc: %lf stat= %d, lb= %lf, ub= %lf\n\n",
						j,lp->x[j],lp->iphase1obj[j],lp->colstat[j],
						lp->ilowerbound[j],lp->iupperbound[j]);
				}
			}

			goto hell;
		}

		for (k=0; k<lp->im; k++) { /* restore bounds */
			j = lp->basisheader[k];
			lp->varstat[j] = lp->origvarstat[j];
			if (j>=lp->inorig) {
				lp->ilowerbound[j] = 0.0;
				if (j>=lp->inonartif) lp->iupperbound[j] = 0.0;
				else lp->iupperbound[j] = ALPS_REAL_INFINITY;
			}
			else if (lp->iartifbounds[j]) {
				lp->iupperbound[j] = lp->iorigupperbound[j];
				lp->ilowerbound[j] = lp->ioriglowerbound[j];
				lp->iartifbounds[j] = ALPS_FALSE;
			}
		}

		lp->problemstatus=ALPS_UNKNOWN;
		if (pril>=1) printf("\nPhase 2 (primal)\n\n");
		lp->phase = ALPS_PHASE2;
		lp->howtopisel = truepisel;
		if (pril>=3) printf("\n==> Phase 2: Primal Simplex\n\n");
		lp->iobj = lp->iphase2obj;
		lp->objval = 0.0;
		for (j=0;j<lp->in;j++) {
			lp->objval += lp->x[j]*lp->iobj[j];
		}
		if (pril>=4) printprimalsol(lp);
		status = primalsimplex(lp,pril);
		if (status==ALPS_LU_NOPIVOT) {
			lp->problemstatus = ALPS_UNKNOWN;
			return status;
		}
		if (lp->problemstatus==ALPS_UNBOUNDED) 
			if (pril>=1) printf("\n\nUNBOUNDED\n\n");
		else {
			if (pril>=1) printf("\n\nOPTIMAL\n\n");
			if (pril>=4) printprimalsol(lp);
		}
	}

hell:
	if (pril>=1) {
		printf("%5d iterations in phase 1.\n",lp->nrphase1iter);
		printf("%5d iterations in phase 2.\n\n",lp->nrphase2iter);
	}
	/** Unscale the solution */
	/*
	*  If scaling was enabled the solution must be unscaled. Although not
	* necessary, we reevaluate the objective function value.
	*/
	if (lp->scaling) {
		for (j=0;j<lp->inorig;j++) {
			lp->x[j] /= lp->colscale[j];
		}
	}

	// Unscaling the dual solution has been missing
    // Matthias Elf - 16.07.2004
	// BEGIN change
	if (lp->scaling) {
		for (j=0;j<lp->im;j++) {
			lp->y[j] /= lp->rowscale[j];
		}
	}
	if (lp->aobjminmax==ALPS_MINIMIZE) {
		for (j=0;j<lp->im;j++) {
			lp->y[j] = - lp->y[j] ;
		}
	}
    // END change

	lp->objval = 0.0;
	if (lp->preprocessing) {
		for (j=0;j<lp->inorig;j++) {
			lp->objval += lp->x[j]*lp->aobj[org_colnr[j]];
		}
	}
	else {
		for (j=0;j<lp->inorig;j++) {
			lp->objval += lp->x[j]*lp->aobj[j];
		}
	}
	if (lp->aobjminmax==ALPS_MINIMIZE) lp->objval =- lp->objval;  


	if (!lp->preprocessing) {
		for (j=0;j<lp->an;j++) lp->ax[j] = lp->x[j];
		for (j=0;j<lp->an+lp->am;j++) lp->acolstat[j] = lp->colstat[j];
		lp->abasisstatus = lp->basisstatus;
		for (j=lp->inorig;j<lp->in;j++) {
			i = lp->imatcolind[lp->imatcolbeg[j]];
			if (lp->aconstrainttype[i]=='G') 
				lp->aslack[i] = -lp->x[j];
			else
				lp->aslack[i] = lp->x[j];
		}
		/** Determine maximum infeasibility */
		/*
		*  How feasible is the solution? It is unclear which differences should
		* be considered as strong violations. This cannot be measured in
		* absolute value.
		*/
		if (pril>=1) printf("\n*** Feasibility check on original problem data ***\n");
		maxviol = 0.0;
		for (j=0;j<lp->an;j++) {
			viol = lp->ax[j] - lp->aupperbound[j];
			if (viol>0.0001) {
				if (viol>maxviol) maxviol = viol;
				if (pril>=1)
					printf("  Upper bound violation (%.4lf>%.4lf) of variable %d: (%s)!\n",
					lp->ax[j],lp->aupperbound[j],j,lp->acolnamestr+lp->acolnameind[j]);
			}
			viol = lp->alowerbound[j] - lp->ax[j];
			if (viol>0.0001) {
				if (viol>maxviol) maxviol = viol;
				if (pril>=1)
					printf("  Lower bound violation (%.4lf<%.4lf) of variable %d (%s)!\n",
					lp->ax[j],lp->alowerbound[j],j,lp->acolnamestr+lp->acolnameind[j]);
			}
		}
		if (pril>=1) printf("\n  Maximum bound violation      : %.4le\n",maxviol);
		if (maxviol>0.0001) 
			if (pril>=1) printf("\n\n  Strong bound violation!\n\n");

		if (pril>=1) printf("\n");
		maxviol = 0.0;
		trhs = (double *) malloc(lp->am * sizeof(double));
		for (i=0; i<lp->am; i++) trhs[i] = lp->arhs[i];
		for (j=0;j<lp->an;j++) {
			for (k=lp->amatcolbeg[j]; k<lp->amatcolbeg[j]+lp->amatcolcount[j]; k++) {
				i = lp->amatcolind[k];
				trhs[i] -=  lp->ax[j]*lp->amatcolcoeff[k];
			}
		}
		for (i=0;i<lp->am;i++) { 
			viol = trhs[i];
			if (lp->aconstrainttype[i]=='E') {
				if (viol<0.0) viol = -viol;
			}
			else if (lp->aconstrainttype[i]=='L') {
				if (viol<0.0) viol = -viol; else viol = 0.0;
			}
			else {
				if (viol<0.0) viol = 0.0;    
			}  
			if (viol>0.001) {
				if (pril>=1) {
					if(lp->arownamestr)
						printf("  Constraint violation (%.4lf) in constraint %d (%s)!\n",
							viol,i,lp->arownamestr+lp->arownameind[i]);
					else
						printf("  Constraint violation (%.4lf) in constraint %d (NA)!\n",
							viol,i);
				}
			}
			if (viol>maxviol) {
				maxviol = viol;
				maxviolindex = i;
			}
		}
		ffree((char **) &trhs);
		if (pril>=1) printf("\n  Maximum constraint violation : %.4le\n",maxviol);
		if (maxviol>0.001) 
			if (pril>=1) printf("\n\n  Strong constraint violation!\n");


	}
	else {
		alps_prepundo(pril,lp);
		/** Determine maximum infeasibility */
		/*
		*  How feasible is the solution? It is unclear which differences should
		* be considered as strong violations. This cannot be measured in
		* absolute value.
		*/
		if (pril>=1) printf("\n*** Feasibility check on original problem data ***\n");
		maxviol = 0.0;
		for (j=0;j<lp->an;j++) {
			viol = lp->ax[j] - lp->aupperbound[j];
			if (viol>0.0001) {
				if (viol>maxviol) maxviol = viol;
				if (pril>=1)
					printf("  Upper bound violation (%.4lf>%.4lf) of variable %d: (%s)!\n",
					lp->ax[j],lp->aupperbound[j],j,lp->acolnamestr+lp->acolnameind[j]);
			}
			viol = lp->alowerbound[j] - lp->ax[j];
			if (viol>0.0001) {
				if (viol>maxviol) maxviol = viol;
				if (pril>=1)
					printf("  Lower bound violation (%.4lf<%.4lf) of variable %d (%s)!\n",
					lp->ax[j],lp->alowerbound[j],j,lp->acolnamestr+lp->acolnameind[j]);
			}
		}
		if (pril>=1) printf("\n  Maximum bound violation      : %.4le\n",maxviol);
		if (maxviol>0.0001) 
			if (pril>=1) printf("\n\n  Strong bound violation!\n\n");

		if (pril>=1) printf("\n");
		maxviol = 0.0;
		trhs = (double *) malloc(lp->am * sizeof(double));
		for (i=0; i<lp->am; i++) trhs[i] = lp->arhs[i];
		for (j=0;j<lp->an;j++) {
			for (k=lp->amatcolbeg[j]; k<lp->amatcolbeg[j]+lp->amatcolcount[j]; k++) {
				i = lp->amatcolind[k];
				trhs[i] -=  lp->ax[j]*lp->amatcolcoeff[k];
			}
		}
		for (i=0;i<lp->am;i++) { 
			viol = trhs[i];
			if (lp->aconstrainttype[i]=='E') {
				if (viol<0.0) viol = -viol;
			}
			else if (lp->aconstrainttype[i]=='L') {
				if (viol<0.0) viol = -viol; else viol = 0.0;
			}
			else {
				if (viol<0.0) viol = 0.0;    
			}  
			if (viol>0.001) {
				if (pril>=1) 
					printf("  Constraint violation (%.4lf) in constraint %d (%s)!\n",
					viol,i,lp->arownamestr+lp->arownameind[i]);
			}
			if (viol>maxviol) {
				maxviol = viol;
				maxviolindex = i;
			}
		}
		ffree((char **) &trhs);
		if (pril>=1) printf("\n  Maximum constraint violation : %.4le\n",maxviol);
		if (maxviol>0.001) 
			if (pril>=1) printf("\n\n  Strong constraint violation!\n");
		if (pril>=1) printf("\n");
	}
	return 0;
}

} // end namespace ogdf
