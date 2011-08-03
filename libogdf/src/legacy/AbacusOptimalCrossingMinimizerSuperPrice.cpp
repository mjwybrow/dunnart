/*
 * $Revision: 1.24 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2007-12-12 10:07:52 +0100 (Wed, 12 Dec 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Implements class AbacusOptimalCrossingMinimizerSuperPrice
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


#ifdef USE_ABACUS

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/List.h>
#include <ogdf/basic/Array.h>
#include <ogdf/basic/MinHeap.h>
#include <ogdf/legacy/AbacusOptimalCrossingMinimizerSuperPrice.h>
#include <ogdf/planarity/planarity/SubgraphPlanarizer.h>

using namespace ogdf;
namespace ogdf_legacy {
	
	const double AbacusOptimalCrossingMinimizerSuperPrice::SEG_EPS = 0.0001;
	
		template<class X> class Valued {
			double v;
			X x;
		public:
			Valued() {};  //be carefull, bro!
			Valued(double vt, X xt) : v(vt), x(xt) {}
			Valued(const Valued& V) : v(V.v), x(V.x) {}
			double value() const { return v; }
			X item() const { return x;}
			bool operator<(const Valued<X>& V) const { return v<V.v; }
			bool operator<=(const Valued<X>& V) const { return v<=V.v; }
			bool operator>(const Valued<X>& V) const { return v>V.v; }
			bool operator>=(const Valued<X>& V) const { return v>=V.v; }
			bool operator==(const Valued<X>& V) const { return v==V.v; }
			bool operator!=(const Valued<X>& V) const { return v!=V.v; }
		};
		
		template<class X> class KillingTop10Heap : public Top10Heap<Valued<X*> > {
		public:
			KillingTop10Heap(int size) : Top10Heap<Valued<X*> >(size) {}
			void pushAndKill(double val, X* x) {
				Valued<X*> vo;
				Valued<X*> nv(val, x);
				if(returnedSomething( push(nv, vo) ))
					delete vo.item();
			}
		};
		
		/*
		template<class X> class BufferList : protected List<Valued<X> > {
			int capacity;
		public:
			BufferList(int cap) : List<Valued<X> >(), capacity(cap) {};
			void add(double val, X x) {// deletes X of worst, tells it taken
				Valued<X> E(val, x);
				if(size() == 0) {
					pushFront(E);
				} else {
					if(val >= back().value()) { // put it last
						if(size() == capacity) delete x;
						else pushBack(E);
					} else {
						ListIterator<Valued<X> > it = rbegin();
						while(it.valid()) {
							if((*it).value() <= val) break;
							--it;
						}
						if(it.valid()) insert(E, it, after);
						else pushFront(E);
						if(size()>capacity) {
							delete back().item();
							popBack();
						}
					}
				}
			}
			bool full() { return size() >= capacity; }
			int size() { return List<Valued<X> >::size(); }
			ListConstIterator<Valued<X> > begin() { return List<Valued<X> >::begin(); } 
		};*/
	
	
void AbacusOptimalCrossingMinimizerSuperPrice::Master::setDefaultSettings() {
	m_numBoundHeuristics = 5;
	m_numAddVariables = 50;
	m_numAddSimilarVariables = 5;
	m_numTryVariables = 300;
	m_numHeuristics = 50;
	m_numStartKuratowskis = 100;
	m_numTryKuratowskis = 50;
	m_numCutKuratowskis = 20;
	m_maxMinutes = 0;
//	m_simpleDrawings = false;
	m_tryCrossingAdjacentVarsFirst = true;
	m_pricingInitialization = PI_EachPair;
	m_duplicateKuratowskis = true;
	m_roundUp = 0.7;
	m_writeResult = NULL;
	m_graphHint = GH_None;
	m_hintEffects = HE_KuratowskisMinusOne | HE_EdgeOrder;
}

void AbacusOptimalCrossingMinimizerSuperPrice::KuratowskiConstraint::addAccordingCrossing(
		const Subproblem* S, const PlanRep& I, edge e, int eid, List<CrossingInfo*>& L) {
	const List<edge>& le = I.chain(e);
	
	OGDF_ASSERT( le.size() == 2 );
	OGDF_ASSERT( le.front()->target() == le.back()->source() );
	
	node n = le.front()->target();
	edge c, te;
	forall_adj_edges(te, n) {
		c = I.original(te);
		if(c != e) break;
	}
	OGDF_ASSERT( c != e );
	
	const GraphCopy& exp = (const GraphCopy&) I.original();
	int cid = master()->activeVars->minIndex(exp.original(c));
	ListConstIterator<edge> lci = exp.chain(exp.original(c)).begin();
	for(; lci.valid(); lci++, cid++) {
		if(*lci == c) break;
	}
	OGDF_ASSERT( *lci == c );
	
//	if(!CrossingVariable::preferedOrder(e,c))
//		return 0;
		
	const List<CrossingInfo*>& all = *(S->currentRoundedCrossings);
		
	CrossingInfo ci(exp.original(e), eid, exp.original(c), cid);
	for(ListConstIterator<CrossingInfo*> it = all.begin(); it.valid(); it++) {
		if(master()->crossingInfoComparer.equal(&ci, *it)) {
			if(L.search(*it, master()->crossingInfoComparer)<0)
				L.pushBack(*it);
			return;
		}
	}
	
	lout(LL_ALARM) << "Should NEVER be here...\n";
	lout(LL_ALARM) << "Looking for: " << ci << " [c=" << exp.original(c)->index() << ", csz=" << exp.chain(exp.original(c)).size() << "]\n";
	lout(LL_ALARM) << "In: ";
	for(ListConstIterator<CrossingInfo*> it = all.begin(); it.valid(); it++) {
		lout(LL_ALARM) << " " << **it;
	}
	lout(LL_ALARM) << "\n";
	
	OGDF_ASSERT( false );
}

