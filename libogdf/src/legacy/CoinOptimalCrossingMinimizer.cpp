/*
 * $Revision: 1.10 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Implements class CoinOptimalCrossingMinimizer.
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

#ifdef USE_COIN

#include <math.h>

#include <ogdf/legacy/CoinOptimalCrossingMinimizer.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/Array.h>
#include <ogdf/basic/List.h>
#include <ogdf/planarity/planarity/SubgraphPlanarizer.h>

#include <OsiRowCut.hpp>
#include <OsiCpxSolverInterface.hpp>

using namespace ogdf;
namespace ogdf_legacy {


void CoinOptimalCrossingMinimizer::setDefaultSettings() {
	m_createVariables = VC_BeforeBnC;
	m_heuristicCallback = true;
	m_numHeuristics = 20;
	m_numStartKuratowskis = 30;
	m_numCutKuratowskis = 10;
	m_usePerturbation = true;
}
	
void CoinOptimalCrossingMinimizer::prepareILP() {
	// init edge lookups / numbering scheme
	edge2id.init(*expandedGraph);
	id2edge.init(numExpEdges);
	pair2ilp.init(/*numExpMaxCrossingPairs*/numExpEdges*numExpEdges); // MINOR-TODO: its possible to shrink by a better pair-calculation-scheme
	ilp2pair.init(numExpMaxCrossingPairs); // MINOR-TODO: start smaller and increase as necessary

	edge e1,e2;
	int i = 0;
	forall_edges(e1, *expandedGraph) {
		id2edge[i] = e1;
		edge2id[e1] = i++;
	}
	forall_validpairs(e1, e2, *expandedGraph) {
		pair2ilp[pairOrdered(e1,e2)] = UNASSIGNED;
	}

	// Generate OSI
	posi = CoinManager::createCorrectOsiSolverInterface(this, CT_Cut | CT_Incumbent | (m_heuristicCallback ? CT_Heuristic : 0));
	posi->setObjSense(1); // minimize
	posi->messageHandler()->logLevel(2);
	
	OsiCpxSolverInterface* x = (OsiCpxSolverInterface*) posi;
	CPXENVptr envptr = x->getEnvironmentPtr();
	int param;
	CPXsetintparam( envptr,	CPX_PARAM_COLGROWTH, numExpMaxCrossingPairs);
	CPXsetintparam( envptr,	CPX_PARAM_NZGROWTH, numExpMaxCrossingPairs);
	CPXsetintparam( envptr,	CPX_PARAM_ROWGROWTH, 2000);
	
	CPXsetintparam( envptr, CPX_PARAM_BRDIR, CPX_BRDIR_UP); //

//	CPXsetintparam( envptr,	CPX_PARAM_IMPLBD, -1); //

//	CPXsetintparam( envptr,	CPX_PARAM_BARDISPLAY, 2);
//	CPXsetintparam( envptr,	CPX_PARAM_SIFTDISPLAY, 2);
//	CPXsetintparam( envptr,	CPX_PARAM_SIMDISPLAY, 2);
//	CPXsetintparam( envptr,	CPX_PARAM_SCRIND, 1);
	CPXsetintparam( envptr,	CPX_PARAM_MIPDISPLAY, 3);
	CPXsetintparam( envptr,	CPX_PARAM_MIPINTERVAL, 1);

	// difference between two different solutions is at least 1 (minus the preturbation stuff)
	CPXsetdblparam( envptr, CPX_PARAM_EPAGAP, 1 - getPerturbationError() - EPS);

	// Add simplicity constraints => one crossing per edge in expanded graph
	CoinPackedVector empty;
	for(int i = numExpEdges; i-->0;) {
		posi->addRow(empty, 'L', 1, 0);
	}

	// BOUNDS constraints
	posi->addRow(empty, 0, 0); // sum vars into objVal-variable
	OBJECTIVE_ROW = numExpEdges;
	CoinPackedVector objective;
	objective.insert(OBJECTIVE_ROW, -1);
	posi->addCol(objective, lowerbound, upperbound + getPerturbationError(), 1);
	OBJECTIVE_COL = 0;
	if(!usePerturbation())
		posi->setInteger(0); // not if we perturbate
	numILPCrossingPairs = 1; // well, I added one, right?
}

