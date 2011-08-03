/*
 * $Revision: 1.19 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-12-08 02:02:23 +1100 (Mon, 08 Dec 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Implements class OrderedOptimalCrossingMinimizer
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

#ifdef USE_ABACUS

#include <ogdf/basic/Graph.h>
#include <ogdf/basic/List.h>
#include <ogdf/basic/SList.h>
#include <ogdf/basic/Array.h>
#include <ogdf/basic/ArrayBuffer.h>
#include <ogdf/planarity/OrderedOptimalCrossingMinimizer.h>
#include <ogdf/planarity/SubgraphPlanarizer.h>
#include <ogdf/planarity/VariableEmbeddingInserter.h>
#include <ogdf/basic/math.h>
#include <ogdf/planarity/BoyerMyrvold.h>
#include <ogdf/basic/simple_graph_alg.h>

//#include <ogdf/planarity/OptimalCrossingMinimizer.h>

namespace ogdf {
	
    const double OrderedOptimalCrossingMinimizer::EPS =  0.001;
	const double OrderedOptimalCrossingMinimizer::SEG_EPS = 0.001;
	const OrderedOptimalCrossingMinimizer::KuratowskiConstraintBase::KuratowskiType OrderedOptimalCrossingMinimizer::KuratowskiConstraintBase::KT_K33 = 0;
	const OrderedOptimalCrossingMinimizer::KuratowskiConstraintBase::KuratowskiType OrderedOptimalCrossingMinimizer::KuratowskiConstraintBase::KT_K5 = -1;
	
void OrderedOptimalCrossingMinimizer::Master::setDefaultSettings() {
	m_numStartHeuristics = 10;
	
	{	SubgraphPlanarizer* sp = new SubgraphPlanarizer();
		VariableEmbeddingInserter* vei = new VariableEmbeddingInserter();
		vei->removeReinsert(VariableEmbeddingInserter::rrIncremental);
		sp->setInserter(vei);
		sp->permutations(100);
		m_startHeuristic.set(sp); }
	
	{	SubgraphPlanarizer* sp = new SubgraphPlanarizer();
		VariableEmbeddingInserter* vei = new VariableEmbeddingInserter();
		vei->removeReinsert(VariableEmbeddingInserter::rrAll);
		sp->setInserter(vei);
		sp->permutations(20);
		m_boundHeuristic.set(sp); }
		
	m_pricingInit = PI_Normal;
	m_branchingMode = BM_Traditional;
	m_separationMode = SM_BoyerMyrvold;
	
	m_maxLinearOrderCuts = 30;
	m_maxTriangleCuts = 30;
	m_maxNewVars = 500;
	m_numCutHighKuratowskis = 10;
	m_numBaseEdgesForCutHighKuratowskis = 5;
	m_maxMinutes = 0;
	m_roundUp = 0.7;
	m_writeResult = NULL;
	m_writeIntermediateResultsToo = false;
	m_graphHint = GH_None;
	m_hintEffects = HE_KuratowskisMinusOne | HE_EdgeOrder | HE_IterativeLowerBound /*| HE_HighKuratowskiCutsStatic*/;
	tailOffPercent(0.001);
	m_reduceMemory = false;
}

//void OrderedOptimalCrossingMinimizer::KuratowskiConstraint::addAccordingCrossing(
//		const Subproblem* S, const PlanRep& I, edge e, int eid, List<CrossingLocation*>& L) {
//	const List<edge>& le = I.chain(e);
//	
//	OGDF_ASSERT( le.size() == 2 );
//	OGDF_ASSERT( le.front()->target() == le.back()->source() );
//	
//	node n = le.front()->target();
//	edge c, te;
//	forall_adj_edges(te, n) {
//		c = I.original(te);
//		if(c != e) break;
//	}
//	OGDF_ASSERT( c != e );
//	
//	const GraphCopy& exp = (const GraphCopy&) I.original();
//	int cid = 0;
//	ListConstIterator<edge> lci = exp.chain(exp.original(c)).begin();
//	for(; lci.valid(); lci++, cid++) {
//		if(*lci == c) break;
//	}
//	OGDF_ASSERT( *lci == c );
//	
////	if(!CrossingVariable::preferedOrder(e,c))
////		return 0;
//	const List<CrossingLocation*>& all = *(S->currentRoundedCrossings);
//		
//	CrossingLocation ci(Segment(exp.original(e), eid), Segment(exp.original(c), cid));
//	for(ListConstIterator<CrossingLocation*> it = all.begin(); it.valid(); it++) {
//		if(master()->crossingVariableComparer.equal(&ci, *it)) {
//			if(L.search(*it, master()->crossingVariableComparer)<0)
//				L.pushBack(*it);
//			return;
//		}
//	}
//	
//	lout(LL_ALARM) << "Should NEVER be here...\n";
//	lout(LL_ALARM) << "Looking for: " << ci << " [c=" << exp.original(c)->index() << ", csz=" << exp.chain(exp.original(c)).size() << "]\n";
//	lout(LL_ALARM) << "In: ";
//	for(ListConstIterator<CrossingLocation*> it = all.begin(); it.valid(); it++) {
//		lout(LL_ALARM) << " " << **it;
//	}
//	lout(LL_ALARM) << "\n";
//	
//	OGDF_ASSERT( false );
//}

//void OrderedOptimalCrossingMinimizer::KuratowskiConstraint::build(const Subproblem* S, const GraphReduction& R, const KuratowskiSubdivision& K) {
//	
//	type = K.size()==9 ? KT_K33 : KT_K5;
//
//	const PlanRep& I = (const PlanRep&) R.original();
//
//	List<CrossingLocation*> DD;
//
//	for(int path = 0; path<K.size(); path++) {
//		ListConstIterator<edge> it = K[path].begin();
//		for(; it.valid(); it++) {
//			const List<edge>& il = R.original(*it); // il in I
//			edge expfront = I.original(il.front()); // in expandedGraph
//			edge expback = I.original(il.back()); // in expandedGraph
//			
//			const GraphCopy& exp = (const GraphCopy&) I.original();
//			OGDF_ASSERT( exp.original(expfront) == exp.original(expback) );
//			edge orig = exp.original(expfront);
//	
//			ListConstIterator<edge> lci = exp.chain(orig).begin();
//			int startId = -1;
//			int endId = +master()->expansionFactor+1;
//	
//			OGDF_ASSERT( il.front()->source() == R.original((*it)->source()) );
//			OGDF_ASSERT( il.back()->target() == R.original((*it)->target()) );
//	
//			//calc start
//			if(I.original(il.front()->source()) == 0) { // was crossing in I			
//				startId = 0; // really?
//				for(; lci.valid(); lci++, startId++) {
//					if(*lci == expfront) break;
//				}
//				OGDF_ASSERT( *lci == expfront );
//				OGDF_ASSERT( I.chain(expfront).size() == 2 ); // this did crash...?! <<< CHECK THAT
//				addAccordingCrossing(S, I, expfront, startId, DD);
//			} else {
//				OGDF_ASSERT( exp.original(I.original(il.front()->source())) );
//			}
//			//calc end
//			if(I.original(il.back()->target()) == 0) { // was crossing in I			
//				endId = max( startId, 0 );
//				for(; lci.valid(); lci++, endId++) {
//					if(*lci == expback) break;
//				}
//				OGDF_ASSERT( *lci == expback );
//				OGDF_ASSERT( I.chain(expback).size() == 2 );
//				addAccordingCrossing(S, I, expback, endId, DD);
//			} else {
//				OGDF_ASSERT( exp.original(I.original(il.back()->target())) );
//			}
//			
//			edges[orig] = new SegmentRange(startId, endId, path);
//		}
//	}
//
//	OGDF_ASSERT( rhs_ == 0 || rhs_ == 1 );
//	rhs_ = rhs_ + 1 - DD.size(); // original rhs might be either 0 or 1, depending on the Restrictiveness, given in the constructor
//
//	D.init(DD.size());
//	int idx = 0;
//	for(ListConstIterator<CrossingLocation*> dit = DD.begin(); dit.valid(); dit++, idx++) {
//		D[idx] = *dit;
//	}
//	
//	D.quicksort(master()->crossingVariableComparer);
//}

bool OrderedOptimalCrossingMinimizer::Subproblem::feasible() {
	
	return false; // "improve" will check that
/*	
	if(master()->numPermutationsBoundHeuristic()) 
		return false; 	// if it's feasible, improve will recognize that...
		
	// the code below is only needed if no heuristics are normally run during the B&B (that might haoppen for Kn-proofing)
	if(generatedConVar != 0)
		return false;
	if(!integerFeasible())
		return false;
		
	PlanarModule pm;
	if(!pm.planarityTest(*currentIntegerSolution))
		return false;
	
	// this will force "improve" to check the solution
	master()->numPermutationsBoundHeuristic(-1);
	
	return false;	*/
}

int OrderedOptimalCrossingMinimizer::Subproblem::generateBranchRules(ABA_BUFFER<ABA_BRANCHRULE*> &rules) {
	if (master()->branchingMode() == BM_CompleteOdd) {
		lout() << "Analyzing LP-relaxation for possible CompleteOdd-Branchings...";
		
		ABA_ACTIVE<ABA_CONSTRAINT, ABA_VARIABLE> *aC = actCon(); // should i really only check the active constraints??
		KuratowskiConstraintBase* bestK = NULL;
		double bestImp = 1;
		int tight = 0;
		for(int i = aC->number(); i-->0;) {
			KuratowskiConstraintBase* k = dynamic_cast<KuratowskiConstraintBase*>((*aC)[i]);
			if(!k) continue;
			if(k->isCompleteOdd()) {
				double imp = fabs(1 - k->slack(actVar(), xVal_) );
				if(imp < bestImp) {
					bestK = k;
					bestImp = imp;
				}
			}			
		}	
		
		if(bestK && bestImp < 0.7) {
			lout() << "successful (" << bestImp << ", Typ=" << bestK->type << ") -> adding branching constraints.\n";	
			KuratowskiConstraintBase *k1 = NULL, *k2 = NULL;
			bestK->branchMe(this, k1, k2);			
			rules.push(new ABA_CONBRANCHRULE(master(), master()->branchingPool->insert(k1)));
			rules.push(new ABA_CONBRANCHRULE(master(), master()->branchingPool->insert(k2)));	
			return 0;
		}
		if(bestK)
			lout() << "failed (" << bestImp << ", Typ=" << bestK->type << "). Using traditional branching.\n";
		else	
			lout() << "failed (" << bestImp << ", Typ=none). Using traditional branching.\n";	
	}
	return ABA_SUB::generateBranchRules(rules);  // resort to standard branching
}


int OrderedOptimalCrossingMinimizer::Subproblem::pricing() {	
	// the real column generation is done at solveLp... we just report it here
	return (generatedConVar > 0) ? generatedConVar : 0;
}


int OrderedOptimalCrossingMinimizer::Subproblem::separate() {
	// the real separation is done at solveLp... we just report it here
	return (generatedConVar < 0) ? -generatedConVar : 0;
}

