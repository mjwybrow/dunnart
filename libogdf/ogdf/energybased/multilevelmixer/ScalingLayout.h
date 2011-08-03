/*
 * $Revision: 1.0 $
 *
 * last checkin:
 *   $Author: bartel $
 *   $Date: 2008-01-26 15:44:58 +0100 (Sa, 26 Jan 2008) $
 ***************************************************************/

/** \file
 * \brief ScalingLayout scales and calls a secondary layout
 *
 * \author Gereon Bartel
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

#ifndef OGDF_SCALING_LAYOUT_H
#define OGDF_SCALING_LAYOUT_H

#include <ogdf/module/LayoutModule.h>
#include <ogdf/internal/energybased/MultilevelGraph.h>

namespace ogdf {

class OGDF_EXPORT ScalingLayout : public ogdf::LayoutModule
{
public:
	enum ScalingType {
		st_relativeToDrawing,      // scales by a factor relative to the drawing
		st_relativeToAvgLength,    // scales by a factor relative to the avg edge weights
								   // to be used in combination with the fixed edge length
								   // setting in ModularMultilevelMixerLayout  
		st_relativeToDesiredLength // scales by a factor relative to the desired Edgelength m_desEdgeLength
	};

	ScalingLayout();

	void call(GraphAttributes &GA);
	void call(MultilevelGraph &MLG);

	void setScaling(float min, float max);
	void setExtraScalingSteps(unsigned int steps);
	void setSecondaryLayout(LayoutModule * layout);
	void setScalingType(ScalingType type);
	void setLayoutRepeats(unsigned int repeats);
	//TODO: only a workaround, this should be retrieved from the layout module
	//when we have a interface class on top of Layoutmodule that allows this
	void setDesiredEdgeLength(double eLength);

private:

	// Usually a simple force-directed / energy-based Layout should be chosen.
	LayoutModule * m_secondaryLayoutModule;

	float m_minScaling;
	float m_maxScaling;
	double m_desEdgeLength;

	// 0 = scale to maxScaling only
	unsigned int m_extraScalingSteps;

	unsigned int m_layoutRepeats;

	ScalingType m_scalingType;
};

} // namespace ogdf

#endif
