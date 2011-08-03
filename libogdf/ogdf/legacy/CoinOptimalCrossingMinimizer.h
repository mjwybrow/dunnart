/*
 * $Revision: 1.8 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-01-17 22:56:41 +0100 (Thu, 17 Jan 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class CoinOptimalCrossingMinimizer.
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

#ifndef OGDF_OPTIMAL_CROSSING_MINIMIZER_H
#define OGDF_OPTIMAL_CROSSING_MINIMIZER_H



#include <ogdf/OptimalCrossingMinimizerBase.h>
#include <ogdf/external/coin.h>
#include <ogdf/basic/Logger.h>

#ifdef USE_COIN
	#include <OsiSolverInterface.hpp>
	#include <CoinPackedVector.hpp>
	#include <OsiCuts.hpp>
	#include <ogdf/basic/Graph.h>
	#include <ogdf/basic/GraphCopy.h>
	#include <ogdf/graphalg/GraphReduction.h>
	#include <ogdf/basic/List.h>
	#include <ogdf/basic/Skiplist.h>
	
	#define forall_validpairs(e1,e2,G) forall_edges(e1,G) forall_edges(e2,G) if(isOrdered(e1,e2))
	
#endif

using namespace ogdf;
namespace ogdf_legacy {
	
#ifdef USE_COIN
	class KuratowskiConstraint;
#endif

class CoinOptimalCrossingMinimizer : public OptimalCrossingMinimizerBase, public CoinCallbacks {
	
#ifndef USE_COIN
protected:
	// Guess what...
	virtual ReturnType doCall(PlanRep &PG,
		int cc,
		const EdgeArray<int>  &_cost,
		const EdgeArray<bool> &_forbid,
		const EdgeArray<unsigned int>  &subgraphs,
		int& crossingNumber)
	{ THROW_NO_COIN_EXCEPTION; };
};
#else // USE_COIN
	
public:
	enum VariableCreation { VC_OnTheFly, VC_BeforeBnC, VC_Initially };

	// construction
	CoinOptimalCrossingMinimizer() :
		edge2id(),
		id2edge(),
		pair2ilp(),
		ilp2pair(),
		kuratowskiConstraints() {
			setDefaultSettings();
	}
	
	// destruction
	virtual ~CoinOptimalCrossingMinimizer() { }

	void setDefaultSettings();
	
	VariableCreation createVariables() const { return m_createVariables; }
	void createVariables(VariableCreation b) { m_createVariables = b; }
	
	bool heuristicCallback() const { return m_heuristicCallback; }
	void heuristicCallback(bool hc) {m_heuristicCallback = hc; }

	bool usePerturbation() const { return m_usePerturbation; }
	void usePerturbation(bool p) { m_usePerturbation = p; }

protected:
	// Guess what...
	virtual ReturnType doCall(PlanRep &PG,
		int cc,
		const EdgeArray<int>  &_cost,
		const EdgeArray<bool> &_forbid,
		const EdgeArray<unsigned int>  &subgraphs,
		int& crossingNumber);
		
friend class KuratowskiInfo;

private:
	enum { DOESNT_EXIST = -3, SHADOWED = -2, UNASSIGNED = -1 };

	// params
	VariableCreation m_createVariables;
	bool             m_heuristicCallback;
	bool             m_usePerturbation;

	// helpers		
	OsiSolverInterface *posi;
	int numILPCrossingPairs;
	EdgeArray<int>  edge2id;
	Array<edge>     id2edge;
	Array<int>      pair2ilp;
	Array<int>      ilp2pair;
	int OBJECTIVE_ROW;
	int OBJECTIVE_COL;
	Skiplist<KuratowskiConstraint*> kuratowskiConstraints;
	int cutCounter;
	
    // bunch of shortcuts. all inline.
	bool isOrdered(int e1, int e2)           { return e1 < e2;                                                    }
	bool isOrdered(edge e1,edge e2)          { return isOrdered(edge2id[e1],edge2id[e2]);                         }
	int  pairOrdered(int e1, int e2)         { return e1*numExpEdges + e2;                                        }	
	int  pairOrdered(edge e1, edge e2)       { return pairOrdered(edge2id[e1], edge2id[e2]);                      }
	int  pairUnchecked(int e1, int e2)       { return (e1 == e2) ? DOESNT_EXIST : 
                                                  ( isOrdered(e1,e2) ? pairOrdered(e1,e2) : pairOrdered(e2,e1) ); }
	int  pairUnchecked(edge e1, edge e2)     { return pairUnchecked(edge2id[e1],edge2id[e2]);                     }
	int  edge1Id(int pair)                   { return pair / numExpEdges;                                         }
	int  edge2Id(int pair)                   { return pair % numExpEdges;                                         }
	int  edge1Id(int e1, int e2)             { return isOrdered(e1,e2) ? e1 : e2;                                 }
	int  edge2Id(int e1, int e2)             { return isOrdered(e1,e2) ? e2 : e1;                                 }
	int  edge1Id(edge e1, edge e2)           { return edge1Id(edge2id[e1],edge2id[e2]);                           }
	int  edge2Id(edge e1, edge e2)           { return edge2Id(edge2id[e1],edge2id[e2]);                           }
	edge edge1(int pair)                     { return id2edge[edge1Id(pair)];                                     }
	edge edge2(int pair)                     { return id2edge[edge2Id(pair)];                                     }
	edge edge1(int e1, int e2)               { return id2edge[edge1Id(e1,e2)];                                    }
	edge edge2(int e1, int e2)               { return id2edge[edge2Id(e1,e2)];                                    }
	edge edge1(edge e1, edge e2)             { return id2edge[edge1Id(e1,e2)];                                    }
	edge edge2(edge e1, edge e2)             { return id2edge[edge2Id(e1,e2)];                                    }
	

	// destroy
	void cleanUp();
	
	// inline helpers
	static const double PERT = 0.001;
	double getPerturbationError() { return usePerturbation() ? PERT+PERT*PERT : 0; }
	int getCost(edge e) { return cost[expandedGraph->original(e)]; }
	int getCost(int pair)  { return useCost() ? getCost(edge1(pair))*getCost(edge2(pair)) : 1; }
	double getPerturbation(edge e) {
		edge oe = expandedGraph->original(e);
		int i = 0;
		for(ListConstIterator<edge> it = expandedGraph->chain(oe).begin(); it.valid(); it++, i++)
			if(*it == e) break;
		double pert1 = i / (double)expandedGraph->chain(oe).size();
		double pert2 = edge2id[e] / (double)numExpEdges;
		return pert1*PERT + pert2*PERT*PERT;
	}
	double getPerturbation(int pair) { return usePerturbation() ? getPerturbation(edge1(pair))*getPerturbation(edge2(pair)) : 0; }
	
	void shadowVariable(int pair)                  { if(pair!=DOESNT_EXIST) pair2ilp[pair] = SHADOWED; }
	void shadowVariableOrdered(int e1, int e2)     { shadowVariable(pairOrdered(e1,e2));               }
	void shadowVariableOrdered(edge e1, edge e2)   { shadowVariable(pairOrdered(e1,e2));               }
	void shadowVariableUnchecked(int e1, int e2)   { shadowVariable(pairUnchecked(e1,e2));             }
	void shadowVariableUnchecked(edge e1, edge e2) { shadowVariable(pairUnchecked(e1,e2));             }
	int  addVariable(int pair);
	int  addVariableOrdered(int e1, int e2)        { return addVariable(pairOrdered(e1,e2));           }
	int  addVariableOrdered(edge e1, edge e2)      { return addVariable(pairOrdered(e1,e2));           }
	int  addVariableUnchecked(int e1, int e2)      { return addVariable(pairUnchecked(e1,e2));         }
	int  addVariableUnchecked(edge e1, edge e2)    { return addVariable(pairUnchecked(e1,e2));         }
	int  ensureVariable(int pair)                  { if(pair == DOESNT_EXIST) return DOESNT_EXIST;
		                                             int v = pair2ilp[pair]; 
		                                             return (v != UNASSIGNED) ? v : addVariable(pair); }
	int  ensureVariableOrdered(int e1, int e2)     { return ensureVariable(pairOrdered(e1,e2));        }
	int  ensureVariableOrdered(edge e1, edge e2)   { return ensureVariable(pairOrdered(e1,e2));        }
	int  ensureVariableUnchecked(int e1, int e2)   { return ensureVariable(pairUnchecked(e1,e2));      }
	int  ensureVariableUnchecked(edge e1, edge e2) { return ensureVariable(pairUnchecked(e1,e2));      }
	bool existsVariable(int pair)                  { return pair!=DOESNT_EXIST && pair2ilp[pair] >= 0; }
	bool existsVariableOrdered(int e1, int e2)     { return existsVariable(pairOrdered(e1,e2));        }
	bool existsVariableOrdered(edge e1, edge e2)   { return existsVariable(pairOrdered(e1,e2));        }
	bool existsVariableUnchecked(int e1, int e2)   { return existsVariable(pairUnchecked(e1,e2));      }
	bool existsVariableUnchecked(edge e1, edge e2) { return existsVariable(pairUnchecked(e1,e2));      }
	
	void writeLowerBoundToILP() {
			posi->setColLower(OBJECTIVE_COL, lowerbound);
	}
	void writeUpperBoundToILP() {
			posi->setColUpper(OBJECTIVE_COL, upperbound + getPerturbationError());
	}
	
	// HELPERS
	// kuratowski
	SolutionSource solve(int & objValue, int** intsol);
	CoinCallbacks::CutReturn cutCallback(const double objVal, const double* fracSolution, OsiCuts* cuts);
	CoinCallbacks::IncumbentReturn incumbentCallback(const double objValue, const double* solution);
	CoinCallbacks::HeuristicReturn heuristicCallback(double& objValue, double* solution);
	bool isSolutionIntegral(const double* fracSolution);
	int* createIntegerSolution(const double* fracSolution, bool strict);
	void generateExpandedPlanRep(int* solution, PlanRep& R);
	//! \reminder findKuratowski has quadratic time instead of linear!... but at least the code is short...
	void findKuratowski(PlanRep& R, List<int>& K);
	bool ensureConstraint(const List<int>& edges, const int numVarsInSolution, 
				const int* solution, const double* fracSolution, OsiCuts* cuts);
	void setResultingGraph(int* solution);	
	
	// ilps
	template<class T> T* createSolutionVector(const CrossingConfiguration& ci, T* initsolution = NULL);
	void forbidCrossings(edge e1, edge e2, int ifUnassigned);
	void ensureVariables(const CrossingConfiguration& ci);
	void ensureAllVariables();
	
	bool unsuccessfulSolve();
	
	// inits
	void prepareILP();
	void shadowVariables();	

};

//const int CoinOptimalCrossingMinimizer::SHADOWED   = -2;
//const int CoinOptimalCrossingMinimizer::UNASSIGNED = -1;

class KuratowskiConstraint {

public:
	KuratowskiConstraint(const CoinPackedVector& vec, int rhs, const List<int>& theEdges);

	bool isSatisfied(const int numVarsInSolution, const double* fracSolution) const;
		
	inline bool operator==(const KuratowskiConstraint& ki) const;
	inline bool operator<(const KuratowskiConstraint& ki) const;
	
	const Array<int> getEdges() { return edges; }

private:
	Array<int> id;
	Array<int> edges; // addt'l info
};

#endif // USE_COIN

} // end namespace ogdf

#endif // OGDF_OPTIMAL_CROSSING_MINIMIZER_H
