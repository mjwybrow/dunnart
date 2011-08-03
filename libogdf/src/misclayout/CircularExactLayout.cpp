/*
 * $Revision: 1.0 $
 * 
 * last checkin:
 *   $Author:gutwenger $ 
 *   $Date:2007-11-09 12:12:41 +0100 (Fri, 09 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Circular layout for Graphs with minimal number of crossings.
 * Please note, that you need ILOG CPLEX and Concert to run this code.
 * 
 * \author Benjamin Stähr
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


#ifdef OGDF_CONCERT



#include <cmath>
#include <sstream>
#include <ogdf/misclayout/CircularExactLayout.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/GraphCopy.h>
#include <ogdf/cluster/ClusterPlanRep.h>
#include <ogdf/basic/MinHeap.h>
#include <ogdf/basic/List.h>
#include <ogdf/basic/BinaryHeap2.h>



ILOSTLBEGIN

#define _USE_MATH_DEFINES
#define ID2POS(i, j, numberOfNodes)\
	(numberOfNodes * (numberOfNodes - 1) / 2) - ((numberOfNodes - i) * (numberOfNodes - i - 1) / 2) + j - i - 1
#define SIMPLE_BRANCH_AND_CUT(cplex) cplex.setParam(IloCplex::PreInd,0); \
	cplex.setParam(IloCplex::BndStrenInd,0); cplex.setParam(IloCplex::AggInd,0); \
	cplex.setParam(IloCplex::CoeRedInd,0); cplex.setParam(IloCplex::RelaxPreInd,0); \
	cplex.setParam(IloCplex::PrePass,0); cplex.setParam(IloCplex::RepeatPresolve,0); \
	cplex.setParam(IloCplex::Reduce,0); cplex.setParam(IloCplex::HeurFreq,-1);
#define forall_edges(e,	G) \
	for((e)=(G).firstEdge(); (e); (e)=(e)->succ())

ILOHEURISTICCALLBACK2(heuristicCallback, IloBoolVarArray&, var, ogdf::CircularExactLayout *, cel) { 
	IloNumArray             obj;
	IloNumArray             sol;
    obj = IloNumArray(getEnv());
    sol = IloNumArray(getEnv());
    getObjCoefs(obj, var);
    getValues(sol, var);
    IloNum objval = getObjValue();
    IloInt cols   = var.getSize();
	ogdf::BinaryHeap2<double, int> nodePrioQ(cel->m_numberCompNodes);
	//compute the added xijValues für each node i - this is a sum of n-1 different values, should the value xij not exist, take (1 - xji)  
	double * xijValues = new double[cel->m_numberCompNodes];
	for (int i = 0; i < cel->m_numberCompNodes - 1; i++)
	{
		xijValues[i] = 0; //initialize
		for (int j = i + 1; j < cel->m_numberCompNodes; j++)
		{
			xijValues[i] += sol[ID2POS(i,j, cel->m_numberCompNodes)];
		}
		if (i > 0)
		{
			for (int j = 0; j < i; j++)
			{
				xijValues[i] += (1 - sol[ID2POS(i,j, cel->m_numberCompNodes)]);
			}
		}
	}

	//create a list of nodes, represented by their integer numbering
	int * ordering = new int[cel->m_numberCompNodes];

	//in each iteration choose the node with the highest value in xijValues to place it next
	for (int i = 0; i <  cel->m_numberCompNodes; i++)
	{
		//determine which node j currently has the highest sum of xijValues
		int highest = 0;
		for (int j = 1; j < cel->m_numberCompNodes; j++)
		{
			if (xijValues[j] > xijValues[highest])
			{
				highest = j;
			}
		}
		//now update all other nodes' xijValues
		//first all xij with i < highest are set to 0
		if (highest != 0)
		{
			for (int j = 0; j < highest; j++)
			{
				xijValues[j] -= sol[ID2POS(j, highest, cel->m_numberCompNodes)];
				sol[ID2POS(j, highest, cel->m_numberCompNodes)] = 0;
			}
		}
		//then all xij with j > highest are set to 1
		if (highest < cel->m_numberCompNodes)
		{
			for (int j = highest + 1; j < cel->m_numberCompNodes; j++)
			{
				xijValues[j] += (1 - sol[ID2POS(highest, j, cel->m_numberCompNodes)]);
				sol[ID2POS(highest, j, cel->m_numberCompNodes)] = 1;
			}
		}
		ordering[i] = highest;
	}

	//now check for each y_ef, if there is a crossing between e and f and set y_ef accordingly
	objval = 0;
	ogdf::edge e1, e2;
	int modificator = 0;
		forall_edges(e1, *(cel->p_componentCopy))
		{
			forall_edges(e2, *(cel->p_componentCopy))
			{
				bool crossingFound = false;
					//there are four possibilities to generate a constraint for a pair of edges
				for (int y = 0; y < 4;y++)
				{
					ogdf::node i,j,k,l;
					switch (y)
					{
						case 0:
						i = e1->source();
						j = e1->target();
						k = e2->source();
						l = e2->target();
						break;
							
						case 1:
						i = e1->source();
						j = e1->target();
						k = e2->target();
						l = e2->source();
						break;

						case 2:
						i = e1->target();
						j = e1->source();
						k = e2->source();
						l = e2->target();
						break;

						case 3:
						i = e1->target();
						j = e1->source();
						k = e2->target();
						l = e2->source();
						break;
					}
				//only create a cut, if edges have no common node, as straight edges with a common node can never cross
				if (i != k && i != l && j != k && j != l && e1 != e2)
				{
					
					int iNr, jNr, kNr, lNr, e, f;
					double ikValue, kjValue, jlValue, e1e2Value = 0.0;
					iNr = (*(cel->m_componentKnotenindex))[i];
					jNr = (*(cel->m_componentKnotenindex))[j];
					kNr = (*(cel->m_componentKnotenindex))[k];
					lNr = (*(cel->m_componentKnotenindex))[l];
					e = (*(cel->m_componentKantenindex))[e1];
					f = (*(cel->m_componentKantenindex))[e2];		
					//Xik + Xkj + Xjl <= 2
					int ikPosition, kjPosition, jlPosition, e1e2Position; 
					modificator = 0;
					//as we created only variables of the form Xik (none of the form Xki) a transformation is necessary whenever such a variable occurs ---> Xki = 1 - Xik
					//the position of Xik is now specified, it is now necessary to insert the correct values into the current row
					if (iNr < kNr)
					{
						ikPosition = ID2POS(iNr, kNr, cel->m_numberCompNodes);
					}
					else
					{
						ikPosition = ID2POS(kNr, iNr, cel->m_numberCompNodes);
						//at this stage the modification to the constraints upper bound derived from the transformation Xki = 1 - Xik cannot be made directly, so we have to it for later
						modificator += 1;
					}

					if (kNr < jNr)
					{
						kjPosition = ID2POS(kNr, jNr, cel->m_numberCompNodes);
					}
					else
					{
						kjPosition = ID2POS(jNr, kNr, cel->m_numberCompNodes);
						modificator += 1;
					}

					if (jNr < lNr)
					{
						jlPosition = ID2POS(jNr, lNr, cel->m_numberCompNodes);
					}
					else
					{
						jlPosition = ID2POS(lNr, jNr, cel->m_numberCompNodes);
						modificator += 1;
					}
					
					//now insert the Yef Variable, be aware, that Yef and Yfe are the same
					if ((*(cel->m_componentKantenindex))[e1] < (*(cel->m_componentKantenindex))[e2])
					{
						e1e2Position = (cel->m_numberCompNodes * (cel->m_numberCompNodes - 1) / 2) + ID2POS(e, f, cel->m_numberCompEdges);
					}
					else
					{
						e1e2Position = (cel->m_numberCompNodes * (cel->m_numberCompNodes - 1) / 2) + ID2POS(f, e, cel->m_numberCompEdges);
					}
					//now check whether the current fractional solution violates the implicated constraint
					ikValue = sol[ikPosition];					
					kjValue = sol[kjPosition];
					jlValue = sol[jlPosition];
					e1e2Value = sol[e1e2Position];
					
					if ((((iNr < kNr) ? ikValue : -ikValue) + ((kNr < jNr) ? kjValue : -kjValue) + ((jNr < lNr) ? jlValue : -jlValue) - e1e2Value) > (2 - modificator + cel->EPS))
					{
						sol[e1e2Position] = 1;
						objval++;
						crossingFound = true;
					}
					if (!crossingFound)
					{
						sol[e1e2Position] = 0;
					}
				}
			}
			}
		}
	//...
    setSolution(var, sol, objval);
}



class myCutCallbackI : public IloCplex::LazyConstraintCallbackI {
	IloBoolVarArray& var;
	const int nodes;
	const int edges;
	ogdf::CircularExactLayout * cel;
	IloNumArray * p_sol;

public:
 IloCplex::CallbackI* duplicateCallback() const {   return (new (getEnv()) myCutCallbackI(*this));}
  myCutCallbackI(IloEnv env, IloBoolVarArray& xvar, const int xnodes, const int xedges, ogdf::CircularExactLayout * xcel) : IloCplex::LazyConstraintCallbackI(env), var(xvar), nodes(xnodes), edges(xedges), cel(xcel) {}
  void main();
  int lopCuts(ogdf::Top10Heap<ogdf::Prioritized<int *, double > > * constraintPrioQ);
  int crossingCuts(ogdf::Top10Heap<ogdf::Prioritized<int *, double > > * constraintPrioQ);
};

IloCplex::Callback myCutCallback(IloEnv env, IloBoolVarArray& var, const int nodes, const int edges, ogdf::CircularExactLayout * cel) {
  return (IloCplex::Callback(new (env) myCutCallbackI(env, var, nodes, edges, cel)));
}

void myCutCallbackI::main()
{
		ogdf::Top10Heap<ogdf::Prioritized<int *, double > > constraintPrioQ(10);
		IloNumArray sol(getEnv());            /// ein array fr double-werte definieren mit anzahl der variablen
		getValues(sol, var);                  /// derzeitige LP-relaxation auslesen (var bekamen wir bergeben), lsung steht nun in sol
		p_sol = &sol;

		int cutCounter = 0;
		cutCounter += lopCuts(&constraintPrioQ);
		int lopCutsAdded = cutCounter;
		cutCounter += crossingCuts(&constraintPrioQ);
		int numberCutsToAdd = (cutCounter < 10) ? cutCounter : 10;
		
		//add the cuts 
		for (int i = 0; i < numberCutsToAdd; i++)
		{
			int * nextConstraint = constraintPrioQ[i].item();
			if (nextConstraint[3] == -1)
			{
				//nextConstraint[3] is '-1', so this is a LOP constraint
				if (nextConstraint[4] == 1)
				{
					//violated constraint was >1
					add(var[nextConstraint[0]] + var[nextConstraint[1]] - var[nextConstraint[2]] <= 1);
				}
				else
				{
					add(var[nextConstraint[0]] + var[nextConstraint[1]] - var[nextConstraint[2]] >= 0);
				}
			}
			else
			{
				//this is a Crossing Constraint
				int iNr, jNr, kNr, lNr;
				iNr = nextConstraint[4];
				jNr = nextConstraint[5];
				kNr = nextConstraint[6];
				lNr = nextConstraint[7];
				if (iNr < kNr)
						{
							if (kNr < jNr)
							{
								if (jNr < lNr)
								{
									add(var[nextConstraint[0]] + var[nextConstraint[1]]+ var[nextConstraint[2]] - var[nextConstraint[3]] <= 2);
								}
								else
								{
									add(var[nextConstraint[0]] + var[nextConstraint[1]] - var[nextConstraint[2]] - var[nextConstraint[3]] <= 1);
								}
							}
							else
							{
								if (jNr < lNr)
								{
									add(var[nextConstraint[0]] - var[nextConstraint[1]]+ var[nextConstraint[2]] - var[nextConstraint[3]] <= 1);
								}
								else
								{
									add(var[nextConstraint[0]] - var[nextConstraint[1]] - var[nextConstraint[2]] - var[nextConstraint[3]] <= 0);
								}
							}
						}
						else
						{
							if (kNr < jNr)
							{
								if (jNr < lNr)
								{
									add(-var[nextConstraint[0]] + var[nextConstraint[1]]+ var[nextConstraint[2]] - var[nextConstraint[3]] <= 1);
								}
								else
								{
									add(-var[nextConstraint[0]] + var[nextConstraint[1]] - var[nextConstraint[2]] - var[nextConstraint[3]] <= 0);
								}
							}
							else
							{
								if (jNr < lNr)
								{
									add(-var[nextConstraint[0]] - var[nextConstraint[1]]+ var[nextConstraint[2]] - var[nextConstraint[3]] <= 0);
								}
								else
								{
									add(-var[nextConstraint[0]] - var[nextConstraint[1]]- var[nextConstraint[2]] - var[nextConstraint[3]] <= -1);
								}
							}
						}
				
			}
		}
		constraintPrioQ.clear();		
}


int myCutCallbackI::lopCuts(ogdf::Top10Heap<ogdf::Prioritized<int *, double > > * constraintPrioQ)
	{
		int cutCounter = 0;
		for (int i = 0; i < cel->m_numberCompNodes - 2; i++)
		{
			for (int j = i + 1; j < cel->m_numberCompNodes - 1; j++)
			{
				for (int k = j + 1; k < cel->m_numberCompNodes; k++)
				{
					//insert variables
					int ijPosition, jkPosition, ikPosition;
					//Position equals: number of all Xmn, which is the totals formula of the number of nodes minus those Xmn, where m > i (or j respectively) and those where n > j (or k respectively) minus 1 
					ijPosition = ID2POS(i, j, cel->m_numberCompNodes); 
					jkPosition = ID2POS(j, k, cel->m_numberCompNodes);
					ikPosition = ID2POS(i, k, cel->m_numberCompNodes);
					double ijValue, jkValue, ikValue = 0;
					ijValue = (*p_sol)[ijPosition];
					jkValue = (*p_sol)[jkPosition];
					ikValue = (*p_sol)[ikPosition];

					if (((ijValue + jkValue - ikValue) > 1 + cel->EPS) || ((ijValue + jkValue - ikValue) < 0 - cel->EPS))
					{
						//we need 5 entries in constraint array to be compatible with constraints for crossings
						//constraint[3] is always set to '-1' to symbolise that this is a LOP constraint
						//constraint[4] is set to '1' if >1 was violated, or to '-1' if <0 was violated
						int * constraint = new int[5];
						constraint[0] = ijPosition;
						constraint[1] = jkPosition;
						constraint[2] = ikPosition;
						constraint[3] = -1;
						constraint[4] = -1;
						double key = (((ijValue + jkValue - ikValue - 1) > (-ijValue - jkValue + ikValue)) ? (ijValue + jkValue - ikValue - 1) : (-ijValue - jkValue + ikValue));
						if ((ijValue + jkValue - ikValue) > 1)
						{
							constraint[4] = 1;
						}
						ogdf::Prioritized<int *, double> element(constraint, key);
						(*constraintPrioQ).pushBlind(element);
						cutCounter++;
					}						
				}
			}
		}
		return cutCounter;
	}
	
int myCutCallbackI::crossingCuts(ogdf::Top10Heap<ogdf::Prioritized<int *, double > > * constraintPrioQ)
	{
		int cutCounter = 0;
		int modificator = 0;
		ogdf::edge e1, e2;
		forall_edges(e1, *(cel->p_componentCopy))
		{
			forall_edges(e2, *(cel->p_componentCopy))
			{
				//there are four possibilities to generate a constraint for a pair of edges
				for (int y = 0; y < 4;y++)
				{
					ogdf::node i,j,k,l;
					switch (y)
					{
						case 0:
						i = e1->source();
						j = e1->target();
						k = e2->source();
						l = e2->target();
						break;
							
						case 1:
						i = e1->source();
						j = e1->target();
						k = e2->target();
						l = e2->source();
						break;

						case 2:
						i = e1->target();
						j = e1->source();
						k = e2->source();
						l = e2->target();
						break;

						case 3:
						i = e1->target();
						j = e1->source();
						k = e2->target();
						l = e2->source();
						break;
					}
				//only create a cut, if edges have no common node, as straight edges with a common node can never cross
				if (i != k && i != l && j != k && j != l && e1 != e2)
				{
					
					int iNr, jNr, kNr, lNr, e, f;
					double ikValue, kjValue, jlValue, e1e2Value = 0.0;
					iNr = (*(cel->m_componentKnotenindex))[i];
					jNr = (*(cel->m_componentKnotenindex))[j];
					kNr = (*(cel->m_componentKnotenindex))[k];
					lNr = (*(cel->m_componentKnotenindex))[l];
					e = (*(cel->m_componentKantenindex))[e1];
					f = (*(cel->m_componentKantenindex))[e2];		
					//Xik + Xkj + Xjl <= 2
					int ikPosition, kjPosition, jlPosition, e1e2Position; 
					modificator = 0;
					//as we created only variables of the form Xik (none of the form Xki) a transformation is necessary whenever such a variable occurs ---> Xki = 1 - Xik
					//the position of Xik is now specified, it is now necessary to insert the correct values into the current row
					if (iNr < kNr)
					{
						ikPosition = ID2POS(iNr, kNr, cel->m_numberCompNodes);
					}
					else
					{
						ikPosition = ID2POS(kNr, iNr, cel->m_numberCompNodes);
						//at this stage the modification to the constraints upper bound derived from the transformation Xki = 1 - Xik cannot be made directly, so we have to it for later
						modificator += 1;
					}

					if (kNr < jNr)
					{
						kjPosition = ID2POS(kNr, jNr, cel->m_numberCompNodes);
					}
					else
					{
						kjPosition = ID2POS(jNr, kNr, cel->m_numberCompNodes);
						modificator += 1;
					}

					if (jNr < lNr)
					{
						jlPosition = ID2POS(jNr, lNr, cel->m_numberCompNodes);
					}
					else
					{
						jlPosition = ID2POS(lNr, jNr, cel->m_numberCompNodes);
						modificator += 1;
					}
					
					//now insert the Yef Variable, be aware, that Yef and Yfe are the same
					if ((*(cel->m_componentKantenindex))[e1] < (*(cel->m_componentKantenindex))[e2])
					{
						e1e2Position = (cel->m_numberCompNodes * (cel->m_numberCompNodes - 1) / 2) + ID2POS(e, f, cel->m_numberCompEdges);
					}
					else
					{
						e1e2Position = (cel->m_numberCompNodes * (cel->m_numberCompNodes - 1) / 2) + ID2POS(f, e, cel->m_numberCompEdges);
					}
					//now check whether the current fractional solution violates the implicated constraint
					ikValue = (*p_sol)[ikPosition];					
					kjValue = (*p_sol)[kjPosition];
					jlValue = (*p_sol)[jlPosition];
					e1e2Value = (*p_sol)[e1e2Position];
					
					if ((((iNr < kNr) ? ikValue : -ikValue) + ((kNr < jNr) ? kjValue : -kjValue) + ((jNr < lNr) ? jlValue : -jlValue) - e1e2Value) > (2 - modificator + cel->EPS))
					{
						cutCounter++;
						int * constraint = new int[8];
						constraint[0] = ikPosition;
						constraint[1] = kjPosition;
						constraint[2] = jlPosition;
						constraint[3] = e1e2Position;
						constraint[4] = iNr;
						constraint[5] = jNr;
						constraint[6] = kNr;
						constraint[7] = lNr;
						double key = (((iNr < kNr) ? ikValue : -ikValue) + ((kNr < jNr) ? kjValue : -kjValue) + ((jNr < lNr) ? jlValue : -jlValue) - e1e2Value) - (2 - modificator);
						ogdf::Prioritized<int *, double> element(constraint, key);
						(*constraintPrioQ).pushBlind(element);
					}
				}
			}
		}
	}
	return cutCounter;
	}




namespace ogdf {
	
	Graph m_G;
	NodeArray<int> m_knotenindex(m_G);
	EdgeArray<int> m_kantenindex(m_G);

	int m_crossings;
	int * m_nodeOrder; 
	BCTree* p_bct;
	node * m_compNodes;
	int m_count;
	node m_lastCutVertex;
	int m_processedBicompCount;
	String m_theFile;
	node lastProcessedBCTreeNode;
	bool cuts;
	IloModel * p_model;

	
	
	
	
	
	void CircularExactLayout::computeBicomp(node bcTreeNode)
	{
	//std::cout <<std::endl <<std::endl <<std::endl << "Now starting to compute a new biconnected Component!" <<std::endl ;
		/************************************************* prepare ILP **************************************************/
		//	the approach to the ILP is as follows:
		//	only edges in the same biconnected component will ever cross, as in the resulting layout all nodes belonging 
		//	to one biconnected component will be placed in a block-cutpoint tree without producing additional crossings
		
		
	//re-create the subgraph induced by the given biconnected component
	

	SList<edge> componentEdges = p_bct->hEdges(bcTreeNode);
	SListIterator<edge> it;


	edge e1,e2;
	node n;
	edge e;
	int shift  = 0;
	
	GraphCopy GC(m_G);
		forall_edges(e, m_G)
		{
			bool inComponentEdges = false;
			
			for (it = componentEdges.begin(); it.valid(); ++it)
			{
				edge theEdge = *it;
				if (e == p_bct->original(theEdge))
				{
					inComponentEdges = true;
				}
			}
			if (!inComponentEdges)
			{
				GC.delEdge(GC.copy(e));
			}
		}
		forall_nodes(n, m_G)
		{
			if (!GC.copy(n)->degree())
			{
				GC.delNode(GC.copy(n));
			}
		}
		
		
			//determine number of Variables
			int n_cols = 0;
			int n_rows_sequence = 0;
			int numberOfNodes = GC.numberOfNodes();
			int numberOfEdges = GC.numberOfEdges();
			NodeArray<int> knotenindex(GC);
			EdgeArray<int> kantenindex(GC);
			node * compNodes = new node[numberOfNodes];

			//create indices for all nodes and edges to identify them by
			int i=0;
			forall_nodes(n, GC)
			{
				knotenindex[n]=i++;
				compNodes[knotenindex[n]] = n;
			} 
			i=0;
			forall_edges(e, GC)
			{
			kantenindex[e]=i++;
			}
			

			if (m_lastCutVertex != NULL && numberOfNodes > 0)
			{
				//check whether the last cut vertex is the last positioned vertex
				int check = 0;
				while (m_nodeOrder[check] != m_knotenindex[m_lastCutVertex])
				{
					check++;
				}
				if (check != m_count)
				{
					int * shiftedNodes = new int[m_count - check];
					for (int i = 0; i < m_count - check; i++)
					{
						shiftedNodes[i] = m_nodeOrder[check + i + 1];
					}
					for (int i = 0; i < m_count - check; i++)
					{
						m_nodeOrder[check + numberOfNodes + i] = shiftedNodes[i];
					}
					shift = m_count - check;
					m_count = check;
					delete [] shiftedNodes;
					shiftedNodes = NULL;
				}
			}

			//if the current bicomp only consists of 2 nodes, we do not have to construct an LP, as its solution would be trivial
			if (!(numberOfNodes > 2))
			{
				if (numberOfNodes == 0)
				{
					m_lastCutVertex = p_bct->original(p_bct->cutVertex(bcTreeNode, bcTreeNode));
				}
				else
				{
				m_processedBicompCount++;
				e = GC.chooseEdge();
				int count = m_count;

				if (m_lastCutVertex != NULL)
				{
					int cutVertexIndex = m_knotenindex[m_lastCutVertex];
					if (m_knotenindex[GC.original(e->source())] == cutVertexIndex)
					{
						m_nodeOrder[m_count] = m_knotenindex[GC.original(e->source())];
						m_nodeOrder[m_count + 1] = m_knotenindex[GC.original(e->target())];
						m_count += shift;
					}
					else
					{
						m_nodeOrder[m_count] = m_knotenindex[GC.original(e->target())];
						m_nodeOrder[m_count + 1] = m_knotenindex[GC.original(e->source())];
						m_count += shift;
					}
				}
				else
				{
				m_nodeOrder[m_count] = m_knotenindex[GC.original(e->source())];
				m_nodeOrder[m_count + 1] = m_knotenindex[GC.original(e->target())];
				}
				m_count++;
				}
			}
			else
			{
				
				m_processedBicompCount++;
				
				//create CPLEX stuff
				IloEnv env;
				IloModel model(env);
				//there are O(|V|^2) variables of the form: "Is node i placed before node j" 
				n_cols += numberOfNodes * (numberOfNodes - 1) / 2;
				//there are O(|E|^2) sequence constraints, which need a fillup, the exact number of fill-variables is
				n_cols += numberOfEdges * (numberOfEdges - 1) / 2;
				int modificator = 0;//this saves modifications to the constraints upper bounds, which arise due to transformations of Xji = 1 - Xij
				IloBoolVarArray var(env,n_cols); /// generiere gewnschte anzahl von 0/1-variables
				model.add(var); /// fge die variablen dem Modell hinzu
				
				//define the objective coefficients - each coefficient is '1' since it's corresponding variable means one crossing
				//this of course only applies to variables corresponding to sequence fill-variables				
				//define the upper and lower bounds of each variable, these are 0 <= Xi <= 1
				//std::cout << "Create LP Columns" <<std::endl;
				IloObjective obj(env,0,IloObjective::Minimize); /// generiere zielfunktion
				for (int i = 0; i < n_cols; i++)
				{
					if (i < numberOfNodes * (numberOfNodes - 1)/2)
					{	
						obj.setLinearCoef(var[i],-0.0001); /// setze die zielfunktion-koeffizienten, avoid cost 0 (more stable)
					}
					else
					{
						obj.setLinearCoef(var[i], 1.0);
					}
				}
				model.add(obj);
				IloCplex cplex(model);
				p_model = &model;

				if (cuts)
				{
					//User wishes to speed up LP solution by adding cuts
					m_numberCompNodes = numberOfNodes;
					m_numberCompEdges = numberOfEdges;
					p_componentCopy = &GC;
					m_componentKnotenindex = &knotenindex;
					m_componentKantenindex = &kantenindex;				
					cplex.use( myCutCallback(env, var, numberOfNodes, numberOfEdges, this) );   /// registrieren vom callback
					cplex.use( heuristicCallback(env, var, this) );
					cplex.setParam(IloCplex::TiLim, 300.0); 
				}				
				//if cuts are not used, the complete LP has to be created here
				else
				{
					//add all constraints
					IloNumArray sol(env);
					
				}
				

/************************************** Solve the ILP *****************************************************/
				SIMPLE_BRANCH_AND_CUT(cplex); // turn off cplex's presolve, heuristic, etc. 
				cplex.solve();
				anzBranchCutNodes += cplex.getNnodes();
				anzNonTrivialBicomps += 1;
				if(cplex.getStatus()!=IloAlgorithm::Optimal)
				{
					cout << "Not solved to provable optimality: Status=" << cplex.getStatus() << "\n";
					m_crossings = 99999;								
				}
				else
				{
						IloNumArray sol(env);
						cplex.getValues(sol, var);
						std::cout << "Objective value is " << cplex.getObjValue() << std::endl;
						m_crossings += (int) (cplex.getObjValue() + 0.5);
						double *solution = new double [n_cols];
						for (int i = 0; i < n_cols; i++)
						{
							solution[i] = sol[i];
						}

						
						//create an upper triangular matrix to hold all Xij of the current bicomp
						//the index of the node itself is saved at the first position of each row
						int ** upperTriangularMatrix = new int * [numberOfNodes];
						int * nodeOrder = new int[numberOfNodes];
						for (int i = 0; i < numberOfNodes; i++)
						{
							upperTriangularMatrix[i] = new int[numberOfNodes - i];
							nodeOrder[i] = -1;
						}

						//fill the just created matrix accordingly
						SListIterator<node> itn, itm;
						int countRow = 0;
						
						for (int i = 0; i < numberOfNodes; i++)
						{
							upperTriangularMatrix[i][0] = m_knotenindex[GC.original(compNodes[i])];
							for (int j = 1; j < numberOfNodes - i; j++)
							{							
								upperTriangularMatrix[i][j] = (int)(solution[ID2POS(i, j + i, numberOfNodes)] + 0.5);
							}
						}

			
						//now compute the order of nodes in the current biconnected component
						for (int i = 0; i < numberOfNodes; i++)
						{
							int numberOfZeros = 0;
							for (int j = 1; j < numberOfNodes - i; j++)
							{
								if (upperTriangularMatrix[i][j] == 0)
								{
									numberOfZeros++;
								}
							}
							int nodePosition = 0;
						
							while (numberOfZeros > 0)
							{
								if (nodeOrder[nodePosition] == -1)
								{
									numberOfZeros--;
									nodePosition++;
								}
								else 
								{
									nodePosition++;
								}
							}
					
							while (nodeOrder[nodePosition] != -1)
							{
								nodePosition++;
							}
							

							int count = m_count;
							nodeOrder[nodePosition] = upperTriangularMatrix[i][0];
						}

						//now we determined a relativ node order of the current biconnected component
						//it has to start with the current cut node
						if (m_lastCutVertex != NULL)
						{
							int cutVertexInArray = -1;
							int count = m_count;
							bool indexFound = false;
							while (!indexFound)
							{
								cutVertexInArray++;
								if (nodeOrder[cutVertexInArray] == m_lastCutVertex->index())
								{
									indexFound = true;
								}
							}
							for (int i = cutVertexInArray; i < numberOfNodes; i++)
							{
								m_nodeOrder[m_count + i - cutVertexInArray] = nodeOrder[i];
							}
							for (int i = 0; i < cutVertexInArray; i++)
							{
								m_nodeOrder[m_count + numberOfNodes - cutVertexInArray + i] = nodeOrder[i];
							}
						}
						else
						{
							for (int i = 0; i < numberOfNodes; i++)
							{
								m_nodeOrder[i] = nodeOrder[i];
							}
						}

						m_count += numberOfNodes - 1;
						m_count += shift;
						for (int i = 0; i < numberOfNodes; i++)
						{
							delete [] upperTriangularMatrix[i];
							upperTriangularMatrix[i] = NULL;
						}
						delete [] upperTriangularMatrix;
						upperTriangularMatrix = NULL;
						delete [] nodeOrder;
						nodeOrder = NULL;
					
			}
			delete [] compNodes;
			compNodes = NULL;
		}
	}
	
	void CircularExactLayout::traverseBCTree(node encounteredNode, node lastCutVertex)
	{
		if (m_crossings < 99999)
		{
			m_lastCutVertex = lastCutVertex;
			computeBicomp(encounteredNode);
			node nextLastCutVertex = m_lastCutVertex;
			adjEntry adj;
			if (encounteredNode->indeg() > 0)
			{
				adj = encounteredNode->firstAdj();
				if (adj->theEdge()->source() != encounteredNode)
					{
						traverseBCTree(adj->theEdge()->source(), nextLastCutVertex);
					}
				while (adj->succ())
				{
					adj = adj->succ();
					if (adj->theEdge()->source() != encounteredNode)
					{
						traverseBCTree(adj->theEdge()->source(), nextLastCutVertex);
					}
				}
			}
		}
	}
	
	void CircularExactLayout::writeLayout()
	{
		node n;
		node theNode;
		GraphAttributes GA(m_G, GraphAttributes::nodeGraphics |GraphAttributes::nodeLabel | GraphAttributes::edgeGraphics );
		double circleFactor = 2* 3.14159265358979323846 / m_G.numberOfNodes();
		for (int i = 0; i < m_G.numberOfNodes(); i++)
		{
			forall_nodes(n, m_G)
			{
				if (m_knotenindex[n] == m_nodeOrder[i])
				{
					theNode = n;
				}
			}
			
			char buffer[128];
			sprintf(buffer, 128, "%d", m_knotenindex[theNode]);
			GA.x(theNode) = 600 + 300 * cos(circleFactor * (double)i);
			GA.y(theNode) = 600 + 300 * sin(circleFactor * (double)i);
			GA.width(theNode) = 20;
			GA.height(theNode) = 20;
			GA.labelNode(theNode) = buffer;
		}
		String theFile = m_theFile;
		GA.writeGML(m_theFile);
	}
	
	//this heuristic works in two phases, it creates an initial layout by inserting nodes one after another in a greedy fashion.
	//After that it switches neighbouring nodes, if that results in fewer crossings. It stops, if the node order is locally optimal.
	int CircularExactLayout::doGreedyInsertionSwitch(GraphAttributes &GA)
	{
		Graph G = GA.constGraph();
		SList<node> nodeOrder;
		SListIterator<node> it;
		node n;
		edge e;
		NodeArray<int> knotenindex(G);
		EdgeArray<int> kantenindex(G);
		int* unplacedNeighbours = new int[G.numberOfNodes()];
		//initialise the number of open edges for each node
		int * openEdges = new int[G.numberOfNodes()];

		//create indices for all nodes and edges to identify them by
		int i=0;
		forall_nodes(n, G)
		{
			unplacedNeighbours[i] = 0;
			openEdges[i] = n->degree();
			knotenindex[n]=i++;
		} 
		i=0;
		forall_edges(e, G)
		{
			kantenindex[e]=i++;
			unplacedNeighbours[knotenindex[e->source()]]++;
			unplacedNeighbours[knotenindex[e->target()]]++;
		}
		
		
		

		//We start with a random vertex	
		//possible improvement to paper: highest degree vertex
		node next = G.chooseNode();
		forall_nodes (n, G)
		{
			if (n->degree() > next->degree())
			{
				next = n;
			}
		}

		nodeOrder.pushFront(next);
		forall_edges(e, G)
		{
			if (e->source() == next)
			{
				unplacedNeighbours[knotenindex[e->target()]]--;
			}
			if (e->target() == next)
			{
				unplacedNeighbours[knotenindex[e->source()]]--;
			}
		}
		//set unplaced Neighbours of already set node "very high"
		unplacedNeighbours[knotenindex[next]] = 9999;

		//there remain (n-1) steps to be done
		for (int i = 0; i < G.numberOfNodes() - 1; i++)
		{
			//look for the node with the least number of unplaced Neighbours
			int neighbourNumber = 9999;
			
			forall_nodes(n, G)
			{
				if (unplacedNeighbours[knotenindex[n]] < neighbourNumber)
				{
					next = n;
					neighbourNumber = unplacedNeighbours[knotenindex[n]];
				}
			}
			//so node next is the next to be placed
			forall_edges(e, G)
			{
				if (e->source() == next)
				{
					unplacedNeighbours[knotenindex[e->target()]]--;
				}
				if (e->target() == next)
				{
					unplacedNeighbours[knotenindex[e->source()]]--;
				}
			}
			unplacedNeighbours[knotenindex[next]] = 9999;
			//now determine where to append next (front or back of list)
			//NOTE: do that in a later stage, for now append at the back of list
			//check how many crossings arise when appending to the left side
			int crossingsLeft = 0;
			int crossingsRight = 0;
			adjEntry adj;
			adj = next->firstAdj();
			node otherNode;
			if (adj->theEdge()->source() == next)
			{
				otherNode = adj->theEdge()->target();
			}
			else
			{
				otherNode = adj->theEdge()->source();
			}
			bool valid;
			do 
			{
				if (adj->theEdge()->source() == next)
				{
					otherNode = adj->theEdge()->target();
				}
				else
				{
					otherNode = adj->theEdge()->source();
				}
				valid = false;
				bool openEdge = true;
				int crossingsWithCurrentEdge = 0;
				for (it = nodeOrder.begin(); it.valid(); ++it)
				{
					if (openEdge)
					{
						if (*it == otherNode)
						{
							openEdge = false;
							crossingsLeft += crossingsWithCurrentEdge;
						}
						else
						{
							crossingsWithCurrentEdge += openEdges[knotenindex[*it]];
						}
					}
				}
				openEdge = true;
				crossingsWithCurrentEdge = 0;
				nodeOrder.reverse();
				for (it = nodeOrder.begin(); it.valid(); ++it)
				{
					if (openEdge)
					{
						if (*it == otherNode)
						{
							openEdge = false;
							crossingsRight += crossingsWithCurrentEdge;
							//adjust number of open Edges
							openEdges[knotenindex[next]]--;
							openEdges[knotenindex[otherNode]]--;
						}
						else
						{
							crossingsWithCurrentEdge += openEdges[knotenindex[*it]];
						}
					}
				}
				nodeOrder.reverse();
				if (adj->succ())
				{
					valid = true;
					adj = adj->succ();
				}
			}
			while (valid);
			if (crossingsLeft <= crossingsRight)
			{
				nodeOrder.pushFront(next);

			}
			else
			{
				nodeOrder.pushBack(next);
			}
		}
		delete [] openEdges;
		openEdges = NULL;
		//we now have an initial layout and can employ a switch heuristic to improve it
		bool improved = true;
		while (improved)
		{
			improved = false;
			node * formerNodeOrder = new node[G.numberOfNodes()];
			int formerCrossings = crossingNumber(formerNodeOrder, G);
		
		}

		node theNode;
		GraphAttributes GAGS(G, GraphAttributes::nodeGraphics |GraphAttributes::nodeLabel | GraphAttributes::edgeGraphics );
		double circleFactor = 2* 3.14159265358979323846 / G.numberOfNodes();
	
		int z = 0;
		for (it = nodeOrder.begin(); it.valid(); ++it)
		{		
			theNode = *it;
			char buffer[128];
			sprintf(buffer, 128, "%d", knotenindex[theNode]);
			GAGS.x(theNode) = 600 + 300 * cos(circleFactor * (double)z);
			GAGS.y(theNode) = 600 + 300 * sin(circleFactor * (double)z);
			GAGS.width(theNode) = 20;
			GAGS.height(theNode) = 20;
			GAGS.labelNode(theNode) = buffer;
			z++;
		}
		String theFile = "GreedySwitch.gml" ;
		GAGS.writeGML(theFile);
		
		delete [] unplacedNeighbours;
		return 0;
	}
	

	//this is a routine to determine crossing number for a given node ordering
	int CircularExactLayout::crossingNumber(node * nodeOrder, Graph &G)
	{
		node n;
		edge e, f;
		NodeArray<int> knotenindex(m_G);
		NodeArray<int> position(m_G);
		EdgeArray<int> kantenindex(m_G);
		int crossingNumber = 0;
		int i, j, k, l;

		//create indices for all nodes and edges to identify them by
		int m=0;
		forall_nodes(n, G)
		{
			knotenindex[n]=m++;
		} 
		m=0;
		forall_edges(e, G)
		kantenindex[e]=m++;

		m=0;
		SListIterator<node> it;
		//array for determining the position of each node as given by the node order
		for (int i = 0; i < G.numberOfNodes(); i++)
		{
			position[nodeOrder[i]] = i;
		}

		forall_edges(e, G)
		{
			forall_edges(f, G)
			{
				//only look at each edge pair once
				if (kantenindex[e] > kantenindex[f])
				{
					//there are 8 possibilites to create a crossing
					bool crossing = false;
					i = position[e->source()];
					j = position[e->target()];
					k = position[f->source()];
					l = position[f->target()];
					if (i < k && k < j && j < l)
						crossing = true;
					if (i < l && l < j && j < k)
						crossing = true;
					if (j < k && k < i && i < l)
						crossing = true;
					if (j < l && l < i && i < k)
						crossing = true;
					if (k < i && i < l && l < j)
						crossing = true;
					if (k < j && j < l && l < i)
						crossing = true;
					if (l < i && i < k && k < j)
						crossing = true;
					if (l < j && j < k && k < i)
						crossing = true;
					if (crossing)
						crossingNumber++;
				}
			}
		
		}
		return crossingNumber;	
	}

	void CircularExactLayout::call(GraphAttributes &GA)
	{
		int crossings = doCall(GA, "myLayout.gml", true);
	}


	//main routine called to produce a circular layout with minimum number of crossings, this function returns the (optimal) number of crossings in the produced layout
	int CircularExactLayout::doCall(GraphAttributes &GA, String theFile, bool useCuts)
	{
		cuts = useCuts;
		EPS = 0.00000001;
		m_theFile = theFile;
		//declare some variables that will be needed throughout the algorithm 
		m_G = GA.constGraph();
		node n;
		edge e;
		m_crossings = 0;
		m_count = 0;
		m_processedBicompCount = 0;
		anzBranchCutNodes = 0;
		anzNonTrivialBicomps = 0;
		
		m_compNodes = new node[m_G.numberOfNodes()];
		m_nodeOrder = new int[m_G.numberOfNodes()];
		//create indices for all nodes and edges to identify them by
		int i=0;
		forall_nodes(n, m_G)
		{
		    m_knotenindex[n]=i++;
			m_compNodes[m_knotenindex[n]] = n;
		} 
		i=0;
		forall_edges(e, m_G)
	    m_kantenindex[e]=i++;

		//Graph empty? Return!
		if (m_G.numberOfNodes() == 0)
			return 0;

		/*********************************************** prepare BC Tree ************************************************/
		GraphCopy GC(m_G);
		BCTree bct(m_G);
		p_bct = &bct;
		node root;
		m_lastCutVertex = NULL;
		forall_nodes(n, bct.bcTree())
		{
			//we need to find the root of the bcTree, this is the node with 0 indegree:
			if ((n->outdeg()) == 0)
			{
				root = n;
			}
		}
		EdgeArray<int> componentNumber(m_G);
		int componentAnzahl = biconnectedComponents(m_G, componentNumber);
		//do a DFS traversal of the tree
		traverseBCTree(root, NULL);
		std::cout  << "Finished computing the problem. The results are:" <<std::endl;
		std::cout << "Final Objective value is " << m_crossings << std::endl;
		node * nodeOrder = new node[m_G.numberOfNodes()];
		int verifiedCrossings = 99999;

		if(m_crossings < 99999)
		{
			forall_nodes(n, m_G)
			{
				for (int i = 0; i < m_G.numberOfNodes(); i++)
				{
					if (m_nodeOrder[i] == m_knotenindex[n])
					{
						nodeOrder[i] = n;
					}
				}
			}
			verifiedCrossings = crossingNumber(nodeOrder, m_G);
			std::cout << "Verifying the number of crossings: " << verifiedCrossings << std::endl;
			delete [] nodeOrder;
			nodeOrder = NULL;
			std::cout << "Node Order is: ";
			for (int i = 0; i < m_G.numberOfNodes(); i++)
			{
				std::cout << m_nodeOrder[i] << ", ";
			}
			std::cout <<std::endl;
			//now that the node order is computed, create the layout
			writeLayout();
		}
		delete [] m_compNodes;
		m_compNodes = NULL;
		delete [] m_nodeOrder;
		m_nodeOrder = NULL;
		return verifiedCrossings;
		
	}//end of routine "call"

}//end of namespace

#endif // OGDF_CONCERT