void CoinOptimalCrossingMinimizer::shadowVariables() {
	node n;
	edge e,ee;

	//no two adjacent edges cross each other, nor should an edge cross with itself
	forall_nodes(n, *minimizedGraph) {
		forall_adj_edges(e, n) {
			forall_adj_edges(ee, n) {
				forbidCrossings(ee, e, SHADOWED);
			}
		}
	}
	
	// forbidden edges
	if(useForbid()) {
		forall_edges(e, *minimizedGraph) {
			if(forbid[e]) {
				forall_edges(ee, *minimizedGraph) {
					if(forbid[ee])
						forbidCrossings(e,ee, SHADOWED);
				}
			}
		}
	}
}

void CoinOptimalCrossingMinimizer::cleanUp() {
	delete posi;
	kuratowskiConstraints.clear(true); // kill all entries
}


Module::ReturnType CoinOptimalCrossingMinimizer::doCall(PlanRep &_PG,
		int cc,
		const EdgeArray<int>  & _cost,
		const EdgeArray<bool> & _forbid,
		const EdgeArray<unsigned int>  &subgraphs,
		int& crossingNumber) {

	kuratowskiConstraints.clear(); // ensure empty

	CrossingConfiguration* cInfo = helpCall(_PG, cc, _cost, _forbid, crossingNumber);
	
	prepareILP();
	
	shadowVariables();

	// generate variables
	if(m_createVariables == VC_Initially) ensureAllVariables();

	// initialize
	double* initsol = createSolutionVector<double>(*cInfo);
	posi->setColSolution(initsol);
	
	PlanRep* PR = new PlanRep(*expandedGraph);
	PR->initCC(0);
	
	lout() << "Generating Initial Kuratowski Constraints:\n";
	for(int h = m_numStartKuratowskis; h-->0;) {
		List<int> K;
		findKuratowski(*PR,K);
		ensureConstraint(K,0,NULL,NULL,NULL);
	}	
	delete PR;
	lout() << "Fasten your seatbelt! Starting to solve...\n";
	
	int objValue;
	int* sol;
	SolutionSource ss = solve(objValue, &sol);

	switch(ss) {
		case SS_ILP:
			lout() << "The ILP found a solution better than the best start heuristic.\n";
		break;
		case SS_Heuristic:
			lout() << "The ILP proved that the best start heuristic was already optimal.\n";
			objValue = cInfo->getCrossingNo();
			sol = createSolutionVector<int>(*cInfo);
		break;
		case SS_Kn:
			lout() << "The ILP proved that the formula for K_n induces the optimal solution.\n";
			lout() << "No according transformation is implemented. Program will crach soon...\n";
			objValue = upperbound + 1;
		break;
		case SS_Knm:
			lout() << "The ILP proved that the formula for K_{n,m} induces the optimal solution.\n";
			lout() << "No according transformation is implemented. Program will crach soon...\n";
			objValue = upperbound + 1;
		break;
		case SS_NoSolution:
			THROW_PARAM (AlgorithmFailureException, afcNoSolutionFound);
		break;
		default: // note that SS_TRIVIAL will not occur, since heuristic beats it automatically (trivial was 1 too high)
			THROW_PARAM (AlgorithmFailureException, afcUnknown); // unknown solution source
	}

	delete cInfo;
	
	lout() << "********************************************************\n";
	lout() << "* Computation successful!\n";
	lout() << "* Computed crossing number: " << objValue << "\n";
	lout() << "********************************************************\n";
	
	crossingNumber = objValue;
	
	lout() << ">>> Generation of resulting graph isn't implemented yet!\n";

	// setResultingGraph(sol); // TODO
	
	delete[] sol;
	cleanUp();
};

