/*
 * $Revision: 1.2 $
 * 
 * last checkin:
 *   $Author: klein $ 
 *   $Date: 2007-11-14 16:18:10 +0100 (Wed, 14 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Reading a basis. 
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
#include <string.h>

namespace ogdf {

void alps_readbasis(
					char *abfilename,
					int ncols,
					int nrows,
					int *basiccolstat,
					int *basicrowstat
					)
{
	/** Declare alps_readbasis scalars */
	int i,j,k;
	FILE *abfile;
	char basisstatus;
	char keyword[10];


	/** Read the basis */
	abfile = fopen(abfilename,"r");
	if (!abfile) {
		printf("ERROR: Could not open the file.\n");
		exit(1002);
	}
	fgets(keyword,10,abfile);
	if (strncmp(keyword,"COLUMNS",7)) printf("COLUMNS expected, %s found.\n",keyword);
	for (k=0;k<ncols;k++) {
		fscanf(abfile,"%d %c\n",&j,&basisstatus);
		switch (basisstatus) {
	case 'B': basiccolstat[j] = ALPS_BASIC;        break; 
	case 'L': basiccolstat[j] = ALPS_NONBASICLOWB; break;
	case 'U': basiccolstat[j] = ALPS_NONBASICUPB;  break;
	case 'F': basiccolstat[j] = ALPS_NONBASICFREE; break;
		}
	}
	fgets(keyword,10,abfile);
	if (strncmp(keyword,"ROWS",4)) printf("ROWS expected, %s found.\n",keyword);
	for (k=0;k<nrows;k++) {
		fscanf(abfile,"%d %c\n",&i,&basisstatus);
		switch (basisstatus) {
	case 'B': basicrowstat[i] = ALPS_BASIC;        break; 
	case 'L': basicrowstat[i] = ALPS_NONBASICLOWB; break;
	case 'U': basicrowstat[i] = ALPS_NONBASICUPB;  break;
	case 'F': basicrowstat[i] = ALPS_NONBASICFREE; break;
		}
	}
	fgets(keyword,10,abfile);
	if (strncmp(keyword,"END",3)) printf("END expected, %s found.\n",keyword);
}

} // end namespace ogdf
