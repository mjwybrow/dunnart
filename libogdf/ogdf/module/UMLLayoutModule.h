/*
 * $Revision: 1.3 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-01-17 22:56:41 +0100 (Thu, 17 Jan 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Declaration of interface for layout algorithms for
 *        UML diagrams.
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

#ifndef OGDF_UML_LAYOUT_MODULE_H
#define OGDF_UML_LAYOUT_MODULE_H


#include <ogdf/module/LayoutModule.h>
#include <ogdf/basic/UMLGraph.h>


namespace ogdf {


/**
 * \brief Interface of UML layout algorithms.
 */
class OGDF_EXPORT UMLLayoutModule : public LayoutModule
{
public:
	//! Initializes a UML layout module.
	UMLLayoutModule() { }

	virtual ~UMLLayoutModule() { }

	/**
	 * \brief Computes a layout of UML graph \a umlGraph
	 *
	 * Must be implemented by derived classes.
	 * @param umlGraph is the input UML graph and has to be assigned the UML layout.
	 */
	virtual void call(UMLGraph &umlGraph) = 0;

	/**
	 * \brief Computes a layout of UML graph \a umlGraph
	 *
	 * @param umlGraph is the input UML graph and has to be assigned the UML layout.
	 */
	void operator()(UMLGraph &umlGraph) { call(umlGraph); }

	OGDF_MALLOC_NEW_DELETE
};


} // end namespace ogdf


#endif
