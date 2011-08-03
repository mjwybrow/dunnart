/*
 * $Revision: 1.16 $
 * 
 * last checkin:
 *   $Author: chimani $ 
 *   $Date: 2008-02-20 20:33:07 +1100 (Wed, 20 Feb 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class AbacusOptimalCrossingMinimizerSuperPrice
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


#ifndef OGDF_ABACUS_OPTIMAL_CROSSING_MINIMIZER_SUPER_PRICE_H
#define OGDF_ABACUS_OPTIMAL_CROSSING_MINIMIZER_SUPER_PRICE_H

#include <ogdf/external/abacus.h>

#include <ogdf/basic/basic.h>
#include <ogdf/legacy/OptimalCrossingMinimizerBase.h>

using namespace ogdf;
namespace ogdf_legacy {

class AbacusOptimalCrossingMinimizerSuperPrice : public CrossingMinimizationModule {

#ifndef USE_ABACUS
protected:
	// Guess what...
	virtual ReturnType doCall(PlanRep &PG,
		int cc,
		const EdgeArray<int>  &_cost,
		const EdgeArray<bool> &_forbid,
		const EdgeArray<unsigned int>  &_subgraphs,
		int& crossingNumber)
	{ THROW_NO_ABACUS_EXCEPTION; return retError; };
};
#else // USE_ABACUS

public:

	enum PricingInitialization { PI_EachPair, PI_BestHeuristic, PI_None };
	enum GraphHint { GH_None, GH_Complete, GH_CompleteBipartite };
	enum HintEffects { HE_KuratowskisMinusOne = 1,  // 1, 2, 4, 8, ...
		               HE_KuratowskisMinusTwo = 2,
		               HE_EdgeOrder = 4,
		               HE_NodeOrder = 8 };

protected:

	static const double SEG_EPS;
	
	class Master; //forward
	class Subproblem; //forward
	class CrossingInfo;
	class CrossingVariable;
	class CrossingInfoComparer;
	class AbacusConstraint; //forward	
	class SimplicityConstraint; //forward
	class KuratowskiConstraint; //forward
	class EdgeOrderConstraint; //forward
	class NodeOrderConstraint; //forward
	
	struct CrossingInfo {
		CrossingInfo(edge te1, int te1no, edge te2, int te2no) {
			if(preferedOrder(te1,te2)) {
				e1 = te1;
				e2 = te2;
				e1no = te1no;
				e2no = te2no;
			} else {
				e2 = te1;
				e1 = te2;
				e2no = te1no;
				e1no = te2no;
			}			
		}
		CrossingInfo(const CrossingInfo& src) :
			e1(src.e1), e2(src.e2), e1no(src.e1no), e2no(src.e2no) {}
		static bool preferedOrder(edge e1, edge e2) {
			return e1->index() < e2->index();
		}
		edge e1,e2; //originaledge
		int e1no, e2no; //number
	};

	class CrossingInfoComparer {
	public:
		static int compare(const CrossingInfo* & x, const CrossingInfo* & y) {
			int ret;
			if((ret = (x->e1->index()-y->e1->index()))) return ret;
			if((ret = (x->e2->index()-y->e2->index()))) return ret;
			if((ret = (x->e1no-y->e1no))) return ret;
			return x->e2no-y->e2no;
		}
		OGDF_AUGMENT_STATICCOMPARER(CrossingInfo*)
	};
	
	class CrossingVariable : public CrossingInfo, public ABA_VARIABLE {
		friend std::ostream &operator<<(std::ostream &os, const AbacusOptimalCrossingMinimizerSuperPrice::CrossingVariable& v);
	public:
		CrossingVariable(ABA_MASTER* m, edge te1, int te1no, edge te2, int te2no) :
				CrossingInfo(te1, te1no, te2, te2no),
				ABA_VARIABLE(m,0,0,0, // master / sub=0 / dynamic=false / local=false
					((AbacusOptimalCrossingMinimizerSuperPrice::Master*)m)->getCost(te1,te2)+
						((AbacusOptimalCrossingMinimizerSuperPrice::Master*)m)->epsilonify(te1no, te2no), // obj
					0,1,ABA_VARTYPE::Binary) {} // lowerBound / upperBound / type
		bool correspondsTo(edge e, int num) {
			return (e == e1 && e1no == num)|| (e == e2 && e2no == num);
		}
		bool correspondsTo(edge e) {
			return e == e1 || e == e2;
		}
		bool correspondsTo(node n) {
			return e1->source() ==  n || e1->target() ==  n || e2->source() ==  n || e2->target() ==  n;
		}
  	};
	
	class AbacusConstraint : public ABA_CONSTRAINT {
	public:
		AbacusConstraint(ABA_MASTER *master, const ABA_SUB *sub,
				ABA_CSENSE::SENSE sense, double rhs,
				bool dynamic, bool local, bool liftable) :
			ABA_CONSTRAINT(master, sub, sense, rhs, dynamic, local, liftable) {}
		AbacusConstraint(const AbacusConstraint& c) : ABA_CONSTRAINT(c) {};
	protected:
		AbacusOptimalCrossingMinimizerSuperPrice::Master* master() const {
			return (AbacusOptimalCrossingMinimizerSuperPrice::Master*)master_;
		}
	public:
		void printCoeffs(std::ostream& os) /*const*/ {
			os << "{please reimplement Abacus::Constraint::printCoeff}";
		}
	};
	
	struct Segment {
		edge e;
		int segId;
		Segment(edge te, int tsegId) : e(te), segId(tsegId) {}
	};
	
	class SimplicityConstraint : public AbacusConstraint, public Segment {
	public:
		SimplicityConstraint(ABA_MASTER* m, edge te, int tsegId) : 
			AbacusConstraint(m, 0, ABA_CSENSE::Less, 1, 0, 0, 1),  // non-dynamic, non-local, liftable
			Segment(te, tsegId) {}
		double coeff(ABA_VARIABLE* cvar) {
			return (double) ((CrossingVariable*)cvar)->correspondsTo(e, segId);
		}
	};
	
	class KuratowskiConstraint : public AbacusConstraint, protected Logger {
		friend std::ostream &operator<<(std::ostream &os, const AbacusOptimalCrossingMinimizerSuperPrice::KuratowskiConstraint& k);
	public:
		struct EdgeInfo {
			int startId; // -splitnumber-1 => including start node
			int endId;   // splitnumber+1 => including end node
			EdgeInfo(int s, int e) : startId(s), endId(e) {}
			EdgeInfo(const EdgeInfo& E) : startId(E.startId), endId(E.endId) {}
			bool contains(int id) {
				return startId<=id && id<=endId;
			}
			bool equals(const EdgeInfo& ei) {
				return startId == ei.startId && endId == ei.endId;
			}
		};

		KuratowskiConstraint(ABA_MASTER* m, const Subproblem* S, const GraphReduction& R, const List<edge>& K, bool dynamic) : 
				AbacusConstraint(m, 0, ABA_CSENSE::Greater, 0 /*rhs*/, dynamic, 0, 1),  // non-local, liftable
				edges(), D() {
			edges.init(*master()->minimizedGraph,0);
			build(S, R, K);
		}
		
		KuratowskiConstraint(const KuratowskiConstraint& orig) : AbacusConstraint(orig), edges(), D(orig.D) {
			OGDF_ASSERT( this->liftable() );
			edges.init(*master()->minimizedGraph,0);
			edge e;
			forall_edges(e, *orig.edges.graphOf()) {
				if(orig.edges[e])
					edges[e] = new EdgeInfo(*orig.edges[e]);
			}
		} 
		
		KuratowskiConstraint(ABA_MASTER* m, int cr, bool dynamic) : 
				AbacusConstraint(m, 0, ABA_CSENSE::Greater, cr, dynamic, 0, 1),  // non-local, liftable
				edges(), D() {
			edges.init(*master()->minimizedGraph,0);
		}
		
		void addEdge(edge e) {
			edges[e] = new EdgeInfo(-master()->expansionFactor-1, +master()->expansionFactor+1);
		}
		
		void adapt(int varidx, CrossingVariable* newvar, edge e, int oldi, int newi) {
			D[varidx] = newvar;
			if(edges[e]->startId == oldi)
				edges[e]->startId = newi;
			else
				edges[e]->endId = newi;
		}
		
		
		EdgeArray<EdgeInfo*> edges;
		Array<CrossingInfo*> D; //sorted!
		
		/////////////////

		void build(const Subproblem* S, const GraphReduction& R, const List<edge>& K);
		void addAccordingCrossing(const Subproblem* S, const PlanRep& I, edge e, int eid, List<CrossingInfo*>& L);
		
		virtual const char* name() {
			return "KuratowskiConstraint";
		}
		
		virtual unsigned hashKey() {
			int h = D.size();			
			edge e;
			forall_edges(e, *edges.graphOf()) {				
				h = (int) ((1.1*h) + (edges[e] ? (edges[e]->startId + 2*edges[e]->endId) : 0 ));
			}
			for(int i = D.size(); i-->0;) {
				h = (int) ((1.1*h) + D[i]->e1->index() + D[i]->e2->index() + D[i]->e1no + 2*D[i]->e2no);
			}
			return h;
		}
		
		virtual bool equal(ABA_CONVAR *cv) {
			return equals(*(KuratowskiConstraint*)cv);
		}
		
		bool equals(const KuratowskiConstraint& ki) {
			edge e;
			if(D.size() != ki.D.size())
				return false;
			forall_edges(e, *edges.graphOf()) {
				if(edges[e]==0 ^ ki.edges[e]==0) // one is defined one is not
					return false;
				if(edges[e] && !edges[e]->equals(*ki.edges[e])) //defined but not equal
					return false;
			}
			for(int i = D.size(); i-->0;) {
				if(!master()->crossingInfoComparer.equal(D[i],ki.D[i]))
					return false;
			}
			return true; // no missmatch found
		}

		double coeff(ABA_VARIABLE* cvar) {
			CrossingVariable* v = (CrossingVariable*)cvar;
			if(edges[v->e1] && edges[v->e2] && 
					edges[v->e1]->contains(v->e1no) && 
					edges[v->e2]->contains(v->e2no)) //it exists
				return D.binarySearch(v, master()->crossingInfoComparer)>=0 ? -1 : 1;
			else
				return 0;
		}		
				
	};

	class EdgeOrderConstraint : public AbacusConstraint {
		edge e1, e2;
	public:
		EdgeOrderConstraint(ABA_MASTER* m, edge te1, edge te2) : 
			AbacusConstraint(m, 0, ABA_CSENSE::Greater, 0, 0, 0, 1),  // non-dynamic, non-local, liftable
			e1(te1), e2(te2) {}
		double coeff(ABA_VARIABLE* cvar) {
			return ((CrossingVariable*)cvar)->correspondsTo(e1) - ((CrossingVariable*)cvar)->correspondsTo(e2);
		}
	};

	class NodeOrderConstraint : public AbacusConstraint {
		node n1, n2;
	public:
		NodeOrderConstraint(ABA_MASTER* m, node tn1, node tn2) : 
			AbacusConstraint(m, 0, ABA_CSENSE::Greater, 0, 0, 0, 1),  // non-dynamic, non-local, liftable
			n1(tn1), n2(tn2) {}
		double coeff(ABA_VARIABLE* cvar) {
			return ((CrossingVariable*)cvar)->correspondsTo(n1) - ((CrossingVariable*)cvar)->correspondsTo(n2);
		}
	};

	class Subproblem : public ABA_SUB, protected Logger {
		
		void initCurrents() {
			currentIntegerSolution = new PlanRep(*(master()->expandedGraph));
			currentRoundedSolution = new PlanRep(*(master()->expandedGraph));
			currentIntegerCrossings = new List<CrossingInfo*>();
			currentRoundedCrossings = new List<CrossingInfo*>();
			currentIntegerSolution->initCC(0);
			currentRoundedSolution->initCC(0);
			storedCurrentCrossings = false;
		}
		void clearCurrents() {
			if(currentIntegerSolution) delete currentIntegerSolution;
			if(currentRoundedSolution) delete currentRoundedSolution;
			if(!storedCurrentCrossings) {
				if(currentIntegerCrossings) delete currentIntegerCrossings;
				if(currentRoundedCrossings) delete currentRoundedCrossings;
			}
		}
		
	public:
		Subproblem(ABA_MASTER* m, ABA_SUB* father, ABA_BRANCHRULE* br) :
				ABA_SUB(m, father, br),
				currentIntegerSolution(0),
				currentRoundedSolution(0), 
				currentIntegerCrossings(0),
				currentRoundedCrossings(0),
				simplicitySatisfied(false),
				storedCurrentCrossings(false) {
			lout() << "Subproblem " << id() << " generated; Father: " << father->id() << "\n";
		}
		Subproblem(ABA_MASTER* m,
				double conRes, 
				double varRes, 
				double nnzRes, 
				bool relativeRes,
				ABA_BUFFER<ABA_POOLSLOT<ABA_CONSTRAINT, ABA_VARIABLE> *> *constraints = 0,
				ABA_BUFFER<ABA_POOLSLOT<ABA_VARIABLE, ABA_CONSTRAINT> *> *variables = 0) :
					ABA_SUB(m, conRes, varRes, nnzRes, relativeRes, constraints, variables),
					currentIntegerSolution(0),
					currentRoundedSolution(0), 
					currentIntegerCrossings(0),
					currentRoundedCrossings(0),
					simplicitySatisfied(false),
					storedCurrentCrossings(false) {
			lout() << "Subproblem " << id() << " generated. No Father\n";
		}
		~Subproblem() {
			clearCurrents();
		};
		ABA_SUB *generateSon(ABA_BRANCHRULE* br) {
			if(master()->nSub()==1) { // i'm the root subproblem, and i'm gonna die -> let's give a status report...
				sout() << "\t" << ceil(this->dualBound()-0.5) // lower bound
				       << "\t" << master()->primalBound() // upper bound
				       << "\t" << master()->totalTime()->seconds();
			}
			return new Subproblem(master(), this, br); 
		}
		bool feasible();
		int separate();
		int pricing();
		int makeFeasible();
		int improve(double& d);
		int solveLp();
		
		bool checkSimplicitiesAndPrice();
		void duplicateKuratowskis(CrossingVariable* cvar, List<KuratowskiConstraint*>& L);		
		void realizeVariable(CrossingVariable* cvar, double val);
		
		PlanRep* currentIntegerSolution;
		PlanRep* currentRoundedSolution;
		List<CrossingInfo*>* currentIntegerCrossings;
		List<CrossingInfo*>* currentRoundedCrossings;

	protected:
	
		bool simplicitySatisfied;
		bool storedCurrentCrossings;
		
		AbacusOptimalCrossingMinimizerSuperPrice::Master* master() const {
			return (AbacusOptimalCrossingMinimizerSuperPrice::Master*)master_;
		}
		
		//! \reminder findKuratowski has quadratic time instead of linear!... but at least the code is short...
		void findKuratowski(Graph& R, List<edge>& K);
		CrossingConfiguration* callBoundHeuristic(bool integer);
		inline double generateVariable(edge e1, int e1id, edge e2, int e2id, AbacusOptimalCrossingMinimizerSuperPrice::CrossingVariable** var);
	};
	
	friend std::ostream &operator<<(std::ostream &os, const AbacusOptimalCrossingMinimizerSuperPrice::CrossingInfo& v);
	friend std::ostream &operator<<(std::ostream &os, const AbacusOptimalCrossingMinimizerSuperPrice::CrossingVariable& v);
	friend std::ostream &operator<<(std::ostream &os, /*const*/ AbacusOptimalCrossingMinimizerSuperPrice::KuratowskiConstraint& k);
	friend std::ostream &operator<<(std::ostream &os, /*const*/ AbacusOptimalCrossingMinimizerSuperPrice::SimplicityConstraint& k);

	class Master : public OptimalCrossingMinimizerBase, public ABA_MASTER {

		class ActiveVariables {
			class Span {
				int s;
				int e; // index 1 after last
			public:
				Span() : s(0), e(0) {}
				void init() { s=0; e=0; }
				//Span(int st, int num) : s(st), e(st+num) {}
				int start() const { return s; }
				int end() const { return e - 1; }
				int num() const { return e - s; }
				void minmax(int i) {
					if( num() == 0 ) { // a fresh one...
						s = i;
						e = i+1;
					} else {
						OGDF_ASSERT(i >= s-1 );
						OGDF_ASSERT(i <= e );
						if( i < s ) s = i;
						if( i == e ) e = i+1;
					}
				}
			};
			AbacusOptimalCrossingMinimizerSuperPrice::Master* master;
			EdgeArray<Span> perEdge;
			EdgeArray< EdgeArray<Span> > perEdgePair;
			EdgeArray< EdgeArray<Span* > > perEdgeSegmentAndEdge;
			
		public:
			ActiveVariables(AbacusOptimalCrossingMinimizerSuperPrice::Master* m) : master(m),
					perEdge(*(m->minimizedGraph)), perEdgePair(*(m->minimizedGraph)), perEdgeSegmentAndEdge(*(m->minimizedGraph)) {
				edge e;
				const Graph& mG = *(master->minimizedGraph);
				forall_edges(e, mG) {
					perEdgePair[e].init(mG);
					perEdgeSegmentAndEdge[e].init(mG,0);
				}
			}
			~ActiveVariables() {
				edge e, ee;
				const Graph& mG = *(master->minimizedGraph);
				forall_edges(e, mG) {
					forall_edges(ee, mG) {
						if(perEdgeSegmentAndEdge[e][ee])
							delete[] perEdgeSegmentAndEdge[e][ee];
					}
				}			
			}
			
			int id2idx(int id) { return (id+master->expansionFactor)%(master->expansionFactor); }
		
			int minIndex(edge e)                    { return perEdge[e].start(); }
			int maxIndex(edge e)                    { return perEdge[e].end(); }
			int numIndices(edge e)                  { return perEdge[e].num(); }
			int minIndex(edge e, edge e2)           { return perEdgePair[e][e2].start(); }
			int maxIndex(edge e, edge e2)           { return perEdgePair[e][e2].end(); }
			int numIndices(edge e, edge e2)         { return perEdgePair[e][e2].num(); }
			int minIndex(edge e, int id, edge e2)   { return perEdgeSegmentAndEdge[e][e2][id2idx(id)].start(); }
			int maxIndex(edge e, int id, edge e2)   { return perEdgeSegmentAndEdge[e][e2][id2idx(id)].end(); }
			int numIndices(edge e, int id, edge e2) { return perEdgeSegmentAndEdge[e][e2][id2idx(id)].num(); }
	
			bool isNew(edge e, int idx)             { return idx < perEdge[e].start() || idx > perEdge[e].end(); }
			int numFree(edge e)                     { return master->expandedGraph->chain(e).size() - perEdge[e].num(); }
			
			void registerVariable(CrossingVariable* v) {
				perEdge[v->e1].minmax(v->e1no);
				perEdge[v->e2].minmax(v->e2no);
				perEdgePair[v->e1][v->e2].minmax(v->e1no);
				perEdgePair[v->e2][v->e1].minmax(v->e2no);
				if(!perEdgeSegmentAndEdge[v->e1][v->e2]) {
					Span* s = new Span[master->expansionFactor];
					for(int i = master->expansionFactor; i-->0;) s[i].init();
					perEdgeSegmentAndEdge[v->e1][v->e2] = s;
				}
				perEdgeSegmentAndEdge[v->e1][v->e2][id2idx(v->e1no)].minmax(v->e2no);
				if(!perEdgeSegmentAndEdge[v->e2][v->e1]) {
					Span* s = new Span[master->expansionFactor];
					for(int i = master->expansionFactor; i-->0;) s[i].init();
					perEdgeSegmentAndEdge[v->e2][v->e1] = s;
				}
				perEdgeSegmentAndEdge[v->e2][v->e1][id2idx(v->e2no)].minmax(v->e1no);
				master->m_usedVariables++;
			}
		};
		
		friend class ActiveVariables;
		friend class Subproblem;
		friend class CrossingVariable;
		friend class KuratowskiConstraint;
		friend class AbacusOptimalCrossingMinimizerSuperPrice;

		void setDefaultSettings();

	public:
	
		Master() :
				ABA_MASTER("AbacusOptimalCrossingMinimizerSuperPrice::Master",1, true ,ABA_OPTSENSE::Min),
	//			lastIntegerCrossings(0),
	//			lastRoundedCrossings(0),
				bestSolution(0),
				activeVars(0),
				m_isTrivial(false) {
			setDefaultSettings();
			lastIntegerCrossings = new List<CrossingInfo*>;
			lastRoundedCrossings = new List<CrossingInfo*>;
		}	
		~Master() {
			if(lastIntegerCrossings) delete lastIntegerCrossings;
			if(lastRoundedCrossings) delete lastRoundedCrossings;
		}
		
		ABA_SUB* firstSub() {
			return new AbacusOptimalCrossingMinimizerSuperPrice::Subproblem(this, 1000, numExpMaxCrossingPairs, upperbound, false);
		}
	
		void initializeOptimization();
		
		int enumerationStrategy(ABA_SUB* s1, ABA_SUB* s2);
		
		CrossingInfoComparer crossingInfoComparer;
		
		List<CrossingInfo*>* lastIntegerCrossings;
		List<CrossingInfo*>* lastRoundedCrossings;
		
		bool equalCrossingLists(List<CrossingInfo*>* L1, bool integerList) {
			List<CrossingInfo*>* L2 = integerList ? lastIntegerCrossings : lastRoundedCrossings;
			if(L1->size() != L2->size())
				return false;
			ListConstIterator<CrossingInfo*> it1 = L1->begin();
			ListConstIterator<CrossingInfo*> it2 = L2->begin();
			for(; it1.valid(); ++it1, ++it2)
				if(!crossingInfoComparer.equal(*it1, *it2))
					return false;
			return true;
		}
		
	public:
		int numTryKuratowskis() const { return m_numTryKuratowskis; }
		void numTryKuratowskis(int i) { m_numTryKuratowskis = i; }

		int numAddVariables() const { return m_numAddVariables; }
		void numAddVariables(int i) { m_numAddVariables = i; }
	
		int numAddSimilarVariables() const { return m_numAddVariables; }
		void numAddSimilarVariables(int i) { m_numAddSimilarVariables = i; }
	
		int numTryVariables() const { return m_numTryVariables; }
		void numTryVariables(int i) { m_numTryVariables = i; }
	
		int numBoundHeuristics() const { return m_numBoundHeuristics; }
		void numBoundHeuristics(int b) { m_numBoundHeuristics = b; }
	
		int maxMinutes() const { return m_maxMinutes; }
		void maxMinutes(int m) { m_maxMinutes = m; }
		
		double roundUp() const { return m_roundUp; }
		void roundUp(double r) { m_roundUp = r; }
	
//		bool simpleDrawings const { return m_simpleDrawings; }
//		void simpleDrawings(bool b) { m_simpleDrawings = b; }

		bool tryCrossingAdjacentVarsFirst() const { return m_tryCrossingAdjacentVarsFirst; }
		void tryCrossingAdjacentVarsFirst(bool b) { m_tryCrossingAdjacentVarsFirst = b; }

		PricingInitialization pricingInitialization() const { return m_pricingInitialization; }
		void pricingInitialization(PricingInitialization p) { m_pricingInitialization = p; }
		
		bool duplicateKuratowskis() const { return m_duplicateKuratowskis; }
		void duplicateKuratowskis(bool b) { m_duplicateKuratowskis = b; }

		bool isOptimal() { return m_isTrivial || (status() == Optimal); }
	
		char* writeResult() { return m_writeResult; }
		void writeResult(char* b) { m_writeResult = b; }
		
		GraphHint graphHint() const { return m_graphHint; }
		void graphHint(GraphHint h) { m_graphHint = h; }
		
		int hintEffects() const { return m_hintEffects; }
		void hintEffects(int h) { m_hintEffects = h; }
		
	protected:
		virtual ReturnType doCall(PlanRep &PG,
			int cc,
			const EdgeArray<int>  &cost,
			const EdgeArray<bool> &forbid, 
			const EdgeArray<unsigned int>  &subgraphs,
			int& crossingNumber);
			
	private:
	
		CrossingConfiguration* bestSolution;
		ActiveVariables* activeVars;
		
		int m_numTryKuratowskis;
		int m_numAddVariables;
		int m_numTryVariables;
		int m_numAddSimilarVariables;
		int m_numBoundHeuristics;
		int m_maxMinutes;
		bool m_isTrivial;
//		bool m_simpleDrawings;
		bool m_tryCrossingAdjacentVarsFirst;
		PricingInitialization m_pricingInitialization;
		bool m_duplicateKuratowskis;
		double m_roundUp;
		GraphHint m_graphHint;
		int m_hintEffects;
		
		char* m_writeResult;
		
		int m_minVariables;
		int m_maxVariables;
		int m_usedVariables;
		
		void updateBestSolution(CrossingConfiguration* ncc) {
			if(bestSolution == ncc) return; // repetition -> init
			upperBoundSource = AbacusOptimalCrossingMinimizerSuperPrice::Master::SS_ILP_Heuristic;
			if(bestSolution) delete bestSolution;
			bestSolution = ncc;		
		}
	
		int getCost(edge e1, edge e2) { // call with e1, e2 of minimizedGraph			
			return useCost() ? cost[e1]*cost[e2] : 1;
		}		
		double epsilonify(int i1, int i2) {
			return (i1==0 && i2==0) ? 0 : -SEG_EPS; // 0/0 are a *bit* more expensive...
		}
		bool variableAllowed(edge e1, edge e2);
		
		ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>* simplicityPool;
		ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>* hintedPoolPrePrice;
		ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>* hintedPoolPostPrice;
		ABA_NONDUPLPOOL<ABA_CONSTRAINT, ABA_VARIABLE>* kuratowskiPool;
	
		// used by Subproblem::pricing
		List<edge> helperEdgelist1;
		List<edge> helperEdgelist2;
		
		void hintsKnKuratowskiMinusOne(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist);
		void hintsKnKuratowskiMinusTwo(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist);
		void hintsKnNodeOrder(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist);
		void hintsKnEdgeOrder(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist);
		void hintsKnmKuratowskiMinusOne(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist);
		void hintsKnmKuratowskiMinusTwo(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist);
		void hintsKnmNodeOrder(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist);
		void hintsKnmEdgeOrder(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist);

	}; // end Master

