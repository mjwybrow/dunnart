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

#ifndef INTERFERENCEGAPH_H
#define INTERFERENCEGAPH_H

#include <utility>
#include <list>
#include <set>
#include <map>


namespace dunnart {


class Connector;

typedef std::set<std::pair<Connector *, Connector *> > ConnPairSet;

namespace interference_graph {
   // each node of the interference graph contains (pointer to) the connector
   // it represents, degree and color of node, and list of adjacent nodes
   // (ie the graph is reprsented as adjacency lists). The degree is stored
   // even though it is just length of adjacency list as we need to sort
   // nodes by it. 
   class Node {
     public:
       Node(Connector *conn) { this->conn = conn; degree = 0; colornum = 0; }
       ~Node() {}

       //attributes
       Connector *conn;                  // connector represented by this node
       std::list<Node *> NodeList;// list of nodes adjacent to this one
       unsigned degree;             // degree of this node (length of NodeList)
       unsigned colornum;           // color of this node (1,2,...)
   };
   typedef std::list<Node *> NodePtrList;

   // The interference graph itself is a list of Node as defined above.
   class InterferenceGraph {
     public:
       InterferenceGraph(ConnPairSet &interfering_conns);
       ~InterferenceGraph();
       unsigned color_graph();      // color graph by Welsh-Powell algorithm

       // attributes
       std::list<Node *> *nodes;  // List of Node objects in this graph

     private:
       // internal methods
       void debug_print();
   };
}

}
#endif // INTERFERENCEGAPH_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

