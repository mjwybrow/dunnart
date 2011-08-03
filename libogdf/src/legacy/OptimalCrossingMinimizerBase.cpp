/*
 * $Revision: 1.18 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of class OptimalCrossingMinimizerBase.
 * 
 * This class contains common code between Coin- and Abacus-
 * OptimalCrossingMinimizers
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


#include <ogdf/legacy/OptimalCrossingMinimizerBase.h>
#include <ogdf/planarity/SubgraphPlanarizer.h>

using namespace ogdf;
namespace ogdf_legacy {
	
/*static*/ const double OptimalCrossingMinimizerBase::EPS =  0.00001;
	
void CrossingConfiguration::initDirect(const PlanRep& PG, int crNo) {
	const Graph& orig = PG.original();

//	crossingNo = PG.numberOfNodes() - orig.numberOfNodes();
	crossingNo = crNo;
	crossingEdges.init(orig);
	
	edge e,e2;
	forall_edges(e, orig) {
		ListConstIterator<edge> it = PG.chain(e).begin();
		it++; // jump first
		for(; it.valid(); it++) {
			node dummy = (*it)->source();
			e2 = PG.original(dummy->firstAdj()->theEdge());
			if(e2 == e)
				e2 = PG.original(dummy->lastAdj()->theEdge());
			crossingEdges[e].pushBack(e2);			
		}
	}
}

void CrossingConfiguration::initIndirect(const PlanRep& hpg, int crNo) {
	const PlanRep& prs = (const PlanRep&) hpg.original();
	const GraphCopy& exp = (const GraphCopy&) prs.original();
	const Graph& min = (const Graph&) exp.original();
	
//	crossingNo = hpg.numberOfNodes() - exp.numberOfNodes();
	crossingNo = crNo;
	crossingEdges.init(min);
	
	edge e,e2;
	forall_edges(e, min) {
		ListConstIterator<edge> expit = exp.chain(e).begin();
		for(; expit.valid(); expit++) {
			ListConstIterator<edge> prsit = prs.chain(*expit).begin();
			bool first = true;	
			for(; prsit.valid(); prsit++) {
				ListConstIterator<edge> hpgit = hpg.chain(*prsit).begin();
				hpgit++;
				for(; hpgit.valid(); hpgit++) {
					node dummy = (*hpgit)->source();
					e2 = exp.original(prs.original(hpg.original(dummy->firstAdj()->theEdge())));
					if(e2 == e)
						e2 = exp.original(prs.original(hpg.original(dummy->lastAdj()->theEdge())));
					crossingEdges[e].pushBack(e2);			
				}				
			
				if(first)
					first = false;
				else {
					node dummy = (*prsit)->source();
					e2 = exp.original(prs.original(dummy->firstAdj()->theEdge()));
					if(e2 == e)
						e2 = exp.original(prs.original(dummy->lastAdj()->theEdge()));
					crossingEdges[e].pushBack(e2);			
				}
			}
		}
	}
}
	
	
void OptimalCrossingMinimizerBase::generateMinimizedGraphCostAndForbid(
		const EdgeArray<int>  & _cost,
		const EdgeArray<bool> & _forbid) {
	minimizedGraph = new GraphReduction(*givenGraph);

	numMinNodes = minimizedGraph->numberOfNodes();
	numMinEdges = minimizedGraph->numberOfEdges();
	numMinMaxCrossingPairs = MAX_PAIRS(numMinEdges);
	
	// make cost&forbid
	cost.init(*minimizedGraph);
	costEdge.init(*minimizedGraph);
	forbid.init(*minimizedGraph);
	edge e;
	forall_edges(e, *minimizedGraph) {
		ListConstIterator<edge> it = minimizedGraph->original(e).begin();
		int c = INT_MAX;
		edge cheapest;
		bool f = true;
		for(; it.valid(); it++) {
			if(_cost[*it] < c) {
				c = _cost[*it];
				cheapest = *it;
			}
			if(f) f = _forbid[*it];
		}
		cost[e] = c;
		costEdge[e] = cheapest;
		forbid[e] = f;
	}	
}

void OptimalCrossingMinimizerBase::generateExpandedGraph() {

	expandedGraph = new GraphCopy(*minimizedGraph);

	edge em, e;
	forall_edges(em, *minimizedGraph) {
		e = expandedGraph->chain(em).front(); // should have only one
		int maxCross;
		if(!useCost())
			maxCross = min(upperbound,numMinEdges - e->source()->degree() - e->target()->degree() - 1);
		else
			maxCross = upperbound;
		for(int i = maxCross; i-->1;) // 5 segments => 4 splits
			expandedGraph->split(e);
	}
#ifdef OGDF_DEBUG
	forall_edges(em, *minimizedGraph) {
		node t = 0;
		OGDF_ASSERT( expandedGraph->chain(em).front()->source() == expandedGraph->copy(em->source()) );
		for(ListConstIterator<edge> it = expandedGraph->chain(em).begin(); it.valid(); ++it) {
			if(t) {
				OGDF_ASSERT( (*it)->source() == t);
			}
		}
		OGDF_ASSERT( expandedGraph->chain(em).back()->target() == expandedGraph->copy(em->target()) );
	}
#endif

	numExpNodes = expandedGraph->numberOfNodes();
	numExpEdges = expandedGraph->numberOfEdges();
	numExpMaxCrossingPairs = MAX_PAIRS(numExpEdges);	
}