protected:
	Master blaster; //!< delegate
	friend class Master;

	virtual ReturnType doCall(PlanRep &PG,
			int cc,
			const EdgeArray<int>  &cost,
			const EdgeArray<bool> &forbid, 
			const EdgeArray<unsigned int>  &subgraphs,
			int& crossingNumber) {
		return blaster.doCall(PG, cc, cost, forbid, subgraphs, crossingNumber);
	}
	
public:
	AbacusOptimalCrossingMinimizerSuperPrice() : blaster() {}

	int numHeuristics() const { return blaster.numHeuristics(); }
	void numHeuristics(int i) { blaster.numHeuristics(i); }

	int numStartKuratowskis() const { return blaster.numStartKuratowskis(); } // doesn't do anything yet.
	void numStartKuratowskis(int i) { blaster.numStartKuratowskis(i); }

	int numCutKuratowskis() const { return blaster.numCutKuratowskis(); }
	void numCutKuratowskis(int i) { blaster.numCutKuratowskis(i); }

	int numTryKuratowskis() const { return blaster.numTryKuratowskis(); }
	void numTryKuratowskis(int i) { blaster.numTryKuratowskis(i); }

	int numAddVariables() const { return blaster.numAddVariables(); }
	void numAddVariables(int i) { blaster.numAddVariables(i); }

	int numAddSimilarVariables() const { return blaster.numAddVariables(); }
	void numAddSimilarVariables(int i) { blaster.numAddSimilarVariables(i); }

	int numTryVariables() const { return blaster.numTryVariables(); }
	void numTryVariables(int i) { blaster.numTryVariables(i); }

	int numBoundHeuristics() const { return blaster.numBoundHeuristics(); }
	void numBoundHeuristics(int b) { blaster.numBoundHeuristics(b); }

	int maxMinutes() const { return blaster.maxMinutes(); }
	void maxMinutes(int m) { blaster.maxMinutes(m); }
	
	double roundUp() const { return blaster.roundUp(); }
	void roundUp(double r) { blaster.roundUp(r); }
	
