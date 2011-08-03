/*
 * $Revision: 1.2 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Collection of routines for debugging.
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

#include "alps.h"

namespace ogdf {

/** Check library objective function value */
/*
*  The following routine checks for a given problem name and objective
* function value if this is correct as given in the file {\tt LIB.OPT}
* (which has to be present in the current directory). If the function
* returns |-2| then {\tt LIB.OPT} is not present, if it returns |-1|
* then the optimal value is not available, |0| is returned if the
* relative deviation from the optimum is less than |0.01|, otherwise |1|
* is returned. If available, |trueval| gives the optimum value as given
* in {\tt LIB.OPT}.
*/

#include <stdio.h>
#include <string.h>
#include <ogdf/basic/basic.h>

int checkopt( int pril, char *probname, double optval )
{
	FILE *fd;
	int retcode;
	int nprobs,i,ap,ep;
	char fname[100],pname[100],cc[100];
	double trueval;

	retcode = -1;
	if (!(fd=fopen("LIB.OPT","r"))) { retcode = -2; goto eoc; }
	fscanf(fd,"%d",&nprobs);
	fgets(&cc[0],100,fd);

	ap = 0;

	for (i=0;i<(int) strlen(probname);i++) if (probname[i]=='/') ap = i;

	if (ap) ap++;
	ep = (int) strlen(probname);
	i = ep - 1;
	while (i>0 && probname[i]!='.') i--;
	if (i) ep = i;
	for (i=ap;i<ep;i++) fname[i-ap] = probname[i];
	fname[ep-ap] = '\0';

	for (i=1;i<=nprobs;i++) {
		fscanf(fd,"%s %le",&pname[0],&trueval);
		fgets(&cc[0],100,fd);
		if (!OGDF_STRICMP(fname,pname)) { //KK: statt strcmpi
			if (fabs(optval)>0.0001) {
				if (fabs( (optval-trueval)/optval ) < 0.0001) retcode = 0;
				else retcode = 1;
			}
			else {
				if (fabs(optval-trueval) < 0.0001) retcode = 0;
				else retcode = 1;
			}
			goto eoc;
		}
	}

eoc:
	switch (retcode) {
  case -2:
	  if (pril>0) printf("Objective check: File LIB.OPT not found.\n");
	  break;
  case -1:
	  if (pril>0) printf("Objective check: Value for %1s not found.\n",fname);
	  break;
  case  0:
	  if (pril>0) printf("Objective check: Value for %1s correct.\n",fname);
	  break;
  case  1:
	  printf("Objective check: Value for %1s incorrect (%.6le <> %.6le)!\n",
		  fname,optval,trueval);
	  break;
	}
	return retcode;
}


/** Print original data from LP structure */

void printlpdata( lpstruct *lp )
{
	int i,j,k;

	printf("\n------- LP: original data -------\n");
	printf("Number of variables:            %6d\n", lp->an);
	printf("Number of constraints:          %6d\n", lp->am);
	if(lp->aobjminmax==ALPS_MINIMIZE)
		printf("Objective sense:              ALPS_MINIMIZE\n");
	else printf("Objective sense:              ALPS_MAXIMIZE\n");
	printf("Constraint matrix (columnwise):\n");
	for (j=0;j<lp->an;j++) {
		printf("C%1d:",j);
		for (k=lp->amatcolbeg[j];k<lp->amatcolbeg[j]+lp->amatcolcount[j];k++)
			printf(" %.2lf (%1d)",lp->amatcolcoeff[k],lp->amatcolind[k]);
		printf("\n");
	}
	printf("Right hand side:\n");
	for (i=0;i<lp->am;i++) printf(" %.2lf",lp->arhs[i]);
	printf("\n");
	printf("Constraint types:\n");
	for (i=0;i<lp->am;i++) printf(" %1c",lp->aconstrainttype[i]);
	printf("\n");
	printf("Lower bounds:\n");
	for (j=0;j<lp->an;j++)  
		if (lp->alowerbound[j]>-ALPS_INT_INFINITY) printf(" %.2lf",lp->alowerbound[j]);
		else printf(" -Inf");
		printf("\n");
		printf("Upper bounds:\n");
		for (j=0;j<lp->an;j++)
			if (lp->aupperbound[j]<ALPS_INT_INFINITY) printf(" %.2lf",lp->aupperbound[j]);
			else printf(" +Inf");
			printf("\n");
			printf("Objective:\n");
			for (j=0;j<lp->an;j++)  printf(" %.2lf",lp->aobj[j]);
			printf("\n");
}


/** Print internal data from LP structure */

