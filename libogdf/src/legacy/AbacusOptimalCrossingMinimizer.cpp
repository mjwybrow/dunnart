/*
 * $Revision: 1.34 $
 * 
 * last checkin:
 *   $Author: chimani $ 
 *   $Date: 2008-02-20 20:33:07 +1100 (Wed, 20 Feb 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Implements class AbacusOptimalCrossingMinimizer
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
#include <ogdf/legacy/AbacusOptimalCrossingMinimizer.h>
#include <ogdf/planarity/planarity/SubgraphPlanarizer.h>

#define SEGMENT_EPSILON 0.001

using namespace ogdf;
namespace ogdf_legacy {
	
	
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
	
	
void AbacusOptimalCrossingMinimizer::Master::setDefaultSettings() {
	m_numBoundHeuristics = 5;
	m_numAddVariables = 50;
	m_numAddSimilarVariables = 5;
	m_numTryVariables = 300;
	m_numHeuristics = 50;
	m_numStartKuratowskis = 30;
	m_numTryKuratowskis = 50;
	m_numCutKuratowskis = 10;
	m_maxMinutes = 0;
//	m_simpleDrawings = false;
	m_epsilonifySegments = true;
	m_tryCrossingAdjacentVarsFirst = true;
	m_pricingInitialization = PI_BestHeuristic;
	m_duplicateKuratowskis = true;
	m_roundUp = 0.7;
}

void AbacusOptimalCrossingMinimizer::KuratowskiConstraint::addAccordingCrossing(
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

void AbacusOptimalCrossingMinimizer::KuratowskiConstraint::build(const Subproblem* S, const GraphReduction& R, const List<edge>& K) {
	
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
			OGDF_ASSERT( I.chain(expfront).size() == 2 );
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

bool AbacusOptimalCrossingMinimizer::Subproblem::feasible() {
	if(!integerFeasible())
		return false;
	PlanarModule pm;
	if(!pm.planarityTest(*currentIntegerSolution))
		return false;
	
	PlanRep P((const Graph&)*currentIntegerSolution);
	P.initCC(0);
	
	master()->upperBoundSource = AbacusOptimalCrossingMinimizer::Master::SS_ILP;
	if(master()->bestSolution) delete master()->bestSolution;
	master()->bestSolution = new CrossingConfiguration(P, 
		currentIntegerSolution->numberOfNodes() - master()->expandedGraph->numberOfNodes(),
		false);
	return true;
}

int AbacusOptimalCrossingMinimizer::Subproblem::separate() {
	lout() << "Subproblem " << id() << " -> separate: ";

	int ret = constraintPoolSeparation(0, master()->kuratowskiPool);
	if(!ret) {
		lout() << "new ";
		int numtries = master()->m_numTryKuratowskis;
		int numcuts = master()->m_numCutKuratowskis;
		if(!father() && solveIteration==1) {
			lout() << "[Start]";
			numcuts = master()->m_numStartKuratowskis;
			numtries = numcuts * master()->m_numTryKuratowskis / master()->m_numCutKuratowskis;
		}
		ABA_BUFFER<ABA_CONSTRAINT*> cuts(master(),numcuts);
		KillingTop10Heap<KuratowskiConstraint> PL(numcuts);
		GraphReduction R(*currentRoundedSolution);
		for(int h = numtries; h-->0;) {
			List<edge> K;
			findKuratowski(R, K);
			KuratowskiConstraint* kc = new KuratowskiConstraint(master(), this, R, K);
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

int AbacusOptimalCrossingMinimizer::Subproblem::makeFeasible() {
	lout() << "Subproblem " << id() << " -> makeFeasible: -> calling pricing...\n";
	return pricing();
}

void AbacusOptimalCrossingMinimizer::Subproblem::duplicateKuratowskis( 
		AbacusOptimalCrossingMinimizer::CrossingVariable* cvar, 
		List<AbacusOptimalCrossingMinimizer::KuratowskiConstraint*>& L) {
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

double AbacusOptimalCrossingMinimizer::Subproblem::generateVariable(edge e1, int e1id, edge e2, int e2id, AbacusOptimalCrossingMinimizer::CrossingVariable** var) {

	*var = new CrossingVariable(master(), e1, e1id, e2, e2id);
	double rc = (*var)->redCost(actCon(), yVal_);
	if(rc >= 0 ) { // var DOES price out correctly
		delete *var;
		*var = 0;
	}
	return rc;
}

int AbacusOptimalCrossingMinimizer::Subproblem::pricing() {
	
	OGDF_ASSERT( master()->pricing() == true );
	
	lout() << "Subproblem " << id() << " -> pricing: ";

	ABA_BUFFER<ABA_VARIABLE*> vars(master(),master()->numAddVariables());

	KillingTop10Heap<CrossingVariable> PL(master()->numAddVariables());
//	BufferList<CrossingVariable*> PL(master()->numAddVariables());

	List<edge>& es1 = master()->helperEdgelist1;
	List<edge>& es2 = master()->helperEdgelist2;
	es1.permute();
	es2.permute();
	
	// TODO : m_tryCrossingAdjacentVarsFirst
	
	int tries = 0;	
	
	Master::ActiveVariables* AV = master()->activeVars;
	
	for(ListConstIterator<edge> it1 = es1.begin(); it1.valid(); ++it1) {
		edge e1 = *it1;
		for(ListConstIterator<edge> it2 = es2.begin(); it2.valid(); ++it2) {
			edge e2 = *it2;
			
			if( ! master()->variableAllowed(e1, e2) ) // var has to make sense
				continue;
			if( !CrossingInfo::preferedOrder(e1, e2) ) // normed ordering, to prevent variable duplication; 
				continue;
			
			KillingTop10Heap<CrossingVariable> SL(master()->numAddSimilarVariables());
//			BufferList<CrossingVariable*> SL( master()->numAddSimilarVariables() );
			
			CrossingVariable* var;
			double rc;

			int min1 = AV->minIndex(e1,e2);
			int max1 = AV->maxIndex(e1,e2);
			int min2 = AV->minIndex(e2,e1);
			int max2 = AV->maxIndex(e2,e1);

			// interior corner filling 
			int a;
			for(int e1id = min1; e1id <= max1; e1id++) {
				//try top
				a = AV->maxIndex(e1, e1id, e2) + 1;
				if( (e1id > min1 && AV->maxIndex(e1, e1id-1, e2) >= a) ||
						(e1id < max1 && AV->maxIndex(e1, e1id+1, e2) >= a) ) {
					rc = generateVariable(e1, e1id, e2, a, &var);
					if(var)	SL.pushAndKill(-rc, var);
				}
				// try bottom
				a = AV->minIndex(e1, e1id, e2) - 1;
				if( (e1id > min1 && AV->minIndex(e1, e1id-1, e2) <= a) ||
						(e1id < max1 && AV->minIndex(e1, e1id+1, e2) <= a) ) {
					rc = generateVariable(e1, e1id, e2, a, &var);
					if(var)	SL.pushAndKill(-rc, var);						
				}
			}
				
				
			// extending the exterior
			int numFree;
			// left vs. right
			{
				numFree = AV->numFree(e1);
				CrossingVariable* bestLeft = 0;
				CrossingVariable* bestRight = 0;
				double rcLeft = 0;
				double rcRight = 0;
				bool newLeft = false;
				bool newRight = false;
						
				newLeft = AV->isNew(e1, min1 - 1);
				newRight = AV->isNew(e1, max1 + 1);
				
				if( !newLeft || numFree ) { // adding might be possible
					for(int e2id = AV->minIndex(e1, min1, e2); e2id <= AV->maxIndex(e1, min1, e2); e2id++) { // XYZ
						rc = generateVariable(e1, min1 - 1, e2, e2id, &var);
						if(var) {
							if(rc < rcLeft) {
								if(bestLeft) delete bestLeft;
								bestLeft = var;
								rcLeft = rc;
							} else {
								delete var;
							}
						}
					}
				}
				if( !newRight || numFree ) { // adding might be possible
					for(int e2id = AV->minIndex(e1, max1, e2); e2id <= AV->maxIndex(e1, max1, e2); e2id++) {
						rc = generateVariable(e1, max1 + 1, e2, e2id, &var);
						if(var) {
							if(rc < rcRight) {
								if(bestRight) delete bestRight;
								bestRight = var;
								rcRight = rc;
							} else {
								delete var;
							}
						}
					}
				}
				if(numFree == 1 && newRight && bestRight && newLeft && bestLeft) { // i'm gonna restrict myself
					if(rcRight > rcLeft) {
						delete bestRight;
						bestRight = 0;
					} else {
						delete bestLeft;
						bestLeft = 0;
					}
				}
				if(bestLeft)
					SL.pushAndKill(-rcLeft, bestLeft);
				if(bestRight)
					SL.pushAndKill(-rcRight, bestRight);
			}
				
				
			// top vs. bottom
			{
				numFree = AV->numFree(e1);
				CrossingVariable* bestBot = 0;
				CrossingVariable* bestTop = 0;
				double rcBot = 0;
				double rcTop = 0;
				bool newBot = false;
				bool newTop = false;
				
				newBot = AV->isNew(e2, min2 - 1);
				newTop = AV->isNew(e2, max2 + 1);
				
				if( !newBot || numFree ) { // adding might be possible
					for(int e1id = AV->minIndex(e2, min2, e1); e1id <= AV->maxIndex(e2, min2, e1); e1id++) {
						rc = generateVariable(e1, e1id, e2, min2 - 1, &var);
						if(var) {
							if(rc < rcBot) {
								if(bestBot) delete bestBot;
								bestBot = var;
								rcBot = rc;
							} else {
								delete var;
							}
						}
					}
				}
				if( !newTop || numFree ) { // adding might be possible
					for(int e1id = AV->minIndex(e2, max2, e1); e1id <= AV->maxIndex(e2, max2, e1); e1id++) {
						rc = generateVariable(e1, e1id, e2, max2 + 1, &var);
						if(var) {
							if(rc < rcTop) {
								if(bestTop) delete bestTop;
								bestTop = var;
								rcTop = rc;
							} else {
								delete var;
							}
						}
					}
				}
				if(numFree == 1 && newTop && bestTop && newBot && bestBot) { // i'm gonna restrict myself
					if(rcTop > rcBot) {
						delete bestTop;
						bestTop = 0;
					} else {
						delete bestBot;
						bestBot = 0;
					}
				}
				if(bestBot)
					SL.pushAndKill(-rcBot, bestBot);
				if(bestTop)
					SL.pushAndKill(-rcTop, bestTop);
			}
			
			// transfer the (similiar) objects into the main pricing list
			for(int i = SL.size(); i-->0;) {
				tries++;
				PL.pushAndKill(SL[i].value(), SL[i].item());
			}
			
			if(master()->numTryVariables() > 0 && PL.full() && tries > master()->numTryVariables())
				goto enough;
		}
	}
	enough:
	
	List<KuratowskiConstraint*> dkl;
	ABA_BUFFER<ABA_CONSTRAINT*> simpl(master(),PL.size());
		
	int pli = 0;
	for(int i = PL.size(); i-->0;) {
		CrossingVariable* var = PL[i].item();

//		lout() << "Adding variable " << ++pli << "/" << PL.size() << ": " << *var << "\n" << flush;

		OGDF_ASSERT( var->e1->index() != var->e2->index() );

		bool new1 = var->e1no > AV->maxIndex(var->e1) || var->e1no < AV->minIndex(var->e1);
		bool new2 = var->e2no > AV->maxIndex(var->e2) || var->e2no < AV->minIndex(var->e2);
		
		if( ( new1 && AV->numFree(var->e1)==0 ) ||
		    ( new2 && AV->numFree(var->e2)==0 ) ) { // in the meantime, someone took the spot already. i'm sorry
		    delete var;
		} else {
			if( new1 )
				simpl.push(new SimplicityConstraint(master(), var->e1, var->e1no));
			if( new2 )
				simpl.push(new SimplicityConstraint(master(), var->e2, var->e2no));

			if(master()->duplicateKuratowskis())
				duplicateKuratowskis(var, dkl);
			AV->registerVariable(var);
			vars.push(var);
		}
	}
	
	// new top, etc.? -> add simplicityconstraints
	int ret = addVars(vars);
	int sim = 0;
	if(simpl.number())
		sim = addCons(simpl);
	int kur = 0;
	if(dkl.size()) {
		ABA_BUFFER<ABA_CONSTRAINT*> dklb(master(),dkl.size());
		forall_listiterators(KuratowskiConstraint*, i, dkl)
			dklb.push(*i);
		kur = addCons(dklb, master()->kuratowskiPool);
	}
		
	lout() << "(+sim=" << sim << ", +kur=" << kur << ") " << ret << " -> " << master()->m_usedVariables << "/" << master()->m_maxVariables << " \n";
	return ret;
}

void AbacusOptimalCrossingMinimizer::Subproblem::findKuratowski(Graph& R, List<edge>& K) {
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

CrossingConfiguration* AbacusOptimalCrossingMinimizer::Subproblem::callBoundHeuristic(bool integer) {
	int bh = master()->numBoundHeuristics();
	if(!bh) return 0;
	
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
	/*
	for(; bh-->0;) {
		PlanRep HPR((const Graph&)PR);
	
		SubgraphPlanarizer sp; // CrossingMinimizationModule
		int ignore;
		sp.call(HPR, 0, helpcost, helpforbid, ignore );
		
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
		
		if(bcc) { // i found one earlier
			if(bcc->getCrossingNo() > newObj) {
				delete bcc;
				bcc = new CrossingConfiguration(HPR, newObj, false);
			}
		} else if(master()->betterPrimal(newObj)) {
			bcc = new CrossingConfiguration(HPR, newObj, false);
		}
	}
	
//	lout(LL_MINOR) << "[priBnd=" << master()->primalBound() << "; newObj=" << newObj << "]";

	return bcc;*/
}

