/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2008  Alex Stivala
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA  02110-1301, USA.
 * 
 *
 * Author(s): Alex Stivala
*/
/*
**  File:    interferencegraph.cpp
**  Created: March 2008
**
**  An interference graph represents relationships between nodes that
**  'interfere' in some way with each other. We want to be able to
**  'color' each node in the graph in such a way that no two nodes
**  that interfere with each other have the same color, with the least
**  number of different colors used (note we will not guarantee the
**  optimally smallest number of colors, it is computationally
**  intractable (NP-complete), we have some approximation instead).
**  This is often known in computer architecture as 'register
**  coloring'.
**
**  We will use the Welsh-Powell algorithm to color the graph. It is a simple
**  heuristic for obtaining a valid graph coloring (though not of course
**  guaranteeing the minimum number of colors is used).
**
**  Concretely, we are using this to color connectors that 'intefere' with
**  each other, i.e. either cross or have a shared path (are too close too
**  each other and eligible for 'nudging' - see visibility.cpp). In this
**  way such connectors will have different colors and therefore are easier
**  for the user to distinguish (there is no confusion as to the following
**  the connectors by eye, whcih can occur when they have the same color).
**  So here the 'color' abstraction/metaphor really is implemented as 
**  color of the connectors.
**
*/

#include <iostream>
#include <list>

#include "libcola/commondefs.h"

#include "libdunnartcanvas/interferencegraph.h"
#include "libdunnartcanvas/connector.h"