int OrderedOptimalCrossingMinimizer::Subproblem::separateLinearOrder() {
	//TODO: if this is slow, try: start with "promising" edges, and terminate when maxLinearOrderCuts is reached...
	// Alternative 2: if we need a lot of these constraints: generate all in advance and put in pool? (well... there are a LOT of these...)
	
	DeletingTop10Heap<CyclicOrderConstraint, double, Equal<CyclicOrderConstraint> > PL(master()->maxLinearOrderCuts());
	edge e,f,g,h;
	int foundWithMulti = 0;
	forall_edges(e, *(master()->givenGraph) ) {
		if(master()->activeVars->numOrdered(e)>=3) { // ... checksum on edge from checkAmbuigities??
			ArrayBuffer<edge> aes(master()->activeVars->numOrdered(e)); // active edges
			forall_edges(f, *(master()->givenGraph)) {
				if( !master()->activeVars->isOrdered(e,f) ) continue;
				int idx = master()->activeVars->index(e,f);
				if(idx >= 0 && xVal(idx) > EPS)
					aes.push(f);
			}
			for(int fi = aes.size(); fi-->0;) {
				f = aes[fi];
				for(int gi = aes.size(); gi-->0;) {
					if(fi == gi) continue;
					g = aes[gi];
					for(int hi = aes.size(); hi-->0;) {
						if(fi == hi || gi == hi) continue;
						h = aes[hi];
						double lhs = 
							xVal(master()->activeVars->index(e,f,g)) + 
							xVal(master()->activeVars->index(e,g,h)) + 
							xVal(master()->activeVars->index(e,h,f));
						if(lhs > 2 + EPS) { // found 
							PL.pushAndDelete(new CyclicOrderConstraint(master(),e,f,g,h), lhs);
							++foundWithMulti;
						}
					}
				}
			}
		}
	}
	
	ABA_BUFFER<ABA_CONSTRAINT*> cuts(master(), PL.size());
	for(int pli = 0; pli<PL.size(); pli++) {
		cuts.push(PL[pli].item());
	}
	int ret = addCons(cuts, master()->linearOrderPool);
	OGDF_ASSERT( ret == PL.size());
//	lout() << ret << " new lin.ord. constraints (" << foundWithMulti << ")\n";
	return ret;
}

void OrderedOptimalCrossingMinimizer::Master::initNunchakus() {
	nunchakus.init(*givenGraph);
	edge e, a, b;
	ArrayBuffer<Nunchaku> help(50);
	forall_edges(e, *givenGraph) {
		forall_adj_edges(a, e->source()) {
			forall_adj_edges(b, e->target()) {
				if(a->commonNode(b))
					help.push(Nunchaku(a,b));
			}
		}
		help.compactCopy(nunchakus[e]);
		help.clear();
	}
}

//int OrderedOptimalCrossingMinimizer::Subproblem::separateTriangles() {
//	DeletingTop10Heap<TriangleConstraint, double, Equal<TriangleConstraint> > PL(master()->maxTriangleCuts());
//	OrderedCrossingVariable* o;
//	double d;
//	edge a,b,c,r,g;
//	for(int i = nVar(); i-->0;) {
//		if( !(o = dynamic_cast<OrderedCrossingVariable*>(variable(i))) || ((d=xVal(i))<EPS) ) continue;
//		c = o->base();
//		r = o->crossedBy();
//		g = o->before();
//		const Array<Nunchaku>& L = master()->nunchakus[c];
//		for(int j = L.size(); j-->0;) {
//			a = L[j].a;
//			b = L[j].b;
//			int x,y;
//			if( (x=master()->activeVars->index(a,g))<0 || (y=master()->activeVars->index(b,r))<0 ) continue;
//			if( (d+=xVal(x)<1+EPS) || ((d+=xVal(y))<2+EPS) ) continue;
//			if( ((x=master()->activeVars->index(r,g))<0) || ((d-=xVal(x))>2+EPS) ) {
//				if( ((x=master()->activeVars->index(r,a))<0) || ((d-=xVal(x))>2+EPS) ) {
//					if( ((x=master()->activeVars->index(g,b))<0) || ((d-=xVal(x))>2+EPS) ) {
//						PL.pushAndDelete(new TriangleConstraint(master(),c,a->commonNode(b),r,g), d);						
//					}
//				}
//			}
//		}		
//	}
//	
//	ABA_BUFFER<ABA_CONSTRAINT*> cuts(master(), PL.size());
//	for(int pli = 0; pli<PL.size(); pli++) {
//		cuts.push(PL[pli].item());
//	}
//	int ret = addCons(cuts, master()->trianglePool);
//	OGDF_ASSERT( ret == PL.size());
////	lout() << ret << " new triangle constraints\n";
//	return ret;
//}

OrderedOptimalCrossingMinimizer::KuratowskiConstraintBase* OrderedOptimalCrossingMinimizer::Subproblem::constructKuratowskiConstraint(const PlanRep& G, const KuratowskiSubdivision& KP) {
	// transpose KuSub into edges of G
	KuratowskiSubdivision KG;
	KG.init(KP.size());	
	int i;
	for(i = KP.size(); i-->0;) {
		edge last = 0;
		edge next;
		forall_listiterators(edge, it, KP[i]) {
			next = G.original(*it);
			if(next != last) {
				KG[i].pushBack(next);
				last = next;
			}
		}
	}
	
	BasicKuratowskiConstraint* bkc = new BasicKuratowskiConstraint(master(), KG, false);

	ArrayBuffer<CrossingVariableBase*> willBeOld(inducedSeparationSimpleCrossings.size() > 1 ? inducedSeparationSimpleCrossings.size() : 1);
	
	if(inducedSeparationSimpleCrossings.size() > 0) {
		ArrayBuffer<SimpleCrossingVariable*> usedSimpleCrossings(inducedSeparationSimpleCrossings.size());
//		cerr << ">>> ";

		ArrayBuffer<SimpleCrossingVariable*> KN(6); // at most
		for(i = KP.size(); i-->0;) { // detect kuratowski-dummy-nodes
			node n;
			//check front
			if(KP[i].size()==1)
				n = KP[i].front()->source();
			else {
				edge e1 = KP[i].front();
				edge e2 = *(KP[i].begin().succ());
				n = e1->opposite(e1->commonNode(e2));
			}
			if(!G.original(n)) {
				edge c1 = G.original(n->firstAdj()->theEdge());
				edge c2 = G.original(n->firstAdj()->succ()->theEdge());
				if(c1==c2) c2 = G.original(n->lastAdj()->theEdge());
				SimpleCrossingVariable* v = (SimpleCrossingVariable*)variable( master()->activeVars->index(c1,c2) );
				if(KN.linearSearch(v)==-1) {
					KN.push(v);
//					cerr << "KN[" << *v << "] ";
				}
			}
			//check back
			if(KP[i].size()==1)
				n = KP[i].front()->target();
			else {
				edge e1 = KP[i].back();
				edge e2 = *(KP[i].rbegin().pred());
				n = e1->opposite(e1->commonNode(e2));
			}
			if(!G.original(n)) {
				edge c1 = G.original(n->firstAdj()->theEdge());
				edge c2 = G.original(n->firstAdj()->succ()->theEdge());
				if(c1==c2) c2 = G.original(n->lastAdj()->theEdge());
				SimpleCrossingVariable* v = (SimpleCrossingVariable*)variable( master()->activeVars->index(c1,c2) );
				if(KN.linearSearch(v)==-1) {
					KN.push(v);
//					cerr << "KN[" << *v << "] ";
				}
			}
		}
		
		forall_listiterators(SimpleCrossingVariable*, it, inducedSeparationSimpleCrossings) {
			SimpleCrossingVariable* s = *it;
			if( bkc->findInPath(s) || KN.linearSearch(s)>=0 ) {
				usedSimpleCrossings.push(s);
//				cerr << "*";
			}					
//			cerr << *s << " ";
		}
//		cerr << endl;

		Array<bool> used[2];
		used[0].init(0,usedSimpleCrossings.size()-1,false); 
		used[1].init(0,usedSimpleCrossings.size()-1,false); 
		
		for(int ix1 = 0; ix1<usedSimpleCrossings.size(); ++ix1) {
			SimpleCrossingVariable* s1 = usedSimpleCrossings[ix1];
			for(int idx = 2; idx-->0;) {
				if(used[idx][ix1]) continue;
				edge base = s1->e[idx];
				ArrayBuffer<edge> cross(usedSimpleCrossings.size());
				for(int ix2 = ix1+1; ix2<usedSimpleCrossings.size(); ++ix2) {
					SimpleCrossingVariable* s2 = usedSimpleCrossings[ix2];
					for(int k = 2; k-->0;) {
						if(s2->e[k] == base) {
							cross.push(s2->e[1-k]);
							used[k][ix2] = true;
//							cerr << "k" << *s2 << " {" << k << "}\n";
						}
					}
				}
				if(cross.size()>0) {
//					cerr << "1" << *s1 << " {" << idx << "}\n";
					cross.push(s1->e[1-idx]);
					used[idx][ix1] = true;
					
					edge front = 0;
					forall_listiterators(edge, elem, G.chain(base)) {
						edge c = CrossingConfiguration::targetCrossingEdge(G, *elem);
						if(cross.linearSearch(c,master()->edgeComparer) >=0) {
							if(front) {
								willBeOld.push( (CrossingVariableBase*)variable( master()->activeVars->index(base, front, c) ) );
							}
							front = c;
						}
					}					
				}
			}
			if(!used[0][ix1] && !used[1][ix1])
				willBeOld.push(s1);
		}
	}

	if(willBeOld.size()) {
		willBeOld.compactCopy(bkc->oldCrossings);
		bkc->oldCrossings.quicksort(master()->crossingVariableComparer);
		bkc->rhs_ = bkc->rhs_ - bkc->oldCrossings.size();
		
//		delete bkc; /// TODO NONONO-NUNU
//		bkc=0;
		
//		cerr << "Gen (!Add?): " << *bkc << endl;
		
	} else {
		bkc->oldCrossings.init(0);
	}	
	return bkc;
}