template<class T> T* CoinOptimalCrossingMinimizer::createSolutionVector(const CrossingConfiguration& ci, T* initsolution /*= NULL*/) {
	lout() << "Initializing ILP's solution vector with best heuristic solution.\n";
	
	if(!initsolution)
		initsolution = new T[numExpMaxCrossingPairs];

	for(int i = numILPCrossingPairs; i-->0;) // only these plus addt'l new ones will be read. all new vars will be set to 1
		initsolution[i] = 0;
		
	initsolution[OBJECTIVE_COL] = ci.getCrossingNo();

	edge e1,e2;
	forall_edges(e1, *minimizedGraph) {
		ListConstIterator<edge> it = ci.getCrossingEdges(e1).begin();
		for(int i1 = 0; it.valid(); it++, i1++) {
			e2 = *it;
			ListConstIterator<edge> it2 = ci.getCrossingEdges(e2).begin();
			int i2 = 0;
			for (; *it2 != e1; it2++, i2++); // we know it exists. hence no check with "it2.valid()"
			
			if(i1 == expandedGraph->chain(e1).size()) {
				lout() << "Note: Initial solution can not be represented in reduced ILP. Using Double-Crossing Emulation.\n";
				i1--;				
			}
			if(i2 == expandedGraph->chain(e2).size()) {
				lout() << "Note: Initial solution can not be represented in reduced ILP. Using Double-Crossing Emulation.\n";
				i2--;
			}
			
			edge ea = *((expandedGraph->chain(e1)).get(i1));
			edge eb = *((expandedGraph->chain(e2)).get(i2));
			
			int pair = pairUnchecked(ea,eb);
			OGDF_ASSERT(pair != DOESNT_EXIST);
			int idx = pair2ilp[pair];
			if( idx == SHADOWED )
				THROW_PARAM(AlgorithmFailureException, afcForbiddenCrossing);
			else if (idx == UNASSIGNED )
				idx = addVariable(pair);
			initsolution[idx] = 1; // yep, each crossing will be written twice, but who cares...
		}
	}

	return initsolution;
}

void CoinOptimalCrossingMinimizer::ensureAllVariables() {
	edge e1,e2;
	forall_validpairs(e1, e2, *expandedGraph)
		ensureVariableOrdered(e1,e2); // all except the shadowed ones...
}

int CoinOptimalCrossingMinimizer::addVariable(int pair) {
	if(pair == DOESNT_EXIST) return DOESNT_EXIST;
	
	pair2ilp[pair] = numILPCrossingPairs;
	ilp2pair[numILPCrossingPairs] = pair;
	CoinPackedVector variableVec;

	variableVec.insert(OBJECTIVE_ROW, getCost(pair)); // it's used in bounding&objFunction -> links with objVal-Column

	//ensure simple drawing
	variableVec.insert(edge1Id(pair),1);
	variableVec.insert(edge2Id(pair),1);
	
	posi->addCol(variableVec, 0, 1, getPerturbation(pair)); // objVal only uses objVal-Column, except for perturabtion values
	posi->setInteger(numILPCrossingPairs);
	
	return numILPCrossingPairs++;
}


void CoinOptimalCrossingMinimizer::ensureVariables(const CrossingConfiguration& ci) {
	edge e1;
	forall_edges(e1, *minimizedGraph) {
		ListConstIterator<edge> it = ci.getCrossingEdges(e1).begin();
		for(; it.valid(); it++) {
			ListConstIterator<edge> ie1 = expandedGraph->chain(e1).begin();
			for (; ie1.valid(); ie1++) {
				ListConstIterator<edge> ie2 = expandedGraph->chain(*it).begin();
				for(; ie2.valid(); ie2++) {
					ensureVariableUnchecked(*ie1, *ie2);
				}				
			}
		}		
	}
}

