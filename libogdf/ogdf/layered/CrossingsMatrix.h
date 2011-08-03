/*
 * $Revision: 1.2 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-01-17 22:56:41 +0100 (Thu, 17 Jan 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of class CrossingsMatrix.
 * 
 * \author Andrea Wagner
 *         Michael Schulz
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

#ifndef OGDF_CROSSINGS_MATRIX_H
#define OGDF_CROSSINGS_MATRIX_H

#include <ogdf/basic/EdgeArray.h>
#include <ogdf/basic/Array2D.h>
#include <ogdf/layered/Hierarchy.h>

namespace ogdf
{

//---------------------------------------------------------
// CrossingsMatrix
// implements crossings matrix which is used by some
// TwoLayerCrossingMinimization heuristics (e.g. split) 
//---------------------------------------------------------
class OGDF_EXPORT CrossingsMatrix
{
public:
	CrossingsMatrix() : matrix(0,0,0,0) {
		m_bigM = 10000;
	}
	
	CrossingsMatrix(const Hierarchy &H);

	~CrossingsMatrix() { }

	double operator()(int i, int j) const
	{
		return matrix(map[i],map[j]);
	}

	void swap(int i, int j)
	{
		map.swap(i,j);
	}

	//! ordinary init
	void init(Level &L);

	//! SimDraw init
	void init(Level &L, const EdgeArray<unsigned int> *edgeSubGraph);

private:
	Array<int> map;
	Array2D<int> matrix;
	//! need this for SimDraw to grant epsilon-crossings instead of zero-crossings
	int m_bigM; // is set to some big number in both constructors
};

}// end namespace ogdf

#endif
