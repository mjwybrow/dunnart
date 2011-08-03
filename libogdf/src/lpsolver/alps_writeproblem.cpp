/*
 * $Revision: 1.2 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief The routine for writing the problem in lp format.
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
#include "alps_misc.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

namespace ogdf {

void removezeros(double number, char *cleannumber)
{
	int i;
	char ci;
	ogdf::sprintf(cleannumber,20,"%lf",number);
	i = -1;
	do ci = cleannumber[++i];
	while (ci!='\0');
	do ci = cleannumber[--i];
	while (ci=='0');
	if (ci=='.') cleannumber[i] = '\0';
	else cleannumber[++i] = '\0';
}


int alps_writeproblem( lpstruct *lp, char *lpfilename)
{
	/** Declare alps_writeproblem scalars */
	int i,j,k,nonz,rowindex,index,notfirst,numprinted;
	double coeff,lowerbound,upperbound;
	FILE *lpfile;
	char ccoeff[20];
	char clowerbound[20];
	char cupperbound[20];

	/** Declare alps_writeproblem arrays */
	int *rowptr;
	int *amatrowcount;
	int *amatrowbeg;
	int *amatrowind;
	double *amatrowcoeff;

	/** Write the LP */
	lpfile = fopen(lpfilename,"w");
	if (!lpfile) {
		printf("ERROR: Could not open the file.\n");
		exit(1002);
	}
	rowptr = (int *) malloc( lp->am * sizeof(int) );
	amatrowcount = (int *) malloc( lp->am * sizeof(int) );
	nonz = 0;
	for (k=0; k<lp->am; k++) amatrowcount[k] = 0;
	for (k=0;k<lp->amatcolbeg[lp->an-1]+lp->amatcolcount[lp->an-1];k++) {
		rowindex = lp->amatcolind[k];
		amatrowcount[rowindex]++;
		nonz++;
	}
	amatrowbeg = (int *) malloc( lp->am * sizeof(int) );
	amatrowbeg[0] = 0;
	rowptr[0] = 0;
	for (k=1; k<lp->am; k++) {
		rowptr[k] = amatrowbeg[k-1] + amatrowcount[k-1];
		amatrowbeg[k] = rowptr[k];
	}
	amatrowind = (int *) malloc( nonz * sizeof(int) );
	amatrowcoeff = (double *) malloc( nonz * sizeof(double) );
	for (j=0; j<lp->an; j++) {
		for (k=lp->amatcolbeg[j]; k<lp->amatcolbeg[j]+lp->amatcolcount[j]; k++) {
			rowindex = lp->amatcolind[k];
			index = rowptr[rowindex];
			amatrowind[index] = j;
			amatrowcoeff[index] = lp->amatcolcoeff[k];
			rowptr[rowindex] = index + 1;
		}
	}
	ffree( (char **) &rowptr);

	if (lp->aobjminmax==ALPS_MAXIMIZE) fprintf(lpfile,"Maximize\n");
	else                               fprintf(lpfile,"Minimize\n");
	fprintf(lpfile,"OBJ: ");
	notfirst = ALPS_FALSE;
	numprinted = 0;
	for (j=0; j<lp->an; j++) {
		coeff = lp->aobj[j];
		if (coeff>0.0) {
			removezeros(coeff,ccoeff);
			if (notfirst) fprintf(lpfile,"+ ");
			if (lp->acolnameind!=NULL) {
				if (coeff==1.0) fprintf(lpfile,"%s ",lp->acolnamestr+lp->acolnameind[j]);
				else            fprintf(lpfile,"%s %s ",ccoeff,lp->acolnamestr+lp->acolnameind[j]);
			}
			else {
				if (coeff==1.0) fprintf(lpfile,"x%1d ",j);
				else            fprintf(lpfile,"%s x%1d ",ccoeff,j);
			}
			numprinted++;
		}
		else if (coeff<0.0) {
			removezeros(-coeff,ccoeff);
			fprintf(lpfile,"- ");
			if (lp->acolnameind!=NULL) {
				if (coeff==-1.0) fprintf(lpfile,"%s ",lp->acolnamestr+lp->acolnameind[j]);
				else             fprintf(lpfile,"%s %s ",ccoeff,lp->acolnamestr+lp->acolnameind[j]);
			}
			else {
				if (coeff==-1.0) fprintf(lpfile,"x%1d ",j);
				else             fprintf(lpfile,"%s x%1d ",ccoeff,j);
			}
			numprinted++;
		}
		notfirst = ALPS_TRUE;
		if (numprinted%6==0) fprintf(lpfile,"\n      ");
	}
	fprintf(lpfile,"\n");
	fprintf(lpfile,"Subject To\n");
	for (i=0; i<lp->am; i++) {
		notfirst = ALPS_FALSE;
		numprinted = 0;
		if (lp->arownameind!=NULL) {
			fprintf(lpfile,"%s: ",lp->arownamestr+lp->arownameind[i]);
		}
		else {
			fprintf(lpfile,"c%1d: ",i);
		}
		for (k=amatrowbeg[i]; k<amatrowbeg[i]+amatrowcount[i]; k++) {
			j = amatrowind[k];
			coeff = amatrowcoeff[k];
			if (coeff>0.0) {
				removezeros(coeff,ccoeff);
				if (notfirst) fprintf(lpfile,"+ ");
				if (lp->acolnameind!=NULL) {
					if (coeff==1.0) fprintf(lpfile,"%s ",
						lp->acolnamestr+lp->acolnameind[j]);
					else            fprintf(lpfile,"%s %s ",
						ccoeff,lp->acolnamestr+lp->acolnameind[j]);
				}
				else {
					if (coeff==1.0) fprintf(lpfile,"x%1d ",j);
					else            fprintf(lpfile,"%s x%1d ",ccoeff,j);
				}
				numprinted++;
			}
			else if (coeff<0.0) {
				removezeros(-coeff,ccoeff);
				fprintf(lpfile,"- ");
				if (lp->acolnameind!=NULL) {
					if (coeff==-1.0) fprintf(lpfile,"%s ",
						lp->acolnamestr+lp->acolnameind[j]);
					else             fprintf(lpfile,"%s %s ",
						ccoeff,lp->acolnamestr+lp->acolnameind[j]);
				}
				else {
					if (coeff==-1.0) fprintf(lpfile,"x%1d ",j);
					else             fprintf(lpfile,"%s x%1d ",ccoeff,j);
				}
				numprinted++;
			}
			notfirst = ALPS_TRUE;
			if (numprinted%6==0) fprintf(lpfile,"\n      ");
		}
		if (lp->aconstrainttype[i]=='L') fprintf(lpfile,"<= ");
		if (lp->aconstrainttype[i]=='E') fprintf(lpfile,"= ");
		if (lp->aconstrainttype[i]=='G') fprintf(lpfile,">= ");
		coeff = lp->arhs[i];
		if (coeff>=0.0) {
			removezeros(coeff,ccoeff);
			fprintf(lpfile,"%s\n",ccoeff);
		}
		else {
			removezeros(-coeff,ccoeff);
			fprintf(lpfile,"-%s\n",ccoeff);
		}
	}
	fprintf(lpfile,"Bounds\n");
	for (j=0; j<lp->an; j++) {
		lowerbound = lp->alowerbound[j];
		upperbound = lp->aupperbound[j];
		if (lowerbound>-ALPS_INT_INFINITY) {
			if (lowerbound>=0.0) removezeros(lowerbound,clowerbound);
			else removezeros(-lowerbound,clowerbound);
		}
		if (upperbound<ALPS_INT_INFINITY) {
			if (upperbound>=0.0) removezeros(upperbound,cupperbound);
			else removezeros(-upperbound,cupperbound);
		}
		if (lowerbound>-ALPS_INT_INFINITY) {
			if (upperbound<ALPS_INT_INFINITY) { /* bounded twice */
				if (lp->acolnameind!=NULL) {
					if (lowerbound>=0.0) fprintf(lpfile,"%s <= %s <= ",
						clowerbound,lp->acolnamestr+lp->acolnameind[j]);
					else fprintf(lpfile,"-%s <= %s <= ",
						clowerbound,lp->acolnamestr+lp->acolnameind[j]);
				}
				else {
					if (lowerbound>=0.0) fprintf(lpfile,"%s <= x%1d <= ",clowerbound,j);
					else fprintf(lpfile,"-%s <= x%1d <= ",clowerbound,j);
				}
				if (upperbound>=0.0) fprintf(lpfile,"%s\n",cupperbound);
				else fprintf(lpfile,"-%s\n",cupperbound);
			}
			else { /* lower bound only */
				if (lp->acolnameind!=NULL) {
					if (lowerbound>=0.0) fprintf(lpfile,"%s >= %s\n",
						lp->acolnamestr+lp->acolnameind[j],clowerbound);
					else fprintf(lpfile,"%s >= -%s\n",
						lp->acolnamestr+lp->acolnameind[j],clowerbound);
				}
				else {
					if (lowerbound>=0.0) fprintf(lpfile,"x%1d >= %s\n",j,clowerbound);
					else fprintf(lpfile,"x%1d >= -%s\n",j,clowerbound);
				}
			}
		}
		else {
			if (upperbound<ALPS_INT_INFINITY) { /* upper bound only */
				if (lp->acolnameind!=NULL) {
					if (upperbound>=0.0) fprintf(lpfile,"%s <= %s\n",
						lp->acolnamestr+lp->acolnameind[j],cupperbound);
					else fprintf(lpfile,"%s <= -%s\n",
						lp->acolnamestr+lp->acolnameind[j],cupperbound);
				}
				else {
					if (upperbound>=0.0) fprintf(lpfile,"x%1d <= %s\n",j,cupperbound);
					else fprintf(lpfile,"x%1d <= -%s\n",j,cupperbound);
				}
			}
			else { /* no bound */
				if (lp->acolnameind!=NULL) {
					fprintf(lpfile,"%s Free",lp->acolnamestr+lp->acolnameind[j]);
				}
				else {
					fprintf(lpfile,"x%1d Free",j);
				}
			}
		}
	}
	fprintf(lpfile,"End\n");
	fclose(lpfile);
	ffree( (char **) &amatrowcount);
	ffree( (char **) &amatrowbeg);
	ffree( (char **) &amatrowind);
	ffree( (char **) &amatrowcoeff);

	return 0;
}

} // end namespace ogdf