// ifUnassigned = SHADOWED -> unassigned pairs are shadowed
// ifUnassigned = UNASSIGNED -> unassigned pairs remain UNASSIGNED
// ifUnassigned = sth.else (0) -> unassigned pairs are added
void CoinOptimalCrossingMinimizer::forbidCrossings(edge e1, edge e2, int ifUnassigned) {
	ListConstIterator<edge> ie1 = expandedGraph->chain(e1).begin();
	for (; ie1.valid(); ie1++) {
		ListConstIterator<edge> ie2 = expandedGraph->chain(e2).begin();
		for(; ie2.valid(); ie2++) {
			int pair = pairUnchecked(*ie1,*ie2);
			if(pair == DOESNT_EXIST) continue;
			int idx = pair2ilp[pair];
			if( idx == UNASSIGNED ) { // not shadowed, but also not assigned
				if(ifUnassigned == SHADOWED)
					shadowVariable(pair);
				else if(ifUnassigned != UNASSIGNED) // so add it
					posi->setColUpper(addVariable(pair),0);
			} else if( idx != SHADOWED ) { // so it already exists...
				posi->setColUpper(idx,0);
			}
		}				
	}
}

CoinOptimalCrossingMinimizer::SolutionSource CoinOptimalCrossingMinimizer::solve(int& objValue, int** intsol) {
	*intsol = NULL;
	
	lout() << "# of variables: " << posi->getNumCols() << "\n";
	lout() << "# of constraints: " << posi->getNumRows() << "\n";
	lout() << "      regarding objval:     1\n";
	lout() << "      regarding simplicity: " << numExpEdges << "\n";
	lout() << "      regarding kuratowski: " << posi->getNumRows() - numExpEdges - 1 << "\n";
	
	lout() << "--- INITIAL SOLVE ---\n";
	posi->initialSolve(); // resolve?
	if(unsuccessfulSolve())
		return upperBoundSource;
	lout() << "Obj.Value of ILP relaxation: " << posi->getObjValue() << "\n";
	cutCounter = 0;
	int runs = 0;
	CutReturn ret;
	
	while(true) { // after the branching, the solution might be integer optimal, but not planar 
		          // (CPLEX-cut-callbacks are only called as long as integer infeasable)
		          // hence we have to recheck and probably rerun with add'tl constraints...
		lout() << "--- SOLVER RUN " << ++runs << " ---\n";
		int count = 0;
		while( true ) {
			const double* fragile_sol = posi->getColSolution();
			int solsize = sizeof(double) * posi->getNumCols();		
			double* sol = (double*) malloc(solsize);
			memcpy(sol, fragile_sol, solsize);
	
			ret = cutCallback(posi->getObjValue(), sol, NULL);
			if(ret != CR_AddCuts) {
				if(runs == 1 && ret != CR_SolutionValid && m_createVariables == VC_BeforeBnC) { //first time
					int old = numILPCrossingPairs;
					ensureAllVariables();
					if(old < numILPCrossingPairs) {
						lout() << "Created all remaining variables: " << old << " + " 
							<< (numILPCrossingPairs-old) << " = " << numILPCrossingPairs << "\n";
						posi->resolve(); // CPLEX will realize that these variables just have to be zero...
					} else
						lout() << "*** Note: All variables are already existing! ***\n";
				}				
				break;
			}
			free(sol);

			lout() << "--- RESOLVE #" << ++count << " (Run " << runs << ") ---\n";
	//		posi->writeMps("ocm");
			posi->resolve();
			if(unsuccessfulSolve())
				return upperBoundSource;
			lout() << "Obj.Value of ILP relaxation: " << posi->getObjValue() << "\n";
		}
		
		if(ret != CR_SolutionValid) {
			lout() << "--- STARTING BRANCH&CUT (Run " << runs << ") ---\n";			
			posi->branchAndBound();
			if(unsuccessfulSolve())
				return upperBoundSource;
			
			double ov = posi->getObjValue();
			probablyUpdateLowerBound((int)(ov+EPS), true);
			lout() << "B&B ended. Lower Bound: " << lowerbound << "\n";
		} else
			break;
	}
	
	
	if(posi->isProvenOptimal()) {
		objValue = (int)(posi->getObjValue()+EPS);
		const double* sol = posi->getColSolution();
		lout() << "Done\n";
		*intsol = createIntegerSolution(sol, true);
		return SS_ILP;
	} else {
		lout() << "*************************\n";
		lout() << "*** No Solution found ***\n";
		lout() << "isAbandoned? " << posi->isAbandoned() << "\n";
		lout() << "isProvenPrimalInfeasible? " << posi->isProvenPrimalInfeasible() << "\n";
		lout() << "isProvenDualInfeasible? " << posi->isProvenDualInfeasible() << "\n";
		lout() << "isPrimalObjectiveLimitReached? " << posi->isPrimalObjectiveLimitReached() << "\n";
		lout() << "isDualObjectiveLimitReached? " << posi->isDualObjectiveLimitReached() << "\n";
		lout() << "isIterationLimitReached? " << posi->isIterationLimitReached() << "\n";
		lout() << "*************************\n";
		return SS_NoSolution;
	}
}

