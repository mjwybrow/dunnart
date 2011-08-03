/*
 * $Revision: 1.4 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2007-11-09 12:12:41 +0100 (Fri, 09 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of class Overlap
 * 
 * \author Rene Weiskircher
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


#include <ogdf/internal/energybased/Overlap.h>

namespace ogdf {
	
	Overlap::Overlap(GraphAttributes &AG) : NodePairEnergy("Overlap",AG){}

double Overlap::computeCoordEnergy(node v1, node v2, const DPoint &p1, const DPoint &p2)
const
{
	IntersectionRectangle i1(shape(v1)), i2(shape(v2));
	i1.move(p1);
	i2.move(p2);
	IntersectionRectangle intersection = i1.intersection(i2);
	double area = intersection.area();
	if(area < 0.0) {
		OGDF_ASSERT(area > -0.00001);
		area = 0.0;
	}
	double minArea = min(i1.area(),i2.area());
	double energy = area / minArea;
	return energy;
}

}// namespace ogdf