int OrderedOptimalCrossingMinimizer::Subproblem::separateSimple() {
//	lout() << "Subproblem " << id() << " -> separate (Simple): ";
	const SimpleSeparationParams& p = master()->m_simpleSeparationParams;

	PlanRep& G = *inducedPlanarization[I_USEFORSEPARATE];

	int foundWithMulti = 0;
	
	ABA_BUFFER<ABA_CONSTRAINT*> cuts(master(), p.maxCuts());
	DeletingTop10Heap<KuratowskiConstraintBase, double, CheckedEqual<KuratowskiConstraintBase> > PL(p.maxCuts());
	for(int h = 0; h < p.desperateRuns(); h++) {
		KuratowskiSubdivision K;
		findKuratowski(G, K);
		KuratowskiConstraintBase* kc = constructKuratowskiConstraint(G,K);
		if(kc) {
			double slack;
			if(!kc->violated(actVar() , xVal_, &slack ))  {
				delete kc;
			} else {
				foundWithMulti++;
				PL.pushAndDeleteNoRedundancy( kc, slack );
			}
		}
		if(h >= p.runs() && !PL.empty()) // okidoki
			break;
	}

	for(int pli = 0; pli<PL.size(); pli++) {
		KuratowskiConstraintBase* kc = PL[pli].item();
		//lout() << *(BasicKuratowskiConstraint*)kc << endl;
//		forall_listiterators(SimpleCrossingVariable*, tyty, inducedSeparationSimpleCrossings)
//			cerr << " " << *(*tyty);
//		cerr << "\n";
//		cerr << *(BasicKuratowskiConstraint*)kc << "\n";
//#ifdef OGDF_DEBUG
//		BasicKuratowskiConstraint& b = *(BasicKuratowskiConstraint*)kc;
//		if(b.type != KuratowskiConstraintBase::KT_K33)
//			cerr << "Not Checking K5.\n";
//		else {
//		OGDF_ASSERT( !b.oldCrossings.size() )
//		OGDF_ASSERT( b.type == KuratowskiConstraintBase::KT_K33 )
//		OGDF_ASSERT( b.newCrossings.size()==9 )
//		node A[3];
//		node B[3];
//		A[0]=A[1]=A[2]=B[0]=B[1]=B[2]=0;
//		bool usedPath[9];
//		int i;
//		for(i = 9; i-->0;) usedPath[i]=false;
//		for(i = 9; i-->0;) {
//			node first = b.newCrossings[i][0]->source();
//			node last = b.newCrossings[i][0]->target();
//			int all = b.newCrossings[i].size();			
//			bool* t = new bool[all];
//			int j;
//			for(j = all; j-->1;) t[j] = false;
//			t[0] = true;
//			int tt = 1;
//			for(int j = all; j-->1;) {
//				for(int jj = all; jj-->1;) {
//					if(t[jj]) continue;
//					if(b.newCrossings[i][jj]->isIncident(first)) {
//						first = b.newCrossings[i][jj]->opposite(first);
//						t[jj] = true;
//						++tt;
//						break;
//					} else if(b.newCrossings[i][jj]->isIncident(last)) {
//						last = b.newCrossings[i][jj]->opposite(last);
//						t[jj] = true;
//						++tt;
//						break;
//					}
//				}				
//			}
//			OGDF_ASSERT( tt == all )
//			if(i==8) {
//				A[0] = first;
//				B[0] = last;
//				usedPath[0] = true;
//			} else {				
//				int u = 0;
//				if(A[0]==first || A[1]==first || A[2]==first) {
//					if(A[1]==first) u+=3; else if(A[2]==first) u+=6;
//
//					if(B[0]==last || B[1]==last || B[2]==last) {
//						if(B[1]==last) u+=1; else if(B[2]==last) u+=2;
//						OGDF_ASSERT( !usedPath[u] );
//						usedPath[u] = true;
//					} else {
//						if(B[1]==0) {
//							B[1] = last;
//							u+=1; 
//						} else {
//							OGDF_ASSERT( !B[2] );
//							B[2] = last;
//							u+=2;
//						}
//						OGDF_ASSERT( !usedPath[u] );
//						usedPath[u] = true;
//					}
//				} else if(B[0]==first || B[1]==first || B[2]==first) {
//					if(B[1]==first) u+=1; else if(B[2]==first) u+=2;
//
//					if(A[0]==last || A[1]==last || A[2]==last) {
//						if(A[1]==last) u+=3; else if(A[2]==last) u+=6;
//						OGDF_ASSERT( !usedPath[u] );
//						usedPath[u] = true;
//					} else {
//						if(A[1]==0) {
//							A[1] = last;
//							u+=3; 
//						} else {
//							OGDF_ASSERT( !A[2] );
//							A[2] = last;
//							u+=6;
//						}
//						OGDF_ASSERT( !usedPath[u] );
//						usedPath[u] = true;
//					}
//				} else if(A[0]==last || A[1]==last || A[2]==last) {
//					if(A[1]==last) u+=3; else if(A[2]==last) u+=6;
//
//					if(B[1]==0) {
//						B[1] = first;
//						u+=1; 
//					} else {
//						OGDF_ASSERT( !B[2] );
//						B[2] = first;
//						u+=2;
//					}
//					OGDF_ASSERT( !usedPath[u] );
//					usedPath[u] = true;
//
//				} else if(B[0]==last || B[1]==last || B[2]==last) {
//					if(B[1]==last) u+=1; else if(B[2]==last) u+=2;
//
//					if(A[1]==0) {
//						A[1] = first;
//						u+=3; 
//					} else {
//						OGDF_ASSERT( !A[2] );
//						A[2] = first;
//						u+=6;
//					}
//					OGDF_ASSERT( !usedPath[u] );
//					usedPath[u] = true;
//					
//				} else
//					OGDF_ASSERT(false);
//			}			
//			delete[] t;
//		}
//		for(i = 9; i-->0;) 
//			OGDF_ASSERT( usedPath[i] );	
//		}
//#endif		
		cuts.push(kc);
	}
	int ret = addCons(cuts, master()->kuratowskiPool);
//	lout() << ret << " new Kuratowski constraints (" << foundWithMulti << ")\n";
	return ret;
}

int OrderedOptimalCrossingMinimizer::Subproblem::separateBoyerMyrvold() {
	OGDF_ASSERT( false ); // TODO: still uses graphreduction
	
//	lout() << "Subproblem " << id() << " -> separate (Multi): ";
//	const BoyerMyrvoldSeparationParams& p = master()->m_boyerMyrvoldSeparationParams;
//
//	GraphReduction R(*inducedPlanarization[I_USEFORSEPARATE]);
//	makeParallelFreeUndirected(R);
//
//	int found1 = 0;
//	int found2 = 0;
//	ABA_BUFFER<ABA_CONSTRAINT*> cuts(master(), p.maxCuts());
//	DeletingTop10Heap<KuratowskiConstraintBase, double, CheckedEqual<KuratowskiConstraintBase> > PL(p.maxCuts());
//	
//	for(int h = 0; h < p.desperateRuns(); ++h) {
//		BoyerMyrvold bm;
//		DeletingTop10Heap<KuratowskiConstraintBase, double, CheckedEqual<KuratowskiConstraintBase> > PLr(p.runCuts());
//		SList< KuratowskiWrapper > lkw;
//		bm.planarEmbed(R, lkw, p.extractions(), 
//			p.bundle(), false, true, //limit & randomDfs
//			p.noE2());
//		
//		SList< KuratowskiSubdivision > lks;
//		bm.transform(lkw, lks, R, p.veryDifferent());
//		
//		for(SListIterator<KuratowskiSubdivision> it = lks.begin(); it.valid(); ++it) {
//			KuratowskiConstraintBase* kc = constructKuratowskiConstraint(R,*it);
//			double slack;
//			if(!kc->violated(actVar() , xVal_, &slack ))  {
//				delete kc;
//			} else {
//				++found2;
//				PLr.pushAndDeleteNoRedundancy( slack, kc );
//			}		
//		}
//			
//		for(int pli = 0; pli<PLr.size(); pli++) {
//			++found1;
//			PL.pushAndDeleteNoRedundancy( PLr[pli].value(), PLr[pli].item() );
//		}
//
//		if(h >= p.runs() && !PL.empty()) // okidoki
//			break;
//	}
//	
//	for(int pli = 0; pli<PL.size(); pli++) {
//		KuratowskiConstraintBase* kc = PL[pli].item();
//		cuts.push(kc);
//	}
//	int ret = addCons(cuts, master()->kuratowskiPool);
//	lout() << ret << " new Kuratowski constraints (" << found1 << "; " << found2 << ")\n";
//	return ret;
}

int OrderedOptimalCrossingMinimizer::Subproblem::makeFeasible() {
	lout() << "Subproblem " << id() << " -> makeFeasible: -> no chance!\n";
	return 1;
}

int OrderedOptimalCrossingMinimizer::Subproblem::checkKnHighKuratowskiCutsStatic() {
	
//	lout() << "Search for HighKuratowskiCuts ";
//	
//	const Graph& MG = *(master()->givenGraph);
//	EdgeArray<double> crossingsperedge(MG,0);
//	
//	for(int i = 0; i < nVar(); i++) {
//		if(CrossingVariable* cvar = (CrossingVariable*)variable(i)) {
//			if(xVal(i) > EPS) { // there is something
//				double v = /*master()->getCost(cvar->s1.e, cvar->s2.e) **/ xVal(i);
//				crossingsperedge[cvar->s1.e] += v;
//				crossingsperedge[cvar->s2.e] += v;
//			}
//		}
//	}
//		
//	Top10Heap<Prioritized<double, edge> >  maxedges(master()->numBaseEdgesForCutHighKuratowskis());
//	edge e;
//	forall_edges(e, MG) {
//		Prioritized<double, edge> v(crossingsperedge[e],e);
//		maxedges.pushBlind(v);
//	}
//		
//	if(maxedges.empty()) {
//		Prioritized<double, edge> v(0,MG.chooseEdge());
//		maxedges.pushBlind(v);
//	}
//	
//	KuratowskiConstraint::KuratowskiType kt = KuratowskiConstraint::KTComplete(MG.numberOfNodes() - 1);
//	
//	DeletingTop10Heap<KuratowskiConstraint, double, TargetComparer<KuratowskiConstraint> > kura(master()->numCutHighKuratowskis());
//	for(int i = maxedges.size(); i-->0;) {
//		edge maxedge = maxedges[i].item();
//		node n;	
//		node src = maxedge->source();
//		node tgt = maxedge->target();
//		forall_nodes(n, MG) {
//			double sumonk = 0;
//			if(n == src || n == tgt) continue;
//			KuratowskiConstraint* k = new KuratowskiConstraint(master(), kt, false); //non-dynamic
//			int path = 0;
//			forall_edges(e, MG) {
//				if(e == maxedge) continue;
//				if( (e->target()==n && e->source() != src && e->source() != tgt) ||
//					(e->source()==n && e->target() != src && e->target() != tgt) )
//					continue;
//				sumonk += crossingsperedge[e];
//				k->addEdge(e, path++);
//			}
//			if(sumonk < k->rhs() - EPS) { //violated
//				kura.pushAndDelete(-sumonk, k);
//			} else
//				delete k;		
//		}
//	}
//
//	if(kura.empty()) return 0;
//	ABA_BUFFER<ABA_CONSTRAINT*> cuts(master(), kura.size());
//	for(int ki = kura.size(); ki-->0;) {
//		cuts.push(kura[ki].item());
//	}
//	return addCons(cuts, master()->hintedPool);
	
	lout(LL_ALARM) << "\n\n*** checkKnHighKuratowskiCutsStatic NOT IMPLEMENTED ***\n\n";
	OGDF_ASSERT( false );
}

int OrderedOptimalCrossingMinimizer::Subproblem::checkKnmHighKuratowskiCutsStatic(){
//	lout() << "Search for HighKuratowskiCuts ";
//	
//	const Graph& MG = master()->givenGraph;
//	EdgeArray<double> crossingsperedge(MG,0);
//	
//	for(int i = 0; i < nVar(); i++) {
//		if(CrossingVariable* cvar = (CrossingVariable*)variable(i)) {
//			if(xVal(i) > EPS) { // there is something
//				double v = /*master()->getCost(cvar->s1.e, cvar->s2.e) **/ xVal(i);
//				crossingsperedge[cvar->s1.e] += v;
//				crossingsperedge[cvar->s2.e] += v;
//			}
//		}
//	}
//		
//	Top10Heap<Prioritized<double, edge> >  maxedges(master()->numBaseEdgesForCutHighKuratowskis()); 
//	edge e;
//	forall_edges(e, MG) {
//		Prioritized<double, edge> v(crossingsperedge[e],e);
//		maxedges.pushBlind(v);
//	}
//		
//	if(maxedges.empty()) {
//		Prioritized<double, edge> v(0,MG.chooseEdge());
//		maxedges.pushBlind(v);
//	}
//	
//	KuratowskiConstraint::KuratowskiType kt = KuratowskiConstraint::KTBipartite(
//		MG.chooseEdge()->source()->degree()-1,
//		MG.chooseEdge()->target()->degree()-1);
//
//	DeletingTop10Heap<KuratowskiConstraint, double, TargetComparer<KuratowskiConstraint> > kura(master()->numCutHighKuratowskis());
//	for(int i = maxedges.size(); i-->0;) {
//		edge maxedge = maxedges[i].item();
//		node n,m;	
//		node src = maxedge->source();
//		node tgt = maxedge->target();
//		adjEntry an, am;
//		forall_adj(an, src) {
//			n = an->twin()->theNode();
//			if(n == tgt) continue;
//			forall_adj(am, tgt) {
//				m = am->twin()->theNode();
//				if(m == src) continue;
//				double sumonk = 0;
//				KuratowskiConstraint* k = new KuratowskiConstraint(master(), kt, false); //non-dynamic
//				int path = 0;
//				forall_edges(e, MG) {
//					if(e == maxedge) continue;
//					if( (e->source() == n && e->target() != m && e->target() != src && e->target() != tgt) ||
//						(e->target() == n && e->source() != m && e->source() != src && e->source() != tgt) ||
//						(e->source() == m && e->target() != n && e->target() != src && e->target() != tgt) ||
//						(e->target() == m && e->source() != n && e->source() != src && e->source() != tgt) )
//							continue;
//					sumonk += crossingsperedge[e];
//					k->addEdge(e, path++);
//				}
//				if(sumonk < k->rhs() - EPS) { //violated
//					kura.pushAndDelete(-sumonk, k);
//				} else
//					delete k;		
//			}
//		}
//	}
//
//	if(kura.empty()) return 0;
//	ABA_BUFFER<ABA_CONSTRAINT*> cuts(master(), kura.size());
//	for(int ki = kura.size(); ki-->0;) {
//		cuts.push(kura[ki].item());
//	}
//	return addCons(cuts, master()->hintedPool);
	lout(LL_ALARM) << "\n\n*** checkKnmHighKuratowskiCutsStatic NOT IMPLEMENTED ***\n\n";
	OGDF_ASSERT( false );

}