bool CoinOptimalCrossingMinimizer::unsuccessfulSolve() {
//	lout() << "Status: " << CPXgetstat( ((OsiCpxSolverInterface*) posi)->getEnvironmentPtr(), 
//		((OsiCpxSolverInterface*) posi)->getLpPtr()) << "\n"; 
	return posi->isProvenPrimalInfeasible() || 
		posi->isProvenDualInfeasible() || 
		(CPXgetstat( ((OsiCpxSolverInterface*) posi)->getEnvironmentPtr(), 
		((OsiCpxSolverInterface*) posi)->getLpPtr() ) 
			== CPXMIP_INFEASIBLE);
}

CoinCallbacks::CutReturn CoinOptimalCrossingMinimizer::cutCallback(const double objVal, const double* fracSolution, OsiCuts* cuts) {
	//lout() << "Checking for add'l cuts -> ";
	int numVarsInSolution = posi->getNumCols();
	
	bool solutionIntegral = isSolutionIntegral(fracSolution);
	int* intSolution = createIntegerSolution(fracSolution, false);
	PlanRep R(*expandedGraph);
	R.initCC(0);
	generateExpandedPlanRep(intSolution, R);
	
	lout() << "Current: " << (solutionIntegral ? "integral, " : "fractional, ");
	
	PlanarModule pm;
	if(pm.planarityTest(R)) {  // solution is planar -> valid! -> no new constraints necessary
		if(solutionIntegral) {
			lout() << "planar! -> (" << (int)(objVal+EPS) << " CROSSINGS)\n";
			int sum = 0;
			for(int i = numVarsInSolution; i-->0;) {
				if(i == OBJECTIVE_COL) continue;
				sum += intSolution[i];
			}
			int sum2 = 0;
			for(int i = numILPCrossingPairs; i-->0;) {
				if(i == OBJECTIVE_COL) continue;
				sum2 += intSolution[i];
			}
			return CR_SolutionValid;
		} else {
			lout() << "induces a planar drawing. -> no cuts added.\n";
			return CR_DontAddCuts;
		}
	}
	
	lout() << "non-planar! -> ";

	int good = 0;
	for(int h = m_numCutKuratowskis; h-->0;) {
		List<int> K;
		findKuratowski(R,K);
		if(ensureConstraint(K, numVarsInSolution, intSolution, fracSolution, cuts)) good++;
	}
	free(intSolution);
	
	lout() << "Added ";
	if(good)
		lout() << good << (cuts ? " cuts" : " constraints");
	else
		lout() << "NOTHING";
	lout() << " -> #vars=" << posi->getNumCols() 
		<< "; #kuras (#cuts) #allconstr=" << kuratowskiConstraints.size() 
		<< " (" << cutCounter << ") " << (posi->getNumRows()+cutCounter) << "\n";
	return good ? CR_AddCuts : CR_NoCutsFound;
}

