/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief The routine for unloading the problem. This is only a 
 * preliminary version.
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

int alps_unloadproblem(
					   lpstruct *lp
					   ) 

{
	ffree((char **)&lp->amatcolbeg);
	ffree((char **)&lp->amatcolcount);
	ffree((char **)&lp->amatcolind);
	ffree((char **)&lp->amatcolcoeff);
	ffree((char **)&lp->aupperbound);
	ffree((char **)&lp->alowerbound);
	ffree((char **)&lp->ax);
	ffree((char **)&lp->aslack);
	ffree((char **)&lp->acolstat);
	ffree((char **)&lp->arhs);
	ffree((char **)&lp->aobj);
	ffree((char **)&lp->aconstrainttype);
	ffree((char **)&lp->rowscale);
	ffree((char **)&lp->colscale);
	ffree((char **)&lp->imatcolbeg);
	ffree((char **)&lp->imatcolcount);
	ffree((char **)&lp->imatcolind);
	ffree((char **)&lp->imatcolcoeff);
	ffree((char **)&lp->imatrowbeg);
	ffree((char **)&lp->imatrowcount);
	ffree((char **)&lp->imatrowind);
	ffree((char **)&lp->imatrowcoeff);
	ffree((char **)&lp->iupperbound);
	ffree((char **)&lp->ilowerbound);
	ffree((char **)&lp->iorigupperbound);
	ffree((char **)&lp->ioriglowerbound);
	ffree((char **)&lp->iartifbounds);
	ffree((char **)&lp->varstat);
	ffree((char **)&lp->origvarstat);
	ffree((char **)&lp->irhs);
	ffree((char **)&lp->iphase1obj);
	ffree((char **)&lp->iphase2obj);
	ffree((char **)&lp->reducedcost);
	ffree((char **)&lp->u);
	ffree((char **)&lp->z);
	ffree((char **)&lp->zsval);
	ffree((char **)&lp->zsind);
	ffree((char **)&lp->colstat);
	ffree((char **)&lp->basisheader);
	ffree((char **)&lp->basmatbeg);
	ffree((char **)&lp->basmatcount);
	ffree((char **)&lp->basmatind);
	ffree((char **)&lp->basmatcoeff);
	ffree((char **)&lp->etacol);
	ffree((char **)&lp->etaval);
	ffree((char **)&lp->etaind);
	ffree((char **)&lp->etastart);
	ffree((char **)&lp->etacount);
	ffree((char **)&lp->eta);
	ffree((char **)&lp->x);
	ffree((char **)&lp->redcost);
	ffree((char **)&lp->y);
	ffree((char **)&lp->ysval);
	ffree((char **)&lp->ysind);
	ffree((char **)&lp->d);
	ffree((char **)&lp->dsval);
	ffree((char **)&lp->dsind);
	ffree((char **)&lp->cand);
	ffree((char **)&lp->recoheapp);
	ffree((char **)&lp->recoheappindex);

	return 0;
}

} // end namespace ogdf
