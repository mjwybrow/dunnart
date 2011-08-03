/*
 * $Revision: 1.2 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Writing a basis. This is our own format, we should 
 * change this to MPS format some time.
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

namespace ogdf {

int alps_writebasis(
					lpstruct *lp,
					char *abfilename
					)
{
	/** Declare alps_writebasis scalars */

	int j,row_ind_of_slack_art,lpcj;
	FILE *abfile;


	/** Write the basis */

	abfile = fopen(abfilename,"w");
	if (!abfile) {
		printf("ERROR: Could not open the file.\n");
		exit(1002);
	}
	fprintf(abfile,"COLUMNS\n");
	for (j=0;j<lp->inorig;j++) {
		fprintf(abfile,"%d",j);
		lpcj = lp->colstat[j];
		switch (lpcj) {
	case ALPS_BASIC        : fprintf(abfile," B\n"); break;
	case ALPS_NONBASICLOWB : fprintf(abfile," L\n"); break;
	case ALPS_NONBASICUPB  : fprintf(abfile," U\n"); break;
	case ALPS_NONBASICFREE : fprintf(abfile," F\n"); break;
		}
	}
	fprintf(abfile,"ROWS\n");
	for (j=lp->inorig;j<lp->in;j++) {
		row_ind_of_slack_art = lp->imatcolind[lp->imatcolbeg[j]];
		fprintf(abfile,"%d",row_ind_of_slack_art);
		lpcj = lp->colstat[j];
		switch (lpcj) {
	case ALPS_BASIC        : fprintf(abfile," B\n"); break;
	case ALPS_NONBASICLOWB : if (lp->aconstrainttype[row_ind_of_slack_art]=='G')
								 fprintf(abfile," U\n"); 
							 else
								 fprintf(abfile," L\n"); 
		break;
	case ALPS_NONBASICUPB  : printf(" Slack or artificial at upper bound!\n"); break;
	case ALPS_NONBASICFREE : printf(" Free slack or artificial!\n"); break;
		}
	}
	fprintf(abfile,"END\n");
	fclose(abfile);

	return 0;
}

} // end namespace ogdf