bool CoinOptimalCrossingMinimizer::isSolutionIntegral(const double* fracSolution) {
	for(int i=numILPCrossingPairs; i-->0;)
		if(fracSolution[i] > EPS && fracSolution[i] < 1-EPS) return false;
	return true;
}

int* CoinOptimalCrossingMinimizer::createIntegerSolution(const double* fracSolution, bool strict) {
	int* intSolution = (int*) malloc(sizeof(int)*numILPCrossingPairs);
	double cutoff = strict ? 1.0 - EPS : 0.9;
	for(int i=numILPCrossingPairs; i-->0;)
		intSolution[i] = fracSolution[i] > cutoff ? 1 : 0;
	return intSolution;
}

void CoinOptimalCrossingMinimizer::generateExpandedPlanRep(int* solution, PlanRep& R) {	
	for(int i = numILPCrossingPairs; i-->0;) {
		if(i == OBJECTIVE_COL) continue;
		if(solution[i] == 1) {
			int pair = ilp2pair[i];
			edge e1 = R.chain(edge1(pair)).front();
			edge e2 = R.chain(edge2(pair)).front();
			R.insertCrossing(e1, e2, true); // don't care about last param
		}
	}
}

void CoinOptimalCrossingMinimizer::setResultingGraph(int* solution) {	/*
	PlanRep P(*expandedGraph);
	generateExpandedPlanRep(solution, P);
	P.embed();
	
	#warning "OptimalCrossingMinimzer::setResultingGraph: Not implemented yet!"
	
	//          PlanRep P
	//              ^
	//    GraphCopy* expandedGraph
	//                   ^
	// GraphReduction* minimizedGraph      PlanRep resultingGraph
	//                        v              ^
	//                      Graph* originalGraph
	
	// TODO: implement all the necessary crap in here....

	edge ep, ee, em, eo, er;	
	ListConstIterator<edge> ip, ie, ir;
	forall_edges(em, *minimizedGraph) {
		eo = costEdge[em];
		ip = P.chain(eo).begin();
		ie = expandedGraph->chain(eo).begin();
		ir = resultingGraph->chain(eo).begin();
		

		// generate all crossings on em (if they don't exist already)
		
		
		// the edge in which to insert all the crossings:
		const List<edge>& l = resultingGraph->chain(eo); //add somewhere here... 
	}
	
	
		
	for(int i = numILPCrossingPairs; i-->0;) {
		if(solution[i] == 1) {
			int pair = ilp2pair[i];
			edge e1 = edge1(pair); // edges in expanded graph
			edge e2 = edge2(pair);
			e1 = 
			e2 = 
			resultingGraph->insertCrossing(e1,e2,true);
		}
	}
	resultingGraph->embed();*/
}

