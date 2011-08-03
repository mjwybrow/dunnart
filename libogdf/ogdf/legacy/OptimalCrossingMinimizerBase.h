/*
 * $Revision: 1.5 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-15 02:18:10 +1100 (Thu, 15 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class OptimalCrossingMinimizerBase.
 * 
 * This class contains common code between Coin- and Abacus-
 * OptimalCrossingMinimizers.
 * 
 * \author Markus Chimani
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


#ifndef OGDF_OPTIMAL_CROSSING_MINIMIZER_BASE_H
#define OGDF_OPTIMAL_CROSSING_MINIMIZER_BASE_H

#include <ogdf/module/CrossingMinimizationModule.h>
#include <ogdf/basic/Logger.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/EdgeArray.h>
#include <ogdf/basic/GraphCopy.h>
#include <ogdf/graphalg/GraphReduction.h>

using namespace ogdf;
namespace ogdf_legacy {
	
class CrossingConfiguration;

class OptimalCrossingMinimizerBase : public CrossingMinimizationModule, protected Logger {

public:
	OptimalCrossingMinimizerBase() : cost(), costEdge(), forbid() {}
	~OptimalCrossingMinimizerBase() {}

	int numHeuristics() const { return m_numHeuristics; }
	void numHeuristics(int i) { m_numHeuristics = i; }

	int numStartKuratowskis() const { return m_numStartKuratowskis; }
	void numStartKuratowskis(int i) { m_numStartKuratowskis = i; }

	int numCutKuratowskis() const { return m_numCutKuratowskis; }
	void numCutKuratowskis(int i) { m_numCutKuratowskis = i; }

protected:

	static const double EPS /*= 0.00001*/;

	int MAX_PAIRS(int e) {
		return (e*e - e) / 2;
	}

	enum SolutionSource { SS_Trivial, SS_ILP, SS_ILP_Heuristic, SS_Heuristic, SS_Kn, SS_Knm, SS_NoSolution };

	const Graph*           givenGraph;
	GraphReduction*        minimizedGraph;
	GraphCopy*             expandedGraph;
	PlanRep*               resultingGraph;
	EdgeArray<int>         cost;
	EdgeArray<edge>        costEdge;
	EdgeArray<bool>        forbid;

	int m_numHeuristics;
	int m_numStartKuratowskis;
	int m_numCutKuratowskis;

	int numMinNodes;
	int numMinEdges;
	int numMinMaxCrossingPairs;
	int numExpNodes;
	int numExpEdges;
	int numExpMaxCrossingPairs;

	int lowerbound;
	int upperbound;
	SolutionSource upperBoundSource;
	int expansionFactor;
	
	bool probablyUpdateLowerBound(int lb, bool writeILP) {
		if(lb <= lowerbound) return false;
		lout(LL_MINOR) << "New Lower Bound: " << lb << "\n";
		lowerbound = lb;
		if(writeILP) writeLowerBoundToILP();
		return true;    
	}
	bool probablyUpdateUpperBound(int ub, SolutionSource ubs, bool writeILP) {
		ub--; // only something better than that is interesting...
		if(ub >= upperbound) return false;
		lout(LL_MINOR) << "New Upper Bound: " << ub << "\n";
		upperbound = ub;
		upperBoundSource = ubs;
		if(writeILP) writeUpperBoundToILP();
		return true;      
	}
	virtual void writeLowerBoundToILP(){};
	virtual void writeUpperBoundToILP() {};
	
	void calcLowerBounds();
	void calcUpperBounds();	
	
	//! \reminder OptimalCrossingMimimizerBase assumes that the given graph is connected	
	CrossingConfiguration* helpCall(PlanRep &_PG, int cc, const EdgeArray<int>& _cost, const EdgeArray<bool> & _forbid, int& crossingNumber);
	void generateMinimizedGraphCostAndForbid(const EdgeArray<int>  &_cost, const EdgeArray<bool> &_forbid);
	void generateExpandedGraph();
	CrossingConfiguration* createHeuristicStartSolution();
	
};


class CrossingConfiguration {
public:
	CrossingConfiguration() : crossingNo(-1), crossingEdges() {}
	CrossingConfiguration(const PlanRep& PG, int crNo, bool direct) : crossingNo(crNo), crossingEdges() { 
		if(direct) initDirect(PG, crNo);
		else initIndirect(PG, crNo);
	}
	void initDirect(const PlanRep& PG, int crNo);
	void initIndirect(const PlanRep& PG, int crNo);
	int getCrossingNo() const { return crossingNo; }
	const List<edge>& getCrossingEdges(edge e) const {
		return crossingEdges[e];
	}
	
private:
	int crossingNo;
	EdgeArray< List<edge> > crossingEdges;
};

} //namespace

#endif /*OGDF_OPTIMAL_CROSSING_MINIMIZER_BASE_H*/