void printintlpdata(
					lpstruct *lp
					)
{
	int i,j,k;

	printf("\n------- LP: internal data -------\n");
	printf("Number of variables:            %6d\n", lp->in);
	printf("Number of slack variables:      %6d\n", lp->inonartif-lp->inorig);
	printf("Number of artificial variables: %6d\n", lp->in-lp->inonartif);
	printf("Number of constraints:          %6d\n", lp->im);
	if (lp->scaling) {
		printf("Row scaling factors:\n");
		for (i=0;i<lp->im;i++) printf(" %.2lf",lp->rowscale[i]);
		printf("\n");
		printf("Column scaling factors:\n");
		for (j=0;j<lp->inorig;j++) printf(" %.2lf",lp->colscale[j]);
		printf("\n");
	}
	printf("Constraint matrix (columnwise):\n");
	for (j=0;j<lp->in;j++) {
		printf("C%1d:",j);
		for (k=lp->imatcolbeg[j];k<lp->imatcolbeg[j]+lp->imatcolcount[j];k++)
			printf(" %.2lf (%1d)",lp->imatcolcoeff[k],lp->imatcolind[k]);
		printf("\n");
	}
	printf("Constraint matrix (rowwise):\n");
	for (i=0;i<lp->im;i++) {
		printf("R%1d:",i);
		for (k=lp->imatrowbeg[i];k<lp->imatrowbeg[i]+lp->imatrowcount[i];k++)
			printf(" %.2lf (%1d)",lp->imatrowcoeff[k],lp->imatrowind[k]);
		printf("\n");
	}
	printf("Right hand side:\n");
	for (i=0;i<lp->im;i++) printf(" %.2lf",lp->irhs[i]);
	printf("\n");
	printf("Lower bounds:\n");
	for (j=0;j<lp->in;j++)  
		if (lp->ilowerbound[j]>-ALPS_INT_INFINITY) printf(" %.2lf",lp->ilowerbound[j]);
		else printf(" -Inf");
		printf("\n");
		printf("Upper bounds:\n");
		for (j=0;j<lp->in;j++)
			if (lp->iupperbound[j]<ALPS_INT_INFINITY) printf(" %.2lf",lp->iupperbound[j]);
			else printf(" +Inf");
			printf("\n");
			printf("Objective:\n");
			for (j=0;j<lp->in;j++)  printf(" %.2lf",lp->iobj[j]);
			printf("\n");
}


/** Print primal LP solution */

void printprimalsol(
					lpstruct *lp
					)
{
	int j;

	printf("\n------- LP: primal solution -------\n");
	for (j=0;j<lp->in;j++)  {
		if (j<lp->inorig) printf(" x%1d = %.2lf",j,lp->x[j]);
		else {
			printf(" x%1d = %.2lf",j,lp->x[j]);
			if (j<lp->inonartif)
				printf(" (s%1d)",lp->imatcolind[lp->imatcolbeg[j]]);
			else
				printf(" (a%1d)",lp->imatcolind[lp->imatcolbeg[j]]);
		}
		switch (lp->colstat[j]) {
	case ALPS_BASIC:          printf(" ALPS_BASIC"); break;
	case ALPS_NONBASICFREE:   printf(" NB FREE"); break;
	case ALPS_NONBASICLOWB:   printf(" NB LOWB"); break;
	case ALPS_NONBASICUPB:    printf(" NB  UPB"); break;
		}
		printf("\n");
	}
	if (lp->aobjminmax==ALPS_MAXIMIZE) 
		printf("Objective value = %.2lf\n\n",lp->objval);
	else  printf("Objective value = %.2lf\n\n",-lp->objval);
}


/** Check bounds */

int
checkbounds(
			lpstruct *lp,
			int j,
			double x
			)
{
	if (lp->x[j]<lp->ilowerbound[j]-ALPS_EPS) {
		printf("\nLB viol at %1d, %.2le < %.2le (%.4le)",
			j,lp->x[j],lp->ilowerbound[j],x);
		switch (lp->colstat[j]) {
	 case ALPS_BASIC:          printf(" ALPS_BASIC"); break;
	 case ALPS_NONBASICFREE:   printf(" NB FREE"); break;
	 case ALPS_NONBASICLOWB:   printf(" NB LOWB"); break;
	 case ALPS_NONBASICUPB:    printf(" NB  UPB"); break;
		}
		return 1;
	}
	if (lp->x[j]>lp->iupperbound[j]+ALPS_EPS) {
		printf("\nUB viol at %1d, %.2le > %.2le  (%.4le)",
			j,lp->x[j],lp->iupperbound[j],x);
		switch (lp->colstat[j]) {
	 case ALPS_BASIC:          printf(" ALPS_BASIC"); break;
	 case ALPS_NONBASICFREE:   printf(" NB FREE"); break;
	 case ALPS_NONBASICLOWB:   printf(" NB LOWB"); break;
	 case ALPS_NONBASICUPB:    printf(" NB  UPB"); break;
		}
		return 1;
	}
	return 0;
}

} // end namespace ogdf
