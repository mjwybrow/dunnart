/* 
 * File:   KandinskyLayout.h
 * Author: moritz
 *
 * Created on January 28, 2009, 5:26 PM
 */

/** \file
 * \brief KandinskyLayout for orthogonal planar drawings of
 *          graphs with arbitary degree using special bends
 *          at nodes.
 *
 * \author Moritz Schallaböck
 *
 * \par License:
 * This file is part of the Open Graph Drawing Framework (OGDF).
 * Copyright (C) 2005-2009
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

#ifndef OGDF_KANDINSKY_LAYOUT_H
#define OGDF_KANDINSKY_LAYOUT_H

#ifdef COIN_OSI_SYM
#include <ogdf/basic/Graph_d.h>
#include <ogdf/basic/GraphAttributes.h>
#include <ogdf/basic/GraphCopy.h>
#include <ogdf/basic/CombinatorialEmbedding.h>

#include <ogdf/module/LayoutPlanRepModule.h>

#include <ogdf/external/coin.h>

namespace ogdf {

    //! The class \a KandinskyLayout creates a orthogonal, planar drawing of an input \a PlanRepUML. For nodes with a degree > 4, bends at the node
    //! are generated. The orthogonal representation is computed using an Integer Program. The \A OrthoRep is compacted using \a FlowCompaction.
    //! Requires the SYMPHONY IP solver.
    class KandinskyLayout : public LayoutPlanRepModule {
    public:
        // Constructor
        KandinskyLayout();

        //! Calls the layout algorithm. Input is a planarized representation \a PG, output is the layout \a drawing. Uses an IP to compute an orthogonal represantion of \a PG.
        void call(PlanRepUML &PG, adjEntry adjExternal, Layout &drawing);

    protected:

        //! Returns the minimal allowed distance between edges and vertices.
        double separation() const;
        //! Sets the minimal allowed distance between edges and vertices to \a sep.
        void separation(double sep);

        //! Creates the extension graph \H for the input \PlanRep, which has the property of having a degree <= 4 required for the compaction step.
        void createExtension(CombinatorialEmbedding& emb, const double * sol, const int ncols, 
                Graph & H, CombinatorialEmbedding& embH,  AdjEntryArray<int> &angles, AdjEntryArray<BendString> &bends,
                NodeArray<node> & nodeRefs, NodeArray<node> & nodeRefsBack, EdgeArray<edge> & edgeRefs, EdgeArray<edge> & edgeRefsBack, AdjEntryArray<double>& adjEntryOffsets, AdjEntryArray<int>& adjEntryNumNeighbours);

        //! Assigns coordinates to the nodes and edges of the extension graph using \a FlowCompaction. The resulting \a drawing needs to be adjusted to arrive at a drawing for the input graph of the main call.
        void draw(PlanRep &PG, OrthoRep &OR, CombinatorialEmbedding &E, adjEntry adjExternal, Layout &drawing);

        //! Generate an IP yielding the data for an orthogonal representation for \a emb.
        OsiSolverInterface * getIP(CombinatorialEmbedding &emb);

    private:
        inline BendString reverseBendString(const BendString & bs);
        inline void adjustPackedVector(CoinPackedVector &vector, const int index, const double adjust);
        inline int getAngleFor(adjEntry ae, const double * sol, const int E);
        inline int getBendFor(adjEntry ae, const double * sol, const int E);
        inline BendString getBendString(adjEntry ae, const double * sol, const int E);

        bool opt1;
        bool alwaysCenterStraight;
        double sep;

    };


} // end namespace ogdf
#endif // OSI_SYM

#endif	/* OGDF_KANDINSKY_LAYOUT_H */
