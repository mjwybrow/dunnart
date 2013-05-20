/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2011  Monash University
 *
 * --------------------------------------------------------------------
 * Computation of Strongly Connected Components is based upon the
 * method described in:
 *     Tarjan, R. E. (1972), Depth-first search and linear graph
 *     algorithms. SIAM Journal on Computing 1(2), pages 146-–160.
 * --------------------------------------------------------------------
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
 * Author: Michael Wybrow <mjwybrow@users.sourceforge.net>
*/

#include "libdunnartcanvas/stronglyconnectedcomponents.h"
#include "libdunnartcanvas/graphdata.h"
#include "libdunnartcanvas/connector.h"

namespace dunnart {

SCCDetector::SCCDetector()
    : k_undefined(-1)
{
}

QVector<int> SCCDetector::stronglyConnectedComponentIndexes(GraphData *graph)
{
    m_graph = graph;
    m_index = 0;
    m_scc_index = 0;
    int shapesCount = m_graph->shape_vec.size();

    m_indexes = QVector<int>(shapesCount, -1);
    m_scc_indexes = QVector<int>(shapesCount, 0);
    m_lowlinks = QVector<int>(shapesCount, 0);
    m_in_stack = QVector<bool>(shapesCount, false);
    m_stack.clear();

    for (int i = 0; i < shapesCount; ++i)
    {
        if (m_indexes[i] == -1)
        {
            strongConnect(i);
        }
    }
    return m_scc_indexes;
}


void SCCDetector::strongConnect(uint v)
{
    // Set the depth index for v to the smallest unused index
    m_indexes[v] = m_index;
    m_lowlinks[v] = m_index;
    m_index++;
    m_stack.push(v);
    m_in_stack[v] = true;

    // Consider successors of v
    for (uint edgeInd = 0; edgeInd < m_graph->conn_vec.size(); ++edgeInd)
    {
        // for each (v, w) in E
        cola::Edge edge = m_graph->edges[edgeInd];
        if ( (edge.first == v) && m_graph->conn_vec[edgeInd]->isDirected() &&
                m_graph->conn_vec[edgeInd]->obeysDirectedEdgeConstraints() )
        {
            int w = edge.second;
            if (m_indexes[w] == k_undefined)
            {
                // Successor w has not yet been visited; recurse on it
                strongConnect(w);
                m_lowlinks[v] = qMin(m_lowlinks[v], m_lowlinks[w]);
            }
            else if (m_in_stack[w])
            {
                // Successor w is in stack S and hence in the current SCC
                m_lowlinks[v] = qMin(m_lowlinks[v], m_indexes[w]);
            }
        }
    }

    // If v is a root node, pop the stack and generate an SCC
    if (m_lowlinks[v] == m_indexes[v])
    {
        // start a new strongly connected component
        m_scc_index++;
        uint w;
        do
        {
            w = m_stack.pop();
            m_in_stack[w] = false;
            // add w to current strongly connected component
            m_scc_indexes[w] = m_scc_index;
        }
        while (w != v);
    }
}

}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
