/*
 * $Revision: 1.53 $
 * 
 * last checkin:
 *   $Author: chimani $ 
 *   $Date: 2008-02-20 20:33:07 +1100 (Wed, 20 Feb 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class OptimalCrossingMinimizer
 * 
 * \author Markus Chimani
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

#ifndef OGDF_ABACUS_OPTIMAL_CROSSING_MINIMIZER_SUPER_PRICE_H
#define OGDF_ABACUS_OPTIMAL_CROSSING_MINIMIZER_SUPER_PRICE_H

#include <ogdf/external/abacus.h>

#include <ogdf/basic/basic.h>
#include <ogdf/basic/Logger.h>
#include <ogdf/basic/Graph.h>
#include <ogdf/basic/List.h>
#include <ogdf/basic/Array.h>
#include <ogdf/basic/SList.h>
#include <ogdf/basic/ArrayBuffer.h>
#include <ogdf/basic/EdgeArray.h>
#include <ogdf/basic/GraphCopy.h>
#include <ogdf/graphalg/GraphReduction.h>
#include <ogdf/module/CrossingMinimizationModule.h>
#include <ogdf/basic/ModuleOption.h>
#include <ogdf/planarity/KuratowskiSubdivision.h>
#include <ogdf/basic/MinHeap.h>

#define ATTRIBUTE(T, N) private: T _##N; public: const T& N() const { return _##N; } T& N() { return _##N; } 

namespace ogdf {

class OGDF_EXPORT OptimalCrossingMinimizer : public CrossingMinimizationModule {

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

	enum GraphHint { GH_None, GH_Complete, GH_CompleteBipartite };
	enum HintEffects { HE_KuratowskisMinusOne =        0x0001,  // 1, 2, 4, 8, ...
		               HE_AllSubKuratowskis =          0x0002,
		               HE_EdgeOrder =                  0x0004,
		               HE_NodeOrder =                  0x0008,
		               HE_IterativeLowerBound =        0x0010,
		               HE_HighKuratowskiCutsStatic =   0x0020,
		               HE_ExpensiveKuratowski =        0x0040
		                };
	enum SeparationMode { SM_Simple       = 0x0001,
	                      SM_BoyerMyrvold = 0x0002 };

	typedef int PricingInit;
	static const PricingInit PI_NoPricing = 0;
	static const PricingInit PI_Normal = 1; // generate one segment per edge... 
	                                        // any other number will generate up to as many segments per edge
	enum PricingMode { PM_Greedy       = 0,
		               PM_Few          = 1, 
	                   PM_Reasonable   = 2,
	                   PM_Plenty       = 3 };
	                   
	enum BranchingMode { BM_Traditional = 0,
	                     BM_CompleteOdd = 1 };

	static int completeGuy(int n) {
		return int((n/2) * int((n-1)/2) * int((n-2)/2) * int((n-3)/2)) / 4;
	}
	static int bipartiteZara(int n, int m) {
		return int((n)/2) * int((n-1)/2) * int((m)/2) * int((m-1)/2);
	}
	
	class SimpleSeparationParams {
		ATTRIBUTE( int, runs )
		ATTRIBUTE( int, desperateRuns )
		ATTRIBUTE( int, maxCuts )
	public:
		SimpleSeparationParams() :
			_maxCuts(20),
			_runs(50),
			_desperateRuns(300) {}
	};

	class BoyerMyrvoldSeparationParams {
		ATTRIBUTE( int, runs ) 
		ATTRIBUTE( int, desperateRuns )
		ATTRIBUTE( int, extractions )
		ATTRIBUTE( int, runCuts )
		ATTRIBUTE( int, maxCuts )
		ATTRIBUTE( bool, bundle )
		ATTRIBUTE( bool, noE2 )
		ATTRIBUTE( bool, veryDifferent )
	public:	
		BoyerMyrvoldSeparationParams() : 
			_runs(20),
			_desperateRuns(100), 
			_extractions(100),
			_runCuts(80),
			_maxCuts(1000),
			_bundle(false),
			_noE2(true),
			_veryDifferent(false) {}
	};
	
protected:

	static const double EPS /*= 0.00001*/;
	static const double SEG_EPS /*= 0.0001*/;
	
	class Master; //forward
	class Subproblem; //forward
	class CrossingLocation;
	class CrossingVariable;
	class CrossingLocationComparer;
	class AbacusConstraint; //forward	
	class SimplicityConstraint; //forward
	class SegmentOrderConstraint; //forward
	class FirstSegmentConstraint; //forward
	class KuratowskiConstraint; //forward
	class EdgeOrderConstraint; //forward
	class NodeOrderConstraint; //forward
	
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
		//////////////////////		
	private:
		int crossingNo;
		EdgeArray< List<edge> > crossingEdges;
	};

	struct Segment {
		edge e;
		int seg;
		//////////////////////		
		Segment() : e(0), seg(0) {}
		Segment(edge te, int tseg) : e(te), seg(tseg) {}
		Segment(const Segment& src) : e(src.e), seg(src.seg) {}
		void init(edge te, int tseg) { e=te; seg=tseg; }
		void init(const Segment& src) { e=src.e; seg=src.seg; }
		const int operator==(const Segment& s) {
			return (e == s.e) && (seg == s.seg);
		}
	};
	
	struct CrossingLocation {
		Segment s1, s2;
		CrossingLocation(const Segment& t1, const Segment& t2) {
			if(preferedOrder(t1,t2)) {
				s1.init(t1);
				s2.init(t2);
			} else {
				s1.init(t2);
				s2.init(t1);
			}
		}		
		CrossingLocation(const CrossingLocation& src) : s1(src.s1), s2(src.s2) {}
		inline static bool preferedOrder(edge e1, edge e2) {
			return e1->index() < e2->index();
		}
		inline static bool preferedOrder(Segment s1, Segment s2) {
			return preferedOrder(s1.e, s2.e);
		}
	};

	class CrossingLocationComparer {
	public:
		typedef CrossingLocation* CrossingLocationPointer;
		static int compare(const CrossingLocationPointer & x, const CrossingLocationPointer & y) {
			int ret;
			if((ret = (x->s1.e->index()-y->s1.e->index()))) return ret;
			if((ret = (x->s2.e->index()-y->s2.e->index()))) return ret;
			if((ret = (x->s1.seg-y->s1.seg))) return ret;
			return x->s2.seg-y->s2.seg;
		}
		OGDF_AUGMENT_STATICCOMPARER(CrossingLocationPointer)
	};
	
	class CrossingVariable : public CrossingLocation, public ABA_VARIABLE {
		friend std::ostream &operator<<(std::ostream &os, const OptimalCrossingMinimizer::CrossingVariable& v);
	public:
		CrossingVariable(ABA_MASTER* m, const Segment& s1, const Segment& s2) :
				CrossingLocation(s1,s2),
				ABA_VARIABLE(m,0,0,0, // master / sub=0 / dynamic=false / local=false
					((OptimalCrossingMinimizer::Master*)m)->getCost(s1.e, s2.e)+
						((OptimalCrossingMinimizer::Master*)m)->epsilonify(s1.seg, s2.seg), // obj
					0,1,ABA_VARTYPE::Binary) {} // lowerBound / upperBound / type
		const bool correspondsTo(const Segment& s) {
			return (s1 == s) || (s2 == s);
		}
		const bool correspondsTo(edge e, int num) {
			return (e == s1.e && s1.seg == num)|| (e == s2.e && s2.seg == num);
		}
		const bool correspondsTo(edge e) {
			return e == s1.e || e == s2.e;
		}
		const bool correspondsTo(node n) {
			return s1.e->source() ==  n || s1.e->target() ==  n || s2.e->source() ==  n || s2.e->target() ==  n;
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
		inline OptimalCrossingMinimizer::Master* master() const {
			return (OptimalCrossingMinimizer::Master*)master_;
		}
		OGDF_NEW_DELETE
	};
	
	class SimplicityConstraint : public AbacusConstraint {
		friend std::ostream &operator<<(std::ostream &os, /*const*/ OptimalCrossingMinimizer::SimplicityConstraint& k);
		Segment s;
	public:
		SimplicityConstraint(ABA_MASTER* m, const Segment& t) : 
			AbacusConstraint(m, 0, ABA_CSENSE::Less, 1, 0, 0, 1),  // non-dynamic, non-local, pseudo-liftable
			s(t) {}
		double coeff(ABA_VARIABLE* cvar) {
			return (double) ((CrossingVariable*)cvar)->correspondsTo(s);
		}
		virtual const char* name() {
			return "SimplicityConstraint";
		}
	};
	
	class SegmentOrderConstraint : public AbacusConstraint {
		Segment s;
	public:	
		SegmentOrderConstraint(ABA_MASTER* m, Segment t) : 
			AbacusConstraint(m, 0, ABA_CSENSE::Greater, 0, 0, 0, 1),  // non-dynamic, non-local, pseudo-liftable
			s(t) {}
		double coeff(ABA_VARIABLE* cvar) {
			CrossingVariable& v = *(CrossingVariable*)cvar;
			int pred = s.seg-1;
			//OGDF_ASSERT( pred != 0 );
			if(v.correspondsTo(Segment(s.e, pred < 0 ? 1 : pred))) return 1;
			if(v.correspondsTo(s)) return -1;
			return 0;
		}
		virtual const char* name() {
			return "SegmentOrderConstraint";
		}
	};

	class FirstSegmentConstraint : public AbacusConstraint {
		edge e;
		int length;
	public:
		FirstSegmentConstraint(ABA_MASTER* m, edge t) : 
				AbacusConstraint(m, 0, ABA_CSENSE::Greater, 0, 0, 0, 1),  // non-dynamic, non-local, pseudo-liftable
//				AbacusConstraint(m, 0, ABA_CSENSE::Greater, -1, 0, 0, 1),  // non-dynamic, non-local, pseudo-liftable
				e(t) { 
			length = master()->expandedGraph->chain(e).size();
			OGDF_ASSERT(length > 1);
		}
		double coeff(ABA_VARIABLE* cvar) {
			CrossingVariable& v = *(CrossingVariable*)cvar;
			if(v.s1.e == e && master()->activeVars->numIndices(v.s2, e) == length) {
				if( v.s1.seg == length-1)
					return 1;
				else if( v.s1.seg == 0 )
					return -1;
			} else if(v.s2.e == e && master()->activeVars->numIndices(v.s1, e) == length) {
				if( v.s2.seg == length-1)
					return 1;
				else if( v.s2.seg == 0 )
					return -1;
			}
			return 0;
		}
		edge referenceEdge() { return e; }
		virtual const char* name() {
			return "FirstSegmentConstraint";
		}
	};

	class EdgeOrderConstraint : public AbacusConstraint {
		edge e1, e2;
	public:
		EdgeOrderConstraint(ABA_MASTER* m, edge te1, edge te2) : 
			AbacusConstraint(m, 0, ABA_CSENSE::Greater, 0, 0, 0, 1),  // non-dynamic, non-local, pseudo-liftable
			e1(te1), e2(te2) {}
		double coeff(ABA_VARIABLE* cvar) {
			return ((CrossingVariable*)cvar)->correspondsTo(e1) - ((CrossingVariable*)cvar)->correspondsTo(e2);
		}
		virtual const char* name() {
			return "EdgeOrderConstraint";
		}
	};

	class NodeOrderConstraint : public AbacusConstraint {
		node n1, n2;
	public:
		NodeOrderConstraint(ABA_MASTER* m, node tn1, node tn2) : 
			AbacusConstraint(m, 0, ABA_CSENSE::Greater, 0, 0, 0, 1),  // non-dynamic, non-local, pseudo-liftable
			n1(tn1), n2(tn2) {}
		double coeff(ABA_VARIABLE* cvar) {
			return ((CrossingVariable*)cvar)->correspondsTo(n1) - ((CrossingVariable*)cvar)->correspondsTo(n2);
		}
		virtual const char* name() {
			return "NodeOrderConstraint";
		}
	};
	
	class KuratowskiConstraint : public AbacusConstraint, protected Logger {
	public:
		typedef int Restrictiveness;
		typedef int KuratowskiType;
		static const KuratowskiType KT_K33/* = 0*/; // its on purpose, that K33 differes from KTBip(3,3) and K5 from KTComp(5) !
		static const KuratowskiType KT_K5 /*= -1*/;
		static KuratowskiType KTComplete(int n) { return -n; }
		static KuratowskiType KTBipartite(int n, int m) { return (n-3) << 16 + (m-3); }
		static int typeToCr(KuratowskiType kt) {
			if(kt<0) return completeGuy(-kt);
			bipartiteZara(kt >> 16, kt & 0xffff);
		}
		static Restrictiveness RGreaterPlus(int i) { return i; }
		static Restrictiveness REqualPlus(int i) { return -i-1; }
		static int RPlus(Restrictiveness r) { return (r<0) ? -r-1 : r; }
		static ABA_CSENSE::SENSE RSense(Restrictiveness r) { return (r<0) ? ABA_CSENSE::Equal : ABA_CSENSE::Greater; }
	private:	
		struct SegmentRange {
			int startId; // -1 => including start node
			int endId;   // splitnumber+1 => including end node
			int pathNo;
			SegmentRange(int s, int e, int path) : startId(s), endId(e), pathNo(path) {}
			SegmentRange(const SegmentRange& E) : startId(E.startId), endId(E.endId), pathNo(E.pathNo) {}
			bool contains(int id) {
				return startId<=id && id<=endId;
			}
			bool equals(const SegmentRange& ei) {
				return startId == ei.startId && endId == ei.endId;
			}
		};
		
		EdgeArray<SegmentRange*> edges;
		Array<CrossingLocation*> D; //sorted!
		KuratowskiType type;
	public:
		bool isCompleteOdd() { return type == KT_K5 || (type < 0 && (-type)%2==1); }
	
		friend std::ostream &operator<<(std::ostream &os, /*const*/ OptimalCrossingMinimizer::KuratowskiConstraint& k);
		friend class Subproblem;
		
		KuratowskiConstraint(ABA_MASTER* m, const Subproblem* S, const GraphReduction& R, const KuratowskiSubdivision& K, bool dynamic, int rness = 0, bool local = false) : 
				AbacusConstraint(m, 0, RSense(rness), 
				/*rhs*/ RPlus(rness), dynamic, local, 1),  // non-local, pseudo-liftable
				edges(), D() {
			edges.init(*master()->minimizedGraph,0);
			build(S, R, K);
		}
		
		KuratowskiConstraint(const KuratowskiConstraint& orig) : AbacusConstraint(orig), edges(), D(orig.D), type(orig.type) {
//			OGDF_ASSERT( this->liftable() );
			edges.init(*master()->minimizedGraph,0);
			edge e;
			forall_edges(e, *orig.edges.graphOf()) {
				if(orig.edges[e])
					edges[e] = new SegmentRange(*orig.edges[e]);
			}
		} 
		
		void branchMe(const Subproblem* S, KuratowskiConstraint*& k1, KuratowskiConstraint*& k2) {
			OGDF_ASSERT( this->type == KT_K5 || (this->type < 0 && (-this->type)%2==1));
			OGDF_ASSERT( this->sense()->sense() ==  ABA_CSENSE::Greater );
			OGDF_ASSERT( k1 == NULL && k2 == NULL );
			k1 = new KuratowskiConstraint(*this);
			k2 = new KuratowskiConstraint(*this);
			k1->sub_ = k2->sub_= S;
			k1->local_ = k2->local_ = true;
			k1->sense_ = ABA_CSENSE::Equal;
			k2->rhs_ = this->rhs()+2;  
		}
		
		KuratowskiConstraint(ABA_MASTER* m, KuratowskiType ltype, bool dynamic, int rness = 0, bool local = false) : 
				AbacusConstraint(m, 0, RSense(rness), 
				typeToCr(ltype) + RPlus(rness), dynamic, local, 1),  // non-local, pseudo-liftable
				edges(), D(), type(ltype) {
			edges.init(*master()->minimizedGraph,0);
		}
		
		~KuratowskiConstraint() {
			edge e;
			forall_edges(e, *master()->minimizedGraph) {
				if(edges[e])
					delete edges[e];
			}
		}
		
		void addEdge(edge e, int path) {
			edges[e] = new SegmentRange(-1, master()->expansionFactor+1, path);
		}
		
		void adapt(int varidx, CrossingVariable* newvar, edge e, int oldi, int newi) {
			D[varidx] = newvar;
			if(edges[e]->startId == oldi)
				edges[e]->startId = newi;
			else
				edges[e]->endId = newi;
		}
		
		void build(const Subproblem* S, const GraphReduction& R, const KuratowskiSubdivision& K);
		void addAccordingCrossing(const Subproblem* S, const PlanRep& I, edge e, int eid, List<CrossingLocation*>& L);
		
		static bool crossingApplicable(KuratowskiType kt, int p1, int p2) {
			if(p1==p2) return false; // never applicable on the same path
			if( kt == KT_K33) {
				return p1/3 != p2/3 && p1%3 != p2%3;				
			} 
			if( kt == KT_K5 ) {
				if(p1 > p2) { int t = p1; p1=p2; p2=t; }
				switch(p1) {
				case 0: return p2>=7;
				case 1: return p2==5 || p2==6 || p2==9;
				case 2: return p2==4 || p2==6 || p2==8;
				case 3: return p2==4 || p2==5 || p2==7;
				case 4: return p2==9;
				case 5: return p2==8;
				case 6: return p2==7;
				default: return false;
				}
			}
			return true; // TODO: gosh... i have no idea on the creation... need to check that more thoroughly
		}
		
		virtual const char* name() /*const*/ {
			return "KuratowskiConstraint";
		}
		
		virtual unsigned hashKey() /*const*/ {
			int h = D.size();			
			edge e;
			forall_edges(e, *edges.graphOf()) {				
				h = (int) ((1.1*h) + (edges[e] ? (edges[e]->startId + 2*edges[e]->endId) : 0 ));
			}
			for(int i = D.size(); i-->0;) {
				h = (int) ((1.1*h) + D[i]->s1.e->index() + D[i]->s2.e->index() + D[i]->s1.seg + 2*D[i]->s2.seg);
			}
			return h;
		}
		
		virtual bool equal(ABA_CONVAR *cv) /*const*/ {
			return equals(*(KuratowskiConstraint*)cv);
		}
		
		bool equals(const KuratowskiConstraint& ki) const {
			edge e;
			if( rhs_ != ki.rhs_ || sense_.sense() != ki.sense_.sense() )
				return false;  
			if(D.size() != ki.D.size())
				return false;
			forall_edges(e, *edges.graphOf()) {
				if(edges[e]==0 ^ ki.edges[e]==0) // one is defined one is not
					return false;
				if(edges[e] && !edges[e]->equals(*ki.edges[e])) //defined but not equal
					return false;
			}
			for(int i = D.size(); i-->0;) {
				if(!master()->crossingLocationComparer.equal(D[i],ki.D[i]))
					return false;
			}
			return true; // no missmatch found
		}

		double coeff(ABA_VARIABLE* cvar) /*const*/ {
			CrossingVariable* v = (CrossingVariable*)cvar;
			if(edges[v->s1.e] && edges[v->s2.e] && 
					edges[v->s1.e]->contains(v->s1.seg) && 
					edges[v->s2.e]->contains(v->s2.seg) ) //it exists
				return D.binarySearch(v, master()->crossingLocationComparer)>=0 ? -1 : 
					( crossingApplicable(type, edges[v->s1.e]->pathNo, edges[v->s2.e]->pathNo) ? 1 : 0);
			else
				return 0;
		}		
				
	};
	
	class Subproblem : public ABA_SUB, protected Logger {
		
		void initCurrents() {
			currentIntegerSolution = new PlanRep(*(master()->expandedGraph));
			currentRoundedSolution = new PlanRep(*(master()->expandedGraph));
			currentIntegerCrossings = new List<CrossingLocation*>();
			currentRoundedCrossings = new List<CrossingLocation*>();
			currentIntegerSolution->initCC(0);
			currentRoundedSolution->initCC(0);
			storedCurrentCrossings = false;
		}
		void clearCurrents() {
			if(currentIntegerSolution) { delete currentIntegerSolution; currentIntegerSolution = 0; }
			if(currentRoundedSolution) { delete currentRoundedSolution; currentRoundedSolution = 0; }
			if(!storedCurrentCrossings) {
				if(currentIntegerCrossings) { delete currentIntegerCrossings; currentIntegerCrossings = 0; }
				if(currentRoundedCrossings) { delete currentRoundedCrossings; currentRoundedCrossings = 0; }
			}
		}
		
	public:
		Subproblem(ABA_MASTER* m, ABA_SUB* father, ABA_BRANCHRULE* br) :
				ABA_SUB(m, father, br),
				currentIntegerSolution(0),
				currentRoundedSolution(0), 
				currentIntegerCrossings(0),
				currentRoundedCrossings(0),
				generatedConVar(0),
				storedCurrentCrossings(false),
				duplicatedKuratowskiList(),
				bufferedSimplicities(),
				deactNonLiftCons(10) {
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
					generatedConVar(0),
					storedCurrentCrossings(false),
					duplicatedKuratowskiList(),
					bufferedSimplicities(),
					deactNonLiftCons(10) {
			lout() << "Subproblem " << id() << " generated. No Father\n";
		}
		~Subproblem() {
			clearCurrents();
			clearDuplicatedKuratowskiList();
			clearDeactNonLiftCons();
			OGDF_ASSERT( bufferedSimplicities.empty() );
		};
		ABA_SUB *generateSon(ABA_BRANCHRULE* br) {
			if(master()->nSub()==1) { // i'm the root subproblem, and i'm gonna die -> let's give a status report...
				sout() << "\t" << ceil(this->dualBound()-0.5) // lower bound
				       << "\t" << master()->primalBound() // upper bound
				       << "\t" << master()->totalTime()->seconds();
			}
			clearCurrents();
			clearDuplicatedKuratowskiList();
			clearDeactNonLiftCons();
			OGDF_ASSERT( bufferedSimplicities.empty() );
			return new Subproblem(master(), this, br); 
		}
		bool feasible();
		int separate();
		int pricing();
		int makeFeasible();
		int improve(double& d);
		int solveLp();
		int generateBranchRules(ABA_BUFFER<ABA_BRANCHRULE*> &rules);
		
		int separateBoyerMyrvold(const BoyerMyrvoldSeparationParams& p);
		int separateSimple(const SimpleSeparationParams& p);
		
		bool checkSimplicitiesAndPrice();
		void deactivateNonLiftableConstraints(/*const*/ ABA_BUFFER<ABA_VARIABLE*>* vars);
		void duplicateKuratowskis(CrossingVariable* cvar, SList<KuratowskiConstraint*>& L);		
		void realizeVariable(CrossingVariable* cvar, double val);
		int checkKnHighKuratowskiCutsStatic();
		int checkKnmHighKuratowskiCutsStatic();

		PlanRep* currentIntegerSolution;
		PlanRep* currentRoundedSolution;
		List<CrossingLocation*>* currentIntegerCrossings;
		List<CrossingLocation*>* currentRoundedCrossings;

	protected:
	
		int generatedConVar;
		bool storedCurrentCrossings;
		SList<KuratowskiConstraint*> duplicatedKuratowskiList;
		SList<AbacusConstraint*> bufferedSimplicities;
		ArrayBuffer< ABA_POOLSLOT<ABA_CONSTRAINT, ABA_VARIABLE>* > deactNonLiftCons;
		
		void clearDuplicatedKuratowskiList() {
			while(!duplicatedKuratowskiList.empty())
				delete duplicatedKuratowskiList.popFrontRet();
		}
		
		void clearDeactNonLiftCons() {
			deactNonLiftCons.init(0);
		}
		
		OptimalCrossingMinimizer::Master* master() const {
			return (OptimalCrossingMinimizer::Master*)master_;
		}
		
		void findKuratowski(Graph& R, KuratowskiSubdivision& K);
		OptimalCrossingMinimizer::CrossingConfiguration* callBoundHeuristic(bool integer);
		inline double generateVariable(edge e1, int e1id, edge e2, int e2id, OptimalCrossingMinimizer::CrossingVariable** var);
	};
	
	friend std::ostream &operator<<(std::ostream &os, const OptimalCrossingMinimizer::CrossingLocation& v);
	friend std::ostream &operator<<(std::ostream &os, const OptimalCrossingMinimizer::CrossingVariable& v);
	friend std::ostream &operator<<(std::ostream &os, /*const*/ OptimalCrossingMinimizer::KuratowskiConstraint& k);
	friend std::ostream &operator<<(std::ostream &os, /*const*/ OptimalCrossingMinimizer::SimplicityConstraint& k);

	class Master : /*public CrossingMinimizationModule,*/ public ABA_MASTER, protected Logger {

		static int MAX_PAIRS(int e) {
			return (e*e - e) / 2;
		}
		
		class ActiveVariables {
			OptimalCrossingMinimizer::Master* master;
			EdgeArray<int> perEdge;
			EdgeArray< EdgeArray<int* > > perSegmentAndEdge;
			
		public:
			ActiveVariables(OptimalCrossingMinimizer::Master* m) : master(m),
					perEdge(*(m->minimizedGraph),0), perSegmentAndEdge(*(m->minimizedGraph)) {
				edge e, ee;
				const Graph& mG = *(master->minimizedGraph);
				forall_edges(e, mG) {
					perSegmentAndEdge[e].init(mG);
					forall_edges(ee, mG) {
						int* s = new int[master->expansionFactor];
						for(int i = master->expansionFactor; i-->0;) s[i] = 0;
						perSegmentAndEdge[e][ee] = s;
					}
				}			
			}
			~ActiveVariables() {
				edge e, ee;
				const Graph& mG = *(master->minimizedGraph);
				forall_edges(e, mG) {
					forall_edges(ee, mG) {
						if(perSegmentAndEdge[e][ee])
							delete[] perSegmentAndEdge[e][ee];
					}
				}			
			}
			
			const int numIndices(edge e)                   { return perEdge[e]; }
//			int numIndices(edge e, edge e2)          { return perEdgePair[e][e2].num(); }
			const int numIndices(const Segment& s1, edge e2) { return perSegmentAndEdge[s1.e][e2][s1.seg]; }
//			int numIndices(edge e, int seg, edge e2) { return perEdgeSegmentAndEdge[e][e2][seg].num(); }

			const bool isNew(edge e, int seg)              { return seg == perEdge[e]; }
			const int numFree(edge e)                      { return master->expandedGraph->chain(e).size() - perEdge[e]; }
			
			void registerVariable(CrossingVariable* v) {
//				#ifdef OGDF_DEBUG
//				cout << "Registering Variable: (" << v->s1.e->index() << "/" << v->s1.seg << ") x ("  << v->s2.e->index() << "/" << v->s2.seg << ")\n";
//				#endif
				OGDF_ASSERT(
					perSegmentAndEdge[v->s1.e][v->s2.e][v->s1.seg] == v->s2.seg ||
					perSegmentAndEdge[v->s2.e][v->s1.e][v->s2.seg] == v->s1.seg
				);
				ensure(perEdge[v->s1.e], v->s1.seg);
				ensure(perEdge[v->s2.e], v->s2.seg);
				ensure(perSegmentAndEdge[v->s1.e][v->s2.e][v->s1.seg], v->s2.seg);
				ensure(perSegmentAndEdge[v->s2.e][v->s1.e][v->s2.seg], v->s1.seg);
				master->m_usedVariables++;
			}
		private:
			void ensure(int& reg, int seg) {
				OGDF_ASSERT( seg >= 0 );
				OGDF_ASSERT( seg <= reg );
				if(reg == seg) ++reg;
			}
		};
		
		friend class ActiveVariables;
		friend class Subproblem;
		friend class CrossingVariable;
		friend class KuratowskiConstraint;
		friend class FirstSegmentConstraint;
		friend class OptimalCrossingMinimizer;

		void setDefaultSettings();

	public:
	
		Master() :
				ABA_MASTER("OptimalCrossingMinimizer::Master",1, true ,ABA_OPTSENSE::Min),
	//			lastIntegerCrossings(0),
	//			lastRoundedCrossings(0),
				bestSolution(0),
				activeVars(0),
				m_isTrivial(false) {
			setDefaultSettings();
			lastIntegerCrossings = new List<CrossingLocation*>;
			lastRoundedCrossings = new List<CrossingLocation*>;
		}	
		~Master() {
			if(lastIntegerCrossings) delete lastIntegerCrossings;
			if(lastRoundedCrossings) delete lastRoundedCrossings;
			delete activeVars;
			delete simplicityPool;
			delete kuratowskiPool;
			delete hintedPool;
			delete branchingPool;
		}
		
		ABA_SUB* firstSub() {
			return new OptimalCrossingMinimizer::Subproblem(this, 1000, numExpMaxCrossingPairs, upperbound, false);
		}
	
		void initializeOptimization();
		
		int enumerationStrategy(ABA_SUB* s1, ABA_SUB* s2);
		
		CrossingLocationComparer crossingLocationComparer;
		
		List<CrossingLocation*>* lastIntegerCrossings;
		List<CrossingLocation*>* lastRoundedCrossings;
		
		bool equalCrossingLists(List<CrossingLocation*>* L1, bool integerList) {
			List<CrossingLocation*>* L2 = integerList ? lastIntegerCrossings : lastRoundedCrossings;
			if(L1->size() != L2->size())
				return false;
			ListConstIterator<CrossingLocation*> it1 = L1->begin();
			ListConstIterator<CrossingLocation*> it2 = L2->begin();
			for(; it1.valid(); ++it1, ++it2)
				if(!crossingLocationComparer.equal(*it1, *it2))
					return false;
			return true;
		}
		
	public:
		int numStartHeuristics() const { return m_numStartHeuristics; }
		void numStartHeuristics(int i) { m_numStartHeuristics = i; }
	
		void setStartHeuristic(CrossingMinimizationModule* p) { m_startHeuristic.set(p); }
		void setBoundHeuristic(CrossingMinimizationModule* p) { m_boundHeuristic.set(p); }

		PricingInit pricingInit() const { return m_pricingInit; }
		void pricingInit(PricingInit p) { m_pricingInit = p; }

		PricingMode pricingMode() const { return m_pricingMode; }
		void pricingMode(PricingMode p) { m_pricingMode = p; }

		int separationMode() const { return m_separationMode; }
		void separationMode(int i) { m_separationMode=i; }

		BranchingMode branchingMode() const { return m_branchingMode; }
		void branchingMode(BranchingMode p) { m_branchingMode = p; }

		const BoyerMyrvoldSeparationParams& startBoyerMyrvoldSeparationParams() const { return m_startBoyerMyrvoldSeparationParams; }
		BoyerMyrvoldSeparationParams& startBoyerMyrvoldSeparationParams() { return m_startBoyerMyrvoldSeparationParams; }
	
		const SimpleSeparationParams& startSimpleSeparationParams() const { return m_startSimpleSeparationParams; }
		SimpleSeparationParams& startSimpleSeparationParams() { return m_startSimpleSeparationParams; }

		const BoyerMyrvoldSeparationParams& boyerMyrvoldSeparationParams() const { return m_boyerMyrvoldSeparationParams; }
		BoyerMyrvoldSeparationParams& boyerMyrvoldSeparationParams() { return m_boyerMyrvoldSeparationParams; }
	
		const SimpleSeparationParams& simpleSeparationParams() const { return m_simpleSeparationParams; }
		SimpleSeparationParams& simpleSeparationParams() { return m_simpleSeparationParams; }
/*
		int numCutKuratowskis() const { return m_numCutKuratowskis; }
		void numCutKuratowskis(int i) { m_numCutKuratowskis = i; }

		int numTryKuratowskis() const { return m_numTryKuratowskis; }
		void numTryKuratowskis(int i) { m_numTryKuratowskis = i; }

		int numDesperateTryKuratowskis() const { return m_numDesperateTryKuratowskis; }
		void numDesperateTryKuratowskis(int i) { m_numDesperateTryKuratowskis = i; }

		int kuratowskiExtractionDesperateRuns() const { return m_kuratowskiExtractionDesperateRuns; }
		void kuratowskiExtractionDesperateRuns(int i) { m_kuratowskiExtractionDesperateRuns=i; }

		int kuratowskiExtractionRuns() const { return m_kuratowskiExtractionRuns; }
		void kuratowskiExtractionRuns(int i) { m_kuratowskiExtractionRuns=i; }
	
		int kuratowskiExtractionExtractions() const { return m_kuratowskiExtractionExtractions; }
		void kuratowskiExtractionExtractions(int i) { m_kuratowskiExtractionExtractions=i; }
	
		int kuratowskiExtractionRunLimit() const { return m_kuratowskiExtractionRunLimit; }
		void kuratowskiExtractionRunLimit(int i) { m_kuratowskiExtractionRunLimit=i; }
	
		int kuratowskiExtractionAllLimit() const { return m_kuratowskiExtractionAllLimit; }
		void kuratowskiExtractionAllLimit(int i) { m_kuratowskiExtractionAllLimit=i; }
	
		bool kuratowskiExtractionBundle() const { return m_kuratowskiExtractionBundle; }
		void kuratowskiExtractionBundle(bool b) { m_kuratowskiExtractionBundle=b; }
	
		bool kuratowskiExtractionNoE2() const { return m_kuratowskiExtractionNoE2; }
		void kuratowskiExtractionNoE2(bool b) { m_kuratowskiExtractionNoE2=b; }
	
		bool kuratowskiExtractionVeryDifferent() const { return m_kuratowskiExtractionVeryDifferent; }
		void kuratowskiExtractionVeryDifferent(bool b) { m_kuratowskiExtractionVeryDifferent=b; }

		int kuratowskiExtractionStartRuns() const { return m_kuratowskiExtractionStartRuns; }
		void kuratowskiExtractionStartRuns(int i) { m_kuratowskiExtractionStartRuns=i; }
	
		int kuratowskiExtractionStartExtractions() const { return m_kuratowskiExtractionStartExtractions; }
		void kuratowskiExtractionStartExtractions(int i) { m_kuratowskiExtractionStartExtractions=i; }
	
		int kuratowskiExtractionStartRunLimit() const { return m_kuratowskiExtractionStartRunLimit; }
		void kuratowskiExtractionStartRunLimit(int i) { m_kuratowskiExtractionStartRunLimit=i; }
	
		int kuratowskiExtractionStartAllLimit() const { return m_kuratowskiExtractionStartAllLimit; }
		void kuratowskiExtractionStartAllLimit(int i) { m_kuratowskiExtractionStartAllLimit=i; }
	
		bool kuratowskiExtractionStartBundle() const { return m_kuratowskiExtractionStartBundle; }
		void kuratowskiExtractionStartBundle(bool b) { m_kuratowskiExtractionStartBundle=b; }
	
		bool kuratowskiExtractionStartNoE2() const { return m_kuratowskiExtractionStartNoE2; }
		void kuratowskiExtractionStartNoE2(bool b) { m_kuratowskiExtractionStartNoE2=b; }

		bool kuratowskiExtractionStartVeryDifferent() const { return m_kuratowskiExtractionStartVeryDifferent; }
		void kuratowskiExtractionStartVeryDifferent(bool b) { m_kuratowskiExtractionStartVeryDifferent=b; }
*/
		int numCutHighKuratowskis() const { return m_numCutHighKuratowskis; }
		void numCutHighKuratowskis(int i) { m_numCutHighKuratowskis = i; }

		int numBaseEdgesForCutHighKuratowskis() const { return m_numBaseEdgesForCutHighKuratowskis; }
		void numBaseEdgesForCutHighKuratowskis(int i) { m_numBaseEdgesForCutHighKuratowskis = i; }

		int maxMinutes() const { return m_maxMinutes; }
		void maxMinutes(int m) { m_maxMinutes = m; }
		
		double roundUp() const { return m_roundUp; }
		void roundUp(double r) { m_roundUp = r; }
	
		bool isOptimal() { return m_isTrivial || (status() == Optimal); }
	
		char* writeResult() { return m_writeResult; }
		void writeResult(char* b) { m_writeResult = b; }
		
		bool writeIntermediateResultsToo() { return m_writeIntermediateResultsToo; }
		void writeIntermediateResultsToo(bool b) { m_writeIntermediateResultsToo = b; }
		
		GraphHint graphHint() const { return m_graphHint; }
		void graphHint(GraphHint h) { m_graphHint = h; }
		
		int hintEffects() const { return m_hintEffects; }
		void hintEffects(int h) { m_hintEffects = h; }
		
		int duplicateKuratowskis() const { return m_duplicateKuratowskis; }
		void duplicateKuratowskis(int n) { m_duplicateKuratowskis = n; }
		
		bool reduceMemory() const { return m_reduceMemory; }
		void reduceMemory(bool b) { m_reduceMemory = b; }
		
	protected:
		virtual ReturnType doCall(PlanRep &PG,
			int cc,
			const EdgeArray<int>  &cost,
			const EdgeArray<bool> &forbid, 
			const EdgeArray<unsigned int>  &subgraphs,
			int& crossingNumber);
			
	private:
	
		OptimalCrossingMinimizer::CrossingConfiguration* bestSolution;
		ActiveVariables* activeVars;
		
		bool m_isTrivial;
		
		int m_minVariables;
		int m_maxVariables;
		int m_startVariables;
		int m_usedVariables;
		
		void updateBestSolution(OptimalCrossingMinimizer::CrossingConfiguration* ncc) {
			if(bestSolution == ncc) return; // repetition -> init
			upperBoundSource = OptimalCrossingMinimizer::Master::SS_ILP_Heuristic;
			if(bestSolution) delete bestSolution;
			bestSolution = ncc;
			if(writeIntermediateResultsToo()) doWriteBestSolution();
		}
	
		int getCost(edge e1, edge e2) { // call with e1, e2 of minimizedGraph			
			return useCost() ? cost[e1]*cost[e2] : 1;
		}		
		double epsilonify(int i1, int i2) {
			// espilonify++
			return (i1==0 && i2==0) ? 0 : -SEG_EPS-(SEG_EPS*(1-(2*i1+3*i2)/double(5*(expansionFactor-1)))); // 0/0 are a *bit* more expensive...			
			// simple espilonify
//			return (i1==0 && i2==0) ? 0 : -SEG_EPS; // 0/0 are a *bit* more expensive...			
		}
		bool variableAllowed(edge e1, edge e2);
		
		ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>* simplicityPool;
		ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>* hintedPool;
		ABA_NONDUPLPOOL<ABA_CONSTRAINT, ABA_VARIABLE>* kuratowskiPool;
		ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>* branchingPool;
		
	
		// used by Subproblem::pricing
//		List<edge> helperEdgelist1;
//		List<edge> helperEdgelist2;
		
		void hintsKnKuratowskiMinusOne(List<ABA_CONSTRAINT*>& prelist);
		void hintsKnAllSubKuratowskis(List<ABA_CONSTRAINT*>& prelist);
		void hintsKnNodeOrder(List<ABA_CONSTRAINT*>& prelist);
		void hintsKnEdgeOrder(List<ABA_CONSTRAINT*>& prelist);
		void helperHintsKnAllSubKuratowskis(NodeArray<bool>& aktnodes, node posNode, int num, List<ABA_CONSTRAINT*>& prelist);
		void hintsKnExpensiveKuratowski(List<ABA_CONSTRAINT*>& prelist);

		void hintsKnmKuratowskiMinusOne(List<ABA_CONSTRAINT*>& prelist);
		void hintsKnmAllSubKuratowskis(List<ABA_CONSTRAINT*>& prelist);
		void hintsKnmNodeOrder(List<ABA_CONSTRAINT*>& prelist);
		void hintsKnmEdgeOrder(List<ABA_CONSTRAINT*>& prelist);
		
		enum SolutionSource { SS_Trivial, SS_ILP, SS_ILP_Heuristic, SS_Heuristic, SS_Kn, SS_Knm, SS_NoSolution };
	
		const Graph*           givenGraph;
		GraphReduction*        minimizedGraph;
		GraphCopy*             expandedGraph;
		PlanRep*               resultingGraph;
		EdgeArray<int>         cost;
		EdgeArray<edge>        costEdge;
		EdgeArray<bool>        forbid;
		EdgeArray<unsigned int> subgraphs;
	
		int m_numStartHeuristics;
		ModuleOption<CrossingMinimizationModule> m_startHeuristic;
		ModuleOption<CrossingMinimizationModule> m_boundHeuristic;
//		SubgraphPlanarizer m_solutionChecker;

		int m_separationMode;
		PricingInit m_pricingInit;
		PricingMode m_pricingMode;
		BranchingMode m_branchingMode;
		BoyerMyrvoldSeparationParams m_startBoyerMyrvoldSeparationParams;
		BoyerMyrvoldSeparationParams m_boyerMyrvoldSeparationParams;
		SimpleSeparationParams m_startSimpleSeparationParams;
		SimpleSeparationParams m_simpleSeparationParams;
		
/*		int m_numStartKuratowskis;
		int m_numCutKuratowskis;
		int m_numTryKuratowskis;
		int m_numDesperateTryKuratowskis;
		int m_kuratowskiExtractionDesperateRuns;
		int m_kuratowskiExtractionRuns;
		int m_kuratowskiExtractionExtractions;
		int m_kuratowskiExtractionRunLimit;
		int m_kuratowskiExtractionAllLimit;
		bool m_kuratowskiExtractionBundle;
		bool m_kuratowskiExtractionNoE2;
		bool m_kuratowskiExtractionVeryDifferent;
		int m_kuratowskiExtractionStartRuns;
		int m_kuratowskiExtractionStartExtractions;
		int m_kuratowskiExtractionStartRunLimit;
		int m_kuratowskiExtractionStartAllLimit;
		bool m_kuratowskiExtractionStartBundle;
		bool m_kuratowskiExtractionStartNoE2;
		bool m_kuratowskiExtractionStartVeryDifferent;*/

		int m_numCutHighKuratowskis;
		int m_numBaseEdgesForCutHighKuratowskis;
		
		int m_duplicateKuratowskis;

		int m_maxMinutes;
		double m_roundUp;

		GraphHint m_graphHint;
		int m_hintEffects;
		
		char* m_writeResult;
		bool m_writeIntermediateResultsToo;
	
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
		
		bool m_reduceMemory;
		
		bool probablyUpdateLowerBound(int lb) {
			if(lb <= lowerbound) return false;
			lout(LL_MINOR) << "New Lower Bound! " << lb << "\n";
			lowerbound = lb;
			return true;    
		}
		bool probablyUpdateUpperBound(int ub, SolutionSource ubs) {
			--ub; // only something better than that is interesting...
			if(ub >= upperbound) return false;
			lout(/*LL_MINOR*/) << "New Upper Bound: " << ub << " [" << upperBoundSource << "->" << ubs << "]\n";
			upperbound = ub;
			upperBoundSource = ubs;
			return true;      
		}
		
		void calcLowerBounds();
		void calcUpperBounds();	
		
		//! \reminder OptimalCrossingMinimizerBase assumes that the given graph is connected	
		OptimalCrossingMinimizer::CrossingConfiguration* helpCall(PlanRep &_PG, int cc, const EdgeArray<int>& _cost, const EdgeArray<bool> & _forbid, int& crossingNumber);
		void generateMinimizedGraphCostAndForbid(const EdgeArray<int>  &_cost, const EdgeArray<bool> &_forbid);
		void generateExpandedGraph();
		OptimalCrossingMinimizer::CrossingConfiguration* createHeuristicStartSolution();
		
		void doWriteBestSolution();
		
		bool m_useCost;
		bool m_useForbid;
		bool m_useSubgraphs;
		bool useCost() { return m_useCost; }
		bool useForbid() { return m_useForbid; }
		bool useSubgraphs() { return m_useSubgraphs; }
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
		blaster.m_useCost = useCost();
		blaster.m_useForbid = useForbid();
		blaster.m_useSubgraphs = useSubgraphs();
		return blaster.doCall(PG, cc, cost, forbid, subgraphs, crossingNumber);
	}
	
public:
	OptimalCrossingMinimizer() : blaster() {}

	int numStartHeuristics() const { return blaster.numStartHeuristics(); }
	void numStartHeuristics(int i) { blaster.numStartHeuristics(i); }

	void setStartHeuristic(CrossingMinimizationModule* p) { blaster.setStartHeuristic(p); }
	void setBoundHeuristic(CrossingMinimizationModule* p) { blaster.setBoundHeuristic(p); }
	
	PricingInit pricingInit() const { return blaster.pricingInit(); }
	void pricingInit(PricingInit p) { blaster.pricingInit(p); }

	PricingMode pricingMode() const { return blaster.pricingMode(); }
	void pricingMode(PricingMode p) { blaster.pricingMode(p); }

	int separationMode() const { return blaster.separationMode(); }
	void separationMode(int i) { blaster.separationMode(i); }
	
	BranchingMode branchingMode() const { return blaster.branchingMode(); }
	void branchingMode(BranchingMode p) { blaster.branchingMode(p); }

	const BoyerMyrvoldSeparationParams& startBoyerMyrvoldSeparationParams() const { return blaster.startBoyerMyrvoldSeparationParams(); }
	BoyerMyrvoldSeparationParams& startBoyerMyrvoldSeparationParams() { return blaster.startBoyerMyrvoldSeparationParams(); }

	const SimpleSeparationParams& startSimpleSeparationParams() const { return blaster.startSimpleSeparationParams(); }
	SimpleSeparationParams& startSimpleSeparationParams() { return blaster.startSimpleSeparationParams(); }

	const BoyerMyrvoldSeparationParams& boyerMyrvoldSeparationParams() const { return blaster.boyerMyrvoldSeparationParams(); }
	BoyerMyrvoldSeparationParams& boyerMyrvoldSeparationParams() { return blaster.boyerMyrvoldSeparationParams(); }

	const SimpleSeparationParams& simpleSeparationParams() const { return blaster.simpleSeparationParams(); }
	SimpleSeparationParams& simpleSeparationParams() { return blaster.simpleSeparationParams(); }
	
/*
	int numStartKuratowskis() const { return blaster.numStartKuratowskis(); } // doesn't do anything yet.
	void numStartKuratowskis(int i) { blaster.numStartKuratowskis(i); }

	int numCutKuratowskis() const { return blaster.numCutKuratowskis(); }
	void numCutKuratowskis(int i) { blaster.numCutKuratowskis(i); }

	int numTryKuratowskis() const { return blaster.numTryKuratowskis(); }
	void numTryKuratowskis(int i) { blaster.numTryKuratowskis(i); }

	int numDesperateTryKuratowskis() const { return blaster.numDesperateTryKuratowskis(); }
	void numDesperateTryKuratowskis(int i) { blaster.numDesperateTryKuratowskis(i); }

	int kuratowskiExtractionDesperateRuns() const { return blaster.kuratowskiExtractionDesperateRuns(); }
	void kuratowskiExtractionDesperateRuns(int i) { blaster.kuratowskiExtractionDesperateRuns(i); }

	int kuratowskiExtractionRuns() const { return blaster.kuratowskiExtractionRuns(); }
	void kuratowskiExtractionRuns(int i) { blaster.kuratowskiExtractionRuns(i); }

	int kuratowskiExtractionExtractions() const { return blaster.kuratowskiExtractionExtractions(); }
	void kuratowskiExtractionExtractions(int i) { blaster.kuratowskiExtractionExtractions(i); }

	int kuratowskiExtractionRunLimit() const { return blaster.kuratowskiExtractionRunLimit(); }
	void kuratowskiExtractionRunLimit(int i) { blaster.kuratowskiExtractionRunLimit(i); }

	int kuratowskiExtractionAllLimit() const { return blaster.kuratowskiExtractionAllLimit(); }
	void kuratowskiExtractionAllLimit(int i) { blaster.kuratowskiExtractionAllLimit(i); }

	bool kuratowskiExtractionBundle() const { return blaster.kuratowskiExtractionBundle(); }
	void kuratowskiExtractionBundle(bool b) { blaster.kuratowskiExtractionBundle(b); }

	bool kuratowskiExtractionNoE2() const { return blaster.kuratowskiExtractionNoE2(); }
	void kuratowskiExtractionNoE2(bool b) { blaster.kuratowskiExtractionNoE2(b); }

	bool kuratowskiExtractionVeryDifferent() const { return blaster.kuratowskiExtractionVeryDifferent(); }
	void kuratowskiExtractionVeryDifferent(bool b) { blaster.kuratowskiExtractionVeryDifferent(b); }

	int kuratowskiExtractionStartRuns() const { return blaster.kuratowskiExtractionStartRuns(); }
	void kuratowskiExtractionStartRuns(int i) { blaster.kuratowskiExtractionStartRuns(i); }

	int kuratowskiExtractionStartExtractions() const { return blaster.kuratowskiExtractionStartExtractions(); }
	void kuratowskiExtractionStartExtractions(int i) { blaster.kuratowskiExtractionStartExtractions(i); }

	int kuratowskiExtractionStartRunLimit() const { return blaster.kuratowskiExtractionStartRunLimit(); }
	void kuratowskiExtractionStartRunLimit(int i) { blaster.kuratowskiExtractionStartRunLimit(i); }

	int kuratowskiExtractionStartAllLimit() const { return blaster.kuratowskiExtractionStartAllLimit(); }
	void kuratowskiExtractionStartAllLimit(int i) { blaster.kuratowskiExtractionStartAllLimit(i); }

	bool kuratowskiExtractionStartBundle() const { return blaster.kuratowskiExtractionStartBundle(); }
	void kuratowskiExtractionStartBundle(bool b) { blaster.kuratowskiExtractionStartBundle(b); }

	bool kuratowskiExtractionStartNoE2() const { return blaster.kuratowskiExtractionStartNoE2(); }
	void kuratowskiExtractionStartNoE2(bool b) { blaster.kuratowskiExtractionStartNoE2(b); }

	bool kuratowskiExtractionStartVeryDifferent() const { return blaster.kuratowskiExtractionStartVeryDifferent(); }
	void kuratowskiExtractionStartVeryDifferent(bool b) { blaster.kuratowskiExtractionStartVeryDifferent(b); }
*/


	int numCutHighKuratowskis() const { return blaster.numCutHighKuratowskis(); }
	void numCutHighKuratowskis(int i) { blaster.numCutHighKuratowskis(i); }

	int numBaseEdgesForCutHighKuratowskis() const { return blaster.numBaseEdgesForCutHighKuratowskis(); }
	void numBaseEdgesForCutHighKuratowskis(int i) { blaster.numBaseEdgesForCutHighKuratowskis(i); }

	int maxMinutes() const { return blaster.maxMinutes(); }
	void maxMinutes(int m) { blaster.maxMinutes(m); }
	
	double roundUp() const { return blaster.roundUp(); }
	void roundUp(double r) { blaster.roundUp(r); }
	
	bool isOptimal() { return blaster.isOptimal(); }

	char* writeResult() { return blaster.writeResult(); }
	void writeResult(char* b) { blaster.writeResult(b); }
	
	bool writeIntermediateResultsToo() { return blaster.writeIntermediateResultsToo(); }
	void writeIntermediateResultsToo(bool b) { blaster.writeIntermediateResultsToo(b); }

	GraphHint graphHint() const { return blaster.graphHint(); }
	void graphHint(GraphHint h) { blaster.graphHint(h); }
	
	int hintEffects() const { return blaster.hintEffects(); }
	void hintEffects(int h) { blaster.hintEffects(h); }

	int tailOffNLp() const { return blaster.tailOffNLp(); }
	void tailOffNLp(int n) { blaster.tailOffNLp(n); }

	double tailOffPercent() const { return blaster.tailOffPercent(); }
	void tailOffPercent(double p) { blaster.tailOffPercent(p); }

	int duplicateKuratowskis() const { return blaster.duplicateKuratowskis(); }
	void duplicateKuratowskis(int n) { blaster.duplicateKuratowskis(n); }
	
	bool reduceMemory() const { return blaster.reduceMemory(); }
	void reduceMemory(bool b) { blaster.reduceMemory(b); }
};

std::ostream &operator<<(std::ostream &os, const OptimalCrossingMinimizer::CrossingLocation& v);
std::ostream &operator<<(std::ostream &os, const OptimalCrossingMinimizer::CrossingVariable& v);
std::ostream &operator<<(std::ostream &os, /*const*/ OptimalCrossingMinimizer::SimplicityConstraint& k);
std::ostream &operator<<(std::ostream &os, /*const*/ OptimalCrossingMinimizer::KuratowskiConstraint& k);

std::ostream &operator<<(std::ostream &os, const OptimalCrossingMinimizer::SimpleSeparationParams& p);
std::ostream &operator<<(std::ostream &os, const OptimalCrossingMinimizer::BoyerMyrvoldSeparationParams& p);

#endif // USE_ABACUS

} // namespace

#endif // OGDF_ABACUS_OPTIMAL_CROSSING_MINIMIZER_SUPER_PRICE_H