void AbacusOptimalCrossingMinimizerSuperPrice::KuratowskiConstraint::build(const Subproblem* S, const GraphReduction& R, const List<edge>& K) {
	
	const PlanRep& I = (const PlanRep&) R.original();

	List<CrossingInfo*> DD;

	ListConstIterator<edge> it = K.begin();
	for(; it.valid(); it++) {
		const List<edge>& il = R.original(*it); // il in I
		edge expfront = I.original(il.front()); // in expandedGraph
		edge expback = I.original(il.back()); // in expandedGraph
		
		const GraphCopy& exp = (const GraphCopy&) I.original();
		OGDF_ASSERT( exp.original(expfront) == exp.original(expback) );
		edge orig = exp.original(expfront);

		ListConstIterator<edge> lci = exp.chain(orig).begin();
		int startId = -master()->expansionFactor-1;
		int endId = +master()->expansionFactor+1;

		OGDF_ASSERT( il.front()->source() == R.original((*it)->source()) );
		OGDF_ASSERT( il.back()->target() == R.original((*it)->target()) );

		//calc start
		if(I.original(il.front()->source()) == 0) { // was crossing in I			
			startId = master()->activeVars->minIndex(orig); // really?
			for(; lci.valid(); lci++, startId++) {
				if(*lci == expfront) break;
			}
			OGDF_ASSERT( *lci == expfront );
			OGDF_ASSERT( I.chain(expfront).size() == 2 ); // this did crash...?! <<< CHECK THAT
			addAccordingCrossing(S, I, expfront, startId, DD);
		} else {
			OGDF_ASSERT( exp.original(I.original(il.front()->source())) );
		}
		//calc end
		if(I.original(il.back()->target()) == 0) { // was crossing in I			
			endId = std::max( startId, master()->activeVars->minIndex(orig) );
			for(; lci.valid(); lci++, endId++) {
				if(*lci == expback) break;
			}
			OGDF_ASSERT( *lci == expback );
			OGDF_ASSERT( I.chain(expback).size() == 2 );
			addAccordingCrossing(S, I, expback, endId, DD);
		} else {
			OGDF_ASSERT( exp.original(I.original(il.back()->target())) );
		}
		
		edges[orig] = new EdgeInfo(startId, endId);
	}

	rhs_ = 1 - DD.size();

	D.init(DD.size());
	int idx = 0;
	for(ListConstIterator<CrossingInfo*> dit = DD.begin(); dit.valid(); dit++, idx++) {
		D[idx] = *dit;
	}
	
	D.quicksort(master()->crossingInfoComparer);
}

bool AbacusOptimalCrossingMinimizerSuperPrice::Subproblem::feasible() {
	
	if(master()->numBoundHeuristics()) 
		return false; 	// if it's feasible, improve will recognize that...
		
	// the code below is only needed if no heuristics are normally run during the B&B (that might haoppen for Kn-proofing)
	if(!simplicitySatisfied)
		return false;	
	if(!integerFeasible())
		return false;
		
	PlanarModule pm;
	if(!pm.planarityTest(*currentIntegerSolution))
		return false;
	
	
	// this will force "improve" to check the solution
	master()->numBoundHeuristics(-1);
	
	return false;	
/*	PlanRep P((const Graph&)*currentIntegerSolution);
	P.initCC(0);

	master()->upperBoundSource = AbacusOptimalCrossingMinimizerSuperPrice::Master::SS_ILP;
	if(master()->bestSolution) delete master()->bestSolution;
	master()->bestSolution = new CrossingConfiguration(P, 
		currentIntegerSolution->numberOfNodes() - master()->expandedGraph->numberOfNodes(),
		false);
	return true;*/
}

int AbacusOptimalCrossingMinimizerSuperPrice::Subproblem::separate() {
	if(!simplicitySatisfied) return 0;	
	
	lout() << "Subproblem " << id() << " -> separate: ";

	int ret = constraintPoolSeparation(0, master()->hintedPoolPostPrice);
	if(ret)
		lout() << "[hinted pool separation (post-price)] ";
	else
		ret = constraintPoolSeparation(0, master()->kuratowskiPool);
	if(!ret) {
		lout() << "new ";
		int numtries = master()->m_numTryKuratowskis;
		int numcuts = master()->m_numCutKuratowskis;
		if(!father() && nIter_==1) {
			lout() << "[Start] ";
			numcuts = master()->m_numStartKuratowskis;
			numtries = numcuts * master()->m_numTryKuratowskis / master()->m_numCutKuratowskis;
		}
		ABA_BUFFER<ABA_CONSTRAINT*> cuts(master(),numcuts);
		KillingTop10Heap<KuratowskiConstraint> PL(numcuts);
		GraphReduction R(*currentRoundedSolution);
		for(int h = numtries; h-->0;) {
			List<edge> K;
			findKuratowski(R, K);
			KuratowskiConstraint* kc = new KuratowskiConstraint(master(), this, R, K, false); //dynamic!
			double slack; // might be interesting; the higher, the more violated...
			if(!kc->violated(actVar() , xVal_, &slack ))  {
				delete kc;
			} else {
				PL.pushAndKill( slack, kc );
			}
		}
		for(int pli = PL.size(); pli-->0;) {
			KuratowskiConstraint* kc = PL[pli].item();
			cuts.push(kc);
		}
		ret = addCons(cuts, master()->kuratowskiPool);
	} else
		lout() << "existing ";
	
	lout() << ret << "\n";
	return ret;
}

int AbacusOptimalCrossingMinimizerSuperPrice::Subproblem::makeFeasible() {
//	lout() << "Subproblem " << id() << " -> makeFeasible: -> calling pricing...\n";
//	return pricing();
	lout() << "Subproblem " << id() << " -> makeFeasible: -> no chance!\n";
	return 1;
}

void AbacusOptimalCrossingMinimizerSuperPrice::Subproblem::duplicateKuratowskis( 
		AbacusOptimalCrossingMinimizerSuperPrice::CrossingVariable* cvar, 
		List<AbacusOptimalCrossingMinimizerSuperPrice::KuratowskiConstraint*>& L) {
	ABA_NONDUPLPOOL<ABA_CONSTRAINT, ABA_VARIABLE>* pool = master()->kuratowskiPool;
	ABA_BUFFER<ABA_CONSTRAINT*> dups(master(), pool->size());
	
	for(int i = pool->size(); i-->0;) {
		ABA_CONSTRAINT* c = pool->slot(i)->conVar();
		if(c) {
			KuratowskiConstraint* kc = (KuratowskiConstraint*) c;\
			bool cnt = 0;
			for(int i = kc->D.size(); i-->0 && cnt<2;) { // max 2  might be adjacent
				const CrossingInfo* ci = kc->D[i];
				if(cvar->e1 == ci->e1 && cvar->e2 == ci->e2) {
					if(cvar->e1no == ci->e1no && abs(cvar->e2no - ci->e2no) <= 1) { // 0 won't happen
						cnt++;
						KuratowskiConstraint* nkc = new KuratowskiConstraint(*kc);
						nkc->adapt(i, cvar, cvar->e2, ci->e2no, cvar->e2no);
						L.pushBack(nkc);
					}
					if(cvar->e2no == ci->e2no && abs(cvar->e1no - ci->e1no) <= 1) { // 0 won't happen
						cnt++;
						KuratowskiConstraint* nkc = new KuratowskiConstraint(*kc);
						nkc->adapt(i, cvar, cvar->e1, ci->e1no, cvar->e1no);
						L.pushBack(nkc);
					}
				}
			}
		}		
	}
}

