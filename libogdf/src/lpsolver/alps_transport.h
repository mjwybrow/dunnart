/*
 * $Revision: 1.5 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-01-17 10:47:44 +0100 (Thu, 17 Jan 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Header file.
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
//   Header file |transport.h|. This header contains the main data
//   structure |BASIS| and prototypes of the following functions.
//

#ifdef _MSC_VER
#pragma once
#endif

#ifndef OGDF_ALPS_TRANSPORT_H
#define OGDF_ALPS_TRANSPORT_H

namespace ogdf {

typedef struct _BASIS {

	/* statistics  */
	int pivot;                   /* Number of Pivots */
	int rerooted;               
	int degpivot;                /* Number of degenerated pivots*/
	int no_artificial;           /* Number of artificial arcs */

	/* Definitions for the network */
	int no_nodes;			/* Number of nodes */
	int no_arcs;			/* Numer of arcs */
	int artificial;		/* Index of artificial arcs */

	/* Definitions for the cost matrix (sparse column format)*/
	int m,n;                        /* Dimensions of the matrix */			
	int *colbeg;                  /* Begin of a column */
	int *colcount;                /* Number of nonzeros in a column */
	int *colind;                  /* i-indices of  nonzeros */
	double *cost;                 /* Costs of arcs  (elements of the cost-matrix) */

	double bigM;                  /* Cost of artificial arcs */
	double maxcost;               /* maximum absolut value of cost */
	int *demand;			/* Demands of nodes */

	/* Definition of the basis-spanning-tree XPI-structure */
	int *pred;			/* Predecessor index*/
	int *son;			/* "Oldest" son index*/
	int *brother;			/* Next brother index*/
	int *family;			/* Number of successors in a subtree including the root*/
	int *reverse;			/* Index of arc i-pred[i] in the cost matrix */
	int *upward; /*  1: i-pred[i] is the tree-arc;  0: pred[i]-i is the tree-arc */
	int *flow;			/* Flow of arc i-pred(i); */
	double *dual;			/* Dual variables */
} BASIS;

void   update_dual(BASIS*, int, double); 
int    check_solution(BASIS *basis); 
int    find_cycle(BASIS*,int, int, int, int*);
double find_entering_arc(BASIS*, int*, int*, int*); 
int    init_basis(BASIS*);	
void   init_dual(BASIS *basis);
int    optimize_transport(BASIS*); 
void   print_basis(BASIS*);	
void   print_data(BASIS*);	

} // end namespace ogdf


#endif