int AbacusOptimalCrossingMinimizer::Subproblem::improve(double &primalValue) {
	lout() << "Subproblem " << id() << " -> improve:";

	CrossingConfiguration* heuristicSolution;
	if(!father() && solveIteration==1) {
		lout() << " Init: ";
		heuristicSolution = master()->bestSolution;
		master()->updateBestSolution(heuristicSolution);
		primalValue = heuristicSolution->getCrossingNo();
		master()->primalBound(primalValue);
		lout() << "[" << primalValue << "]";
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
	return 1;
}

void AbacusOptimalCrossingMinimizer::Subproblem::realizeVariable(CrossingVariable* cvar, double val) {
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

int AbacusOptimalCrossingMinimizer::Subproblem::solveLp() {
	clearCurrents();
	lout() << "Subproblem " << id() << " -> solve -> ";
	
	int r = ABA_SUB::solveLp();		
	solveIteration++;	

	if(!r) {  // loesung existiert 
		lout() << "completed:\n";
		initCurrents();
		double sum = 0;
		for(int i = 0; i < nVar(); i++) {
			if(CrossingVariable* cvar = (CrossingVariable*)variable(i)) {
				realizeVariable(cvar, xVal(i));
				sum += cvar->obj() * xVal(i);
			}
		}
		lout() << "\t=> ObjValue: " << sum << "\n";
	} else
		lout() << "failed\n";
		
	return r;
}


Module::ReturnType AbacusOptimalCrossingMinimizer::Master::doCall(PlanRep &_PG,
		int cc,
		const EdgeArray<int>  &_cost,
		const EdgeArray<bool> &_forbid, 
		const EdgeArray<unsigned int>  &_subgraphs,
		int& crossingNumber) {
	
	if(effectiveLogLevel() >= LL_ALARM || effectiveStatisticMode())
		outLevel(AbacusOptimalCrossingMinimizer::Master::Silent);
	else
		outLevel(AbacusOptimalCrossingMinimizer::Master::Full);
		
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
	       << "\nepsilonifySegments = " << m_epsilonifySegments
	       << "\ntryCrossingAdjacentVarsFirst = " << m_tryCrossingAdjacentVarsFirst
	       << "\npricingInitialization = " << ( ( m_pricingInitialization == PI_BestHeuristic ) ? "BestHeuristic" :
	                                          ( ( m_pricingInitialization == PI_EachPair ) ? "EachPair" : "None" ) )
	       << "\nduplicateKuratowskis = " << m_duplicateKuratowskis	                                          
	       << "\n---------------------------------------------\n";

	bestSolution = helpCall(_PG, cc, _cost, _forbid, crossingNumber);
	
	segmentEpsilon = SEGMENT_EPSILON / (double)expansionFactor;
	
	minimizedGraph->allEdges(helperEdgelist1);
	minimizedGraph->allEdges(helperEdgelist2);
	m_maxVariables = 0;
	m_usedVariables = 0;
	

	if(upperbound > lowerbound) { // otherwise cInfo is already optimal...
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
		
		lout() << "Max #Variables: " << m_maxVariables << "; Generated Variables: " << m_usedVariables << "(=" << (100*m_usedVariables/(double)m_maxVariables) << "%)\n";
		
		lout() << "Abacus ended with: ";
		switch(s) {
			case Optimal: lout() << "Optimal\n"; break;
			case Error: lout() << "Error\n"; break;
			case OutOfMemory: lout() << "OutOfMemory\n"; break;
			case Unprocessed: lout() << "Unprocessed\n"; break;
			case Processing: lout() << "Processing\n"; break;
			case Guaranteed: lout() << "Guaranteed\n"; break;
			case MaxLevel: lout() << "MaxLevel\n"; break;
			case MaxCpuTime: lout() << "MaxCpuTime\n"; break;
			case MaxCowTime: lout() << "MaxCowTime\n"; break;
			case ExceptionFathom: lout() << "ExceptionFathom\n"; break;
			default: lout() << "UNKNOWN STATUS!\n"; break;
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
		lout() << "# Simplicity-Constraints: " << conPool()->number() << "+" << cutPool()->number() 
		       << " = " << (conPool()->number() + cutPool()->number()) << "\n";
		lout() << "# Kuratowski-Constraints: " << kuratowskiPool->number() << "\n";
	}
	
	sout() << "\t" << isOptimal()
	       << "\t" << (status()==OutOfMemory)
	       << "\t" << (status()==Error)
	       << "\t" << m_isTrivial
	       << "\t" << (m_isTrivial ? crossingNumber : (int)ceil(dualBound()))
	       << "\t" << crossingNumber
	       << "\t" << (m_isTrivial ? 0 : totalTime()->seconds())
	       << "\t" << (m_isTrivial ? 0 : m_usedVariables)
	       << "\t" << ( (m_isTrivial||(m_maxVariables==0)) ? 0 : (100*m_usedVariables/(double)m_maxVariables))
	       << "\t" << (m_isTrivial ? 0 : conPool()->number())
	       << "\t" << (m_isTrivial ? 0 : cutPool()->number())
	       << "\t" << (m_isTrivial ? 0 : kuratowskiPool->number());
	       
	return isOptimal() ? retOptimal : (status()==OutOfMemory || status()==Error) ? retError : retFeasible;
}

bool AbacusOptimalCrossingMinimizer::Master::variableAllowed(edge e1, edge e2) {
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


void AbacusOptimalCrossingMinimizer::Master::initializeOptimization() {
	
	lout() << "Initializing Optimization...";
	
	int numMaxCols = pricing() ? numMinMaxCrossingPairs : numExpMaxCrossingPairs;
	ABA_BUFFER<ABA_VARIABLE*> variables(this, numMaxCols);
	int numSimConstraints = numExpEdges;
	ABA_BUFFER<ABA_CONSTRAINT*> simplicityConstraints(this,numSimConstraints);
	
	if(pricing()) { // PRICING
		// add one pair each...
		if( pricingInitialization() == PI_EachPair ) {
			edge e1, e2;
			forall_edges(e1,*minimizedGraph)  {
				int se1 = expandedGraph->chain(e1).size();
				forall_edges(e2, *minimizedGraph) {
					if( CrossingInfo::preferedOrder(e1,e2) && variableAllowed(e1,e2) ) {
						CrossingVariable* var = new CrossingVariable(this, e1, 0, e2, 0);
						activeVars->registerVariable(var);
						variables.push(var);
						m_maxVariables += se1*expandedGraph->chain(e2).size();
					}
				}
			}
			edge e;
			forall_edges(e, *minimizedGraph) { // don't forget to add addtl simplicity constraints as new segments occur!
				simplicityConstraints.push(new SimplicityConstraint(this, e, 0));
			}
		} else if( pricingInitialization() == PI_BestHeuristic ) {
			if(bestSolution) {
				// TODO : PI_BestHeuristic
				cerr << "ATTN: PI_BestHeuristic isn't implemented yet! -> using PI_None...";
			}
		} // else: PI_None -> Nothing to do
	} else { // NO PRICING
		edge e1, e2;
		forall_edges(e1,*minimizedGraph)  {
			int se1 = expandedGraph->chain(e1).size();
			forall_edges(e2, *minimizedGraph) {
				if( CrossingInfo::preferedOrder(e1,e2) && variableAllowed(e1,e2) ) {
					int se2 = expandedGraph->chain(e2).size();
					for(int i1 = 0; i1<se1; i1++) {
						for(int i2 = 0; i2<se2; i2++) {
							CrossingVariable* var = new CrossingVariable(this, e1, i1, e2, i2);
							activeVars->registerVariable(var);
							variables.push(var);
							m_maxVariables++;
						}
					}
				}
			}
		}
		edge e;
		forall_edges(e, *minimizedGraph) { // add all simplicity constraints
			for(int i = activeVars->numIndices(e); i-->0;) {
				simplicityConstraints.push(new SimplicityConstraint(this, e, i + activeVars->minIndex(e)));
			}
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
	
	initializePools(simplicityConstraints, variables,
		numExpMaxCrossingPairs, // worst case: all variables
		numSimConstraints, // the default cut pool will contain the additional SimplicityConstraints
		false); // dynamic cut pool
		
	kuratowskiPool = new ABA_NONDUPLPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, 1000, true); // dynamic
	
	lout() << "done\n";
}

int AbacusOptimalCrossingMinimizer::Master::enumerationStrategy(ABA_SUB* s1, ABA_SUB* s2) {
	// TODO: select the one with more fixed "1s" first !
	return ABA_MASTER::enumerationStrategy(s1, s2);
}

std::ostream &operator<<(std::ostream &os, const AbacusOptimalCrossingMinimizer::CrossingInfo& v) {
	os << "(e1=" << v.e1->index() << "/" << v.e1no << ",e2=" << v.e2->index() << "/" << v.e2no <<")";
	return os;
}

std::ostream &operator<<(std::ostream &os, const AbacusOptimalCrossingMinimizer::CrossingVariable& v) {
	os << (const AbacusOptimalCrossingMinimizer::CrossingInfo&)v;
	return os;
}


std::ostream &operator<<(std::ostream &os, /*const*/ AbacusOptimalCrossingMinimizer::KuratowskiConstraint& k) {
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

std::ostream &operator<<(std::ostream &os, /*const*/ AbacusOptimalCrossingMinimizer::SimplicityConstraint& k) {
	os << "(SC edge=" << k.e->index() << " segId=" << k.segId << " RHS=" << k.rhs() << ")";
	//k.printCoeffs(os);
	return os;
}


} //namespace

#endif // USE_ABACUS