double AbacusOptimalCrossingMinimizerSuperPrice::Subproblem::generateVariable(edge e1, int e1id, edge e2, int e2id, AbacusOptimalCrossingMinimizerSuperPrice::CrossingVariable** var) {

	*var = new CrossingVariable(master(), e1, e1id, e2, e2id);
	double rc = (*var)->redCost(actCon(), yVal_);
	if(rc >= 0 ) { // var DOES price out correctly
		delete *var;
		*var = 0;
	}
	return rc;
}

int AbacusOptimalCrossingMinimizerSuperPrice::Subproblem::pricing() {	
	// *I* price at solveLp...
	return 0;
}

void AbacusOptimalCrossingMinimizerSuperPrice::Subproblem::findKuratowski(Graph& R, List<edge>& K) {
	PlanarModule pm;
	if(pm.planarityTest((const Graph&)R))
		return;
		
//	lout() << "\nReduced Graph:\n";
//	edge ee;
//	forall_edges(ee, R) {
//		lout() << "\t" << ee->index() << ": " << ee->source() << "->" << ee->target() << "\n";
//	}
	
	List<edge> es;
	R.allEdges(es);
	es.permute(); // randomize order
	
//	lout() << "Search Kura:\n";
	for(ListIterator<edge> it = es.begin(); it.valid(); it++) { // each edge once
		edge e = *it;
//		lout() << "\t" << e->index();
		R.hideEdge(e);
		if(pm.planarityTest(R)) {
			R.restoreEdge(e);
			K.pushBack( e );
//			lout() << " KURA";
		}
//		lout() << "\n";
	}
	R.restoreAllEdges();
	
	// below is not neccessary (i.e. sorting is not necc.)
//	K.quicksort();
//	lout() << "Kuratowski:";
//	for(ListConstIterator<edge> it = K.begin(); it.valid(); it++) {
//		lout() << " " << (*it)->index();
//	}
//	lout() << "\n";
}

CrossingConfiguration* AbacusOptimalCrossingMinimizerSuperPrice::Subproblem::callBoundHeuristic(bool integer) {
	int bh = master()->numBoundHeuristics();
	if(bh < 0) {
		bh *= -1;
		master()->numBoundHeuristics(0);
	}
	if(bh == 0) return 0;
	
	const PlanRep& PR = *(integer ? currentIntegerSolution : currentRoundedSolution);
	
	EdgeArray<int> helpcost(PR);
	EdgeArray<bool> helpforbid(PR);
	edge e, oe;
	forall_edges(e, PR) {
		oe = master()->expandedGraph->original(PR.original(e));
		helpcost[e] = master()->cost[oe];
		helpforbid[e] = master()->forbid[oe];
	}
	
	PlanRep HPR((const Graph&)PR);
	SubgraphPlanarizer sp;
	sp.permutations(bh);
	int ignore;
	sp.call(HPR, 0, ignore, &helpcost, &helpforbid, 0);
	int newObj; // i'll have to calculate that manually...
	if(!master()->useCost())
		newObj = HPR.numberOfNodes() - master()->expandedGraph->numberOfNodes();
	else {
		newObj = 0;
		node n;
		forall_nodes(n, HPR) {
			if(HPR.original(n) == NULL || PR.original(HPR.original(n)) == NULL) { // dummy found -> calc cost
				newObj += (int) master()->getCost( // integer is enough. no epsilonify here...
					master()->expandedGraph->original(PR.original(HPR.original(n->firstAdj()->theEdge()))),
					master()->expandedGraph->original(PR.original(HPR.original(n->lastAdj()->theEdge()))));
			}
		}
	}
	if(master()->betterPrimal(newObj)) {
		return new CrossingConfiguration(HPR, newObj, false);
	}
	return 0;
}

int AbacusOptimalCrossingMinimizerSuperPrice::Subproblem::improve(double &primalValue) {
	int success = 0;
	
	if(!simplicitySatisfied) return success;
	
	lout() << "Subproblem " << id() << " -> improve:";

	CrossingConfiguration* heuristicSolution;
	if(!father() && nIter_==1) {
		lout() << "[Init: ";
		heuristicSolution = master()->bestSolution;
		master()->updateBestSolution(heuristicSolution);
		primalValue = heuristicSolution->getCrossingNo();
		master()->primalBound(primalValue);
		lout() << primalValue << "]";
	} else {
		bool trys = false;
		if(!master()->equalCrossingLists(currentIntegerCrossings, true)) {
			trys = true;
			lout() << " integer=";
			if(heuristicSolution = callBoundHeuristic(true)) {
				master()->updateBestSolution(heuristicSolution);
				primalValue = heuristicSolution->getCrossingNo();
				master()->primalBound(primalValue);
				lout() << "YES[" << primalValue << "]";
				success = 1;
			} else 
				lout() << "no";
		}		
			
		if(currentIntegerCrossings->size() != currentRoundedCrossings->size() &&
				!master()->equalCrossingLists(currentRoundedCrossings, false)) {
			trys = true;
			lout() << " rounded=";
			if(heuristicSolution = callBoundHeuristic(false)) {
				master()->updateBestSolution(heuristicSolution);
				primalValue = heuristicSolution->getCrossingNo();
				master()->primalBound(primalValue);
				lout() << "YES[" << primalValue << "]";
				success = 1;
			} else 
				lout() << "no";
		}

		if(!trys)
			lout() << "not neccessary";
		else {
			delete master()->lastIntegerCrossings;
			master()->lastIntegerCrossings = currentIntegerCrossings;
			delete master()->lastRoundedCrossings;
			master()->lastRoundedCrossings = currentRoundedCrossings;
			storedCurrentCrossings = true;
		}
	}
	
	lout() << "\n";
	return success;
}

