/*
 * $Revision: 1.3 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-13 18:03:43 +0100 (Tue, 13 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class MinimumEdgeDistances which maintains
 *        minimum distances between attached edges at a vertex
 *       (delta's and epsilon's)
 * 
 * \author Carsten Gutwenger
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


#ifndef OGDF_MINIMUM_EDGE_DISTANCE_H
#define OGDF_MINIMUM_EDGE_DISTANCE_H


#include <ogdf/orthogonal/OrthoRep.h>


namespace ogdf {


//---------------------------------------------------------
// MinimumEdgeDistances
// maintains input sizes for improvement compaction (delta's
// and epsilon's)
//---------------------------------------------------------
template <class ATYPE>
class MinimumEdgeDistances {
public:
	// constructor
	MinimumEdgeDistances(const Graph &G, ATYPE sep) : m_delta(G), m_epsilon(G)
	{
		m_sep = sep;
	}

	// returns delta_s(v)^i (with i = 0 => l, i = 1 => r)
	const ATYPE &delta(node v, OrthoDir s, int i) const {
		OGDF_ASSERT(0 <= int(s) && int(s) <= 3 && 0 <= i && i <= 1);
		return m_delta[v].info[s][i];
	}

	ATYPE &delta(node v, OrthoDir s, int i) {
		OGDF_ASSERT(0 <= int(s) && int(s) <= 3 && 0 <= i && i <= 1);
		return m_delta[v].info[s][i];
	}

	// returns epsilon_s(v)^i (with i = 0 => l, i = 1 => r)
	const ATYPE &epsilon(node v, OrthoDir s, int i) const {
		OGDF_ASSERT(0 <= int(s) && int(s) <= 3 && 0 <= i && i <= 1);
		return m_epsilon[v].info[s][i];
	}

	ATYPE &epsilon(node v, OrthoDir s, int i) {
		OGDF_ASSERT(0 <= int(s) && int(s) <= 3 && 0 <= i && i <= 1);
		return m_epsilon[v].info[s][i];
	}

	ATYPE separation() const {
		return m_sep;
	}

	void separation(ATYPE sep) {m_sep = sep;}


private:
	struct InfoType {
		ATYPE info[4][2];
	};

	NodeArray<InfoType> m_delta;
	NodeArray<InfoType> m_epsilon;
	ATYPE m_sep;
};


} // end namespace ogdf


#endif
