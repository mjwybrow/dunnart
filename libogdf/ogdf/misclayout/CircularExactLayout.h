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


#ifdef _MSC_VER
#pragma once
#endif


#ifndef OGDF_CIRCULAR_EXACT_LAYOUT_H
#define OGDF_CIRCULAR_EXACT_LAYOUT_H

#ifdef OGDF_CONCERT
#include <ogdf/module/LayoutModule.h>
#include <ogdf/external/coin.h>
#include <ogdf/decomposition/BCTree.h>
#ifdef _MSC_VER
#include <windows.h>
#include <ostream>
#define ILOUSESTL
#define ILO_WINDOWS
using namespace std;
#endif


#include <ilcplex/ilocplex.h>
#include <vector>
#endif
namespace ogdf {

class OGDF_EXPORT GraphCopyAttributes;


/**
 * \brief Crossing minimal circular layout .
 * 
 * This class provides a layout algorithm for crossing minimal circular layouts.\n
 * Please note, that in order to use this class, you need to define OGDF_CONCERT within your makefile.
 * Also, you need to link your application against CPLEX and CPLEX CONCERT.
 * This class is only tested for use with concert23 and cplex10.1\n
 * The layout algorithm first divides the graph into its biconnected components - using BCTree, after which
 * it creates an ILP-Formulation for each biconnected component, which is solved via cplex' Branch&Cut,
 * using an Heuristic- and Cut-Callback. From each ILP-solution a separate crossing minimal circular layout is 
 * generated. These layouts are then combined to a layout for the original graph, using the BCTree information.
 */
class OGDF_EXPORT CircularExactLayout : public LayoutModule
{
public:
/**
 * \brief The number of nodes in the bicomp currently being layouted.
 */
	int m_numberCompNodes;
/**
 * \brief The number of edges in the bicomp currently being layouted.
 */
	int m_numberCompEdges;
/**
 * \brief A small epsilon value for use with violated constraints - preventing rounding errors.
 */
	double EPS;
/**
 * \brief Number of nodes of the Branch Tree, the last ILP solution evaluated.
 */
	int anzBranchCutNodes;
/**
 * \brief Number of ILPs solved in the layout process so far.
 */
	int anzNonTrivialBicomps;

#ifdef OGDF_CONCERT
/**
 * \brief Pointer to a copy of the biconnected component currently being layouted
 */
	Graph * p_componentCopy;
/**
 * \brief A numbering of the current bicomp's nodes.
 */
	NodeArray<int> * m_componentKnotenindex;
/**
 * \brief A numbering of the current bicomp's edges.
 */
	EdgeArray<int> * m_componentKantenindex;
#endif
/**
 * \brief Creates an instance of circular exact layout.
 */
	CircularExactLayout() : LayoutModule(){ }
/**
 * \brief destructor
 */
	virtual ~CircularExactLayout() { }

#ifdef OGDF_CONCERT
/**
 * \brief This function starts the layout algorithm.
 */
void call(GraphAttributes &GA);
/**
 * \brief Another way to start the layout algorithm, returning the number of crossingscd  (deprecated), please use call() instead.
 */
int doCall(GraphAttributes &GA, String theFile, bool useCuts);
/**
 * \brief A greedy heuristic to create a circular layout inspired by a paper of Baur&Brandes.
 */
int doGreedyInsertionSwitch(GraphAttributes &GA);
/**
 * \brief A helper function, which computes the number of crossings in the current layout determined by a given order of nodes.
 */
int crossingNumber(node * nodeOrder, Graph &G);
 
private:
/**
 * \brief Traverses the BCTree and calls layout function for each bicomp accordingly
 */
void traverseBCTree(node encounteredNode, node lastCutVertex);
/**
 * \brief This does the actual layout of each biconnected component by creating the ILP, calling CPLEX to solve it and calculating the appropriate node order.
 */
void computeBicomp(node bcTreeNode);
/**
 * \brief Helper function to write the .gml file directly 
 */
void writeLayout();

#endif
};

}

#endif