void AbacusOptimalCrossingMinimizerSuperPrice::Subproblem::realizeVariable(CrossingVariable* cvar, double val) {
	if(val > master()->roundUp() ) {
		lout() << "\trealizeVariable: " << (*cvar) << " with " << val << "\n";
		const List<edge>& c1 = master()->expandedGraph->chain(cvar->e1);
		const List<edge>& c2 = master()->expandedGraph->chain(cvar->e2);
	
		OGDF_ASSERT( cvar->e1no >= master()->activeVars->minIndex(cvar->e1) );
		OGDF_ASSERT( cvar->e1no <= master()->activeVars->maxIndex(cvar->e1) );
		OGDF_ASSERT( cvar->e2no >= master()->activeVars->minIndex(cvar->e2) );
		OGDF_ASSERT( cvar->e2no <= master()->activeVars->maxIndex(cvar->e2) );
		
		edge f1 = *(c1.get(cvar->e1no - master()->activeVars->minIndex(cvar->e1)));
		edge f2 = *(c2.get(cvar->e2no - master()->activeVars->minIndex(cvar->e2)));

		edge r1 = currentRoundedSolution->chain(f1).front();
		edge r2 = currentRoundedSolution->chain(f2).front();	
		currentRoundedSolution->insertCrossing(r1, r2, true);
		currentRoundedCrossings->pushBack(cvar);
		if(val > 1 - Master::EPS) {
			edge i1 = currentIntegerSolution->chain(f1).front();
			edge i2 = currentIntegerSolution->chain(f2).front();
			currentIntegerSolution->insertCrossing(i1, i2, true);
			currentIntegerCrossings->pushBack(cvar);
		}
	}
}

bool AbacusOptimalCrossingMinimizerSuperPrice::Subproblem::checkSimplicitiesAndPrice() {
	// guaranteed: all existing simplicity constraints are guaranteed...
	lout() << "Simplicity check: ";
	
	EdgeArray<double> sum(*master()->minimizedGraph, 0);
	EdgeArray<List<Segment> > lst(*master()->minimizedGraph);
	
	for(int i = 0; i < nVar(); i++) {
		if(CrossingVariable* cvar = (CrossingVariable*)variable(i)) {
			if(xVal(i) > Master::EPS) { // there is something
				double v = master()->getCost(cvar->e1, cvar->e2) * xVal(i);
				if(cvar->e1no == 0) {
					sum[cvar->e1] += v;
					lst[cvar->e1].pushBack(Segment(cvar->e2, cvar->e2no));
				}
				if(cvar->e2no == 0) {
					sum[cvar->e2] += v;
					lst[cvar->e2].pushBack(Segment(cvar->e1, cvar->e1no));
				}
			}
		}
	}
	
	edge e;

	int maxNewVars = 0;
	forall_edges(e, *master()->minimizedGraph) {
		if(sum[e] > 1) { // crap... too many...
			forall_listiterators(Segment, it, lst[e]) {				
				maxNewVars += (*it).segId+1; // VERY crude estimation -- this can be greatly improved!
			}
			maxNewVars++;
			//maxNewVars += lst[e].size();
		}
	}

	List<KuratowskiConstraint*> dkl;
	ABA_BUFFER<ABA_VARIABLE*> vars(master(), maxNewVars);
	ABA_BUFFER<ABA_CONSTRAINT*> simpl(master(), maxNewVars);
	
	forall_edges(e, *master()->minimizedGraph) {
		if(sum[e] > 1) { // crap... too many...
			// find crossing segments with shortest extension yet.
			int sh = master()->expansionFactor*2+5;
			List<Segment> shortest;
			forall_listiterators(Segment, i, lst[e]) {
				int mI = master()->activeVars->maxIndex((*i).e, (*i).segId, e);
				if(mI < sh) {
					sh = mI;
					shortest.clear();
				}
				if(mI == sh) {
					shortest.pushBack(*i);
				}				
			}
			
			// if extension not possible:
			//      -> add simplicity-constraint for main? 
			//            -> unneccessary, since there is no minimal solution wich needs >1 there...
			// hence: -> do nothing
			OGDF_ASSERT( sh+1 <= master()->expandedGraph->chain(e).size() );
			OGDF_ASSERT( sh <= master()->activeVars->maxIndex(e) );
			
			if(sh+1 == master()->expandedGraph->chain(e).size()) {
				lout() << "[simplicity even on index 0 of edge "<< e->index() <<"] ";
				simpl.push(new SimplicityConstraint(master(), e, 0));
				continue;
			} else {
				// check whether i need a new simplicity-constraint
				if(sh == master()->activeVars->maxIndex(e)) {
					SimplicityConstraint* sc = new SimplicityConstraint(master(), e, sh+1);
//					lout() << "\tSC: " << *sc << "\n";
					simpl.push(sc);
				}
				
				// all shortest exts get extended by one. 
				forall_listiterators(Segment, ii, shortest) {
					// ensure convexity
///					int sid = master()->activeVars->maxIndex(e, sh+1, (*ii).e) + 1;
					for(int sid = master()->activeVars->maxIndex(e, sh+1, (*ii).e) + 1; sid <= (*ii).segId; sid++) {
						CrossingVariable* cvar = new CrossingVariable(master(), e, sh+1, (*ii).e, /*(*ii).segId*/ sid);
						
						if(master()->activeVars->maxIndex((*ii).e) < sid) {
							SimplicityConstraint* sc = new SimplicityConstraint(master(), (*ii).e, sid);
							lout() << "\tSC (secondary): " << *sc << "\n";
							simpl.push(sc);
						}
						
						if(master()->duplicateKuratowskis())
							duplicateKuratowskis(cvar, dkl);
						master()->activeVars->registerVariable(cvar);
//						lout() << "\tV: " << *cvar << "\n";
						vars.push(cvar);	
					}
				}
			}
		}
	}
	
	int sim = 0;
	if(simpl.number())
		sim = addCons(simpl, master()->simplicityPool);

	int kur = 0;
	if(dkl.size()) {
		ABA_BUFFER<ABA_CONSTRAINT*> dklb(master(),dkl.size());
		forall_listiterators(KuratowskiConstraint*, i, dkl) {
//			lout() << "\tKC: " << **i << "\n";
			dklb.push(*i);
		}
		kur = addCons(dklb, master()->kuratowskiPool);
	}
	
	int ret = 0;
	if(vars.number())
		ret = addVars(vars);
	
	if(ret)
		lout() << "failed -> +Vars=" << ret << "; +SC=" << sim << "; +KC=" << kur << "\n";
	else
		lout() << "passed\n";
	
	return !ret;
}

int AbacusOptimalCrossingMinimizerSuperPrice::Subproblem::solveLp() {
	
	if(master()->totalTime()->exceeds(master()->maxCpuTime())) { // get me outa here!
		simplicitySatisfied = false;
		return 0;
	}
	
	lout() << "Subproblem " << id() << " -> solve (" << ++nIter_ << ") -> \n";
	
	int error = ABA_SUB::solveLp();
	if(error) {
		lout() << "Error happend in internal solving module (" << error << ")\n";
		return error;
	}
	
	if(constraintPoolSeparation(0, master()->hintedPoolPrePrice)) {
		lout() << "-> Hinted Pool (Pre-Price) Separation\n";
		simplicitySatisfied = false; // i dont know yet
		return 0; // resolve
	}

	if(constraintPoolSeparation(0, master()->simplicityPool)) {
		lout() << "-> Simplicity Pool Separation\n";
		simplicitySatisfied = false;
		return 0;
	}
	
	if(!checkSimplicitiesAndPrice()) {
		simplicitySatisfied = false;
		lout() << "-> resolve (pricing)\n";
	} else {
		simplicitySatisfied = true;		
		lout() << "simplicity satisfied -> solve completed:\n"; // simplicity is now guaranteed...
		clearCurrents();
		initCurrents();
		double sum = 0;
		for(int i = 0; i < nVar(); i++) {
			if(CrossingVariable* cvar = (CrossingVariable*)variable(i)) {
				realizeVariable(cvar, xVal(i));
				sum += cvar->obj() * xVal(i);
			}
		}
		lout() << "\t=> ObjValue: " << sum << "\n";
	}
	return 0;
}


Module::ReturnType AbacusOptimalCrossingMinimizerSuperPrice::Master::doCall(PlanRep &_PG,
		int cc,
		const EdgeArray<int>  &_cost,
		const EdgeArray<bool> &_forbid, 
		const EdgeArray<unsigned int>  &subgraphs,
		int& crossingNumber) {
	
	if(effectiveLogLevel() >= LL_ALARM || effectiveStatisticMode())
		outLevel(AbacusOptimalCrossingMinimizerSuperPrice::Master::Silent);
	else
		outLevel(AbacusOptimalCrossingMinimizerSuperPrice::Master::Full);
		
	lout() << "---------------------------------------------"
	       << "\nnumHeuristics = " << m_numHeuristics
	       << "\nnumBoundHeuristics = " << m_numBoundHeuristics
	       << "\nnumStartKuratowskis = " << m_numStartKuratowskis
	       << "\nnumTryKuratowskis = " << m_numTryKuratowskis
	       << "\nnumCutKuratowskis = " << m_numCutKuratowskis
	       << "\nnumAddVariables = " << m_numAddVariables
	       << "\nnumAddSimilarVariables = " << m_numAddSimilarVariables
	       << "\nnumTryVariables = " << m_numTryVariables
	       << "\nmaxMinutes = " << m_maxMinutes
	       << "\nroundUp = " << m_roundUp
	       << "\ntryCrossingAdjacentVarsFirst = " << m_tryCrossingAdjacentVarsFirst
	       << "\npricingInitialization = " << ( ( m_pricingInitialization == PI_BestHeuristic ) ? "BestHeuristic" :
	                                          ( ( m_pricingInitialization == PI_EachPair ) ? "EachPair" : "None" ) )
	       << "\nduplicateKuratowskis = " << m_duplicateKuratowskis	
	       << "\ngraphHint = " << ( ( m_graphHint == GH_Complete ) ? "Complete Graph" :
	                               ( ( m_graphHint == GH_CompleteBipartite ) ? "Complete Bipartite Graph" : "None" ) )                                          
	       << "\nhintEffects = {" << (( m_hintEffects & HE_KuratowskisMinusOne ) ? "KuratowskisMinusOne " : "")
	                              << (( m_hintEffects & HE_KuratowskisMinusTwo ) ? "KuratowskisMinusTwo " : "")
	                              << (( m_hintEffects & HE_EdgeOrder ) ? "EdgeOrder " : "")
	                              << (( m_hintEffects & HE_NodeOrder ) ? "NodeOrder " : "")
	                              << "}\n"
	       << "\n---------------------------------------------\n";

	bestSolution = helpCall(_PG, cc, _cost, _forbid, crossingNumber);
	
	minimizedGraph->allEdges(helperEdgelist1);
	minimizedGraph->allEdges(helperEdgelist2);
	m_minVariables = 0;
	m_maxVariables = 0;
	m_usedVariables = 0;
	

	if(upperbound+1 > lowerbound) { // otherwise cInfo is already optimal...
		activeVars = new ActiveVariables(this);
	
		STATUS s;
		try {
			lout() << "Fasten your seatbelt: Starting to solve...\n";
			s = optimize(); // weave your magic...
		} catch(int ex) { //brrr
			if(ex == -666)
				s = Error;
			else
				s = OutOfMemory;
		} catch(...) {
				s = OutOfMemory;
		}
		
		lout() << "Start #Variables: " << m_minVariables << "; Max #Variables: " << m_maxVariables << "; Generated Variables: " << m_usedVariables << "(=" << (100*m_usedVariables/(double)m_maxVariables) << "%)\n";
		
		lout(LL_FORCE) << "Abacus ended with: ";
		switch(s) {
			case Optimal: lout(LL_FORCE) << "Optimal\n"; break;
			case Error: lout(LL_FORCE) << "Error\n"; break;
			case OutOfMemory: lout(LL_FORCE) << "OutOfMemory\n"; break;
			case Unprocessed: lout(LL_FORCE) << "Unprocessed\n"; break;
			case Processing: lout(LL_FORCE) << "Processing\n"; break;
			case Guaranteed: lout(LL_FORCE) << "Guaranteed\n"; break;
			case MaxLevel: lout(LL_FORCE) << "MaxLevel\n"; break;
			case MaxCpuTime: lout(LL_FORCE) << "MaxCpuTime\n"; break;
			case MaxCowTime: lout(LL_FORCE) << "MaxCowTime\n"; break;
			case ExceptionFathom: lout(LL_FORCE) << "ExceptionFathom\n"; break;
		}
		lout() << "Abacus Primal Bound: " << primalBound() << "\n";
		lout() << "Abacus Dual Bound: " << dualBound() << "\n";
	} else {
		m_isTrivial = true;
	}
	
	lout() << "best solution obj value: " << bestSolution->getCrossingNo() << "\n";
	
	switch(upperBoundSource) {
		case SS_ILP:
			lout() << "The ILP found a solution better than the best start heuristic, based on constraints.\n";
		break;
		case SS_ILP_Heuristic:
			lout() << "The ILP found a solution better than the best start heuristic, based on bounding heuristics.\n";
		break;
		case SS_Heuristic:
			lout() << "The ILP found no solution better then the best start heuristic.\n";
		break;
		default:
			lout() << "Unknown upper bound source.\n";
	}
	
	crossingNumber = bestSolution->getCrossingNo();
	//TODO backtransformation from bestSolution.

	if(!m_isTrivial) {
		lout() << "Time req'd: " << totalTime()->seconds() << "sec = " 
		       << totalTime()->minutes() << "min " << (totalTime()->seconds()%60) << "sec\n";
		lout() << "# Variables: " << m_usedVariables << "/" << m_maxVariables 
		       << " = " << (100*m_usedVariables/(double)m_maxVariables) << "%\n";
		lout() << "# Simplicity-Constraints: " << simplicityPool->number() << "\n";
		lout() << "# Kuratowski-Constraints: " << kuratowskiPool->number() << "\n";
	}
	
	if(m_isTrivial)
		sout() << "\t" << crossingNumber << "\t" << crossingNumber << "\t0\t0\t0"; // 0 time, 0 subs selected, 0 subs generated
	else {
		if(nSub() <= 1)
			sout() << "\t" << crossingNumber << "\t" << crossingNumber << "\t" << totalTime()->seconds(); // done within first sub
		sout() << "\t" << nSubSelected() << "\t" << nSub();
	}
	
	sout() << "\t" << isOptimal()
	       << "\t" << (status()==OutOfMemory)
	       << "\t" << (status()==Error)
	       << "\t" << m_isTrivial
	       << "\t" << (m_isTrivial ? crossingNumber : (int)ceil(dualBound()))
	       << "\t" << crossingNumber
	       << "\t" << (m_isTrivial ? 0 : totalTime()->seconds())
	       << "\t" << (m_isTrivial ? 0 : m_minVariables)
	       << "\t" << (m_isTrivial ? 0 : m_maxVariables)
	       << "\t" << (m_isTrivial ? 0 : m_usedVariables)
	       << "\t" << ( (m_isTrivial||(m_maxVariables==0)) ? 0 : (100*m_usedVariables/(double)m_maxVariables))
	       << "\t" << (m_isTrivial ? 0 : simplicityPool->number())
	       << "\t" << (m_isTrivial ? 0 : kuratowskiPool->number());	
	       
	if(isOptimal() && (effectiveLogLevel()<=LL_DEFAULT || writeResult()) ) {
		lout() << "=================================\n"
			<< "Resulting Crossing Configuration:\n"
			<< "---------------------------------\n";
		PlanRep RRR(*expandedGraph);
		RRR.initCC(0);
		edge e;
		forall_edges(e, *minimizedGraph) {
			const List<edge>& L = bestSolution->getCrossingEdges(e);
			if(!L.empty()) {
				lout() << e << ": " << L << "\n";
			
				ListConstIterator<edge> eei = expandedGraph->chain(e).begin();
				forall_listiterators(edge, it, L) {
					edge o = *it;
					if(e->index() < o->index()) {
						int idx = bestSolution->getCrossingEdges(o).search(e);
						edge ee = *eei;
						edge oo = *expandedGraph->chain(o).get(idx);
						edge eee = RRR.chain(ee).front();
						edge ooo = RRR.chain(oo).front();
						RRR.insertCrossing(eee, ooo, true);
					}
					++eei;
				}			
			}		
		}
		GraphReduction RR(RRR);
		GraphAttributes GAR(RR, GraphAttributes::nodeGraphics | GraphAttributes::nodeColor);
		node n;
		forall_nodes(n, RR) {
			if(!RRR.original(RR.original(n))) {
				GAR.colorNode(n) = "red";
			}
		}
		if(writeResult())
			GAR.writeGML(writeResult());
		lout() << "=================================\n";
	}       
	return isOptimal() ? retOptimal : (status()==OutOfMemory || status()==Error) ? retError : retFeasible;
}