void OrderedOptimalCrossingMinimizer::Subproblem::findKuratowski(Graph& R, KuratowskiSubdivision& K) {
	PlanarModule pm;
	if(pm.planarityTest((const Graph&)R))
		return;
		
	List<edge> es;
	R.allEdges(es);
	es.permute(); // randomize order
	
	//List<edge> KK;
	
	for(ListIterator<edge> it = es.begin(); it.valid(); it++) { // each edge once
		edge e = *it;
		R.hideEdge(e);
		if(pm.planarityTest(R)) {
			R.restoreEdge(e);
			//KK.pushBack( e );
		}
	}
	
	// extract K in the new and cool way
	node n,kn[6];
	int k=0;
	edge e,f;
	forall_nodes(n,R) {
		OGDF_ASSERT( n->degree() != 1)
		if(n->degree() > 2)
			kn[k++] = n;
	}
	
	OGDF_ASSERT( k==5 || k==6 )
	
	List<edge> L;
	if(k==5) {
		kn[5] = 0;
		K.init(10);
		for(int k = 0; k<5; k++) {
			forall_adj_edges(e, kn[k]) {
				n = kn[k];
				f = e;
				while( (n = f->opposite( n ))->degree() == 2) {
					L.pushBack(f);
					f = ((f != n->firstAdj()->theEdge()) ? n->firstAdj() : n->lastAdj())->theEdge();
				}
				L.pushBack(f);
				int i = 0;
				while(kn[i] != n) i++;
				if(i > k) {
					if(k==0) i--;
					else if(k==1) i+=2;
					else i+=k+2;
					K[i].conc(L);
				} else L.clear();
			}
		}		
	} else { // k33
		K.init(9);
		int touched[6] = { -1, -1, -1, -1, -1, -1}, t=0, i=0;
		for(int k = 0; k<6; k++) {
			if(touched[k] != -1) continue;
			forall_adj_edges(e, kn[k]) {
				n = kn[k];
				f = e;
				while( (n = f->opposite( n ))->degree() == 2) {
					L.pushBack(f);
					f = ((f != n->firstAdj()->theEdge()) ? n->firstAdj() : n->lastAdj())->theEdge();
				}
				L.pushBack(f);
				int j = 0;
				while(kn[j] != n) j++;
				if(touched[j] == -1)
					touched[j] = t++;
				K[ i*3 + touched[j] ].conc(L);
			}
			i++;
		}
	}
	
	R.restoreAllEdges();
	
	// below is not neccessary (i.e. sorting is not necc.)
//	K.quicksort();
}

OrderedOptimalCrossingMinimizer::CrossingConfiguration* OrderedOptimalCrossingMinimizer::Subproblem::callBoundHeuristic(int indIdx) {

	const PlanRep& PR = *(inducedPlanarization[indIdx]);
	OGDF_ASSERT(PR.numberOfNodes() >= master()->givenGraph->numberOfNodes() );
	
	EdgeArray<int>* helpCost = master()->useCost() ? new EdgeArray<int>(PR) : NULL;	
	EdgeArray<bool>* helpForbid = master()->useForbid() ? new EdgeArray<bool>(PR) : NULL;
	
	edge e, oe;
	if(helpCost || helpForbid) {
		forall_edges(e, PR) {
			oe = PR.original(e);
			if(helpCost)
				(*helpCost)[e] = (*(master()->cost))[oe];
			if(helpForbid)
				(*helpForbid)[e] = (*(master()->forbid))[oe];
		}
	}
	
	PlanRep HPR((const Graph&)PR);
	int ignore;
	if(!master()->m_boundHeuristic.valid()) { // no bound heuristic...
		HPR.initCC(0);
		PlanarModule pm;
		if( !pm.planarityTest(HPR) ) return 0; // no trivial solution
		// --> trivial solution is usable		
	} else
		master()->m_boundHeuristic.get().call(HPR, 0, ignore, helpCost, helpForbid, 0);
	
	if(helpCost) delete helpCost;
	if(helpForbid) delete helpForbid;

	int newObj; // i'll have to calculate that manually...
	if(!master()->useCost())
		newObj = HPR.numberOfNodes() - master()->givenGraph->numberOfNodes();
	else {
		newObj = 0;
		node n;
		forall_nodes(n, HPR) {
			if(HPR.original(n) == NULL || PR.original(HPR.original(n)) == NULL) { // dummy found -> calc cost
				newObj += (int) master()->getCost( // integer is enough. no epsilonify here...
					PR.original(HPR.original(n->firstAdj()->theEdge())),
					PR.original(HPR.original(n->lastAdj()->theEdge())));
			}
		}
	}
	if(master()->betterPrimal(newObj)) {
		return new CrossingConfiguration(HPR, newObj, false);
	}
	return 0;
}

int OrderedOptimalCrossingMinimizer::Subproblem::improve(double& d) {
	// heuristics are run in realizeSolutions
	return 0;
}

void OrderedOptimalCrossingMinimizer::Subproblem::primalHeuristics(int inducement) {
	lout() << "[Primal heuristic] on " << ((inducement == I_INT) ? "INT: " : "RND: ");
	OGDF_ASSERT( inducedPlanarization[inducement] );
	PlanRep& P = *(inducedPlanarization[inducement]);
	
	CrossingConfiguration* crossConf = 0;
	
	PlanarModule pm;
	bool origPlanar = pm.planarEmbed(P);
	/*if(pm.planarEmbed(P)) { // TEST WITH 1476.71 in ReleaseMode!!
		int primalValue = 0;
		if(master()->useCost()) {
			node n;
			forall_nodes(n,P) {
				if(!P.original(n)) {
					primalValue += master()->getCost( n->firstAdj()->theEdge(), n->lastAdj()->theEdge() ); 
				}
			}		
		} else {
			primalValue = P.numberOfNodes() - master()->givenGraph->numberOfNodes();			
		}
		crossConf = new CrossingConfiguration(P, primalValue, true);
		master()->updateBestSolution(crossConf, false);
		master()->primalBound(primalValue);
		lout() << "NEW BOUND (LP) [" << primalValue << "]\n";
		
	} else*/ 
	if(crossConf = callBoundHeuristic(inducement)) {

		master()->updateBestSolution(crossConf, !origPlanar);
		int primalValue = crossConf->getCrossingNo();
		master()->primalBound(primalValue);
		lout() << "NEW BOUND (" << (origPlanar?"LP":"Heur") << ") [" << primalValue << "]\n";
		
	} else
		lout() << "no new bound (" << master()->primalBound() << ")\n";
}

double OrderedOptimalCrossingMinimizer::Subproblem::orderingWeight(const edge e, const edge f, const ListIterator<edge>& l, const ListIterator<edge>& r) {
	double w = 0;
	ListIterator<edge> i = l;
	while(i != r) {
		edge g = *i;
		if(g != f) {
			OGDF_ASSERT( master()->activeVars->vars[e][g].before );
			w += xVal(master()->activeVars->index(e,g,f)) - 
			     xVal(master()->activeVars->index(e,f,g));
		}
		++i;
	}
	return w;
}

void OrderedOptimalCrossingMinimizer::Subproblem::orderCrossings(const edge e, List<edge>& L) {
	if(L.size()<2) return;	
//	forall_listiterators(edge, it, L) {
//		cerr << " " << (*it)->index();
//	}
//	cerr << "\n";
	ListIterator<edge> l = L.begin();
	ListIterator<edge> r = L.end();
	ListIterator<edge> i;
	ListIterator<edge> m; // thing to move
	bool toLeft; // move m to the Left?
	while(l!=r) {		
//		cerr << " l=" << (*l)->index();
		// find thing to move
		double w = 0;
		i = l;
		while(i != r) {
			double tmp = orderingWeight(e,*i,l,r);
			double atmp = fabs(tmp);
			if(atmp >= w) {
				m = i;
				w = atmp;
				toLeft = (tmp<=0);
			}
			++i;		
		}		
//		cerr << " m=" << (*m)->index();
//		cerr << " toLeft=" << toLeft;
		//move
		if(toLeft) {
			if(m == l) {
				++l;
			}
			else {
				L.moveToPrec(m,l);
			}
		} else {
			if(m.succ() != r) {
				if(r.valid()) {
					L.moveToPrec(m,r);
					r = m;
				}
				else {					
					L.moveToBack(m);
					r = L.rbegin();
				}
			} else
				r = m;
		}		
	} 
}

double OrderedOptimalCrossingMinimizer::Subproblem::realizeSolutions() {
	double sum = 0;
	EdgeArray< List<edge> > ecrossRnd(*(master()->givenGraph));
	EdgeArray< List<edge> > ecrossInt(*(master()->givenGraph));
	int crRnd = 0;
	double crRndC = 0;
	int crInt = 0;
	double crIntC = 0;
	bool different = false;
	edge e;
	
	OGDF_ASSERT( I_USEFORSEPARATE == I_RND)

	inducedSeparationSimpleCrossings.clear();
	
	// filter crossings per edge
	int lastI = -1;
	for(int i = 0; i < nVar(); i++) {
		if(lastI==i) continue;
		if(SimpleCrossingVariable* s = dynamic_cast<SimpleCrossingVariable*>(variable(i))) {
			double x = xVal(i);
			sum += s->obj() * x;
			if(x > master()->roundUp()) {
//				lout() << "ecrossRnd " << s->e[0]->index() << "+" << s->e[1]->index() << "\n";
				forall_listiterators(edge, jkl1, ecrossRnd[s->e[0]]) {
					OGDF_ASSERT( master()->activeVars->exists(s->e[0], *jkl1, s->e[1]) );
					OGDF_ASSERT( master()->activeVars->exists(s->e[0], s->e[1], *jkl1) );
				}
				forall_listiterators(edge, jkl2, ecrossRnd[s->e[1]]) {
					OGDF_ASSERT( master()->activeVars->exists(s->e[1], *jkl2, s->e[0]) );
					OGDF_ASSERT( master()->activeVars->exists(s->e[1], s->e[0], *jkl2) );
				}
				ecrossRnd[s->e[0]].pushBack(s->e[1]);
				ecrossRnd[s->e[1]].pushBack(s->e[0]);
				inducedSeparationSimpleCrossings.pushBack(s);
				++crRnd;
				crRndC += s->obj();
				if(x > 1-EPS) {
					ecrossInt[s->e[0]].pushBack(s->e[1]);
					ecrossInt[s->e[1]].pushBack(s->e[0]);
					++crInt;
					crIntC += s->obj();
				} else different = true;
			}
//		} else {			
//			if(xVal(i) > (2*master()->roundUp()-1)/2 ) {
//				OGDF_ASSERT( i>0 );
//				OrderedCrossingVariable* o = dynamic_cast<OrderedCrossingVariable*>(variable(i));
//				OrderedCrossingVariable* t = dynamic_cast<OrderedCrossingVariable*>(variable(i-1));
//				if(!t || !o->isTwin(t)) {
//					t = dynamic_cast<OrderedCrossingVariable*>(variable(i+1));
//					lastI = i+1;
//				}
//				OGDF_ASSERT( t && o->isTwin(t) );
//				inducedSeparationOrderedCrossings.pushBack(o);
//				inducedSeparationOrderedCrossings.pushBack(t);				
//			}
		}
	}
	
	lout() << " cr[RND]=" << crRnd << ", " << crRndC << "; cr[INT]=" << crInt << ", " << crIntC << "\n";
	//lout() << "\t(Dual Bound: " << sum << ")\n";

	// put the crossings-per-edge in linear order and load them into the 
	bool exchgRnd = false;
	bool exchgInt = false;
	forall_edges(e, *(master()->givenGraph)) {
		orderCrossings(e, ecrossRnd[e]);
		exchgRnd |= master()->inducedCrossingConfiguration[I_RND]->probablyExchangeCrossingEdges(e, ecrossRnd[e]);
		if(different) {
			orderCrossings(e, ecrossInt[e]);
			exchgInt |= master()->inducedCrossingConfiguration[I_INT]->probablyExchangeCrossingEdges(e, ecrossInt[e]);
		}
	}

	// generate planarizations
	if(exchgRnd || !inducedPlanarization[I_RND]) {
		if(inducedPlanarization[I_RND]) delete inducedPlanarization[I_RND];
		(inducedPlanarization[I_RND] = new PlanRep(*(master()->givenGraph)))->initCC(0);
		master()->inducedCrossingConfiguration[I_RND]->paste(*(inducedPlanarization[I_RND]));
		primalHeuristics(I_RND);
	}
	if(exchgInt || !inducedPlanarization[I_INT]) {
		if(inducedPlanarization[I_INT]) delete inducedPlanarization[I_INT];
		(inducedPlanarization[I_INT] = new PlanRep(*(master()->givenGraph)))->initCC(0);
		master()->inducedCrossingConfiguration[I_INT]->paste(*(inducedPlanarization[I_INT]));
		primalHeuristics(I_INT);
	}

	//lout() << "\t=> ObjValue: " << sum << "\n";
	return sum;
}