CrossingConfiguration* OptimalCrossingMinimizerBase::createHeuristicStartSolution() {
	CrossingConfiguration* cInfo = NULL;
	lout() << "Generating " << m_numHeuristics << " heuristic solution(s):";
	int h;
	for(h = m_numHeuristics; h-->0;) {
		PlanRep hpr(*minimizedGraph);		
		hpr.initCC(0); // Achtung: An neue SubgraphPlanarizer Schnittstelle anpassen

		SubgraphPlanarizer sp; // CrossingMinimizationModule
		int crno;
		sp.call(hpr, 0, crno, useCost() ? &cost : 0, useForbid() ? &forbid : 0, 0); //Achtung: Schnittstelle anpassen!!
		lout() << " " << crno;
		if(cInfo && cInfo->getCrossingNo() > crno) {
			delete cInfo;
			cInfo = NULL;
		}
		if(!cInfo) cInfo = new CrossingConfiguration(hpr,crno, true);
	}
	lout() << "\n";
	return cInfo;
}

CrossingConfiguration* OptimalCrossingMinimizerBase::helpCall(PlanRep &_PG,
		int cc,
		const EdgeArray<int>  & _cost,
		const EdgeArray<bool> & _forbid,
		int& crossingNumber) {

	givenGraph = &(_PG.original());
	resultingGraph = &_PG;
	resultingGraph->initCC(cc);

	lout(LL_MINOR) << "Generating MinimizedGraph...\n";
	generateMinimizedGraphCostAndForbid(_cost, _forbid);

	lowerbound = 0;
	if(!useCost())
		upperbound = numMinMaxCrossingPairs; // trivial
	else  {
		int sum = 0;
		edge e;
		forall_edges(e, *minimizedGraph) {
			sum += cost[e];
		}
		upperbound = MAX_PAIRS(sum);
	}
	upperBoundSource = SS_Trivial; // heuristics will always top that, since THIS bound isn't reduced by one...

	lout(LL_MINOR) << "Running Heuristic Start Solutions...\n";
	// get a heuristic solution as a starting point & upper bound
	CrossingConfiguration* cInfo = createHeuristicStartSolution();
 
	calcLowerBounds(); // set ilp bounds..
	calcUpperBounds(); // formulae for special graphs (Kn, Knm)
	
	if(upperbound == 1) {
		cout << "Hey, why do you give me that graph?! non-planar, heuristic needs one crossing... guess what...\n";
		crossingNumber = 1;
		//TODO backtransformation
		return cInfo;
	}
	
	probablyUpdateUpperBound(cInfo->getCrossingNo(), SS_Heuristic, false); 
	
	expansionFactor = upperbound;
	lout(LL_MINOR) << "Generating ExpandedGraph...\n";
	generateExpandedGraph(); // this function needs upperbound set. the tighter the better

	lout() << "Input Graph: |V|=" << givenGraph->numberOfNodes() << ", |E|=" << givenGraph->numberOfEdges() << "\n";
	lout() << "Minimized Graph: |V|=" << minimizedGraph->numberOfNodes() << ", |E|=" << minimizedGraph->numberOfEdges() << "\n";
	lout() << "Expanded Graph (factor=" << expansionFactor << "): |V|=" << expandedGraph->numberOfNodes() << ", |E|=" << expandedGraph->numberOfEdges() << "\n";
	
	sout() /*<< "\t" << givenGraph->numberOfNodes()
	       << "\t" << givenGraph->numberOfEdges()*/
	       << "\t" << minimizedGraph->numberOfNodes()
	       << "\t" << minimizedGraph->numberOfEdges()
	       << "\t" << lowerbound
	       << "\t" << (upperbound+1) << flush;

	return cInfo;
}

void OptimalCrossingMinimizerBase::calcLowerBounds () {
//	int edgecount;
//	if(!useCost())
//		edgecount = numMinEdges;
//	else {
//		edgecount = 0;
//		edge e;
//		forall_edges(e, *minimizedGraph) {
//			edgecount += cost[e];
//		}
//	}
	
	probablyUpdateLowerBound(1, false);
	
	//Euler
	probablyUpdateLowerBound(numMinEdges - 3*numMinNodes + 6, false);
	
	//Pach&Todt
	double ptb = numMinEdges*numMinEdges*numMinEdges/(33.75*numMinNodes*numMinNodes)- 0.9*numMinNodes;
	probablyUpdateLowerBound((int)ceil(ptb), false);
	
	// TODO (later): better lower bounds...
	// based on maxim_um_ planar subgraph... maximum not implemented in OGDF...
}

void OptimalCrossingMinimizerBase::calcUpperBounds () {
	// K_n
	int Kn = ((int)((numMinEdges)/2))*((int)((numMinEdges-1)/2))*((int)((numMinEdges-2)/2))*((int)((numMinEdges-3)/2))/4;
	probablyUpdateUpperBound(Kn, SS_Kn, false);
	
	/*
	// K_{n,m}
	// TODO !!
	int n = ; // aeh...
	int m = ; // aeh...
	int Knm = ((int)((n)/2))*((int)((n-1)/2))*((int)((m)/2))*((int)((m-1)/2));
	probablyUpdateUpperBound(Knm, SS_Knm, false);
	*/
}



} //namespace