bool AbacusOptimalCrossingMinimizerSuperPrice::Master::variableAllowed(edge e1, edge e2) {
	// thou shalt not cross
	if( forbid[e1] || forbid[e2] )
		return false;
		
	// adjacent edges do not cross
	node n;	
	edge ee;
	{ forall_adj_edges(ee, e1->source()) if(ee == e2) return false;	}
	{ forall_adj_edges(ee, e1->target()) if(ee == e2) return false; }
	
	// oh well, go ahead and pair...
	return true;
}

void AbacusOptimalCrossingMinimizerSuperPrice::Master::hintsKnKuratowskiMinusOne(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist) {
	node n;
	edge e;
	
	int ns = minimizedGraph->numberOfNodes() - 1;
	int cr = (ns/2) * ((ns-1)/2) * ((ns-2)/2) * ((ns-3)/2) / 4;
	NodeArray<KuratowskiConstraint*> N(*minimizedGraph, 0);
				
	forall_nodes(n, *minimizedGraph) {
		KuratowskiConstraint* k = new KuratowskiConstraint(this, cr, false); // non-dynamic
		forall_edges(e, *minimizedGraph) {
			if(e->source() != n && e->target() != n) {
				k->addEdge(e);
			}
		}
		postlist.pushBack(N[n]);
	}			
	lout() << "Complete-Graph-hint, KuratowskiMinusOne-effect:\n"
		<< "\tadding " << ns+1 << " K_" << ns << " (postprice) constraints, requiring " << cr << " crossings each.\n";
}

void AbacusOptimalCrossingMinimizerSuperPrice::Master::hintsKnKuratowskiMinusTwo(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist) {
	node n,nn;
	edge e;
	
	int oldsize = postlist.size();
	
	int ns = minimizedGraph->numberOfNodes() - 2;
	int cr = (ns/2) * ((ns-1)/2) * ((ns-2)/2) * ((ns-3)/2) / 4;
				
	forall_nodes(n, *minimizedGraph) {
		forall_nodes(nn, *minimizedGraph) {
			if(nn->index() > n->index()) {
				KuratowskiConstraint* k = new KuratowskiConstraint(this, cr, false); // non-dynamic				
				forall_edges(e, *minimizedGraph) {
					if(e->source() != n && e->target() != n && e->source() != nn && e->target() != nn)
						k->addEdge(e);
				}
				postlist.pushBack(k);
			}
		}
	}			
	lout() << "Complete-Graph-hint, KuratowskiMinusTwo-effect:\n"
		<< "\tadding " << ns+1 << " K_" << ns << " (postprice) constraints, requiring " << cr << " crossings each.\n";
}