void OrderedOptimalCrossingMinimizer::Subproblem::createVarsAndCorrespondingLOCs(edge e, edge f, edge g, 
		ArrayBuffer<OrderedCrossingVariable*>& vars) {
	OrderedCrossingVariable *v1,*v2;
	vars.push( v1 = new OrderedCrossingVariable(master(), e, f, g) );
	vars.push( v2 = new OrderedCrossingVariable(master(), e, g, f) );
	const OrderedCrossingVariable *vp = AbacusConstraint::preferredVariable(v1, v2);
//	cerr << "\t\t\tAdding variables: " << v1->base()->index() << "/" << v1->crossedBy()->index() << "/" << v1->before()->index() << "; " 
//		<< v2->base()->index() << "/" << v2->crossedBy()->index() << "/" << v2->before()->index() << "; " << (vp==v1?"[1]":"[2]") << "\n";
	bufferedLinearOrders.pushFront( new OmegaExistenceConstraint(master(), v1) );
	bufferedLinearOrders.pushFront( new ReverseOmegaExistenceConstraint(master(), v1) );
	bufferedLinearOrders.pushFront( new OmegaExistenceConstraint(master(), v2) );
	bufferedLinearOrders.pushFront( new ReverseOmegaExistenceConstraint(master(), v2) );
	bufferedLinearOrders.pushFront( new StackedOrderConstraint(master(), vp) );
	bufferedLinearOrders.pushFront( new MirrorOrderConstraint(master(), vp) );
	master()->m_usedVariables += 2;
}

int OrderedOptimalCrossingMinimizer::Subproblem::checkAmbiguitiesAndPrice() {
	// guaranteed: all Linear-Order constraints are satisfied...
//	lout() << "Checking Ambiguities: ";
	
	OGDF_ASSERT( bufferedLinearOrders.empty() );
	ArrayBuffer<OrderedCrossingVariable*> vars(master()->maxNewVars());
	
	EdgeArray<double> max(*(master()->givenGraph), 0.5-EPS);
	EdgeArray<edge> by(*(master()->givenGraph), 0);
	edge e, f, g;
	forall_edges(e, *(master()->givenGraph)) {
		forall_edges(f, *(master()->givenGraph)) {
			if(e==f || !master()->variableAllowed(e,f)) continue;
			int i = master()->activeVars->index(e,f);
			OGDF_ASSERT( i >= 0 );
			OGDF_ASSERT( master()->activeVars->index(e,f)==master()->activeVars->index(f,e) );
			double x = xVal(i);
//			lout() << "\t\t\t" << e->index() << "x" << f->index() << " [" << i << "]: " << x << "\n";
			if(max[e] < x) { max[e] = x; by[e] = f; }
		}
//		if(by[e])
//			lout() << "\t\t" << e->index() << "x" << by[e]->index() << " [" << master()->activeVars->index(e,by[e]) << "]: max " << max[e] << "\n";
	}
	forall_edges(e, *(master()->givenGraph)) {
		if(!by[e]) continue;
		forall_edges(f, *(master()->givenGraph)) {
//			if(EPS < xVal(master()->activeVars->index(e,f)))
//				lout() << "\t\t" << e->index() << "x" << f->index() << " [" << master()->activeVars->index(e,f) << "]: " << xVal(master()->activeVars->index(e,f)) << "\n";
			if(e==f || by[e]==f || !master()->variableAllowed(e,f) || (master()->activeVars->isOrdered(e,f)&&master()->activeVars->isOrdered(e,by[e]))) continue;
			int i = master()->activeVars->index(e,f);
			OGDF_ASSERT( i >= 0 );
			double x = xVal(i);
			if(max[e] + x > 1 + EPS) {
//				lout() << "\tAmbiguity found: Base=" << e->index() << "; CrossedBy=" << f->index() << " (MaxBy=" << by[e]->index() << "): " << max[e] + x << "\n";

				if( !master()->activeVars->isOrdered(e,by[e]) ) { // have to add&register by[e] first
					if( master()->activeVars->numOrdered(e) ) {
						forall_edges(g, *(master()->givenGraph)) {
							if( g==e || g==by[e] || !master()->variableAllowed(e,g) || !master()->activeVars->isOrdered(e,g) ) continue;
							createVarsAndCorrespondingLOCs(e,by[e],g, vars);
						}
					}
					master()->activeVars->registerNewOrdered(e,by[e]);
				}					
				if( !master()->activeVars->isOrdered(e,f) ) {
					forall_edges(g, *(master()->givenGraph)) {
						if( g==e || g==f || !master()->variableAllowed(e,g) || !master()->activeVars->isOrdered(e,g) ) continue;
						createVarsAndCorrespondingLOCs(e,f,g, vars);
					}
					master()->activeVars->registerNewOrdered(e,f);
				}
			}
		}		
	}
	// done generating vars
	
	int nV = vars.size();
//	lout() << "# new vars: " << nV << "\n";
	int ret = 0;
	if(nV) {
		ABA_BUFFER<ABA_VARIABLE*> bufvars(master(), nV);
		ABA_BUFFER<bool> putInPool(master(), nV);
		for(int i = vars.size(); i-->0;) {
			bufvars.push(vars[i]);
			putInPool.push(true);
		}
		ret = addVars(bufvars, 0, &putInPool, 0);
	}
	OGDF_ASSERT(nV == ret);

	OGDF_ASSERT( !ret || bufferedLinearOrders.size() );
//	if(!ret) {
//		lout() << "Passed\n";
//	}
//	else {
//		lout() << "\nAmbiguity Check Failed -> +Vars=" << ret << "(" << nVar() << "); +bufLO=" << bufferedLinearOrders.size() << "\n";
//	}
	return ret;
}

int OrderedOptimalCrossingMinimizer::Subproblem::solveLp() {
	
	if(master()->totalTime()->exceeds(master()->maxCpuTime())) { // get me outa here!
		generatedConVar = -1;
		return 0;
	}
	
	OGDF_ASSERT(nIter_ > 0);
	
	int s = bufferedLinearOrders.size();
	if(s) {
		generatedConVar = 0;
		ABA_BUFFER<ABA_CONSTRAINT*> simpl(master(), s);
		ABA_BUFFER<bool> put(master(), s);
		for(SListConstIterator<AbacusConstraint*> it = bufferedLinearOrders.begin(); it.valid(); ++it) {
			simpl.push(*it);
			put.push(true);
		}
		bufferedLinearOrders.clear();
		lout() << "[Buffered Constraints] Introduced " << s << " buffered linear order constraints for the new vars.\n";
		generatedConVar -= addCons(simpl, master()->linearOrderPool, &put);
		OGDF_ASSERT( -generatedConVar == s );
		return 0; //rerun...
	}

	lout() << "\n===== Subproblem " << id() << " (Iteration " << ++realIter << ") ===== \n";
	
	lout() << "Solving LP: " << nVar() << "x" << nCon();
	int error = ABA_SUB::solveLp();
	switch(error) {
		case 0: lout() << " => " << lp()->value() << "\n"; break; // all right, sankt feit, sag'n d'leut
		case 1: lout() << " => LP is infeasible.\n"; return 1;
		case 2: lout() << " => LP is infeasible. Abacus wants to price without non-liftables. I reject.\n"; return 2;
		default:
			lout() << " => Unknown error happend in Abacus::solveLp (" << error << ")\n";
			return error;
	}
//	lout() << "ObjValue: " << dualBound() << "\n";
	
	if( lp()->value() > master()->primalBound()-1 + EPS ) { // hopeless branch node
		lout() << "[Prune] LP value higher than primalBound-1.\n";
		return 1;
	}

	master()->activeVars->loadIndices(this);

	if(generatedConVar = -constraintPoolSeparation(0, master()->linearOrderPool)) {
		lout() << "[Pool] LinearOrder (+" << -generatedConVar << ")\n";
		return 0;
	}
	
//	if(generatedConVar = -constraintPoolSeparation(0, master()->trianglePool)) {
//		lout() << "[Pool] Triangle (+" << -generatedConVar << ")\n";
//		return 0;
//	}

	if(generatedConVar = -constraintPoolSeparation(0, master()->hintedPool)) {
		lout() << "[Pool] Hinted (+" << -generatedConVar << ")\n";
		return 0; // resolve
	}
	
	if(master()->hintEffects() & HE_HighKuratowskiCutsStatic) {
		int ret;
		if( (master()->graphHint() == GH_Complete && (ret=checkKnHighKuratowskiCutsStatic())) ||
			(master()->graphHint() == GH_CompleteBipartite && (ret=checkKnmHighKuratowskiCutsStatic()))) {
			lout() << "[Separation] Generated " << ret << " static HighKuratowskiCuts (->" << nCon() << ")\n";
			generatedConVar = -ret;
			return 0; // resolve
		}
	}
	
	if(generatedConVar = -constraintPoolSeparation(0, master()->kuratowskiPool)) {
		lout() << "[Pool] Kuratowski (+" << -generatedConVar << ")\n";
		return 0; // resolve
	}
	
	if(generatedConVar = -separateLinearOrder()) {
		lout() << "[Separation] LinearOrder (+" << -generatedConVar << ")\n";
		return 0; // resolve
	}

	if( (master()->pricingInit() != PI_NoPricing) && (generatedConVar = checkAmbiguitiesAndPrice()) ) {
		lout() << "[Check&Price] Ambiguities detected -> new Variables (" << generatedConVar << "->" << (nVar()+generatedConVar) << ")\n";
		return 0;
	}

	generatedConVar = 0;	
	lout() << "[Check&Price] No Ambiguities, realizing relaxation: ";
	//clearInduced();
	double objval = realizeSolutions();
	
//	if(generatedConVar = -separateTriangles()) {
//		lout() << "[Separation] Triangle (+" << -generatedConVar << ")\n";
//		return 0; // resolve
//	}

	if((master()->m_separationMode & SM_Simple) && (generatedConVar = -separateSimple())) {
		lout() << "[Separation] SimpleKuratowski (+" << -generatedConVar << ")\n";
		return 0;
	}
	
	if((master()->m_separationMode & SM_BoyerMyrvold) && (generatedConVar = -separateBoyerMyrvold())) {
		lout() << "[Separation] MultiKuratowski (+" << -generatedConVar << ")\n";
		return 0;
	}

	return 0;
}


Module::ReturnType OrderedOptimalCrossingMinimizer::Master::doCall(PlanRep &_PG,
		int cc,
		const EdgeArray<int>  &_cost,
		const EdgeArray<bool> &_forbid,
		int& crossingNumber) {
	
	if(_PG.numberOfCCs() > 1) {
		OGDF_THROW_PARAM(PreconditionViolatedException, pvcBiconnected);
	}
	
	if(effectiveLogLevel() >= LL_ALARM || effectiveStatisticMode())
		outLevel(OrderedOptimalCrossingMinimizer::Master::Silent);
	else
		outLevel(OrderedOptimalCrossingMinimizer::Master::Full);
		
	lout() << "---------------------------------------------"
	       << "\nnumStartHeuristics = " << m_numStartHeuristics
	       << "\nbranchingMode = " << ((m_branchingMode == BM_Traditional) ? "Traditional" : "CompleteOdd") 
	       << "\npricingInit = " << ((m_pricingInit == PI_NoPricing) ? "NoPricing" : "Normal")
	       << "\nmaxNewVars = " << m_maxNewVars
	       << "\nseparationMode = {" << ((m_separationMode & SM_Simple) ? " Simple" : "")
	                                 << ((m_separationMode & SM_BoyerMyrvold) ? " BoyerMyrvold" : "") << " }"
	       << "\nSimpleSeparation = { " << m_simpleSeparationParams << " }"
	       << "\nBoyerMyrvoldSeparation = { " << m_boyerMyrvoldSeparationParams << " }"
	       << "\nmaxLinearOrderCuts = " << m_maxLinearOrderCuts
//	       << "\nmaxTriangleCuts = " << m_maxTriangleCuts
	       << "\nnumCutHighKuratowskis = " << m_numCutHighKuratowskis
	       << "\nnumBaseEdgesForCutHighKuratowskis = " << m_numBaseEdgesForCutHighKuratowskis
	       << "\nmaxMinutes = " << m_maxMinutes
	       << "\nroundUp = " << m_roundUp
	       << "\ntailOffNLp = " << tailOffNLp()
	       << "\ntailOffPercent = " << tailOffPercent()
	       << "\ngraphHint = " << ( ( m_graphHint == GH_Complete ) ? "Complete Graph" :
	                               ( ( m_graphHint == GH_CompleteBipartite ) ? "Complete Bipartite Graph" : "None" ) )                                          
	       << "\nhintEffects = {" << (( m_hintEffects & HE_KuratowskisMinusOne ) ? "KuratowskisMinusOne " : "")
	                              << (( m_hintEffects & HE_AllSubKuratowskis ) ? "AllSubKuratowskis " : "")
	                              << (( m_hintEffects & HE_EdgeOrder ) ? "EdgeOrder " : "")
	                              << (( m_hintEffects & HE_NodeOrder ) ? "NodeOrder/KuratowskiOrder " : "")
	                              << (( m_hintEffects & HE_ExpensiveKuratowski ) ? "ExpensiveKuratowski " : "")	                              
	                              << (( m_hintEffects & HE_IterativeLowerBound ) ? "IterativeLowerBound " : "")	                              
	                              << (( m_hintEffects & HE_HighKuratowskiCutsStatic ) ? "HighKuratowskiCutsStatic " : "")	                              
	                              << "}"
	       << "\nreduceMemory = " << m_reduceMemory
	       << "\n---------------------------------------------\n";

	cost = &_cost;
	forbid = &_forbid;
	givenGraph = &(_PG.original());
	resultingGraph = &_PG;
	resultingGraph->initCC(0);
//	initNunchakus(); // for triangle separation
	
//#ifdef OGDF_DEBUG
//	cerr << "******************\nGiven Graph:\n";
//	int abc=0;
//	edge efg;
//	forall_edges(efg,*givenGraph) {
//		cerr << "\t " << efg->index() << "-"<< efg;
//		if(!(++abc%5)) cerr << "\n";		
//	}
//	cerr << "\n***********************\n";
//#endif
	
	for(int i = INDUCEMENTS; i-->0;)
		inducedCrossingConfiguration[i] = new CrossingConfiguration(*givenGraph);

	
	bestSolution = initBounds();
	if(writeIntermediateResultsToo()) doWriteBestSolution();
	
	m_minVariables = 0;
	m_maxVariables = 0;
	m_startVariables = 0;
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
		
		lout() << "Start #Variables: " << m_startVariables << "; Min #Variables: " << m_minVariables << "; Max #Variables: " << m_maxVariables << "; Generated Variables: " << m_usedVariables << "(" << (m_usedVariables-m_startVariables) << "/" << (100*m_usedVariables/(double)m_maxVariables) << "%)\n";
		
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
		if(graphHint() == GH_Complete && givenGraph->numberOfNodes()%2)
			lout() << "Guy's Primal Bound (" << primalBound()+1 << ") was reduced because of parity argument.\n";
		lout() << "Abacus Primal Bound: " << primalBound() << "\n";
		lout() << "Abacus Dual Bound: " << dualBound() << "\n";
	} else {
		m_isTrivial = true;
	}
	
	crossingNumber = -1;
	if(bestSolution) {
		lout() << "best solution obj value: " << bestSolution->getCrossingNo() << "\n";
		crossingNumber = bestSolution->getCrossingNo();
	} 
	
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
		case SS_Kn:
			lout() << "The ILP found no solution better Guy's conjecture.\n";			
			crossingNumber = upperbound+1;
			if(graphHint() == GH_Complete && givenGraph->numberOfNodes()%2) ++crossingNumber;
			break;
		case SS_Knm:
			lout() << "The ILP found no solution better Zarankiewicz's conjecture.\n";
			crossingNumber = upperbound+1;
			break;
		default:
			lout() << "Unknown upper bound source.\n";
	}
	
	if(!m_isTrivial) {
		lout() << "Time req'd: " << totalTime()->seconds() << "sec = " 
		       << totalTime()->minutes() << "min " << (totalTime()->seconds()%60) << "sec\n";
		lout() << "# Variables: " << m_usedVariables << "/" << m_maxVariables 
		       << " = " << (100*m_usedVariables/(double)m_maxVariables) << "%\n";
		lout() << "# LinearOrder-Constraints: " << linearOrderPool->number() << "\n";
//		lout() << "# Triangle-Constraints: " << trianglePool->number() << "\n";
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
	       << "\t" << (m_isTrivial ? 0 : m_startVariables)
	       << "\t" << (m_isTrivial ? 0 : m_usedVariables)
	       << "\t" << ( (m_isTrivial||(m_maxVariables==0)) ? 0 : (100*m_usedVariables/(double)m_maxVariables))
	       << "\t" << (m_isTrivial ? 0 : linearOrderPool->number())
//	       << "\t" << (m_isTrivial ? 0 : trianglePool->number())
	       << "\t" << (m_isTrivial ? 0 : kuratowskiPool->number());	
	       
	if(isOptimal() && (effectiveLogLevel()<=LL_DEFAULT || writeResult()) ) {
		doWriteBestSolution();
	}
	if(bestSolution) { // Backtransformation!
		bestSolution->paste(*resultingGraph);
	} else {
		cerr << "Error since there is no bestSolution stored...\n";
		clearAfterCall();
		return retError;
	}

	// clear some stuff...
	clearAfterCall();	

	return isOptimal() ? retOptimal : (status()==OutOfMemory || status()==Error) ? retError : retFeasible;
}

void OrderedOptimalCrossingMinimizer::Master::doWriteBestSolution() {
	if(!bestSolution) return;
	lout() << "=================================\n"
		<< "Crossing Configuration:\n"
		<< "---------------------------------\n";
	edge e;
	forall_edges(e, *givenGraph) {
		const List<edge>& L = bestSolution->getCrossingEdges(e);
		if(!L.empty()) {
			lout() << e << " [" << (*cost)[e] << "]: " << L << "\n";
		}		
	}
	lout() << "=================================\n";
	if(writeResult()) {
		PlanRep P(*givenGraph);
		P.initCC(0);
		bestSolution->paste(P);
		GraphAttributes PA(P, GraphAttributes::nodeGraphics | GraphAttributes::nodeColor);
		node n;
		forall_nodes(n, P) {
			if(!P.original(n)) {
				PA.colorNode(n) = "red";
			}
		}
		PA.writeGML(writeResult());
	}
}

bool OrderedOptimalCrossingMinimizer::Master::variableAllowed(edge e1, edge e2) {
	// thou shalt not cross
	if( (*forbid)[e1] || (*forbid)[e2] )
		return false;
		
	// adjacent edges do not cross
	if( e1->commonNode(e2) ) return false;
	
	// oh well, go ahead and pair...
	return true;
}

void OrderedOptimalCrossingMinimizer::Master::hintsKnKuratowskiMinusOne(List<ABA_CONSTRAINT*>& prelist) {
//	node n;
//	edge e;
//	
//	int ns = givenGraph->numberOfNodes() - 1;
//
//	KuratowskiConstraintBase::KuratowskiType kt = KuratowskiConstraintBase::KTComplete(ns);
//	forall_nodes(n, *givenGraph) {
//		KuratowskiConstraint* k = new KuratowskiConstraint(this, kt, false); // non-dynamic
//		int path = 0;
//		forall_edges(e, givenGraph) {
//			if(e->source() != n && e->target() != n) {
//				k->addEdge(e, path++);
//			}
//		}
//		prelist.pushBack(k);
//	}
//	lout() << "Complete-Graph-hint, KuratowskiMinusOne-effect:\n"
//		<< "\tadding " << ns+1 << " K_" << ns << " constraints, requiring " << "\"some\""/*cr*/ << " crossings each.\n";

	lout(LL_ALARM) << "\n\n*** hintsKnKuratowskiMinusOne NOT IMPLEMENTED ***\n\n";
	OGDF_ASSERT( false );
}

void OrderedOptimalCrossingMinimizer::Master::helperHintsKnAllSubKuratowskis(NodeArray<bool>& aktnodes, node posNode, int num, List<ABA_CONSTRAINT*>& prelist) {
//	node n; 
//	edge e;
//	if(posNode) {
//		helperHintsKnAllSubKuratowskis(aktnodes, posNode->succ(), num, prelist );
//		if( num>5 ) {
//			aktnodes[posNode] = false;
//			helperHintsKnAllSubKuratowskis(aktnodes, posNode->succ(), num-1, prelist );
//			aktnodes[posNode] = true;
//		}
//	} else if( num < givenGraph.numberOfNodes() ) {
//		KuratowskiConstraint* k = new KuratowskiConstraint(this, KuratowskiConstraint::KTComplete(num), false); // non-dynamic
//		int path = 0;
//		forall_edges(e, givenGraph) {
//			if(aktnodes[e->source()] && aktnodes[e->target()])
//				k->addEdge(e, path++);
//		}
////		cout << "Pfade: " << path << "\n";
//		prelist.pushBack(k);
//	}
	lout(LL_ALARM) << "\n\n*** helperHintsKnAllSubKuratowskis NOT IMPLEMENTED ***\n\n";
	OGDF_ASSERT( false );
}

void OrderedOptimalCrossingMinimizer::Master::hintsKnAllSubKuratowskis(List<ABA_CONSTRAINT*>& prelist) {
	NodeArray<bool> aktnodes(*givenGraph, true);
	helperHintsKnAllSubKuratowskis(aktnodes, givenGraph->firstNode(), givenGraph->numberOfNodes(), prelist);
				
	lout() << "Complete-Graph-hint, AllSubKuratowskis-effect:\n"
		<< "\tadding " << prelist.size() << " Kuratowski (subgraph) constraints.\n";
}

void OrderedOptimalCrossingMinimizer::Master::hintsKnNodeOrder(List<ABA_CONSTRAINT*>& prelist) {
	node n;

	node lastNode = 0;
	forall_nodes(n, *givenGraph) {
		if(lastNode)
			prelist.pushBack(new NodeOrderConstraint(this, lastNode, n));
		lastNode = n;
	}
	lout() << "Complete-Graph-hint, NodeOrder-effect:\n\tadding " << givenGraph->numberOfNodes()-1 << " NodeOrder constraints.\n";
}

