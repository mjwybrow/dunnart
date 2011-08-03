/*
 * $Revision: 1.3 $
 * 
 * last checkin:
 *   $Author: chimani $ 
 *   $Date: 2008-02-20 20:33:07 +1100 (Wed, 20 Feb 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief The NodeComparer compares nodes on base of the value
 *        stored in a node array given as constructor parameter
 * 
 * \author Karsten Klein
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


#ifndef OGDF_NODECOMPARER_H
#define OGDF_NODECOMPARER_H

#include <ogdf/basic/basic.h>

namespace ogdf {

	template<class sorterType>
	class NodeComparer
	{
	public: 
		NodeComparer(NodeArray<sorterType> &valArray, bool ascending = true)
			: m_valArray(&valArray)
		{
			if (ascending) 
			{
				m_smaller = -1;
//				m_greater = 1;
			}//if
			else 
			{
				m_smaller = 1;
//				m_greater = -1;
			}

		}//constructor

		int compare(const node &v1, const node &v2) const 
		{
			if ((*m_valArray)[v1] < (*m_valArray)[v2]) return m_smaller;
			else if ((*m_valArray)[v1] > (*m_valArray)[v2]) return -m_smaller;//m_greater;
			else return 0;
		}

		void init(NodeArray<sorterType> &valArray) {m_valArray = &valArray;}

		OGDF_AUGMENT_COMPARER(node)
		
	private:
		NodeArray<sorterType> *m_valArray;
		int m_smaller;
//		int m_greater;

	};//NodeComparer


}//namespace

#endif

