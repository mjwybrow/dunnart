/*
 * $Revision: 1.5 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-01-17 10:47:44 +0100 (Thu, 17 Jan 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Globals for preprocessing.
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
//   
// Globals for preprocessing. We need some definitions and structures
// for storing information about preprocessing actions. This is stored in
// an include file.
//

#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_ALPS_PREPROC_H
#define OGDF_ALPS_PREPROC_H

#define PREP_UNKNOWN          100000

#define PREP_ROW_EQUAL        110001
#define PREP_ROW_LESS         110002
#define PREP_ROW_GREATER      110003

#define PREP_ROW_OK           110004
#define PREP_ROW_REDUND       110005
#define PREP_ROW_INFEAS       110006

#define PREP_VAR_BOUNDEDBELOW 120001
#define PREP_VAR_BOUNDEDABOVE 120002
#define PREP_VAR_BOUNDEDTWICE 120003
#define PREP_VAR_FIXED        120004
#define PREP_VAR_FREE         120005

#define PREP_PROB_OK          130001
#define PREP_PROB_INFEAS      130002
#define PREP_PROB_UNSOLVABLE  130003

namespace ogdf {

extern int *org_colnr;     /* original column number of internal variable */
extern int *org_rownr;     /* original row number of internal row */
extern int *new_rownr;     /* new row number of external row */
extern int *new_colnr;     /* new col number of external col */

extern int *prep_rowclass; /* class of row G,L,E */
extern int *prep_rowstat;  /* elimination possible? */
extern int *prep_nrowvars; /* nr of nonfixed vars in row */
extern int *prep_colstat;  /* bound status */
extern double *prep_obj;   /* current objective */
extern double *prep_lowerbound; /* current lower bound */
extern double *prep_upperbound; /* current upper bound */
extern double *prep_rhs; /* current right hand side */

extern int prep_nrfixvars; /* total number of fixed variables */
extern int prep_nrredrows; /* total number of redundant rows */
extern int prep_probstat; /* problem status */

extern double prep_objcorr; /* obj constant */

extern int *prep_rowptr; /* for row format */
extern int *prep_amatrowcount;
extern int *prep_amatrowbeg;
extern int *prep_amatrowind;
extern double *prep_amatrowcoeff;

#define REDUNDANT_ROW 140001
#define INFEASIBLE_ROW 140002
#define FIX_VARIABLE 140003
#define UPPER_BOUND_CHANGE 140004
#define LOWER_BOUND_CHANGE 140005
#define EMBEDDED_SLACK_VAR 140007
#define ROW_SENSE_TO_LESS 140008
#define ROW_SENSE_TO_GREATER 140009

struct prep_struct {
	int action;
	int rowind;
	int colind;
	double val1;
	double val2;
	double val3;
};

extern struct prep_struct *prep_stack;
extern int prep_stacksize;
extern int prep_stackptr;

extern int randseed;

} // end namespace ogdf

#endif
