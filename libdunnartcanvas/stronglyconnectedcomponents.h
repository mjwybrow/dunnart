/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2011  Monash University
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

#include <QVector>
#include <QStack>

#ifndef STRONGLYCONNECTEDCOMPONENTS_H
#define STRONGLYCONNECTEDCOMPONENTS_H

namespace dunnart {

class GraphData;

class SCCDetector
{
    public:
        SCCDetector();
        QVector<int> stronglyConnectedComponentIndexes(GraphData *graph);

    private:
        void strongConnect(int v);

        const int k_undefined;
        int m_index;
        int m_scc_index;
        GraphData *m_graph;
        QVector<int> m_indexes;
        QVector<int> m_scc_indexes;
        QVector<int> m_lowlinks;
        QVector<bool> m_in_stack;
        QStack<int> m_stack;
};

}

#endif // STRONGLYCONNECTEDCOMPONENTS_H
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent
