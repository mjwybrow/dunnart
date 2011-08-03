/*
 * $Revision: 1.73 $
 * 
 * last checkin:
 *   $Author: chimani $ 
 *   $Date: 2008-02-20 20:33:07 +1100 (Wed, 20 Feb 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Implements class OptimalCrossingMinimizer
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
#include <ogdf/planarity/OptimalCrossingMinimizer.h>
#include <ogdf/planarity/SubgraphPlanarizer.h>
#include <ogdf/planarity/VariableEmbeddingInserter.h>
#include <ogdf/basic/math.h>
#include <ogdf/planarity/BoyerMyrvold.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <stdlib.h>

namespace ogdf {
	
    const double OptimalCrossingMinimizer::EPS =  0.001;
	const double OptimalCrossingMinimizer::SEG_EPS = 0.001;
	const OptimalCrossingMinimizer::KuratowskiConstraint::KuratowskiType OptimalCrossingMinimizer::KuratowskiConstraint::KT_K33 = 0;
	const OptimalCrossingMinimizer::KuratowskiConstraint::KuratowskiType OptimalCrossingMinimizer::KuratowskiConstraint::KT_K5 = -1;
	
void OptimalCrossingMinimizer::Master::setDefaultSettings() {
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
		
/*	{	FixedEmbeddingInserter* fei = new FixedEmbeddingInserter();
		fei->removeReinsert(FixedEmbeddingInserter::rrNone);
		m_solutionChecker.setInserter(fei);
		m_solutionChecker.permutations(1); }*/		

	m_pricingInit = PI_Normal;
	m_pricingMode = PM_Few;
	m_branchingMode = BM_Traditional;
	m_separationMode = SM_BoyerMyrvold;
	m_startSimpleSeparationParams.runs() = 100;
	m_startSimpleSeparationParams.maxCuts() = 50;
/*	m_kuratowskiExtractionDesperateRuns = 100;
	m_kuratowskiExtractionRuns = 20;
	m_kuratowskiExtractionExtractions = 100;
	m_kuratowskiExtractionRunLimit = 80;
	m_kuratowskiExtractionAllLimit = 1000;
	m_kuratowskiExtractionBundle = false;
	m_kuratowskiExtractionNoE2 = true;
	m_kuratowskiExtractionVeryDifferent = false;
	m_kuratowskiExtractionStartRuns = 20;
	m_kuratowskiExtractionStartExtractions = 100;
	m_kuratowskiExtractionStartRunLimit = 80;
	m_kuratowskiExtractionStartAllLimit = 1000;
	m_kuratowskiExtractionStartBundle = false;
	m_kuratowskiExtractionStartNoE2 = true;
	m_kuratowskiExtractionStartVeryDifferent = false;
	m_numStartKuratowskis = 100;
	m_numDesperateTryKuratowskis = 300;
	m_numTryKuratowskis = 50;
	m_numCutKuratowskis = 20; */
	
	m_numCutHighKuratowskis = 10;
	m_numBaseEdgesForCutHighKuratowskis = 5;
	m_maxMinutes = 0;
//	m_simpleDrawings = false;
	m_roundUp = 0.7;
	m_writeResult = NULL;
	m_writeIntermediateResultsToo = false;
	m_graphHint = GH_None;
	m_hintEffects = HE_KuratowskisMinusOne | HE_EdgeOrder | HE_IterativeLowerBound /*| HE_HighKuratowskiCutsStatic*/;
	tailOffPercent(0.001);
	m_duplicateKuratowskis = 0;
	m_reduceMemory = false;
}

void OptimalCrossingMinimizer::KuratowskiConstraint::addAccordingCrossing(
		const Subproblem* S, const PlanRep& I, edge e, int eid, List<CrossingLocation*>& L) {
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
	int cid = 0;
	ListConstIterator<edge> lci = exp.chain(exp.original(c)).begin();
	for(; lci.valid(); lci++, cid++) {
		if(*lci == c) break;
	}
	OGDF_ASSERT( *lci == c );
	
//	if(!CrossingVariable::preferedOrder(e,c))
//		return 0;
		
	const List<CrossingLocation*>& all = *(S->currentRoundedCrossings);
		
	CrossingLocation ci(Segment(exp.original(e), eid), Segment(exp.original(c), cid));
	for(ListConstIterator<CrossingLocation*> it = all.begin(); it.valid(); it++) {
		if(master()->crossingLocationComparer.equal(&ci, *it)) {
			if(L.search(*it, master()->crossingLocationComparer)<0)
				L.pushBack(*it);
			return;
		}
	}
	
	lout(LL_ALARM) << "Should NEVER be here...\n";
	lout(LL_ALARM) << "Looking for: " << ci << " [c=" << exp.original(c)->index() << ", csz=" << exp.chain(exp.original(c)).size() << "]\n";
	lout(LL_ALARM) << "In: ";
	for(ListConstIterator<CrossingLocation*> it = all.begin(); it.valid(); it++) {
		lout(LL_ALARM) << " " << **it;
	}
	lout(LL_ALARM) << "\n";
	
	OGDF_ASSERT( false );
}

void OptimalCrossingMinimizer::KuratowskiConstraint::build(const Subproblem* S, const GraphReduction& R, const KuratowskiSubdivision& K) {
	
	type = K.size()==9 ? KT_K33 : KT_K5;

	const PlanRep& I = (const PlanRep&) R.original();

	List<CrossingLocation*> DD;

	for(int path = 0; path<K.size(); path++) {
		ListConstIterator<edge> it = K[path].begin();
		for(; it.valid(); it++) {
			const List<edge>& il = R.original(*it); // il in I
			edge expfront = I.original(il.front()); // in expandedGraph
			edge expback = I.original(il.back()); // in expandedGraph
			
			const GraphCopy& exp = (const GraphCopy&) I.original();
			OGDF_ASSERT( exp.original(expfront) == exp.original(expback) );
			edge orig = exp.original(expfront);
	
			ListConstIterator<edge> lci = exp.chain(orig).begin();
			int startId = -1;
			int endId = +master()->expansionFactor+1;
	
			OGDF_ASSERT( il.front()->source() == R.original((*it)->source()) );
			OGDF_ASSERT( il.back()->target() == R.original((*it)->target()) );
	
			//calc start
			if(I.original(il.front()->source()) == 0) { // was crossing in I			
				startId = 0; // really?
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
				endId = max( startId, 0 );
				for(; lci.valid(); lci++, endId++) {
					if(*lci == expback) break;
				}
				OGDF_ASSERT( *lci == expback );
				OGDF_ASSERT( I.chain(expback).size() == 2 );
				addAccordingCrossing(S, I, expback, endId, DD);
			} else {
				OGDF_ASSERT( exp.original(I.original(il.back()->target())) );
			}
			
			edges[orig] = new SegmentRange(startId, endId, path);
		}
	}

	OGDF_ASSERT( rhs_ == 0 || rhs_ == 1 );
	rhs_ = rhs_ + 1 - DD.size(); // original rhs might be either 0 or 1, depending on the Restrictiveness, given in the constructor

	D.init(DD.size());
	int idx = 0;
	for(ListConstIterator<CrossingLocation*> dit = DD.begin(); dit.valid(); dit++, idx++) {
		D[idx] = *dit;
	}
	
	D.quicksort(master()->crossingLocationComparer);
}