void OrderedOptimalCrossingMinimizer::Master::hintsKnEdgeOrder(List<ABA_CONSTRAINT*>& prelist) {
	edge e;

	node baseNode = givenGraph->firstNode();
	edge lastEdge = 0;
	forall_adj_edges(e, baseNode) {
		if(lastEdge)
			prelist.pushBack(new EdgeOrderConstraint(this, lastEdge, e));
		lastEdge = e;
	}
	lout() << "Complete-Graph-hint, EdgeOrder-effect:\n\tadding " << givenGraph->numberOfNodes()-2 << " EdgeOrder constraints.\n";
}

void OrderedOptimalCrossingMinimizer::Master::hintsKnmKuratowskiMinusOne(List<ABA_CONSTRAINT*>& prelist) {
//	node n;
//	edge e;
//
//	int ns1 = givenGraph->firstNode()->degree();
//	int ns2 = givenGraph->firstNode()->firstAdj()->twin()->theNode()->degree();
//	
//	KuratowskiConstraint::KuratowskiType kt1 = KuratowskiConstraint::KTBipartite(ns2,ns1-1);
//	KuratowskiConstraint::KuratowskiType kt2 = KuratowskiConstraint::KTBipartite(ns2-1,ns1);
//	
//	forall_nodes(n, givenGraph) {
//		KuratowskiConstraint* k = new KuratowskiConstraint(this, n->degree()==ns1 ? kt1 : kt2, false); // non-dynamic
//		int path = 0;
//		forall_edges(e, givenGraph) {
//			if(e->source() != n && e->target() != n) {
//				k->addEdge(e, path++);
//			}
//		}				
//		prelist.pushBack(k);
//	}			
//	lout() << "Complete-Bipartite-Graph-hint, KuratowskiMinusOne-effect: "
//	    << "\tadding " << ns1 << " K_{" << ns1-1 << "," << ns2 << "} constraints requiring " << "\"some\""/*cr*/ << " crossings each.\n";
//	lout() << "\tadding " << ns2 << " K_{" << ns1 << "," << ns2-1 << "} constraints requiring " << "\"some\""/*cr*/ << " crossings each.\n";
	lout(LL_ALARM) << "\n\n*** hintsKnmKuratowskiMinusOne NOT IMPLEMENTED ***\n\n";
	OGDF_ASSERT( false );
}

void OrderedOptimalCrossingMinimizer::Master::hintsKnExpensiveKuratowski(List<ABA_CONSTRAINT*>& prelist) {
//	node n,nn;
//	edge e;
//	
//	int ns = givenGraph.numberOfNodes() - 2;
//	KuratowskiConstraint::KuratowskiType kt = KuratowskiConstraint::KTComplete(ns);
//	
//	KuratowskiConstraint::Restrictiveness r = (numMinNodes%2) ? KuratowskiConstraint::RGreaterPlus(2) : KuratowskiConstraint::RGreaterPlus(1);
//	
//	forall_nodes(n, givenGraph) {
//		forall_nodes(nn, givenGraph) {
//			if(nn->index() > n->index()) {
//				KuratowskiConstraint* k = new KuratowskiConstraint(this, kt, false, r); // non-dynamic			
//				int path = 0;
//				forall_edges(e, givenGraph) {
//					if(e->source() != n && e->target() != n && e->source() != nn && e->target() != nn)
//						k->addEdge(e, path++);
//				}
//				prelist.pushBack(k);
//			}
//		}
//	}			
//	lout() << "Complete-Graph-hint, KuratowskiExpensive-effect: adding expensive-K_" << ns << " constraints (Restrictiveness: " << r << ").\n";
	lout(LL_ALARM) << "\n\n*** hintsKnmKuratowskiMinusOne NOT IMPLEMENTED ***\n\n";
	OGDF_ASSERT( false );
}

void OrderedOptimalCrossingMinimizer::Master::hintsKnmAllSubKuratowskis(List<ABA_CONSTRAINT*>& prelist) {
	lout(LL_FORCE) << "WARNING: KnmAllSubKuratowskis not implemented! -> no such hints generated!!!";
	OGDF_ASSERT(false);
}

void OrderedOptimalCrossingMinimizer::Master::hintsKnmNodeOrder(List<ABA_CONSTRAINT*>& prelist) {
	node n;
	edge e;

	node lastNode[2] = { 0, 0};
	NodeArray<int> NA(*givenGraph, 0);
	forall_adj_edges(e, givenGraph->firstNode()) {
		NA[e->opposite(n)] = 1;
	}
	forall_nodes(n, *givenGraph) {
		if(lastNode[NA[n]])
			prelist.pushBack(new NodeOrderConstraint(this, lastNode[NA[n]], n));
		lastNode[NA[n]] = n;							
	}
	lout() << "Complete-Bipartite-Graph-hint, NodeOrder-effect:\n\tadding " << givenGraph->numberOfNodes()-2 << " NodeOrder constraints.\n";
}

void OrderedOptimalCrossingMinimizer::Master::hintsKnmEdgeOrder(List<ABA_CONSTRAINT*>& prelist) {
	node n;
	edge e;

	node baseNode = givenGraph->firstNode();
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
	lout() << "Complete-Bipartite-Graph-hint, EdgeOrder-effect:\n\tadding " << givenGraph->numberOfNodes()-2 << " EdgeOrder constraints.\n";
}

void OrderedOptimalCrossingMinimizer::Master::initializeOptimization() {
	
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
	maxConAdd(30000); // + (was: 1000) // TODO: REDUCE!!!
	maxConBuffered(30000); // + (was: 1000) 
	maxVarAdd(10000); // + (was: 1000)
	maxVarBuffered(10000); // + (was: 1000)
	maxIterations(-1);
	eliminateFixedSet(false);
	newRootReOptimize(false); // could try that for performance! // TO DO: try!
	conElimMode(Basic); // << warning? -> secure alternative: NoConElim
//	conElimMode(NoConElim); // << warning? -> secure alternative: NoConElim // TO DO!!! restore "Basic"
	conElimEps(EPS); // was: 0.001
	conElimAge(1);
//	varElimMode(ReducedCost); // << warning? -> secure alternative: NoVarElim
	varElimMode(NoVarElim); // << warning? -> secure alternative: NoVarElim // TO DO!!! restore "ReducedCost"
	varElimEps(EPS); // was: 0.001
	varElimAge(1);	
	// Abacus settings END
		
	List<ABA_CONSTRAINT*> hintedConstraints;
	
	const PricingInit& pm = m_pricingInit;
	ABA_BUFFER<ABA_CONSTRAINT*> linearOrderStarters(this, pm==PI_NoPricing ? givenGraph->numberOfEdges()*5 : 1);
	ABA_BUFFER<ABA_VARIABLE*> variables(this, pm==PI_NoPricing ? MAX_PAIRS(givenGraph->numberOfEdges())*5 : MAX_PAIRS(givenGraph->numberOfEdges()));
	
	// add one pair each...
	edge e1, e2;
	forall_edges(e1,*givenGraph)  {
		int anz = 0; 
		forall_edges(e2, *givenGraph) {
			if( variableAllowed(e1,e2) && SimpleCrossingVariable::preferedOrder(e1, e2)) {
				SimpleCrossingVariable* var = new SimpleCrossingVariable(this, e1, e2);
				variables.push(var);
				anz++;
			}
		}
		m_startVariables += anz;
		m_maxVariables += anz*(anz-1);				
	}
	m_usedVariables = m_minVariables = m_startVariables;
	
	if(m_pricingInit != PI_Normal) {
		lout(LL_ALARM) << "Only PricingInit==PI_NORMAL is supported!!";
		OGDF_ASSERT( false );
	}
	
	if(graphHint() != GH_None) {
		if( ( (hintEffects() & HE_NodeOrder) ? 1 : 0 ) +
		    ( (hintEffects() & HE_EdgeOrder) ? 1 : 0 ) 
		    > 1) {
			hintEffects(0);
			lout(LL_FORCE) << "Conflicting HintEffects Selected (1)! Deactivating Hinting...\n";
		}
		
		switch(graphHint()) {
			case GH_Complete: {
				if(hintEffects() & HE_KuratowskisMinusOne)
					hintsKnKuratowskiMinusOne(hintedConstraints);				
				if(hintEffects() & HE_AllSubKuratowskis)
					hintsKnAllSubKuratowskis(hintedConstraints);				
				if(hintEffects() & HE_EdgeOrder)
					hintsKnEdgeOrder(hintedConstraints);				
				if(hintEffects() & HE_NodeOrder)
					hintsKnNodeOrder(hintedConstraints);
				if(hintEffects() & HE_ExpensiveKuratowski)
					hintsKnExpensiveKuratowski(hintedConstraints);
			}
			break;
			case GH_CompleteBipartite: {
				if(hintEffects() & HE_KuratowskisMinusOne)
					hintsKnmKuratowskiMinusOne(hintedConstraints);				
				if(hintEffects() & HE_AllSubKuratowskis)
					hintsKnmAllSubKuratowskis(hintedConstraints);				
				if(hintEffects() & HE_EdgeOrder)
					hintsKnmEdgeOrder(hintedConstraints);				
				if(hintEffects() & HE_NodeOrder)
					hintsKnmNodeOrder(hintedConstraints);
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
	
	lout() << "Using " << hintedConstraints.size() << " hinted constraints, " << 
		( ( graphHint() == GH_Complete ) ? "based on a Complete-Graph-hint.\n" :
		( ( graphHint() == GH_CompleteBipartite ) ? "based on a Complete-Bipartite-Graph-hint.\n" : 
		"since there was no hint.\n" ) );
	
	ABA_BUFFER<ABA_CONSTRAINT*> empty(this, 0);
	initializePools(empty, /*hintedConstraints,*/ variables,
		m_reduceMemory ? 2*MAX_PAIRS(givenGraph->numberOfEdges()) : 100*MAX_PAIRS(givenGraph->numberOfEdges()), // worst case: all variables
		0, /*hintedConstraints.number(),*/
		false); // and it's not dynamic 

	if(m_reduceMemory)
		linearOrderPool = new ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, 2*givenGraph->numberOfEdges(), true); // dynamic
	else
		linearOrderPool = new ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, givenGraph->numberOfEdges()*givenGraph->numberOfEdges()/2, false); // not dynamic
	trianglePool = new ABA_NONDUPLPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, m_reduceMemory ? 500 : 1000, true); // dynamic
	hintedPool = new ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, hintedConstraints.size(), (hintEffects() & HE_HighKuratowskiCutsStatic)); // dynamic, if i add static kuratowskis to it...
	forall_listiterators(ABA_CONSTRAINT*, it, hintedConstraints)
		hintedPool->insert(*it);
	kuratowskiPool = new ABA_NONDUPLPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, m_reduceMemory ? 500 : 1000, true); // dynamic
	branchingPool = new ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, 50, true); //dynamic // VERY rough estimate...

	for(int i = linearOrderStarters.number(); i-->0;)
		if(linearOrderStarters[i])
			linearOrderPool->insert(linearOrderStarters[i]);
			
}

OrderedOptimalCrossingMinimizer::CrossingConfiguration* OrderedOptimalCrossingMinimizer::Master::createHeuristicStartSolution() {
	CrossingConfiguration* cInfo = NULL;
	lout() << "Generating " << m_numStartHeuristics << " heuristic solution(s):";
	int h;
	for(h = m_numStartHeuristics; h-->0;) {
		PlanRep hpr(*givenGraph);
		hpr.initCC(0);
		int crno;
		m_startHeuristic.get().call(hpr, 0, crno, useCost() ? cost : 0, useForbid() ? forbid : 0, 0); //Achtung: Schnittstelle anpassen!!
		lout() << " " << crno << flush;
		if(cInfo && cInfo->getCrossingNo() > crno) {
			delete cInfo;
			cInfo = NULL;
		}
		if(!cInfo) cInfo = new CrossingConfiguration(hpr, crno, true);
	}
	lout() << "\n";
	return cInfo;
}