namespace dunnart {

using namespace interference_graph;
using namespace std;

//
// Constructor for InterferenceGraph.
//
// Parameters:
//     interfering_conns - ConnPairSet (list of <conn,conn> pairs) to construct
//                        the inteference graph from. Each pair represents
//                        connectors that interfere with each other (crossing
//                        or shared path). So each pair represents an edge
//                        in the interference graph. May contain duplicates
//                        but these are ignored, it is not a multigraph.
//
// 
InterferenceGraph::InterferenceGraph(ConnPairSet &interfering_conns) {
   std::map<Connector *, Node *> connMap;  // map connectors to nodes in graph
   Connector *conn1, *conn2;   // connectors that interfere, from interefering_conns
   Node *node1, *node2;   // nodes (new or existing) corresponding to above

   nodes = new list<Node *>;
   // build the graph by iterating through the list of interfereing connector
   // pairs, creating a node for any connector that does not already have
   // one, and adding an edge between the pairs if there isn't already one.
   for (ConnPairSet::iterator iter = interfering_conns.begin();
        iter != interfering_conns.end(); ++iter)
   {
       conn1 = iter->first;
       conn2 = iter->second;
       node1 = connMap[conn1];
       node2 = connMap[conn2];
       if (!node1) {
           node1 = new Node(conn1);
           nodes->push_back(node1);
           connMap[conn1] = node1;
       }
       if (!node2) {
           node2 = new Node(conn2);
           nodes->push_back(node2);
           connMap[conn2] = node2;
       }
       // The graph is undirected, we represent this by making the adj
       // list represetnation symmetric ie if node1 and node2
       // interfere (are adjacent) then node1 is in node2's adj list
       // and also node2 is in node1's adj list.

       // FIXME: use of find() in detecting duplicate edges is linear,
       // maybe should use a map to make this more efficient
       NodePtrList::iterator node_it = find(node1->NodeList.begin(), 
                                            node1->NodeList.end(), node2);
       if (node_it == node1->NodeList.end() || *node_it != node2) {
           node1->NodeList.push_back(node2);
           node2->NodeList.push_back(node1);
       }
//        else {
//            cout << "duplicate: " << node1->conn->getId() << "," << node2->conn->getId() << endl;
//        }
   }

   // set the degree in each node
   for (NodePtrList::iterator node_iter = nodes->begin();
        node_iter != nodes->end(); ++node_iter) {
       (*node_iter)->degree = (*node_iter)->NodeList.size();
   }
}

//
// Destructor for InterferenceGraph.
//
InterferenceGraph::~InterferenceGraph() {
   for (NodePtrList::iterator node_iter = nodes->begin();
        node_iter != nodes->end(); ++node_iter) {
       delete (*node_iter);
   }
   delete nodes;
}


//
// degree_comp() - compare Node degrees for std::sort()
//
// Parameters:
//    node1 - Node* for first node to compare
//    node2 - Node* for second node to compare
//
// Return value:
//    True if degree(node1) >= degree(node2) else False.
//
// Note used as comparison function to std::sort()
// The fact that we use >= rather than the usual < means that the sort
// will be in descending order instead of ascending.
// 
static bool degree_comp(Node *node1, Node *node2)
{
   return node1->degree >= node2->degree;
}

//
// color_graph
//
// Color the inteference graph so that no two adjancent nodes have the same
// color, attempting to minimize the number of colors used (although not
// guaranteed to use the minimum number, which is an NP-complete problem).
//
// Parameters:
//    None.
//
// Return value:
//    The number of colors used.
//
// Updates data members:
//    nodes (list is sorted)
//    colornum in each Node of the nodes attribute.
//    
// We will use the Welsh-Powell algorithm to color the graph. It is a simple
// heuristic for obtaining a valid graph coloring (though not of course
// guaranteeing the minimum number of colors is used).
// The reference for the Welsh-Powell algorithm is:
//
// @Article{welsh67,
//   author =       {Welsh, D. J. A. and Powell, M. B.},
//   title =        {An upper bound for the chromatic number of a graph and its application to timetabling problems},
//   journal =      {The Computer Journal},
//   year =         1967,
//   volume =       10,
//   number =       1,
//   pages =        {85--86}
// }
// 
// The algorithm is basically:
//    1. Every node is initially uncolored (color 0). Initial color x = 1.
//    2. sort nodelist by degree decreasing.
//    3. for each node (in sorted order):
//      a. give node color x if uncolored and has no neighbor with color x.
//    4. Increment x, and repeat from 3 until no node uncolored.
//
unsigned InterferenceGraph::color_graph() {
#ifdef ADS_DEBUG
   debug_print();
#endif

   // Nodes are already initialized with colornum 0
   // Sort nodes by degree descending
   nodes->sort(degree_comp);

   unsigned colornum = 1;
   bool found_uncolored = false;
   do {
       found_uncolored = false;
       for (NodePtrList::iterator node_iter = nodes->begin();
            node_iter != nodes->end(); ++node_iter) {
           Node *node = *node_iter;
           if (node->colornum == 0) {
               found_uncolored = true;
               bool found_samecolor = false;
               for (NodePtrList::iterator nodelist_iter = node->NodeList.begin();
                    nodelist_iter != node->NodeList.end(); ++nodelist_iter) {
                   Node *adjnode = *nodelist_iter;
                   if (adjnode->colornum == colornum) {
                       found_samecolor = true;
                       break;
                   }
               }
               if (!found_samecolor) {
                   node->colornum = colornum;
               }
           }
       }
       colornum++;
   } while (found_uncolored);
#ifdef ADS_DEBUG
   debug_print();
   cout << "used " << colornum-2 << " colors\n" << endl;
#endif
   return colornum-2;
}


//
// Print graph to stdout for debugging
//
// Parameters:
//   None
// Return value:
//   None
//
void InterferenceGraph::debug_print() {
   for (NodePtrList::iterator node_iter = nodes->begin();
        node_iter != nodes->end(); ++node_iter) {
       cout << *node_iter << ' ';
       cout << (*node_iter)->conn->internalId() << "(" << (*node_iter)->degree
            << ")[" << (*node_iter)->colornum << "] : ";
       for (NodePtrList::iterator nodelist_iter=(*node_iter)->NodeList.begin();
            nodelist_iter != (*node_iter)->NodeList.end(); ++nodelist_iter) {
           Node *node = *nodelist_iter;
           cout << node->conn->internalId() << "[" << node->colornum << "] ";
       }
       cout << endl;
   }
}


}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