void AbacusOptimalCrossingMinimizerSuperPrice::Master::hintsKnNodeOrder(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist) {
	node n;

	node lastNode = 0;
	forall_nodes(n, *minimizedGraph) {
		if(lastNode)
			prelist.pushBack(new NodeOrderConstraint(this, lastNode, n));
		lastNode = n;
	}
	lout() << "Complete-Graph-hint, NodeOrder-effect:\n\tadding " << minimizedGraph->numberOfNodes()-1 << " NodeOrder (preprice)constraints.\n";
}

void AbacusOptimalCrossingMinimizerSuperPrice::Master::hintsKnEdgeOrder(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist) {
	edge e;

	node baseNode = minimizedGraph->firstNode();
	edge lastEdge = 0;
	forall_adj_edges(e, baseNode) {
		if(lastEdge)
			prelist.pushBack(new EdgeOrderConstraint(this, lastEdge, e));
		lastEdge = e;
	}
	lout() << "Complete-Graph-hint, EdgeOrder-effect:\n\tadding " << minimizedGraph->numberOfNodes()-2 << " EdgeOrder (preprice)constraints.\n";
}

void AbacusOptimalCrossingMinimizerSuperPrice::Master::hintsKnmKuratowskiMinusOne(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist) {
	node n;
	edge e;

	int ns1 = minimizedGraph->firstNode()->degree();
	int ns2 = minimizedGraph->firstNode()->firstAdj()->twinNode()->degree();
	int cr1 = (ns2/2) * ((ns2-1)/2) * ((ns1-1)/2) * ((ns1-2)/2); //nds1 reduced
	int cr2 = (ns1/2) * ((ns1-1)/2) * ((ns2-1)/2) * ((ns2-2)/2); //nds2 reduced

	forall_nodes(n, *minimizedGraph) {
		KuratowskiConstraint* k = new KuratowskiConstraint(this, n->degree()==ns1 ? cr1 : cr2, false); // non-dynamic
		forall_edges(e, *minimizedGraph) {
			if(e->source() != n && e->target() != n) {
				k->addEdge(e);
			}
		}				
		postlist.pushBack(k);
	}			
	lout() << "Complete-Bipartite-Graph-hint, KuratowskiMinusOne-effect: "
	    << "\tadding " << ns1 << " K_{" << ns1-1 << "," << ns2 << "} (postprice)constraints requiring " << cr1 << " crossings each.\n";
	lout() << "\tadding " << ns2 << " K_{" << ns1 << "," << ns2-1 << "} (postprice)constraints requiring " << cr2 << " crossings each.\n";
}

void AbacusOptimalCrossingMinimizerSuperPrice::Master::hintsKnmKuratowskiMinusTwo(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist) {
	lout(LL_FORCE) << "WARNING: KnmKuratowskiMinusTwo not implemented! -> no such hints generated!!!";
	OGDF_ASSERT(false);
}

void AbacusOptimalCrossingMinimizerSuperPrice::Master::hintsKnmNodeOrder(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist) {
	node n;
	edge e;

	node lastNode[2] = { 0, 0};
	NodeArray<int> NA(*minimizedGraph, 0);
	forall_adj_edges(e, minimizedGraph->firstNode()) {
		NA[e->opposite(n)] = 1;
	}
	forall_nodes(n, *minimizedGraph) {
		if(lastNode[NA[n]])
			prelist.pushBack(new NodeOrderConstraint(this, lastNode[NA[n]], n));
		lastNode[NA[n]] = n;							
	}
	lout() << "Complete-Bipartite-Graph-hint, NodeOrder-effect:\n\tadding " << minimizedGraph->numberOfNodes()-2 << " NodeOrder (preprice)constraints.\n";
}

void AbacusOptimalCrossingMinimizerSuperPrice::Master::hintsKnmEdgeOrder(List<ABA_CONSTRAINT*>& prelist, List<ABA_CONSTRAINT*>& postlist) {
	node n;
	edge e;

	node baseNode = minimizedGraph->firstNode();				
	edge lastEdge = 0;
	edge baseEdge;
	{ forall_adj_edges(e, baseNode) {
		if(lastEdge != 0)
			prelist.pushBack(new EdgeOrderConstraint(this, lastEdge, e));
		else
			baseEdge = e;
		lastEdge = e;
	}}
	baseNode = baseEdge->opposite(baseNode);
	lastEdge = baseEdge;
	{ forall_adj_edges(e, baseNode) {
		if(baseEdge != e)
			prelist.pushBack(new EdgeOrderConstraint(this, lastEdge, e));
		lastEdge = e;
	}}
	lout() << "Complete-Bipartite-Graph-hint, EdgeOrder-effect:\n\tadding " << minimizedGraph->numberOfNodes()-2 << " EdgeOrder (preprice)constraints.\n";
}