bool OptimalCrossingMinimizer::Subproblem::feasible() {
	
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

int OptimalCrossingMinimizer::Subproblem::generateBranchRules(ABA_BUFFER<ABA_BRANCHRULE*> &rules) {
	if (master()->branchingMode() == BM_CompleteOdd) {
		lout() << "Analyzing LP-relaxation for possible CompleteOdd-Branchings...";
		
		ABA_ACTIVE<ABA_CONSTRAINT, ABA_VARIABLE> *aC = actCon(); // should i really only check the active constraints??
		KuratowskiConstraint* bestK = NULL;
		double bestImp = 1;
		int tight = 0;
		for(int i = aC->number(); i-->0;) {
			if(strcmp(((*aC)[i])->name(),"KuratowskiConstraint")) continue;
			KuratowskiConstraint* k = (KuratowskiConstraint*)((*aC)[i]);
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
			KuratowskiConstraint *k1 = NULL, *k2 = NULL;
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

int OptimalCrossingMinimizer::Subproblem::separate() {
	if(generatedConVar < 0) return -generatedConVar;
	else if(generatedConVar > 0) return 0;
	int sum = 0;

	if(!father() && nIter_==1) {
		lout() << "[Start] ";
		if(master()->m_separationMode & SM_Simple) sum += separateSimple(master()->m_startSimpleSeparationParams);
		if(master()->m_separationMode & SM_BoyerMyrvold) sum += separateBoyerMyrvold(master()->m_startBoyerMyrvoldSeparationParams);
	} else {
		if(master()->m_separationMode & SM_Simple) sum += separateSimple(master()->m_simpleSeparationParams);
		if(master()->m_separationMode & SM_BoyerMyrvold) sum += separateBoyerMyrvold(master()->m_boyerMyrvoldSeparationParams);
	}

}

int OptimalCrossingMinimizer::Subproblem::separateSimple(const SimpleSeparationParams& p) {
	lout() << "Subproblem " << id() << " -> separate (Simple): ";

	GraphReduction R(*currentRoundedSolution);

	int foundWithMulti = 0;

	ABA_BUFFER<ABA_CONSTRAINT*> cuts(master(), p.maxCuts());
	DeletingTop10Heap<KuratowskiConstraint, double, TargetComparer<KuratowskiConstraint> > PL(p.maxCuts());
	for(int h = 0; h < p.desperateRuns(); h++) {
		KuratowskiSubdivision K;
		findKuratowski(R, K);
		KuratowskiConstraint* kc = new KuratowskiConstraint(master(), this, R, K, false); //dynamic!
		double slack;
		if(!kc->violated(actVar() , xVal_, &slack ))  {
			delete kc;
		} else {
			foundWithMulti++;
			PL.pushAndDeleteNoRedundancy( kc, slack );
		}
		if(h >= p.runs() && !PL.empty()) // okidoki
			break;
	}

	for(int pli = 0; pli<PL.size(); pli++) {
		KuratowskiConstraint* kc = PL[pli].item();
		cuts.push(kc);
	}
	int ret = addCons(cuts, master()->kuratowskiPool);
	lout() << ret << " new constraints (" << foundWithMulti << ")\n";
	return ret;
}

int OptimalCrossingMinimizer::Subproblem::separateBoyerMyrvold(const BoyerMyrvoldSeparationParams& p) {
	lout() << "Subproblem " << id() << " -> separate (Multi): ";

	GraphReduction R(*currentRoundedSolution);
	makeParallelFreeUndirected(R);

	int found1 = 0;
	int found2 = 0;
	ABA_BUFFER<ABA_CONSTRAINT*> cuts(master(), p.maxCuts());
	DeletingTop10Heap<KuratowskiConstraint, double, TargetComparer<KuratowskiConstraint> > PL(p.maxCuts());
	
	for(int h = 0; h < p.desperateRuns(); ++h) {
		BoyerMyrvold bm;
		DeletingTop10Heap<KuratowskiConstraint, double, TargetComparer<KuratowskiConstraint> > PLr(p.runCuts());
		SList< KuratowskiWrapper > lkw;
		bm.planarEmbed(R, lkw, p.extractions(), 
			p.bundle(), false, true, //limit & randomDfs
			p.noE2());
		
		SList< KuratowskiSubdivision > lks;
		bm.transform(lkw, lks, R, p.veryDifferent());
		
		for(SListIterator<KuratowskiSubdivision> it = lks.begin(); it.valid(); ++it) {
			KuratowskiConstraint* kc = new KuratowskiConstraint(master(), this, R, *it, false); //dynamic!
			double slack;
			if(!kc->violated(actVar() , xVal_, &slack ))  {
				delete kc;
			} else {
				++found2;
				PLr.pushAndDeleteNoRedundancy( kc, slack );
			}		
		}
			
		for(int pli = 0; pli<PLr.size(); pli++) {
			++found1;
			PL.pushAndDeleteNoRedundancy( PLr[pli].item(), PLr[pli].priority() );
		}

		if(h >= p.runs() && !PL.empty()) // okidoki
			break;
	}
	
	for(int pli = 0; pli<PL.size(); pli++) {
		KuratowskiConstraint* kc = PL[pli].item();
		cuts.push(kc);
	}
	int ret = addCons(cuts, master()->kuratowskiPool);
	lout() << ret << " new constraints (" << found1 << "; " << found2 << ")\n";
	return ret;
}

int OptimalCrossingMinimizer::Subproblem::makeFeasible() {
//	lout() << "Subproblem " << id() << " -> makeFeasible: -> calling pricing...\n";
//	return pricing();
	lout() << "Subproblem " << id() << " -> makeFeasible: -> no chance!\n";
	return 1;
}

void OptimalCrossingMinimizer::Subproblem::duplicateKuratowskis( 
		OptimalCrossingMinimizer::CrossingVariable* cvar, 
		SList<OptimalCrossingMinimizer::KuratowskiConstraint*>& L) {
	//ABA_NONDUPLPOOL<ABA_CONSTRAINT, ABA_VARIABLE>* pool = master()->kuratowskiPool;
	
	int max = master()->duplicateKuratowskis();
	for(int j = nCon(); j-->0;) {
		if(!max--) // max==0
			return;
		ABA_CONSTRAINT* c = constraint(j);
		if( c->dynamic() && !strcmp(c->name(),"KuratowskiConstraint") ) {
			KuratowskiConstraint* kc = (KuratowskiConstraint*) c;
			bool cnt = 0;
			for(int i = kc->D.size(); i-->0 && cnt<2;) { // max 2  might be adjacent
				const CrossingLocation* ci = kc->D[i];
				if(cvar->s1.e == ci->s1.e && cvar->s2.e == ci->s2.e) {
					if(cvar->s1.seg == ci->s1.seg && abs(cvar->s2.seg - ci->s2.seg) <= 1) { // 0 won't happen
						cnt++;
						KuratowskiConstraint* nkc = new KuratowskiConstraint(*kc);
						nkc->adapt(i, cvar, cvar->s2.e, ci->s2.seg, cvar->s2.seg);
						L.pushBack(nkc);
					}
					if(cvar->s2.seg == ci->s2.seg && abs(cvar->s1.seg - ci->s1.seg) <= 1) { // 0 won't happen
						cnt++;
						KuratowskiConstraint* nkc = new KuratowskiConstraint(*kc);
						nkc->adapt(i, cvar, cvar->s1.e, ci->s1.seg, cvar->s1.seg);
						L.pushBack(nkc);
					}
				}
			}
		}		
	}
}

int OptimalCrossingMinimizer::Subproblem::checkKnHighKuratowskiCutsStatic() {
	
	lout() << "Search for HighKuratowskiCuts ";
	
	const Graph& MG = *(master()->minimizedGraph);
	EdgeArray<double> crossingsperedge(MG,0);
	
	for(int i = 0; i < nVar(); i++) {
		if(CrossingVariable* cvar = (CrossingVariable*)variable(i)) {
			if(xVal(i) > EPS) { // there is something
				double v = master()->getCost(cvar->s1.e, cvar->s2.e) * xVal(i);
				crossingsperedge[cvar->s1.e] += v;
				crossingsperedge[cvar->s2.e] += v;
			}
		}
	}
		
	Top10Heap<Prioritized<edge> >  maxedges(master()->numBaseEdgesForCutHighKuratowskis()); //TODO: 
	edge e;
	forall_edges(e, MG) {
		Prioritized<edge> v(e,crossingsperedge[e]);
		maxedges.pushBlind(v);
	}
		
	if(maxedges.empty()) {
		Prioritized<edge> v(MG.chooseEdge(),0);
		maxedges.pushBlind(v);
	}
	
	KuratowskiConstraint::KuratowskiType kt = KuratowskiConstraint::KTComplete(MG.numberOfNodes() - 1);
	
	DeletingTop10Heap<KuratowskiConstraint,double,TargetComparer<KuratowskiConstraint> > kura(master()->numCutHighKuratowskis());
	for(int i = maxedges.size(); i-->0;) {
		edge maxedge = maxedges[i].item();
		node n;	
		node src = maxedge->source();
		node tgt = maxedge->target();
		forall_nodes(n, MG) {
			double sumonk = 0;
			if(n == src || n == tgt) continue;
			KuratowskiConstraint* k = new KuratowskiConstraint(master(), kt, false); //non-dynamic
			int path = 0;
			forall_edges(e, MG) {
				if(e == maxedge) continue;
				if( (e->target()==n && e->source() != src && e->source() != tgt) ||
					(e->source()==n && e->target() != src && e->target() != tgt) )
					continue;
				sumonk += crossingsperedge[e];
				k->addEdge(e, path++);
			}
			if(sumonk < k->rhs() - EPS) { //violated
				kura.pushAndDelete(k, -sumonk);
			} else
				delete k;		
		}
	}

	if(kura.empty()) return 0;
	ABA_BUFFER<ABA_CONSTRAINT*> cuts(master(), kura.size());
	for(int ki = kura.size(); ki-->0;) {
		cuts.push(kura[ki].item());
	}
	return addCons(cuts, master()->hintedPool);	
}

int OptimalCrossingMinimizer::Subproblem::checkKnmHighKuratowskiCutsStatic(){
	lout() << "Search for HighKuratowskiCuts ";
	
	const Graph& MG = *(master()->minimizedGraph);
	EdgeArray<double> crossingsperedge(MG,0);
	
	for(int i = 0; i < nVar(); i++) {
		if(CrossingVariable* cvar = (CrossingVariable*)variable(i)) {
			if(xVal(i) > EPS) { // there is something
				double v = master()->getCost(cvar->s1.e, cvar->s2.e) * xVal(i);
				crossingsperedge[cvar->s1.e] += v;
				crossingsperedge[cvar->s2.e] += v;
			}
		}
	}
		
	Top10Heap<Prioritized<edge> >  maxedges(master()->numBaseEdgesForCutHighKuratowskis()); //TODO: 
	edge e;
	forall_edges(e, MG) {
		Prioritized<edge> v(e, crossingsperedge[e]);
		maxedges.pushBlind(v);
	}
		
	if(maxedges.empty()) {
		Prioritized<edge> v(MG.chooseEdge(), 0);
		maxedges.pushBlind(v);
	}
	
	KuratowskiConstraint::KuratowskiType kt = KuratowskiConstraint::KTBipartite(
		MG.chooseEdge()->source()->degree()-1,
		MG.chooseEdge()->target()->degree()-1);

	DeletingTop10Heap<KuratowskiConstraint, double, TargetComparer<KuratowskiConstraint> > kura(master()->numCutHighKuratowskis());
	for(int i = maxedges.size(); i-->0;) {
		edge maxedge = maxedges[i].item();
		node n,m;	
		node src = maxedge->source();
		node tgt = maxedge->target();
		adjEntry an, am;
		forall_adj(an, src) {
			n = an->twin()->theNode();
			if(n == tgt) continue;
			forall_adj(am, tgt) {
				m = am->twin()->theNode();
				if(m == src) continue;
				double sumonk = 0;
				KuratowskiConstraint* k = new KuratowskiConstraint(master(), kt, false); //non-dynamic
				int path = 0;
				forall_edges(e, MG) {
					if(e == maxedge) continue;
					if( (e->source() == n && e->target() != m && e->target() != src && e->target() != tgt) ||
						(e->target() == n && e->source() != m && e->source() != src && e->source() != tgt) ||
						(e->source() == m && e->target() != n && e->target() != src && e->target() != tgt) ||
						(e->target() == m && e->source() != n && e->source() != src && e->source() != tgt) )
							continue;
					sumonk += crossingsperedge[e];
					k->addEdge(e, path++);
				}
				if(sumonk < k->rhs() - EPS) { //violated
					kura.pushAndDelete(k, -sumonk);
				} else
					delete k;		
			}
		}
	}

	if(kura.empty()) return 0;
	ABA_BUFFER<ABA_CONSTRAINT*> cuts(master(), kura.size());
	for(int ki = kura.size(); ki-->0;) {
		cuts.push(kura[ki].item());
	}
	return addCons(cuts, master()->hintedPool);	
}


int OptimalCrossingMinimizer::Subproblem::pricing() {	
	// *I* price at solveLp...
	if(generatedConVar > 0)
		return generatedConVar;
	return 0;
}

void OptimalCrossingMinimizer::Subproblem::findKuratowski(Graph& R, KuratowskiSubdivision& K) {
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

OptimalCrossingMinimizer::CrossingConfiguration* OptimalCrossingMinimizer::Subproblem::callBoundHeuristic(bool integer) {
/*	int bh = master()->numPermutationsBoundHeuristic();
	if(bh < 0) {
		bh *= -1;
		master()->numPermutationsBoundHeuristic(0);
	}
	if(bh == 0) return 0;*/
	
	const PlanRep& PR = *(integer ? currentIntegerSolution : currentRoundedSolution);
	
	OGDF_ASSERT(PR.numberOfNodes() > master()->minimizedGraph->numberOfNodes() );
	
	EdgeArray<int>* helpcost = NULL;
	
	EdgeArray<bool>* helpforbid = NULL;
	EdgeArray<unsigned int>* helpsubgraphs = NULL;	
	if(master()->useCost())      helpcost      = new EdgeArray<int>(PR);
	if(master()->useForbid())    helpforbid    = new EdgeArray<bool>(PR);
	if(master()->useSubgraphs()) helpsubgraphs = new EdgeArray<unsigned int>(PR);
	
	edge e, oe;
	forall_edges(e, PR) {
		oe = master()->expandedGraph->original(PR.original(e));
		if(helpcost)      (*helpcost)[e]      = master()->cost[oe];
		if(helpforbid)    (*helpforbid)[e]    = master()->forbid[oe];
		if(helpsubgraphs) (*helpsubgraphs)[e] = master()->subgraphs[oe];
	}
	
	PlanRep HPR((const Graph&)PR);
//	sp.permutations(bh);
	int ignore;
	if(!master()->m_boundHeuristic.valid()) { // no bound heuristic...
		HPR.initCC(0);
		PlanarModule pm;
		if( !pm.planarityTest(HPR) ) return 0; // no trivial solution
		// --> trivial solution is usable		
	} else
		master()->m_boundHeuristic.get().call(HPR, 0, ignore, helpcost, helpforbid, helpsubgraphs);
	
	if(helpcost)     delete helpcost;
	if(helpforbid)    delete helpforbid;
	if(helpsubgraphs) delete helpsubgraphs;

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

int OptimalCrossingMinimizer::Subproblem::improve(double &primalValue) {
	int success = 0;
	
	if(generatedConVar != 0) return success;
	
	lout() << "Subproblem " << id() << " -> improve:";

	CrossingConfiguration* heuristicSolution = 0;
	if(!father() && nIter_==1) {
		lout() << " [Init: ";
		if(master()->bestSolution) {
			heuristicSolution = master()->bestSolution;
			master()->updateBestSolution(heuristicSolution);
			primalValue = heuristicSolution->getCrossingNo();
			master()->primalBound(primalValue);
			lout() << primalValue << "]";
		} else { // no init heur
			master()->updateBestSolution(0);
			lout() << "NO HEURISTIC WAS RUN (->" << primalValue << ") ]";
		}
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
			lout() << " not neccessary";
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

void OptimalCrossingMinimizer::Subproblem::realizeVariable(CrossingVariable* cvar, double val) {
	if(val > master()->roundUp() ) {
		lout() << "\trealizeVariable: " << (*cvar) << " with " << val << "\n";
		const List<edge>& c1 = master()->expandedGraph->chain(cvar->s1.e);
		const List<edge>& c2 = master()->expandedGraph->chain(cvar->s2.e);
	
		OGDF_ASSERT( cvar->s1.seg >= 0 );
		OGDF_ASSERT( cvar->s1.seg < master()->activeVars->numIndices(cvar->s1.e) );
		OGDF_ASSERT( cvar->s2.seg >= 0 );
		OGDF_ASSERT( cvar->s2.seg < master()->activeVars->numIndices(cvar->s2.e) );
		
		edge f1 = *(c1.get(cvar->s1.seg));
		edge f2 = *(c2.get(cvar->s2.seg));

		edge r1 = currentRoundedSolution->chain(f1).front();
		edge r2 = currentRoundedSolution->chain(f2).front();	
		currentRoundedSolution->insertCrossing(r1, r2, true);
		currentRoundedCrossings->pushBack(cvar);
		if(val > 1 - EPS) {
			edge i1 = currentIntegerSolution->chain(f1).front();
			edge i2 = currentIntegerSolution->chain(f2).front();
			currentIntegerSolution->insertCrossing(i1, i2, true);
			currentIntegerCrossings->pushBack(cvar);
		}
	}
}

bool OptimalCrossingMinimizer::Subproblem::checkSimplicitiesAndPrice() {
	// guaranteed: all existing simplicity constraints are guaranteed...
	lout() << "Simplicity check: ";
	
	EdgeArray<double> sum(*master()->minimizedGraph, 0);
	EdgeArray<List<Segment> > lst(*master()->minimizedGraph);
	
	if(master()->pricingMode() == PM_Greedy) {
		for(int i = 0; i < nVar(); i++) {
			if(CrossingVariable* cvar = (CrossingVariable*)variable(i)) {
				if(xVal(i) >= master()->roundUp() - EPS) { // there is something which will turn into a crossing
					double v = /*master()->getCost(cvar->s1.e, cvar->s2.e) **/ xVal(i);
					if(cvar->s1.seg == 0) {
						sum[cvar->s1.e] += v;
						lst[cvar->s1.e].pushBack(Segment(cvar->s2.e, cvar->s2.seg));
					}
					if(cvar->s2.seg == 0) {
						sum[cvar->s2.e] += v;
						lst[cvar->s2.e].pushBack(Segment(cvar->s1.e, cvar->s1.seg));
					}
				}
			}
		}
	} else { // PM_Few AND above. safe choice.
		for(int i = 0; i < nVar(); i++) {
			if(CrossingVariable* cvar = (CrossingVariable*)variable(i)) {
				if(xVal(i) > EPS) { // there is something
					double v = /*master()->getCost(cvar->s1.e, cvar->s2.e) **/ xVal(i);
					if(cvar->s1.seg == 0) {
						sum[cvar->s1.e] += v;
						lst[cvar->s1.e].pushBack(Segment(cvar->s2.e, cvar->s2.seg));
					}
					if(cvar->s2.seg == 0) {
						sum[cvar->s2.e] += v;
						lst[cvar->s2.e].pushBack(Segment(cvar->s1.e, cvar->s1.seg));
					}
				}
			}
		}
	}
	
	edge e;

	int maxNewVars = 0;
	forall_edges(e, *master()->minimizedGraph) {
		if(sum[e] > 1+EPS) { // crap... too many...
			forall_listiterators(Segment, it, lst[e]) {				
				maxNewVars += (*it).seg+1; // VERY crude estimation -- this can be greatly improved!
			}
			maxNewVars++;
			//maxNewVars += lst[e].size();
		}
	}

	ABA_BUFFER<ABA_VARIABLE*> vars(master(), maxNewVars);
	
//	bool crap = 0;
	
	forall_edges(e, *master()->minimizedGraph) {
		if(sum[e] > 1+EPS) { // crap... too many...
			// find crossing segments with shortest extension yet.
			lout() << "Sum on segment 0 (" << master()->activeVars->numIndices(e) << ") of edge " << e->index() << ": " << sum[e] << "\n";
			int sh;
			if(master()->pricingMode() <= PM_Few)
				sh = master()->expansionFactor*2+5;
			else  // PM_Reasonable, PM_Plenty
				sh = 1;
			List<Segment> shortest;
			forall_listiterators(Segment, i, lst[e]) {
				if(master()->pricingMode() <= PM_Few) {
					int mI = master()->activeVars->numIndices(*i, e);
	//				lout() << "\t" << (*i).e->index() << "/" << (*i).seg << "(" << mI << ")";
					if(mI < sh) {
						sh = mI;
						shortest.clear();
						shortest.pushBack(*i);
					} else if(mI == sh) {
						shortest.pushBack(*i);
					}
				} else { // PM_Reasonable, PM_Plenty -> take all involved
					shortest.pushBack(*i);
				}				
			}
//			lout() << "\n";
			OGDF_ASSERT( shortest.size() > 0 );
			OGDF_ASSERT( sh > 0 );

			// if extension not possible:
			//      -> add simplicity-constraint for main? 
			//            -> unneccessary, since there is no minimal solution wich needs >1 there...
			// hence: -> do nothing
					
			OGDF_ASSERT( sh <= master()->expandedGraph->chain(e).size() );
			OGDF_ASSERT( sh <= master()->activeVars->numIndices(e) );

//			#ifdef OGDF_DEBUG
//			if( sh>1 && sh == master()->expandedGraph->chain(e).size() ) {
//				ABA_CONSTRAINT* c = new FirstSegmentConstraint(master(), e);
//				ABA_ACTIVE<ABA_VARIABLE, ABA_CONSTRAINT> *act = actVar();
//				double* d = new double[nVar()];
//				for(int i = nVar(); i-->0;) {
//					d[i] = xVal(i);
//				}
//				lout() << (c->violated(act,d,0) ? "\nVIOLATED\n" : "\nSATISFIED\n" ) << flush;
//				for(int i = master()->simplicityPool->size(); i-->0;) {
//					ABA_POOLSLOT<ABA_CONSTRAINT, ABA_VARIABLE>* p = master()->simplicityPool->slot(i);
//					if(p) {
//						ABA_CONSTRAINT* c = p->conVar();
//						if(c && !strcmp(c->name(),"FirstSegmentConstraint")) {							
//							FirstSegmentConstraint* f = (FirstSegmentConstraint*)c;
//							if(f->referenceEdge() == e) {
//								lout() << " { Active? " << f->active() << "  ";
//								if(c->violated(act,d,0)) lout() << " IN AND VIOLATED } ";
//								else lout() << " IN BUT SATISFIED } ";
//							}
//						}
//					}
//				}
//				lout()<<"check done";
//				delete[] d;
//				delete c;
//			}
//			#endif //OGDF_DEBUG
			
			OGDF_ASSERT( sh==1 || sh < master()->expandedGraph->chain(e).size() )
			
			Segment novelSeg(e, sh);
			// all shortest exts get extended by one. 
			forall_listiterators(Segment, ii, shortest) {
				// ensure convexity
				edge f = (*ii).e;
//				OGDF_ASSERT( master()->activeVars->numIndices(novelSeg, f) >= 0 );
//				OGDF_ASSERT( (*ii).seg <= master()->expandedGraph->chain(e).size() );
//				OGDF_ASSERT( (*ii).seg < master()->activeVars->numIndices(f) );
//				OGDF_ASSERT( master()->activeVars->numIndices(novelSeg, f) <= (*ii).seg );
				if(master()->pricingMode() == PM_Reasonable) {
					novelSeg.seg = master()->activeVars->numIndices(*ii, e);
					OGDF_ASSERT( /*novelSeg.seg==1 ||*/ novelSeg.seg < master()->expandedGraph->chain(e).size() );
				}
				for(int sid = master()->activeVars->numIndices(novelSeg, f); // loop to ensure convexity
						sid <= (*ii).seg;
						sid++) {
					Segment segA(f, sid);
					CrossingVariable* cvar = new CrossingVariable(master(), novelSeg, segA);

					for(int i = vars.number(); i-->0;) {
//						lout() << "  Comparing (" << cvar->s1.e->index() << "/" << cvar->s1.seg << ")x("  << cvar->s2.e->index() << "/" << cvar->s2.seg << 
//							") with (" << ((CrossingVariable*)vars[i])->s1.e->index() << "/" << ((CrossingVariable*)vars[i])->s1.seg << ")x("  << ((CrossingVariable*)vars[i])->s2.e->index() << "/" << ((CrossingVariable*)vars[i])->s2.seg << "): ";
						if(master()->crossingLocationComparer.equal(cvar,(CrossingVariable*)vars[i])) { // i handled that already...
//							lout() << "ident -> kick\n";
							delete cvar;
							cvar = NULL;
							break;
						}
//						lout() << "different\n";
					}
//					lout() << "all different -> hold\n";
					if(!cvar) continue; // "break" might also work?!...
					
//					lout() << "\tV: " << *cvar << "  ";
					vars.push(cvar);	
				}
			}
		}
	}
	
	int nV = vars.number();

	clearDuplicatedKuratowskiList();
	OGDF_ASSERT( bufferedSimplicities.empty() );

	Segment* segs[2];
	for(int i = 0; i<nV; ++i) {
		CrossingVariable* cvar = (CrossingVariable*)vars[i];
		
		segs[0] = &(cvar->s1);
		segs[1] = &(cvar->s2);

		int oldlength[2];
		for(int p=0; p<2; ++p) {
			oldlength[p] = master()->activeVars->numIndices(segs[p]->e);
			if(segs[p]->seg == oldlength[p]) { // check whether i need a new simplicity-constraint
				if(segs[p]->seg == 1)
					bufferedSimplicities.pushFront( new SimplicityConstraint(master(), *(segs[p])) );
				else
					bufferedSimplicities.pushFront( new SegmentOrderConstraint(master(), *(segs[p])) );
			}
		}

		master()->activeVars->registerVariable(cvar);
		duplicateKuratowskis(cvar, duplicatedKuratowskiList);

		for(int p=0; p<2; ++p) {
			int chainlength = master()->expandedGraph->chain(segs[p]->e).size();
			if(master()->activeVars->numIndices(*(segs[1-p]), segs[p]->e) == chainlength) {
				if(oldlength[p] < chainlength) {
					bufferedSimplicities.pushFront(new FirstSegmentConstraint(master(), segs[p]->e));
				} /*else {
					removeConstraints.push(segs[p]->e);
				}*/
			}
		}
	}
	
//	int rnum = removeConstraints.size();
//	for(int j = 0; rnum>0 && j<nCon(); ++j) {
//		ABA_CONSTRAINT* c = (*actCon())[j];
//		if(c && !strcmp(c->name(),"FirstSegmentConstraint")) {
//			FirstSegmentConstraint* f = (FirstSegmentConstraint*)c;
//			for(int i = removeConstraints.size(); i-->0;) {
//				if(f->referenceEdge()==removeConstraints[i]) {
//					lout() << "REMOVE FirstSegmentConstraint for edge " << removeConstraints[i]->index() << " from active constraints!\n";
//					f->soil(this->id());
//					removeCon(j);
//					rnum--;
//					break;
//				}
//			}
//		}
//	}
		
	int ret = 0;
	if(nV) {
//		deactivateNonLiftableConstraints(&vars);
//		ret = addVars(vars);
		ABA_BUFFER<bool> putInPool(master(), nV);
		for(int i = nV; i-->0;)
			putInPool.push(true);
		deactivateNonLiftableConstraints(&vars);
		ret = addVars(vars, 0, &putInPool, 0);
	}

//	int nK = dkl.size();
//	if(nK) {
//		clearDuplicatedKuratowskiList();
//		duplicatedKuratowskiBuffer = new ABA_BUFFER<ABA_CONSTRAINT*>(master(), nK);
//		forall_listiterators(KuratowskiConstraint*, i, dkl) {
//			duplicatedKuratowskiBuffer->push(*i);
//		}
//	}
	
	OGDF_ASSERT(nV == ret);

	lout() << "Simplicity Check: ";
	if(ret) {
		lout() << "Failed -> +Vars=" << ret << "(" << nVar() << "); +bufSimp=" << bufferedSimplicities.size() << " dupKC=" << duplicatedKuratowskiList.size() << "\n";
	} else {
		OGDF_ASSERT( bufferedSimplicities.size() == 0 );
		OGDF_ASSERT( duplicatedKuratowskiList.size() == 0 );
		lout() << "Passed\n";
	}
	return ret;
}

void OptimalCrossingMinimizer::Subproblem::deactivateNonLiftableConstraints(ABA_BUFFER<ABA_VARIABLE*>* buf) {
	int remd = 0;
	for(int i = nCon(); i-->0;) {
		bool reset = false;
		ABA_POOLSLOT<ABA_CONSTRAINT, ABA_VARIABLE>* slot = actCon()->poolSlotRef(i)->slot();
		ABA_CONSTRAINT* c = slot->conVar();
		edge e = 0;
		int sz = 0;
		if(strcmp(c->name(), "FirstSegmentConstraint") == 0) {
			e = ((FirstSegmentConstraint*)c)->referenceEdge();
			sz = master()->expandedGraph->chain(e).size() - 1;
		}
		if(!buf) reset = e;
		else for(int j = buf->number(); j-->0;) {
			CrossingVariable* v = (CrossingVariable*) (*buf)[j]; 
			if( /*c->coeff(v) // != 0...
					||*/ ( v->s1.e == e && v->s1.seg == sz )
					|| ( v->s2.e == e && v->s2.seg == sz )) { 
				reset = true;
				break;
			}
		}
		if(reset) {
//			if(aC->dynamic()) {
//				OGFD_ASSERT( strcmp(aC->name(), "KuratowskiConstraint"));
//				aC = new KuratowskiConstraint((KuratowskiConstraint*)aC);
//			}
			deactNonLiftCons.push(slot);
			removeCon(i);
			++remd;
		}
	}
	lout() << "Deactivated " << remd << " of " << nCon() << " constraints.\n";
}

int OptimalCrossingMinimizer::Subproblem::solveLp() {
	
//	#ifdef OGDF_DEBUG
//	lout() << "ACTIVE CONSTRAINTS: " << this->nCon() << "\n";
//	for(int i = master()->simplicityPool->size(); i-->0;) {
//		ABA_CONSTRAINT* c = master()->simplicityPool->slot(i)->conVar();
//		if(c && !strcmp(c->name(),"FirstSegmentConstraint")) {							
//			FirstSegmentConstraint* f = (FirstSegmentConstraint*)c;
//			lout() << " { " << f->referenceEdge()->index() << " Active? " << f->active() << "}\t";
//		}
//	}
//	#endif //OGDF_DEBUG
	
	if(master()->totalTime()->exceeds(master()->maxCpuTime())) { // get me outa here!
		generatedConVar = -1;
		return 0;
	}
	
	lout() << "Subproblem " << id() << " -> solve (" << nIter_ << ") -> \n";
	
	OGDF_ASSERT(nIter_ > 0);
	
	if(nIter_ == 1) { // i'm new here... did anything bad happen since my generation?
		// To be sure: deactivate all non-liftabel stuff, and reintroduce them in iteration 2
		deactivateNonLiftableConstraints(0);
	}
	
	if( deactNonLiftCons.size() || bufferedSimplicities.size()) {
		generatedConVar = 0;
		if(	deactNonLiftCons.size() ) {
			int react = deactNonLiftCons.size();
			for(int i = deactNonLiftCons.size(); i-->0;) {
				if(deactNonLiftCons[i]->conVar()) {
					react -= addConBuffer_->insert(deactNonLiftCons[i], true);
				} else
					--react;
			}
			lout() << "Reactivated " << react << " of " << deactNonLiftCons.size() << " deactivated constraints.\n";
			deactNonLiftCons.clear();
			generatedConVar -= react;
		}
		
		int s = bufferedSimplicities.size();
		if(s) {
			ABA_BUFFER<ABA_CONSTRAINT*> simpl(master(), s);
			ABA_BUFFER<bool> put(master(), s);
			for(SListConstIterator<AbacusConstraint*> it = bufferedSimplicities.begin(); it.valid(); ++it) {
				simpl.push(*it);
				put.push(true);
			}
			bufferedSimplicities.clear();
			lout() << "Introduced " << s << " simplicity constraints for the new vars.\n";
			generatedConVar -= addCons(simpl, master()->simplicityPool, &put);
		}
		
		return 0; //rerun...
	}
		
	int error = ABA_SUB::solveLp();
	switch(error) {
		case 0: break; // all right, sankt feit, sag'n d'leut
		case 1: lout() << "LP is infeasible.\n"; return 1;
		case 2: lout() << "LP is infeasible. Abacus wants to price without non-liftables. I reject.\n"; return 2;
		default:
			lout() << "Unknown error happend in Abacus::solveLp (" << error << ")\n";
			return error;
	}
//	double lpRes=0;
//	for(int i = 0; i < nVar(); i++) {
//		if(CrossingVariable* cvar = (CrossingVariable*)variable(i)) {
//			lpRes += master()->getCost(cvar->s1.e, cvar->s2.e) * xVal(i);
//		}
//	}
//	lout() << "-> ObjValue = " << lpRes << "\n";
	
/*	for(int j = 0; j < nCon(); j++) {
		if( !strcmp("KuratowskiConstraint",constraint(j)->name()) ) {
			int sum = 0;
			for(int ii = 0; ii < nVar(); ii++) {
				CrossingVariable* cvar = (CrossingVariable*)variable(ii);
				if(cvar->s1.e->source()->index() == 0 && cvar->s1.e->target()->index()==1 && cvar->s2.e->source()->index()==3 && cvar->s2.e->target()->index()==4) {
					lout() << "Here\n";
				}				
				double c = constraint(j)->coeff(cvar);
				if(c==1) {
					sum++;
					lout() << "[" << cvar->s1.e->source() << "," << cvar->s1.e->target() << "|" << cvar->s2.e->source() << "," << cvar->s2.e->target() << "] ";
				}else if(c!=0)
					cerr << "WRONG COEFF! "<<c<<"\n";
			}
			lout() << " #Var/Con: " << sum << "\n";
		}
	}
*/	
//	lout() << "(checking simplicitypool with " << master()->simplicityPool->number() << " elements)";
	if(generatedConVar = -constraintPoolSeparation(0, master()->simplicityPool)) {
		lout() << "-> Simplicity Pool Separation (+" << -generatedConVar << "->" << nCon() << ")\n";
		return 0;
	}
	
	if(generatedConVar = -constraintPoolSeparation(0, master()->hintedPool)) {
		lout() << "-> Hinted Pool Separation (+" << -generatedConVar << "->" << nCon() << ")\n";
		return 0; // resolve
	}
	
	if(!duplicatedKuratowskiList.empty()) {
		ABA_BUFFER<ABA_CONSTRAINT*> buf(master(), duplicatedKuratowskiList.size());
		for(SListConstIterator<KuratowskiConstraint*> it = duplicatedKuratowskiList.begin(); it.valid(); ++it) {
			buf.push(*it);
		}
		duplicatedKuratowskiList.clear();
		if(generatedConVar = -addCons(buf, master()->kuratowskiPool)) {
			lout() << "-> Duplicated-Kuratowski Separation (+" << -generatedConVar << "->" << nCon() << ")\n";
			return 0; // resolve
		}
	}
	
	if(master()->hintEffects() & HE_HighKuratowskiCutsStatic) {
		int ret;
		if( (master()->graphHint() == GH_Complete && (ret=checkKnHighKuratowskiCutsStatic())) ||
			(master()->graphHint() == GH_CompleteBipartite && (ret=checkKnmHighKuratowskiCutsStatic()))) {
			lout() << "-> Generated " << ret << " static HighKuratowskiCuts (->" << nCon() << ")\n";
			generatedConVar = -ret;
			return 0; // resolve
		} else lout() << "unsuccessful\n";
	}
	
	if(generatedConVar = -constraintPoolSeparation(0, master()->kuratowskiPool)) {
		lout() << "-> Kuratowski Pool Separation (+" << -generatedConVar << "->" << nCon() << ")\n";
		return 0; // resolve
	}
	
	if( (master()->pricingInit() != PI_NoPricing) && (generatedConVar = checkSimplicitiesAndPrice())) {
		lout() << "-> resolve (pricing)\n";
		return 0;
	}

	generatedConVar = 0;	
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

	return 0;
}


Module::ReturnType OptimalCrossingMinimizer::Master::doCall(PlanRep &_PG,
		int cc,
		const EdgeArray<int>  &_cost,
		const EdgeArray<bool> &_forbid, 
		const EdgeArray<unsigned int>  &_subgraphs,
		int& crossingNumber) {
			
	if(useSubgraphs()) {
		lout(Logger::LL_ALARM) << "ERROR: OptimalCrossingMinimizer does not support Simultaneous Subgraphs. Use OptimalSimultaneousCrossingMinimizer instead!\n";
		return Module::retError;
	}
	
	if(effectiveLogLevel() >= LL_ALARM || effectiveStatisticMode())
		outLevel(OptimalCrossingMinimizer::Master::Silent);
	else
		outLevel(OptimalCrossingMinimizer::Master::Full);
		
	lout() << "---------------------------------------------"
	       << "\nnumStartHeuristics = " << m_numStartHeuristics
	       << "\nbranchingMode = " << ((m_branchingMode == BM_Traditional) ? "Traditional" : "CompleteOdd") 
	       << "\npricingInit = " << m_pricingInit
	       << "\npricingMode = " << ((m_pricingMode == PM_Greedy) ? "Greedy" : ((m_pricingMode == PM_Few) ? "Few" : ( (m_pricingMode == PM_Reasonable) ? "Reasonable" : "Plenty") )) 
	       << "\nseparationMode = {" << ((m_separationMode & SM_Simple) ? " Simple" : "")
	                                 << ((m_separationMode & SM_BoyerMyrvold) ? " BoyerMyrvold" : "") << " }"
	       << "\nStart-SimpleSeparation = { " << m_startSimpleSeparationParams << " }"
	       << "\nSimpleSeparation = { " << m_simpleSeparationParams << " }"
	       << "\nStart-BoyerMyrvoldSeparation = { " << m_startBoyerMyrvoldSeparationParams << " }"
	       << "\nBoyerMyrvoldSeparation = { " << m_boyerMyrvoldSeparationParams << " }"
/*	       << "\nnumStartKuratowskis = " << m_numStartKuratowskis
	       << "\nnumDesperateTryKuratowskis = " << m_numDesperateTryKuratowskis
	       << "\nnumTryKuratowskis = " << m_numTryKuratowskis
	       << "\nnumCutKuratowskis = " << m_numCutKuratowskis
	       << "\nkuratowskiExtractionDesperateRuns = " << m_kuratowskiExtractionDesperateRuns
	       << "\nkuratowskiExtractionRuns = " << m_kuratowskiExtractionRuns
	       << "\nkuratowskiExtractionExtractions = " << m_kuratowskiExtractionExtractions
	       << "\nkuratowskiExtractionRunLimit = " << m_kuratowskiExtractionRunLimit
	       << "\nkuratowskiExtractionAllLimit = " << m_kuratowskiExtractionAllLimit
	       << "\nkuratowskiExtractionBundle = " << m_kuratowskiExtractionBundle
	       << "\nkuratowskiExtractionNoE2 = " << m_kuratowskiExtractionNoE2
	       << "\nkuratowskiExtractionVeryDifferent = " << m_kuratowskiExtractionVeryDifferent
	       << "\nkuratowskiExtractionStartRuns = " << m_kuratowskiExtractionStartRuns
	       << "\nkuratowskiExtractionStartExtractions = " << m_kuratowskiExtractionStartExtractions
	       << "\nkuratowskiExtractionStartRunLimit = " << m_kuratowskiExtractionStartRunLimit
	       << "\nkuratowskiExtractionStartAllLimit = " << m_kuratowskiExtractionStartAllLimit
	       << "\nkuratowskiExtractionStartBundle = " << m_kuratowskiExtractionStartBundle
	       << "\nkuratowskiExtractionStartNoE2 = " << m_kuratowskiExtractionStartNoE2
	       << "\nkuratowskiExtractionStartVeryDifferent = " << m_kuratowskiExtractionStartVeryDifferent*/
	       << "\nnumCutHighKuratowskis = " << m_numCutHighKuratowskis
	       << "\nnumBaseEdgesForCutHighKuratowskis = " << m_numBaseEdgesForCutHighKuratowskis
	       << "\nduplicateKuratowskis = " << m_duplicateKuratowskis
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

	bestSolution = helpCall(_PG, cc, _cost, _forbid, crossingNumber);
	if(writeIntermediateResultsToo()) doWriteBestSolution();
	
//	minimizedGraph->allEdges(helperEdgelist1);
//	minimizedGraph->allEdges(helperEdgelist2);
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
		
/*		//rRoot()
		ABA_ACTIVE<ABA_CONSTRAINT, ABA_VARIABLE>* aC = root()->actCon();
		double v;
		int tight = 0;
		for(int i = aC->number(); i-->0;) {
			if(((*aC)[i])->slack(root()->actVar(), &v) < EPS) {
				++tight;
				lout() << aC[i] << " [RHS=" << v << "]\n";
			}
		}		
		
		lout() << "Number of constraints without slack: " << tight << "\n";*/
		
		lout() << "Start #Variables: " << m_startVariables << "; Min #Variables: " << m_minVariables << "; Max #Variables: " << m_maxVariables << "; Generated Variables: " << m_usedVariables << "(=" << (100*m_usedVariables/(double)m_maxVariables) << "%)\n";
		
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
		if(graphHint() == GH_Complete && numMinNodes%2)
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
			if(graphHint() == GH_Complete && numMinNodes%2) ++crossingNumber;
			break;
		case SS_Knm:
			lout() << "The ILP found no solution better Zarankiewicz's conjecture.\n";
			crossingNumber = upperbound+1;
			break;
		default:
			lout() << "Unknown upper bound source.\n";
	}
	
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
	       << "\t" << (m_isTrivial ? 0 : m_startVariables)
	       << "\t" << (m_isTrivial ? 0 : m_usedVariables)
	       << "\t" << ( (m_isTrivial||(m_maxVariables==0)) ? 0 : (100*m_usedVariables/(double)m_maxVariables))
	       << "\t" << (m_isTrivial ? 0 : simplicityPool->number())
	       << "\t" << (m_isTrivial ? 0 : kuratowskiPool->number());	
	       
	if(isOptimal() && (effectiveLogLevel()<=LL_DEFAULT || writeResult()) ) {
		doWriteBestSolution();
	}       
	return isOptimal() ? retOptimal : (status()==OutOfMemory || status()==Error) ? retError : retFeasible;
}

void OptimalCrossingMinimizer::Master::doWriteBestSolution() {
	if(!bestSolution) return;
	lout() << "=================================\n"
		<< "Crossing Configuration:\n"
		<< "---------------------------------\n";
	PlanRep RRR(*expandedGraph);
	RRR.initCC(0);
	edge e;
	forall_edges(e, *minimizedGraph) {
		const List<edge>& L = bestSolution->getCrossingEdges(e);
		if(!L.empty()) {
			lout() << e << " [" << cost[e] << "]: " << L << "\n";
		
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
	if(writeResult()) {
		GraphReduction RR(RRR);
		GraphAttributes GAR(RR, GraphAttributes::nodeGraphics | GraphAttributes::nodeColor);
		node n;
		forall_nodes(n, RR) {
			if(!RRR.original(RR.original(n))) {
				GAR.colorNode(n) = "red";
			}
		}
		GAR.writeGML(writeResult());
	}
	lout() << "=================================\n";
}

bool OptimalCrossingMinimizer::Master::variableAllowed(edge e1, edge e2) {
	// thou shalt not cross
	if( forbid[e1] || forbid[e2] )
		return false;
		
	// adjacent edges do not cross
	if( e1->commonNode(e2) ) return false;
	
	// oh well, go ahead and pair...
	return true;
}

void OptimalCrossingMinimizer::Master::hintsKnKuratowskiMinusOne(List<ABA_CONSTRAINT*>& prelist) {
	node n;
	edge e;
	
	int ns = minimizedGraph->numberOfNodes() - 1;

	KuratowskiConstraint::KuratowskiType kt = KuratowskiConstraint::KTComplete(ns);
	forall_nodes(n, *minimizedGraph) {
		KuratowskiConstraint* k = new KuratowskiConstraint(this, kt, false); // non-dynamic
		int path = 0;
		forall_edges(e, *minimizedGraph) {
			if(e->source() != n && e->target() != n) {
				k->addEdge(e, path++);
			}
		}
		prelist.pushBack(k);
	}			
	lout() << "Complete-Graph-hint, KuratowskiMinusOne-effect:\n"
		<< "\tadding " << ns+1 << " K_" << ns << " constraints, requiring " << "\"some\""/*cr*/ << " crossings each.\n";
}

void OptimalCrossingMinimizer::Master::helperHintsKnAllSubKuratowskis(NodeArray<bool>& aktnodes, node posNode, int num, List<ABA_CONSTRAINT*>& prelist) {
	node n; 
	edge e;
	if(posNode) {
		helperHintsKnAllSubKuratowskis(aktnodes, posNode->succ(), num, prelist );
		if( num>5 ) {
			aktnodes[posNode] = false;
			helperHintsKnAllSubKuratowskis(aktnodes, posNode->succ(), num-1, prelist );
			aktnodes[posNode] = true;
		}
	} else if( num < minimizedGraph->numberOfNodes() ) {
		KuratowskiConstraint* k = new KuratowskiConstraint(this, KuratowskiConstraint::KTComplete(num), false); // non-dynamic
		int path = 0;
		forall_edges(e, *minimizedGraph) {
			if(aktnodes[e->source()] && aktnodes[e->target()])
				k->addEdge(e, path++);
		}
//		cout << "Pfade: " << path << "\n";
		prelist.pushBack(k);
	}
}

void OptimalCrossingMinimizer::Master::hintsKnAllSubKuratowskis(List<ABA_CONSTRAINT*>& prelist) {
	NodeArray<bool> aktnodes(*minimizedGraph, true);
	helperHintsKnAllSubKuratowskis(aktnodes, minimizedGraph->firstNode(), minimizedGraph->numberOfNodes(), prelist);
				
	lout() << "Complete-Graph-hint, AllSubKuratowskis-effect:\n"
		<< "\tadding " << prelist.size() << " Kuratowski (subgraph) constraints.\n";
}

void OptimalCrossingMinimizer::Master::hintsKnNodeOrder(List<ABA_CONSTRAINT*>& prelist) {
	node n;

	node lastNode = 0;
	forall_nodes(n, *minimizedGraph) {
		if(lastNode)
			prelist.pushBack(new NodeOrderConstraint(this, lastNode, n));
		lastNode = n;
	}
	lout() << "Complete-Graph-hint, NodeOrder-effect:\n\tadding " << minimizedGraph->numberOfNodes()-1 << " NodeOrder constraints.\n";
}

void OptimalCrossingMinimizer::Master::hintsKnEdgeOrder(List<ABA_CONSTRAINT*>& prelist) {
	edge e;

	node baseNode = minimizedGraph->firstNode();
	edge lastEdge = 0;
	forall_adj_edges(e, baseNode) {
		if(lastEdge)
			prelist.pushBack(new EdgeOrderConstraint(this, lastEdge, e));
		lastEdge = e;
	}
	lout() << "Complete-Graph-hint, EdgeOrder-effect:\n\tadding " << minimizedGraph->numberOfNodes()-2 << " EdgeOrder constraints.\n";
}

void OptimalCrossingMinimizer::Master::hintsKnmKuratowskiMinusOne(List<ABA_CONSTRAINT*>& prelist) {
	node n;
	edge e;

	int ns1 = minimizedGraph->firstNode()->degree();
	int ns2 = minimizedGraph->firstNode()->firstAdj()->twin()->theNode()->degree();
	
	KuratowskiConstraint::KuratowskiType kt1 = KuratowskiConstraint::KTBipartite(ns2,ns1-1);
	KuratowskiConstraint::KuratowskiType kt2 = KuratowskiConstraint::KTBipartite(ns2-1,ns1);
	
	forall_nodes(n, *minimizedGraph) {
		KuratowskiConstraint* k = new KuratowskiConstraint(this, n->degree()==ns1 ? kt1 : kt2, false); // non-dynamic
		int path = 0;
		forall_edges(e, *minimizedGraph) {
			if(e->source() != n && e->target() != n) {
				k->addEdge(e, path++);
			}
		}				
		prelist.pushBack(k);
	}			
	lout() << "Complete-Bipartite-Graph-hint, KuratowskiMinusOne-effect: "
	    << "\tadding " << ns1 << " K_{" << ns1-1 << "," << ns2 << "} constraints requiring " << "\"some\""/*cr*/ << " crossings each.\n";
	lout() << "\tadding " << ns2 << " K_{" << ns1 << "," << ns2-1 << "} constraints requiring " << "\"some\""/*cr*/ << " crossings each.\n";
}

void OptimalCrossingMinimizer::Master::hintsKnExpensiveKuratowski(List<ABA_CONSTRAINT*>& prelist) {
	node n,nn;
	edge e;
	
	int ns = minimizedGraph->numberOfNodes() - 2;
	KuratowskiConstraint::KuratowskiType kt = KuratowskiConstraint::KTComplete(ns);
	
	KuratowskiConstraint::Restrictiveness r = (numMinNodes%2) ? KuratowskiConstraint::RGreaterPlus(2) : KuratowskiConstraint::RGreaterPlus(1);
	
	forall_nodes(n, *minimizedGraph) {
		forall_nodes(nn, *minimizedGraph) {
			if(nn->index() > n->index()) {
				KuratowskiConstraint* k = new KuratowskiConstraint(this, kt, false, r); // non-dynamic			
				int path = 0;
				forall_edges(e, *minimizedGraph) {
					if(e->source() != n && e->target() != n && e->source() != nn && e->target() != nn)
						k->addEdge(e, path++);
				}
				prelist.pushBack(k);
			}
		}
	}			
	lout() << "Complete-Graph-hint, KuratowskiExpensive-effect: adding expensive-K_" << ns << " constraints (Restrictiveness: " << r << ").\n";
}

void OptimalCrossingMinimizer::Master::hintsKnmAllSubKuratowskis(List<ABA_CONSTRAINT*>& prelist) {
	lout(LL_FORCE) << "WARNING: KnmAllSubKuratowskis not implemented! -> no such hints generated!!!";
	OGDF_ASSERT(false);
}

void OptimalCrossingMinimizer::Master::hintsKnmNodeOrder(List<ABA_CONSTRAINT*>& prelist) {
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
	lout() << "Complete-Bipartite-Graph-hint, NodeOrder-effect:\n\tadding " << minimizedGraph->numberOfNodes()-2 << " NodeOrder constraints.\n";
}

void OptimalCrossingMinimizer::Master::hintsKnmEdgeOrder(List<ABA_CONSTRAINT*>& prelist) {
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
	lout() << "Complete-Bipartite-Graph-hint, EdgeOrder-effect:\n\tadding " << minimizedGraph->numberOfNodes()-2 << " EdgeOrder constraints.\n";
}

void OptimalCrossingMinimizer::Master::initializeOptimization() {
	
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
//	conElimMode(NoConElim); // << warning? -> secure alternative: NoConElim // TODO!!! restore "Basic"
	conElimEps(EPS); // was: 0.001
	conElimAge(1);
//	varElimMode(ReducedCost); // << warning? -> secure alternative: NoVarElim
	varElimMode(NoVarElim); // << warning? -> secure alternative: NoVarElim // TODO!!! restore "ReducedCost"
	varElimEps(EPS); // was: 0.001
	varElimAge(1);	
	// Abacus settings END
		
	List<ABA_CONSTRAINT*> hintedConstraints;
	
	const PricingInit& pm = m_pricingInit;
	ABA_BUFFER<ABA_CONSTRAINT*> simplicityStarters(this, pm==PI_NoPricing ? numExpEdges : numMinEdges*pm);
	ABA_BUFFER<ABA_VARIABLE*> variables(this, pm==PI_NoPricing ? numExpMaxCrossingPairs : numMinMaxCrossingPairs*pm*pm);
	
	// add one pair each...
	edge e1, e2;
	forall_edges(e1,*minimizedGraph)  {
		int size1 = expandedGraph->chain(e1).size();
		forall_edges(e2, *minimizedGraph) {
			if( CrossingLocation::preferedOrder(e1,e2) && variableAllowed(e1,e2) ) {
				int size2 = expandedGraph->chain(e2).size();
				
				for(int es1 = 0; es1 < size1; ) {
					for(int es2 = 0; es2 < size2; ) {
						CrossingVariable* var = new CrossingVariable(this, Segment(e1, es1), Segment(e2, es2));
						activeVars->registerVariable(var);
						variables.push(var);
						m_startVariables++;
						if(++es2 == pm) break;				
					}
					if(++es1 == pm) break;
				}

				m_minVariables++;
				m_maxVariables += size1*size2;				
			}
		}
		if(size1 == 1) simplicityStarters.push(new SimplicityConstraint(this, Segment(e1,0)));
		else { // se > 1
			int priced = (m_pricingInit==0) ? size1 : m_pricingInit;
			if(priced > 1)
				simplicityStarters.push(new SimplicityConstraint(this, Segment(e1,1)));
			if(priced >= size1)
				simplicityStarters.push(new FirstSegmentConstraint(this, e1));
			for(int i = 2; i<priced; ++i )
				simplicityStarters.push(new SegmentOrderConstraint(this, Segment(e1,i)));
		}		
		
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
		m_reduceMemory ? 2*numMinMaxCrossingPairs : numExpMaxCrossingPairs, // worst case: all variables
		0, /*hintedConstraints.number(),*/
		false); // and it's not dynamic 

	if(m_reduceMemory)
		simplicityPool = new ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, 2*numMinEdges, true); // dynamic
	else
		simplicityPool = new ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, numExpEdges, false); // not dynamic
	hintedPool = new ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, hintedConstraints.size(), (hintEffects() & HE_HighKuratowskiCutsStatic)); // dynamic, if i add static kuratowskis to it...
	forall_listiterators(ABA_CONSTRAINT*, it, hintedConstraints)
		hintedPool->insert(*it);
	kuratowskiPool = new ABA_NONDUPLPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, m_reduceMemory ? 500 : 1000, true); // dynamic
	branchingPool = new ABA_STANDARDPOOL<ABA_CONSTRAINT, ABA_VARIABLE>(this, 50, true); //dynamic // VERY rough estimate...

	for(int i = simplicityStarters.number(); i-->0;)
		if(simplicityStarters[i])
			simplicityPool->insert(simplicityStarters[i]);
	
//	lout() << "done\n";
}

void OptimalCrossingMinimizer::Master::generateMinimizedGraphCostAndForbid(
		const EdgeArray<int>  & _cost,
		const EdgeArray<bool> & _forbid) {
	minimizedGraph = new GraphReduction(*givenGraph);
	
//	GraphAttributes GA(*minimizedGraph,GraphAttributes::nodeGraphics | GraphAttributes::nodeLabel);
//	node n;
//	forall_nodes(n,*minimizedGraph) {
//		char buf[4];
//		::sprintf(buf,"%i",n->index());
//		GA.labelNode(n) = String(buf);
//	}
//	GA.writeGML("test.gml");

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

void OptimalCrossingMinimizer::Master::generateExpandedGraph() {

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

OptimalCrossingMinimizer::CrossingConfiguration* OptimalCrossingMinimizer::Master::createHeuristicStartSolution() {
	CrossingConfiguration* cInfo = NULL;
	lout() << "Generating " << m_numStartHeuristics << " heuristic solution(s):";
	int h;
	for(h = m_numStartHeuristics; h-->0;) {
		PlanRep hpr(*minimizedGraph);		
		hpr.initCC(0);
		int crno;
		m_startHeuristic.get().call(hpr, 0, crno, useCost() ? &cost : 0, useForbid() ? &forbid : 0, useSubgraphs() ? &subgraphs : 0); //Achtung: Schnittstelle anpassen!!
		lout() << " " << crno << flush;
		if(cInfo && cInfo->getCrossingNo() > crno) {
			delete cInfo;
			cInfo = NULL;
		}
		if(!cInfo) cInfo = new CrossingConfiguration(hpr,crno, true);
	}
	lout() << "\n";
	return cInfo;
}

OptimalCrossingMinimizer::CrossingConfiguration* OptimalCrossingMinimizer::Master::helpCall(PlanRep &_PG,
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
	
	if(upperbound == 0) {
		lout() << "Hey, why do you give me that graph?! non-planar, heuristic needs one crossing... guess what...\n";
		crossingNumber = 1;
		//TODO backtransformation
		return cInfo;
	}
	
	if(cInfo)
		probablyUpdateUpperBound(cInfo->getCrossingNo(), SS_Heuristic); 
	
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

void OptimalCrossingMinimizer::Master::calcLowerBounds () {
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
	
	probablyUpdateLowerBound(1);
	
	//Euler
	probablyUpdateLowerBound(numMinEdges - 3*numMinNodes + 6);
	
	//Pach&Todt
	double ptb = numMinEdges*numMinEdges*numMinEdges/(33.75*numMinNodes*numMinNodes)- 0.9*numMinNodes;
	probablyUpdateLowerBound((int)ceil(ptb));
	
	if(hintEffects() & HE_IterativeLowerBound) {
		if((graphHint() == GH_Complete)) {
			int n = numMinEdges;
			for(int p = n-int(n/2); p<=n; p++) {
				if(n-p <= 6 && p <= 6) { // otherwise not proven yet // TODO: autoadapt?
					probablyUpdateLowerBound( (int)ceil( double(Math::binomial(n,p) * bipartiteZara(n-p,p)) / double(4 * Math::binomial(n-4, p-2))) );
				}
			}			
		} else if(graphHint() == GH_CompleteBipartite) {
			int n = minimizedGraph->firstNode()->degree();
			int m = minimizedGraph->firstNode()->firstAdj()->twin()->theNode()->degree();
			for(int q = n-n/2; q<=n; q++) {
				if(q <= 6 && m <= 6) { // otherwise not proven yet // TODO: autoadapt?
					probablyUpdateLowerBound( (int)ceil( double((Math::binomial(n,q) * bipartiteZara(m,q)) / double(Math::binomial(n-2, q-2))) ) );
				}
			}			
		}
	}

	// TODO (later): better lower bounds...
	// based on maxim_um_ planar subgraph... maximum not implemented in OGDF...
}

void OptimalCrossingMinimizer::Master::calcUpperBounds () {
	// K_n
	int Kn = completeGuy(numMinNodes);
	if(graphHint() == GH_Complete && numMinNodes%2) {
		lout() << "Kn with n odd: Applying Parity Argument\n";
		--Kn;
	}
	probablyUpdateUpperBound(Kn, SS_Kn);

	if(graphHint() == GH_CompleteBipartite) {
		int n = minimizedGraph->firstNode()->degree();
		int m = minimizedGraph->firstNode()->firstAdj()->twin()->theNode()->degree();
		int Knm = bipartiteZara(n,m);
		probablyUpdateUpperBound(Knm, SS_Knm);
	}
}


int OptimalCrossingMinimizer::Master::enumerationStrategy(ABA_SUB* s1, ABA_SUB* s2) {
	// TODO: select the one with more fixed "1s" first !
	return ABA_MASTER::enumerationStrategy(s1, s2);
}

std::ostream &operator<<(std::ostream &os, const OptimalCrossingMinimizer::CrossingLocation& v) {
	os << "(e1=" << v.s1.e->index() << "/" << v.s1.seg << ",e2=" << v.s2.e->index() << "/" << v.s2.seg <<")";
	return os;
}

std::ostream &operator<<(std::ostream &os, const OptimalCrossingMinimizer::CrossingVariable& v) {
	os << (const OptimalCrossingMinimizer::CrossingLocation&)v;
	return os;
}


std::ostream &operator<<(std::ostream &os, /*const*/ OptimalCrossingMinimizer::KuratowskiConstraint& k) {
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
	os << "] " << ((k.sense_.sense()==ABA_CSENSE::Greater) ? ">": "=") << k.rhs() << ")";
	return os;
}

std::ostream &operator<<(std::ostream &os, /*const*/ OptimalCrossingMinimizer::SimplicityConstraint& k) {
	os << "(SC edge=" << k.s.e->index() << " segId=" << k.s.seg << " RHS=" << k.rhs() << ")";
	return os;
}

std::ostream &operator<<(std::ostream &os, const OptimalCrossingMinimizer::SimpleSeparationParams& p) {
	os << "runs=" << p.runs() << " despRuns=" << p.desperateRuns() << " maxCuts=" << p.maxCuts();
	return os;
}
std::ostream &operator<<(std::ostream &os, const OptimalCrossingMinimizer::BoyerMyrvoldSeparationParams& p) {
	os << "runs=" << p.runs() << " despRuns=" << p.desperateRuns() << " extrct=" << p.extractions() 
		<< " runCuts=" << p.runCuts() << " maxCuts=" << p.maxCuts() << " bndl=" << p.bundle() 
		<< " nE2=" << p.noE2() << " vDif=" << p.veryDifferent();
	return os;
}


void OptimalCrossingMinimizer::CrossingConfiguration::initDirect(const PlanRep& PG, int crNo) {
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

void OptimalCrossingMinimizer::CrossingConfiguration::initIndirect(const PlanRep& hpg, int crNo) {
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

} //namespace

#endif // USE_ABACUS