OrderedOptimalCrossingMinimizer::CrossingConfiguration* OrderedOptimalCrossingMinimizer::Master::initBounds() {

	lowerbound = 0;
	if(!useCost())
		upperbound = MAX_PAIRS(givenGraph->numberOfEdges()); // trivial
	else  {
		int sum = 0;
		edge e;
		forall_edges(e, *givenGraph) {
			sum += (*cost)[e];
		}
		upperbound = MAX_PAIRS(sum);
	}
	upperBoundSource = SS_Trivial; // heuristics will always top that, since THIS bound isn't reduced by one...

	lout(LL_MINOR) << "Running Heuristic Start Solutions...\n";
	// get a heuristic solution as a starting point & upper bound
	CrossingConfiguration* cInfo = createHeuristicStartSolution();
 
	calcLowerBounds(); // set ilp bounds..
	calcUpperBounds(); // formulae for special graphs (Kn, Knm)
	
//	if(upperbound == 0) {
//		lout() << "[Check situation?]\n";
//		crossingNumber = 1;
//		//T O D O backtransformation
//		return cInfo;
//	}
	
	if(cInfo)
		probablyUpdateUpperBound(cInfo->getCrossingNo(), SS_Heuristic); 
	
	lout() << "Input Graph: |V|=" << givenGraph->numberOfNodes() << ", |E|=" << givenGraph->numberOfEdges() << "\n";
	
	sout() << "\t" << givenGraph->numberOfNodes()
	       << "\t" << givenGraph->numberOfEdges()
	       << "\t" << lowerbound
	       << "\t" << (upperbound+1) << flush;

	return cInfo;
}

void OrderedOptimalCrossingMinimizer::Master::calcLowerBounds () {
	probablyUpdateLowerBound(1);
	
	int es = givenGraph->numberOfEdges();
	int ns = givenGraph->numberOfNodes();
	//Euler
	probablyUpdateLowerBound(es - 3*ns + 6);
	
	//Pach&Todt
	double ptb = es*es*es/(33.75*ns*ns)- 0.9*ns;
	probablyUpdateLowerBound((int)ceil(ptb));
	
	if(hintEffects() & HE_IterativeLowerBound) {
		if((graphHint() == GH_Complete)) {
			int n = givenGraph->numberOfNodes();
			for(int p = n-int(n/2); p<=n; p++) {
				if(n-p <= 6 && p <= 6) { // otherwise not proven yet // TODO: autoadapt?
					probablyUpdateLowerBound( (int)ceil( double(Math::binomial(n,p) * bipartiteZara(n-p,p)) / double(4 * Math::binomial(n-4, p-2))) );
				}
			}			
		} else if(graphHint() == GH_CompleteBipartite) {
			int n = givenGraph->firstNode()->degree();
			int m = givenGraph->firstNode()->firstAdj()->twin()->theNode()->degree();
			for(int q = n-n/2; q<=n; q++) {
				if(q <= 6 && m <= 6) { // otherwise not proven yet // TODO: autoadapt?
					probablyUpdateLowerBound( (int)ceil( double((Math::binomial(n,q) * bipartiteZara(m,q)) / double(Math::binomial(n-2, q-2))) ) );
				}
			}			
		}
	}
}

void OrderedOptimalCrossingMinimizer::Master::calcUpperBounds () {
	// K_n
	int Kn = completeGuy(givenGraph->numberOfNodes());
	if(graphHint() == GH_Complete && givenGraph->numberOfNodes()%2) {
		lout() << "Kn with n odd: Applying Parity Argument\n";
		--Kn;
	}
	probablyUpdateUpperBound(Kn, SS_Kn);

	if(graphHint() == GH_CompleteBipartite) {
		int n = givenGraph->firstNode()->degree();
		int m = givenGraph->firstNode()->firstAdj()->twin()->theNode()->degree();
		int Knm = bipartiteZara(n,m);
		probablyUpdateUpperBound(Knm, SS_Knm);
	}
}


int OrderedOptimalCrossingMinimizer::Master::enumerationStrategy(ABA_SUB* s1, ABA_SUB* s2) {
	// TODO: select the one with more fixed "1s" first !
	return ABA_MASTER::enumerationStrategy(s1, s2);
}

std::ostream &operator<<(std::ostream &os, const OrderedOptimalCrossingMinimizer::SimpleCrossingVariable& v) {
	os << "(e[0]=" << v.e[0]->index() << ";e[1]=" << v.e[1]->index() << ")";
	return os;
}
std::ostream &operator<<(std::ostream &os, const OrderedOptimalCrossingMinimizer::OrderedCrossingVariable& v) {
	os << "(base=" << v.base()->index() << ";crossedBy=" << v.crossedBy()->index() << ";before=" << v.before()->index() << ")";
	return os;
}


std::ostream &operator<<(std::ostream &os, /*const*/ OrderedOptimalCrossingMinimizer::BasicKuratowskiConstraint& k) {
	os << "(type=" << k.type << " edges=[";
	edge e;
	int i;
	for(i = k.newCrossings.size(); i-->0;) {
		for(int j = k.newCrossings[i].size(); j-->0;) {
			os << k.newCrossings[i][j]->index();
			if(j>0) os << ",";
		}
		if(i>0) os << "|";
	}
	os << "], cr=[";
	for(i = k.oldCrossings.size(); i-->0;) {
		const OrderedOptimalCrossingMinimizer::SimpleCrossingVariable* s;
		if( s = dynamic_cast<const OrderedOptimalCrossingMinimizer::SimpleCrossingVariable*>(k.oldCrossings[i]) )
			os << *s;
		else
			os << *dynamic_cast<const OrderedOptimalCrossingMinimizer::OrderedCrossingVariable*>(k.oldCrossings[i]);
	}
	os << "] " << ((k.sense_.sense()==ABA_CSENSE::Greater) ? ">": "=") << k.rhs() << ")";
	return os;
}

std::ostream &operator<<(std::ostream &os, const OrderedOptimalCrossingMinimizer::SimpleSeparationParams& p) {
	os << "runs=" << p.runs() << " despRuns=" << p.desperateRuns() << " maxCuts=" << p.maxCuts();
	return os;
}
std::ostream &operator<<(std::ostream &os, const OrderedOptimalCrossingMinimizer::BoyerMyrvoldSeparationParams& p) {
	os << "runs=" << p.runs() << " despRuns=" << p.desperateRuns() << " extrct=" << p.extractions() 
		<< " runCuts=" << p.runCuts() << " maxCuts=" << p.maxCuts() << " bndl=" << p.bundle() 
		<< " nE2=" << p.noE2() << " vDif=" << p.veryDifferent();
	return os;
}

edge OrderedOptimalCrossingMinimizer::CrossingConfiguration::targetCrossingEdge(const PlanRep& PR, edge x) {
	edge e = PR.original(x);
	node n = x->target();	
	adjEntry a = n->firstAdj();	
	if(e != PR.original(a->theEdge())) return PR.original(a->theEdge());
	a = a->succ();
	if(e != PR.original(a->theEdge())) return PR.original(a->theEdge());
	a = a->succ();
	return PR.original(a->theEdge());
}

edge OrderedOptimalCrossingMinimizer::CrossingConfiguration::targetSuccEdge(const PlanRep& PR, edge x) {
	edge e = PR.original(x);
	node n = x->target();	
	adjEntry a = n->firstAdj();
	while(a) {
		edge y = a->theEdge();
		if(e == PR.original(y) && x!=y) return y;
		a = a->succ();
	}
	OGDF_ASSERT( false ); // should never be here
	return 0;
}

edge OrderedOptimalCrossingMinimizer::CrossingConfiguration::findCrossingPosition(const PlanRep& PR, edge onEdge, edge crEdge) {
	ListConstIterator<edge> here = PR.chain(onEdge).begin();
	ListConstIterator<edge> all = crossingEdges[onEdge].begin();
	OGDF_ASSERT(all.valid());
	while(here.succ().valid()) {
		edge next = targetCrossingEdge(PR, *here);
		while(*all != next) {
			if(*all == crEdge)
				return *here;
			++all;
			OGDF_ASSERT(all.valid());
		}
		++all;
		++here;
	}
	return *here;
}

void OrderedOptimalCrossingMinimizer::CrossingConfiguration::paste(PlanRep& PG) {
	OGDF_ASSERT( crossingEdges.graphOf() == &(PG.original()) );
	const Graph& orig = PG.original();
	PG.clear();
	PG.initCC(0);
	
//#ifdef OGDF_DEBUG
//	// consistency check of CrossConf
//	edge t;
//	forall_edges(t, orig) {
//		forall_listiterators(edge, it, crossingEdges[t]) {
//			forall_listiterators(edge, it2, crossingEdges[*it]) {
//				if(*it2 == t) goto ok;
//			}
//			OGDF_ASSERT( false );
//			ok:;
//		}
//	}
//#endif
	
	edge e,f; // in orig
	forall_edges(e, orig) {
		edge here = PG.chain(e).front();
		int len = PG.chain(e).size() - 1;
		edge last = len ? targetCrossingEdge(PG,here) : 0;
		
		forall_listiterators(edge, it, crossingEdges[e]) {			
			f = *it;
//			cerr << "*** e=" << e << " last=" << last << " len=" << len << " f=" << f << "***\n";
			if(f == last) {
				here = targetSuccEdge(PG, here);
				--len;
				last = len ? targetCrossingEdge(PG,here) : 0;
			} else { // insert crossing here
//#ifdef OGDF_DEBUG
//				// check consistency of f
//				cerr << "Chain:";
//				forall_listiterators(edge, t1, PG.chain(f)) {
//					if(t1.succ().valid())
//						cerr  << " " << targetCrossingEdge(PG, *t1);
//				}
//				cerr << "\nCrossEdges:";
//				forall_listiterators(edge, t2, crossingEdges[f]) {
//					cerr << " " << *t2;
//				}
//#endif				
				
				edge y = findCrossingPosition(PG, f, e);
//				cerr << "\n\tRealizing Crossing: " << e << "x" << f << " {bef:" << targetCrossingEdge(PG,y) << "}\n";
				PG.insertCrossing(here, y, true); // modifies "here" 
				OGDF_ASSERT( !len || last==targetCrossingEdge(PG,here) );				
			}
		}
	}
}

void OrderedOptimalCrossingMinimizer::CrossingConfiguration::extractDirect(const PlanRep& PG, int crNo) {
	const Graph& orig = PG.original();

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
			OGDF_ASSERT(e2 != e); // since PG should be embedded in the plane
			crossingEdges[e].pushBack(e2);			
		}
	}
}

void OrderedOptimalCrossingMinimizer::CrossingConfiguration::extractIndirect(const PlanRep& PG, int crNo) {
	const PlanRep& partial = (const PlanRep&) PG.original();
	const Graph& orig = (const Graph&) partial.original();
	
	crossingNo = crNo;
	crossingEdges.init(orig);
	
	edge e,e2;
	forall_edges(e, orig) {
		ListConstIterator<edge> prtit = partial.chain(e).begin();
		bool first = true;	
		for(; prtit.valid(); ++prtit) {
			ListConstIterator<edge> mit = PG.chain(*prtit).begin();
			mit++;
			for(; mit.valid(); mit++) {
				node dummy = (*mit)->source();
				e2 = partial.original(PG.original(dummy->firstAdj()->theEdge()));
				if(e2 == e)
					e2 = partial.original(PG.original(dummy->lastAdj()->theEdge()));
				OGDF_ASSERT(e2 != e);
				crossingEdges[e].pushBack(e2);			
			}				
		
			if(first)
				first = false;
			else {
				node dummy = (*prtit)->source();
				e2 = partial.original(dummy->firstAdj()->theEdge());
				if(e2 == e)
					e2 = partial.original(dummy->lastAdj()->theEdge());
				OGDF_ASSERT(e2 != e);
				crossingEdges[e].pushBack(e2);			
			}
		}
	}
}

} //namespace

#endif // USE_ABACUS
