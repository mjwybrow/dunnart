/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-11-27 03:02:28 +1100 (Thu, 27 Nov 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Retrieving the solution.
 * 
 * At present there are two routines for retrieving either 
 * the solution of the internal problem or the solution of the 
 * original problem. For the original problem we only return 
 * variable values and slacks since in the case of a preprocessed
 * problem reduced costs or dual variables might not be available.
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

int alps_getsolution(
					 lpstruct *lp,
					 int /* pril */,
					 int *stat,
					 double *obj,
					 double *xx,
					 double *yy,
					 double *slack,
					 double *rc
					 )
{
	/** Declare alps_getsolution scalars */
	int i,j,row_ind_of_slack_art;

	/** Copy the data */
	*stat = lp->problemstatus;
	if (lp->aobjminmax==ALPS_MAXIMIZE) 
		*obj = lp->objval;
	else
		*obj = -lp->objval;
	for (j=0;j<lp->inorig;j++) {
		xx[j] = lp->x[j];
		rc[j] = lp->reducedcost[j];
	}
	for (j=lp->inorig;j<lp->in;j++) {
		row_ind_of_slack_art = lp->imatcolind[lp->imatcolbeg[j]];
		if (lp->aconstrainttype[row_ind_of_slack_art]=='G') 
			slack[row_ind_of_slack_art] = -lp->x[j];
		else
			slack[row_ind_of_slack_art] = lp->x[j];
	}
	for (i=0;i<lp->am;i++) {
		if (lp->aconstrainttype[i]=='G') 
			yy[i] = -lp->y[i];
		else
			yy[i] = lp->y[i];
	}

	return 0;
}

int alps_getasolution(
					  lpstruct *lp,
					  int /* pril */,
					  int *stat,
					  double *obj,
					  double *xx,
					  double *slack
					  )
{
	int i,j;

	*stat = lp->problemstatus;
	if (lp->aobjminmax==ALPS_MAXIMIZE) 
		*obj = lp->objval;
	else
		*obj = -lp->objval;
	for (j=0;j<lp->an;j++) xx[j] = lp->ax[j];
	for (i=0;i<lp->am;i++) slack[i] = lp->aslack[i];

	return 0;
}

} // end namespace ogdf
