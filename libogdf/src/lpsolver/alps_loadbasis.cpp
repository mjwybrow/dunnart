/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-11-27 03:02:28 +1100 (Thu, 27 Nov 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Loading a basis.
 * 
 * \author Michael Juenger, Gerhard Reinelt
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

#include "alps.h"
#include "alps_misc.h"

namespace ogdf {

int alps_loadbasis(
				   lpstruct *lp,
				   int /* pril */,
				   int *basiccolstat,
				   int *basicrowstat
				   )
{
	/** Declare alps_loadbasis scalars */
	int i,j,brsi;

	/** Allocate space for external basis */
	if (lp->basisstatus == ALPS_EXTBASISAVAILABLE) {
		ffree((char **) &lp->extcolstat);
		ffree((char **) &lp->extrowstat);
	}
	lp->extcolstat = (int *) malloc(lp->an * sizeof(int));
	lp->extrowstat = (int *) malloc(lp->am * sizeof(int));
	if (!lp->extcolstat || !lp->extrowstat)
		// run out of memory
	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}

	/** Copy the data */
	for (j=0;j<lp->an;j++) {
		lp->extcolstat[j] = basiccolstat[j];
	}
	for (i=0;i<lp->am;i++) {
		brsi = basicrowstat[i];
		if (brsi==ALPS_NONBASICUPB) brsi = ALPS_NONBASICLOWB;
		lp->extrowstat[i] = brsi;
	}
	lp->basisstatus = ALPS_EXTBASISAVAILABLE;

	return 0;
}

} // end namespace ogdf