void AbacusOptimalCrossingMinimizerSuperPrice::Master::initializeOptimization() {
	
	lout() << "Initializing Optimization...\n";

	// .abacus file ain't a good idea...
	// Abacus settings BEGIN
//	enumerationStrategy(BestFirst); // *should be supported... hm...
	branchingStrategy(CloseHalfExpensive);
	nBranchingVariableCandidates(1);
//	guarantee(0.0); // doesn't exist??
	maxLevel(999999);
	objInteger(true);
	tailOffNLp(0);
	tailOffPercent(0.0001);
	delayedBranching(0);
	minDormantRounds(1);
	pricingFreq(0);
	skipFactor(1);
	skippingMode(SkipByNode);
	fixSetByRedCost(true); // << warning? -> secure alternative: false
	maxConAdd(1000); // + (was: 100)
	maxConBuffered(1000); // + (was: 100)
	maxVarAdd(1000); // + (was: 500)
	maxVarBuffered(1000); // + (was: 500)
	maxIterations(-1);
	eliminateFixedSet(false);
	newRootReOptimize(false); // could try that for performance! // TODO: try!
	conElimMode(Basic); // << warning? -> secure alternative: NoConElim
	conElimEps(0.001);
	conElimAge(1);
	varElimMode(ReducedCost); // << warning? -> secure alternative: NoVarElim
	varElimEps(0.001);
	varElimAge(1);	
	// Abacus settings END
		
	List<ABA_CONSTRAINT*> hintedConstraintsPrePrice;
	List<ABA_CONSTRAINT*> hintedConstraintsPostPrice;
	
	ABA_BUFFER<ABA_VARIABLE*> variables(this, numMinMaxCrossingPairs);
	
	// add one pair each...
	edge e1, e2;
	forall_edges(e1,*minimizedGraph)  {
		int se1 = expandedGraph->chain(e1).size();
		forall_edges(e2, *minimizedGraph) {
			if( CrossingInfo::preferedOrder(e1,e2) && variableAllowed(e1,e2) ) {
				CrossingVariable* var = new CrossingVariable(this, e1, 0, e2, 0);
				activeVars->registerVariable(var);
				variables.push(var);
				m_minVariables++;
				m_maxVariables += se1*expandedGraph->chain(e2).size();
			}
		}
	}
	
	if(graphHint() != GH_None) {
		if((hintEffects() & HE_NodeOrder) && (hintEffects() & HE_EdgeOrder)) {
			lout(LL_FORCE) << "Conflicting Hint Effects Selected! NodeOrder- and EdgeOrder-Constraints are not compatible!"
				<< "\nDeactivating NodeOrder...\n";
			hintEffects(hintEffects() ^ HE_NodeOrder);
		}
		
		switch(graphHint()) {
			case GH_Complete: {
				if(hintEffects() & HE_KuratowskisMinusOne)
					hintsKnKuratowskiMinusOne(hintedConstraintsPrePrice, hintedConstraintsPostPrice);				
				if(hintEffects() & HE_KuratowskisMinusTwo)
					hintsKnKuratowskiMinusTwo(hintedConstraintsPrePrice, hintedConstraintsPostPrice);				
				if(hintEffects() & HE_EdgeOrder)
					hintsKnEdgeOrder(hintedConstraintsPrePrice, hintedConstraintsPostPrice);				
				if(hintEffects() & HE_NodeOrder)
					hintsKnNodeOrder(hintedConstraintsPrePrice, hintedConstraintsPostPrice);
			}
			break;
			case GH_CompleteBipartite: {
				if(hintEffects() & HE_KuratowskisMinusOne)
					hintsKnmKuratowskiMinusOne(hintedConstraintsPrePrice, hintedConstraintsPostPrice);				
				if(hintEffects() & HE_KuratowskisMinusTwo)
					hintsKnmKuratowskiMinusTwo(hintedConstraintsPrePrice, hintedConstraintsPostPrice);				
				if(hintEffects() & HE_EdgeOrder)
					hintsKnmEdgeOrder(hintedConstraintsPrePrice, hintedConstraintsPostPrice);				
				if(hintEffects() & HE_NodeOrder)
					hintsKnmNodeOrder(hintedConstraintsPrePrice, hintedConstraintsPostPrice);
			}
			break;
		}
	}

	primalBound(upperbound+1);
	dualBound(lowerbound);

	if(m_maxMinutes) {
		 // l?cherliches interface! pfui gack!
		ABA_STRING timeout(0,"00:00:00");
		int h1 = m_maxMinutes / 60;
		int h2 = h1 % 10;
		h1 /= 10;
		int m1 = m_maxMinutes % 60;
		int m2 = m1 % 10;
		m1 /= 10;
		timeout[0] = '0'+h1;
		timeout[1] = '0'+h2;
		timeout[3] = '0'+m1;
		timeout[4] = '0'+m2;
		lout() << "Timeout after: " << timeout << " (" << m_maxMinutes << ")\n";
		maxCpuTime(timeout);
	}
	
	lout() << "Using " << hintedConstraintsPrePrice.size() << " (preprice-) and " << 
		hintedConstraintsPostPrice.size() << " (postprice-) hinted constraints, " << 
		( ( graphHint() == GH_Complete ) ? "based on a Complete-Graph-hint.\n" :
		( ( graphHint() == GH_CompleteBipartite ) ? "based on a Complete-Bipartite-Graph-hint.\n" : 
		"since there was no hint.\n" ) );
	
	ABA_BUFFER<ABA_CONSTRAINT*> empty(this, 0);
	initializePools(empty, /*hintedConstraints,*/ variables,
		numExpMaxCrossingPairs, // worst case: all variables
		0, /*hintedConstraints.number(),*/
		false); // and it's not dynamic 

	simplicityPool = new ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, numExpEdges, false); // not dynamic
	hintedPoolPrePrice = new ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, hintedConstraintsPrePrice.size(), false); // not dynamic
	hintedPoolPostPrice = new ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, hintedConstraintsPostPrice.size(), false); // not dynamic
	forall_listiterators(ABA_CONSTRAINT*, it, hintedConstraintsPrePrice)
		hintedPoolPrePrice->insert(*it);
	forall_listiterators(ABA_CONSTRAINT*, it, hintedConstraintsPostPrice)
		hintedPoolPostPrice->insert(*it);
	kuratowskiPool = new ABA_NONDUPLPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, 1000, true); // dynamic
	
//	lout() << "done\n";
}

int AbacusOptimalCrossingMinimizerSuperPrice::Master::enumerationStrategy(ABA_SUB* s1, ABA_SUB* s2) {
	// TODO: select the one with more fixed "1s" first !
	return ABA_MASTER::enumerationStrategy(s1, s2);
}

std::ostream &operator<<(std::ostream &os, const AbacusOptimalCrossingMinimizerSuperPrice::CrossingInfo& v) {
	os << "(e1=" << v.e1->index() << "/" << v.e1no << ",e2=" << v.e2->index() << "/" << v.e2no <<")";
	return os;
}

std::ostream &operator<<(std::ostream &os, const AbacusOptimalCrossingMinimizerSuperPrice::CrossingVariable& v) {
	os << (const AbacusOptimalCrossingMinimizerSuperPrice::CrossingInfo&)v;
	return os;
}


std::ostream &operator<<(std::ostream &os, /*const*/ AbacusOptimalCrossingMinimizerSuperPrice::KuratowskiConstraint& k) {
	os << "(KC edges=[";
	edge e;
	forall_edges(e, *k.edges.graphOf()) {
		if(k.edges[e])
			os << e->index() << "(" << k.edges[e]->startId << "|" << k.edges[e]->endId << ")" << ",";
	}
	os << "], D=[";
	for(int i = k.D.size(); i-->0;) {
		os << *k.D[i];
	}
	os << "] RHS=" << k.rhs() << ")";
	//k.printCoeffs(os);
	return os;
}

std::ostream &operator<<(std::ostream &os, /*const*/ AbacusOptimalCrossingMinimizerSuperPrice::SimplicityConstraint& k) {
	os << "(SC edge=" << k.e->index() << " segId=" << k.segId << " RHS=" << k.rhs() << ")";
	//k.printCoeffs(os);
	return os;
}


} //namespace

#endif // USE_ABACUS