//	bool simpleDrawings const { return blaster.simpleDrawings(); }
//	void simpleDrawings(bool b) { blaster.simpleDrawings(b); }
	
	bool tryCrossingAdjacentVarsFirst() const { return blaster.tryCrossingAdjacentVarsFirst(); }
	void tryCrossingAdjacentVarsFirst(bool b) { blaster.tryCrossingAdjacentVarsFirst(b); }

	PricingInitialization pricingInitialization() const { return blaster.pricingInitialization(); }
	void pricingInitialization(PricingInitialization p) { blaster.pricingInitialization(p); }
	
	bool duplicateKuratowskis() const { return blaster.duplicateKuratowskis(); }
	void duplicateKuratowskis(bool b) { blaster.duplicateKuratowskis(b); }

	bool isOptimal() { return blaster.isOptimal(); }

	char* writeResult() { return blaster.writeResult(); }
	void writeResult(char* b) { blaster.writeResult(b); }
	
	GraphHint graphHint() const { return blaster.graphHint(); }
	void graphHint(GraphHint h) { blaster.graphHint(h); }
	
	int hintEffects() const { return blaster.hintEffects(); }
	void hintEffects(int h) { blaster.hintEffects(h); }
};

std::ostream &operator<<(std::ostream &os, const AbacusOptimalCrossingMinimizerSuperPrice::CrossingInfo& v);
std::ostream &operator<<(std::ostream &os, const AbacusOptimalCrossingMinimizerSuperPrice::CrossingVariable& v);
std::ostream &operator<<(std::ostream &os, /*const*/ AbacusOptimalCrossingMinimizerSuperPrice::SimplicityConstraint& k);
std::ostream &operator<<(std::ostream &os, /*const*/ AbacusOptimalCrossingMinimizerSuperPrice::KuratowskiConstraint& k);

#endif // USE_ABACUS

} // namespace

#endif //OGDF_ABACUS_OPTIMAL_CROSSING_MINIMIZER_SUPER_PRICE_H