void CoinOptimalCrossingMinimizer::findKuratowski(PlanRep& R, List<int>& K) {
	K.clear();
	// preprocessing?: first try to get rid of the already known kuratowskis
	
	edge e;
	List<edge> es;
	PlanarModule pm;

	GraphReduction S(R);
	S.allEdges(es);
	es.permute(); // randomize Order

	for(ListIterator<edge> it = es.begin(); it.valid(); it++) { // each edge once
		e = *it;
		S.hideEdge(e);
		if(pm.planarityTest((const Graph&)S)) {
			S.restoreEdge(e);
			for(ListConstIterator<edge> it = S.original(e).begin(); it.valid(); it++) {
				K.pushBack( edge2id[ R.original(*it) ] );
			}
		}
	}
	
	/* // my original code withour reduction. unneccessary now...
	R.allEdges(es);
	es.permute(); // randomize order...

	for(ListIterator<edge> it = es.begin(); it.valid(); it++) { // each edge once
		e = *it;
		R.hideEdge(e);
		if(pm.planarityTest((const Graph&)R)) {
			R.restoreEdge(e);
			int id = edge2id[R.original(e)];
			K.pushBack(id);
		}
	}
	R.restoreAllEdges();
	*/
	
	K.quicksort(); // canonical

	// remove duplicates
	if(K.size()) {
		ListIterator<int> last = K.begin();
		ListIterator<int> it = K.begin().succ();
		for(; it.valid(); it++) {
			if(*last == *it) {
				K.del(last);
			}
			last = it;
		}
	}
}

// returns true if a new constraint is added that is vialated by the current fract. solution
bool CoinOptimalCrossingMinimizer::ensureConstraint(const List<int>& edges, const int numVarsInSolution,
	const int* solution, const double* fracSolution, OsiCuts* cuts) {
	int oldILPVarsNum = numILPCrossingPairs;
	
	CoinPackedVector vec;
	int rhs = 1;
	for(ListConstIterator<int> i1 = edges.begin(); i1.valid(); i1++) {
		for(ListConstIterator<int> i2 = i1.succ(); i2.valid(); i2++) {
			int ilp = ensureVariableUnchecked(*i1, *i2);
			if(ilp != SHADOWED) {
				OGDF_ASSERT( ilp >=0 );
				int sign = 1;
				if(solution && ilp < numVarsInSolution && solution[ilp]) {
					sign = -1;
					rhs--;
				}
				vec.insert(ilp,sign);
			}
		}
	}

	KuratowskiConstraint* kc = new KuratowskiConstraint(vec, rhs, edges);
	if(kuratowskiConstraints.isElement(kc)) {
		lout(LL_MINOR) << "KuratowskiConstraint rejected: Duplication.\n";
		delete kc;
		return false;
	} else if(fracSolution && kc->isSatisfied(numVarsInSolution, fracSolution)) {
		lout(LL_MINOR) << "KuratowskiConstraint rejected: Already Satisfied.\n";
		// removing unneccessarily added variables
		for(int i = numILPCrossingPairs; i-->oldILPVarsNum;) {			
			pair2ilp[ilp2pair[i]] = UNASSIGNED;
			posi->deleteCols(1,&i);
		}
		numILPCrossingPairs = oldILPVarsNum; // restore value
		delete kc;
		return false;
	} else {
		kuratowskiConstraints.add(kc);
		lout(LL_MINOR) << "KuratowskiConstraint with " << vec.getNumElements() << " variables accepted.";
		lout(LL_MINOR) << " => #variables: " << posi->getNumCols() << "\n";
		if(cuts) {
			OsiRowCut* c = new OsiRowCut();
			c->setLb(rhs);
			c->setRow(vec); //clones vec
			c->setGloballyValid();
			cuts->insert(c);
			// c is NULL now. ownership of pointer goes to OsiCuts...
			cutCounter++;
		} else
			posi->addRow(vec,'G',rhs,0);
		return true;
	}
}

CoinCallbacks::IncumbentReturn CoinOptimalCrossingMinimizer::incumbentCallback(const double objValue, const double* solution) {
	int* intsol = createIntegerSolution(solution, true); // basically copying...
	PlanRep R(*expandedGraph);
	R.initCC(0);
	generateExpandedPlanRep(intsol, R);
	PlanarModule pm;
	return pm.planarityTest(R) ? IR_Update : IR_Ignore;
}

