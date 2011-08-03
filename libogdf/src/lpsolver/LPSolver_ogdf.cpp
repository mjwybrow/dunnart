/*
 * $Revision: 1.2 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Implements front-end for LP solver
 * 
 * \author Carsten Gutwenger
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


#include <ogdf/basic/basic.h>

#ifdef OGDF_OWN_LPSOLVER

#include <ogdf/internal/lpsolver/LPSolver_ogdf.h>
#include "alps.h"
#include "alps_misc.h"


namespace ogdf {

LPSolver::LPSolver()
{
}


double LPSolver::infinity() const
{
	return ALPS_REAL_INFINITY;
}


LPSolver::Status LPSolver::optimize(
	OptimizationGoal goal,        // goal of optimization (minimize or maximize)
	Array<double> &obj,           // objective function vector
	Array<int>    &matrixBegin,   // matrixBegin[i] = begin of column i
	Array<int>    &matrixCount,   // matrixCount[i] = number of nonzeroes in column i
	Array<int>    &matrixIndex,   // matrixIndex[n] = index of matrixValue[n] in its column
	Array<double> &matrixValue,	  // matrixValue[n] = non-zero value in matrix
	Array<double> &rightHandSide, // right-hand side of LP constraints
	Array<char>   &equationSense, // 'E' ==   'G' >=   'L' <=
	Array<double> &lowerBound,    // lower bound of x[i]
	Array<double> &upperBound,    // upper bound of x[i]
	double &optimum,              // optimum value of objective function (if result is lpOptimal)
	Array<double> &x              // x-vector of optimal solution (if result is lpOptimal)
)
{
	const int numRows = rightHandSide.size();
	const int numCols = obj.size();
	const int numNonzeroes = matrixIndex.size();

	// assert correctness of array boundaries
	OGDF_ASSERT(obj          .low() == 0 && obj          .size() == numCols);
	OGDF_ASSERT(matrixBegin  .low() == 0 && matrixBegin  .size() == numCols);
	OGDF_ASSERT(matrixCount  .low() == 0 && matrixCount  .size() == numCols);
	OGDF_ASSERT(matrixIndex  .low() == 0 && matrixIndex  .size() == numNonzeroes);
	OGDF_ASSERT(matrixValue  .low() == 0 && matrixValue  .size() == numNonzeroes);
	OGDF_ASSERT(rightHandSide.low() == 0 && rightHandSide.size() == numRows);
	OGDF_ASSERT(equationSense.low() == 0 && equationSense.size() == numRows);
	OGDF_ASSERT(lowerBound   .low() == 0 && lowerBound   .size() == numCols);
	OGDF_ASSERT(upperBound   .low() == 0 && upperBound   .size() == numCols);
	OGDF_ASSERT(x            .low() == 0 && x            .size() == numCols);


	// load problem into lpstruct
	lpstruct lp;
	alps_loadproblem(&lp, numRows, numCols, numNonzeroes,
		(goal == lpMinimize) ? ALPS_MINIMIZE : ALPS_MAXIMIZE, &obj[0],
		&matrixBegin[0], &matrixCount[0], &matrixIndex[0], &matrixValue[0],
		&rightHandSide[0], &equationSense[0],
		&lowerBound[0], &upperBound[0],
		0, 0, 0, 0, 0, 0);

	// optimize LP
	alps_optimize(&lp, ALPS_PRIMALSIMPLEX, 0);

	// retrieve solution
	int stat;
	double *y     = new double[numRows];
	double *slack = new double[numRows];
	double *dj    = new double[numCols];
	alps_getsolution(&lp, 0, &stat, &optimum, &x[0], y, slack, dj);

	delete [] y;
	delete [] slack;
	delete [] dj;

	Status status;
	switch(stat) {
		case ALPS_INFEASIBLE:
			status = lpInfeasible; break;
		case ALPS_UNBOUNDED:
			status = lpUnbounded; break;
		default:
			status = lpOptimal;
	}

	return status;
}

} // end namespace ogdf

#endif