CoinCallbacks::HeuristicReturn CoinOptimalCrossingMinimizer::heuristicCallback(double& objValue, double* solution) {
	bool success = false;
	for(int hc = 2; hc-->0;) {
		bool strict = (hc == 0);
		lout(LL_MINOR) << "Trying Heuristics (sol. interpr.: " << (strict ? " strict" : "rounded") << ")...";
		int* intsol = createIntegerSolution(solution, strict);
		PlanRep PR(*expandedGraph);
		PR.initCC(0);
		generateExpandedPlanRep(intsol, PR);
		delete intsol;
	
//		GraphReduction Red((const Graph&)PR);
		
		EdgeArray<int> helpcost(PR);
		EdgeArray<bool> helpforbid(PR);
		edge e, oe;
		forall_edges(e, PR) {
			oe = expandedGraph->original(PR.original(e));
			helpcost[e] = cost[oe];
			helpforbid[e] = forbid[oe];
		}	
		PlanRep HPR((const Graph&)PR);
		HPR.initCC(0);
	
		SubgraphPlanarizer sp; // CrossingMinimizationModule
		int ignore;
		sp.call(HPR, 0, ignore, &helpcost, &helpforbid, 0);
		
		int newObj; // i'll have to calculate that manually...
		if(!useCost())
			newObj = HPR.numberOfNodes() - expandedGraph->numberOfNodes();
		else {
			newObj = 0;
			node n;
			forall_nodes(n, HPR) {
				if(HPR.original(n) == NULL || PR.original(HPR.original(n)) == NULL) { // dummy found -> calc cost
					newObj += cost[expandedGraph->original(PR.original(HPR.original(n->firstAdj()->theEdge())))] 
						* cost[expandedGraph->original(PR.original(HPR.original(n->lastAdj()->theEdge())))];
				}
			}
		}
		if(((int)(objValue+EPS)) >= newObj) {
			lout() << "Heuristic successful! -> " << newObj << " crossings\n";
			CrossingConfiguration ci(HPR, newObj, false);
			objValue = newObj;
			createSolutionVector<double>(ci, solution);
			success = true;
		} else {
			lout(LL_MINOR) << "unsuccessful\n";			
		}
	}
	return success ? HR_Update : HR_Ignore;
}	

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

KuratowskiConstraint::KuratowskiConstraint(const CoinPackedVector& vec, int rhs, const List<int>& theEdges) : id(), edges() {
	int length = vec.getNumElements() + 1;
	id.init(length);	
	const int* ind = vec.getIndices();
	int i;
	for(i = length; i-->0;)
		id[i] = (int)vec[ind[i]] * (ind[i]+1);
	id[length-1] = rhs;
	
	edges.init(theEdges.size());
	ListConstIterator<int> it = theEdges.rbegin();
	for(i = edges.size(); i-->0; it--)
		edges[i] = *it;
}

bool KuratowskiConstraint::operator==(const KuratowskiConstraint& ki) const {
	if(this->id.size() != ki.id.size())
		return false;
	for(int i = id.size(); i-->0;) {
		if(id[i] != ki.id[i]) return false;
	}
	return true; // no mismatch found
}

bool KuratowskiConstraint::operator<(const KuratowskiConstraint& ki) const {
	if(this->id.size() < ki.id.size()) return true;
	if(this->id.size() > ki.id.size()) return false;
	for(int i = id.size(); i-->0;) {
		if(id[i] < ki.id[i]) return true;
		if(id[i] > ki.id[i]) return false;
	}
	return false; // they are equal...
}

bool KuratowskiConstraint::isSatisfied(const int numVarsInSolution, const double* fracSolution) const {
	double lhs = 0;
	for(int i = id.size()-1; i-->0;) {
		int idx = abs(id[i])-1;
		if( idx < numVarsInSolution) // else: solution[idx] == 0, and lhs doesn't change
			lhs += (id[i] > 0 ? +1 : -1) * fracSolution[idx];
	}
	return lhs >= id[id.size()-1];
}

} // namespace ogdf

#endif // USE_COIN

