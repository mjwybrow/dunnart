/*
 * $Revision: 1.3 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-05-18 23:35:29 +1000 (Sun, 18 May 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Input of a problem in LP format.
 * 
 * We need this module for reading problems in LP format. No 
 * preprocessing is done here, except for the possibility to 
 * ignore void rows. Decimal numbers as variable names are
 * allowed. The layout of this module is as follows.
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
 
/** Global definitions */
/*
*  We need some include files, constants for error messages, constants
* for classifying input tokens, and some buffer sizes.
*/
#include "alps.h"
#include "alps_misc.h"
#include "math.h"
#include "stdio.h"
#include "string.h"

#define ALPS_LPREAD_NOFILE       400000
#define ALPS_LPREAD_NOMEM        400001
#define ALPS_LPREAD_EMPTYFILE    400002
#define ALPS_LPREAD_NAMEINAVLID  400003
#define ALPS_LPREAD_NOROWLABEL   400004
#define ALPS_LPREAD_INVALIDROW   400005
#define ALPS_LPREAD_INVALIDINPUT 400006
#define ALPS_LPREAD_INVALIDBOUND 400007
#define ALPS_LPREAD_SUBJECTERROR 400008
#define ALPS_LPREAD_INVALIDOBJ   400009

#define KEYWORD                  500000
#define WORD                     500001
#define OPPLUS                   500002
#define OPMINUS                  500003
#define LE                       500004
#define GE                       500005
#define EQ                       500006
#define NUM                      500007
#define LABEL                    500008
#define EOI                      500009

#define BUFFER_SIZE              200000
#define TMP_BUFFER_SIZE           20000

#define ALPS_LPREAD_INFINITY    ALPS_REAL_INFINITY

namespace ogdf {

/*
*  Some definitions for accessing the input data. Since the input buffer
* is accessed in a circular way we define a macro for incrementing
* pointers.
*/

//FILE *fopen();
//size_t fread();

FILE *fp;
int endfile;

char *buf;
char *tmpbuf;
char *itembuf;
int lbuf, ltbuf;
int np, ep, op;

#define incr(p) {if (++p>=lbuf) p = 0;}
#define allownumvars 1

/*
*  We use a red-black tree for accessing the variables by name. (It is
* probably advisable to change to hashing in a future version.) The
* following are global definitions for this. Variable names are stored
* in the array |(*colnamestr)| and are accessed by offsets. We use a
* global variable to access the strings.
*/
#define RED 00
#define BLACK 01

int*vptr;
unsigned*color;
int*father;
int*leftnode;
int*rightnode;
int*varindex;

#define RBNIL 0
int T;

/*
*  Further global definitions.
*/
int maxcurnvars,maxcurnrows,maxcurnonz,maxcurnchars,maxcurrowchars;
int curnvars,curnrows,curnonz,curnchars,currowchars;
char *globstr;
int *globind;


/** Functions for input buffer management */
/*
*  To avoid too many file accesses we store all input into buffer |buf|.
* It is filled at the beginning (if the input file is long enough). As
* soon as at least |ltbuf| characters have been read from this buffer
* further input is read (in portions of at most |ltbuf| characters).
* Pointers |np| and |ep| point to the character to be read next,
* respectively to the last character read from the input file.
*/
int
fill_buffer()
{
	int i,nitems;

	if (endfile) return 0;
	if (ep<np) {
		if (np-ep<ltbuf+10) return 0;
	}
	else {
		if (lbuf-ep+np<ltbuf+10) return 0;
	}

	nitems = (int) fread(tmpbuf,1,ltbuf,fp);
	if (!nitems) {
		incr(ep);
		buf[ep] = '\0';
		endfile = 1;
		return 1;
	}
	else {
		for (i=0;i<nitems;i++) {
			incr(ep);
			buf[ep] = tmpbuf[i];
		}
		if (nitems<ltbuf) {
			endfile = 1;
			incr(ep);
			buf[ep] = '\0';
		}
		return 0;
	}
}

void
backup_buffer()
{
	np = op;
}


/** Functions for scanning the input */
/*
*  The function |getnextitem| scans the input buffer and returns the
* next token. This is classified (in |itemtype|) and returned in the
* buffer |item| as a string. |ptr| gives the length of this string which
* is termined by zero. If the file contains numerical data that cannot
* be interpreted by the function |scanf()| this will lead to errors that
* are not detected here.
*/
int 
getnextitem(
			int pril,
			int *itemtype, 
			char *item, 
			int *ptr
			)
{
	int nch, ip;
	char ch;

	if (!endfile) fill_buffer();
	op = np;
	ip = 0;
	while ((ch=buf[np])==' ' || ch=='\n') incr(np);
	if ((int) ch == 0) {
		*itemtype = EOI;
		if (pril>5) printf("EOI ");
		return 0;
	}
	nch = (int) ch - '0';
	if (ch=='+') {
		*itemtype = OPPLUS;
		item[ip++] = ch;
		incr(np);
		if (pril>5) printf("OPPLUS ");
	}
	else if (ch=='-') {
		*itemtype = OPMINUS;
		item[ip++] = ch;
		incr(np);
		if (pril>5) printf("OPMINUS ");
	}
	else if (nch>=0 && nch<=9) {
		// Scan digits

		while ( nch>=0 && nch<=9 ) {
			item[ip++] = ch;
			incr(np);
			ch = buf[np];
			nch = (int) ch - '0';
		}


		if (ch==' ' || ch=='\n') {
			*itemtype = NUM;
			if (pril>5) printf("NUM ");
		}
		else {
			if (ch=='.') {
				item[ip++] = ch;
				incr(np);
				ch = buf[np];
				nch = (int) ch - '0';
				// Scan digits

				while ( nch>=0 && nch<=9 ) {
					item[ip++] = ch;
					incr(np);
					ch = buf[np];
					nch = (int) ch - '0';
				}


			}
			if (ch=='E' || ch=='e' || ch=='D' || ch=='d') {
				item[ip++] = ch;
				incr(np);
				ch = buf[np];
				if (ch=='+' || ch=='-') {
					item[ip++] = ch;
					incr(np);
					ch = buf[np];
				}
				// Scan digits

				while ( nch>=0 && nch<=9 ) {
					item[ip++] = ch;
					incr(np);
					ch = buf[np];
					nch = (int) ch - '0';
				}


				if (ch==' ' || ch=='\n') {
					*itemtype = NUM;
				}
				else {
					// Scan alpha

					while ( ch!=' ' && ch!='\n' ) {
						item[ip++] = ch;
						incr(np);
						ch = buf[np];
					}


					item[ip] = '\0';
					if (item[ip-1]==':') {
						*itemtype = LABEL;
						if (pril>5) printf("LABEL ");
					}
					else {
						*itemtype = WORD;
						if (pril>5) printf("WORD ");
					}
				}
			}
			else {
				*itemtype = NUM;
				if (pril>5) printf("NUM ");
			}
#ifdef OLD
			else if (ch==' ' || ch=='\n') {
				*itemtype = NUM;
				if (pril>5) printf("NUM ");
			}
			else {
				// Scan alpha

				while ( ch!=' ' && ch!='\n' ) {
					item[ip++] = ch;
					incr(np);
					ch = buf[np];
				}


				item[ip] = '\0';
				if (item[ip-1]==':') {
					*itemtype = LABEL;
					if (pril>5) printf("LABEL ");
				}
				else {
					*itemtype = WORD;
					if (pril>5) printf("WORD ");
				}
			}
#endif
		}  
	}
	else if (ch=='.') {
		// Scan digits

		while ( nch>=0 && nch<=9 ) {
			item[ip++] = ch;
			incr(np);
			ch = buf[np];
			nch = (int) ch - '0';
		}


		if (ch==' ' || ch=='\n') {
			*itemtype = NUM;
			if (pril>5) printf("NUM ");
		}
		else if (ch=='E' || ch=='e' || ch=='D' || ch=='d') {
			item[ip++] = ch;
			incr(np);
			ch = buf[np];
			if (ch=='+' || ch=='-') {
				item[ip++] = ch;
				incr(np);
				ch = buf[np];
			}
			// Scan digits

			while ( nch>=0 && nch<=9 ) {
				item[ip++] = ch;
				incr(np);
				ch = buf[np];
				nch = (int) ch - '0';
			}


			if (ch==' ' || ch=='\n') {
				*itemtype = NUM;
			}
			else {
				// Scan alpha

				while ( ch!=' ' && ch!='\n' ) {
					item[ip++] = ch;
					incr(np);
					ch = buf[np];
				}


				item[ip] = '\0';
				if (item[ip-1]==':') {
					*itemtype = LABEL;
					if (pril>5) printf("LABEL ");
				}
				else {
					*itemtype = WORD;
					if (pril>5) printf("WORD ");
				}
			}
		}  
		else {
			// Scan alpha

			while ( ch!=' ' && ch!='\n' ) {
				item[ip++] = ch;
				incr(np);
				ch = buf[np];
			}


			item[ip] = '\0';
			if (item[ip-1]==':') {
				*itemtype = LABEL;
				if (pril>5) printf("LABEL ");
			}
			else {
				*itemtype = WORD;
				if (pril>5) printf("WORD ");
			}
		}
	}
	else if (ch=='<') {
		item[ip++] = ch;
		incr(np);
		if (buf[np]=='=') incr(np);
		*itemtype = LE;
		if (pril>5) printf("LE ");
	}
	else if (ch=='>') {
		item[ip++] = ch;
		incr(np);
		if (buf[np]=='=') incr(np);
		*itemtype = GE;
		if (pril>5) printf("GE ");
	}
	else if (ch=='=') {
		item[ip++] = ch;
		incr(np);
		if (buf[np]=='=') incr(np);
		*itemtype = EQ;
		if (pril>5) printf("EQ ");
	}
	else {
		// Scan alpha

		while ( ch!=' ' && ch!='\n' ) {
			item[ip++] = ch;
			incr(np);
			ch = buf[np];
		}


		item[ip] = '\0';
		if (item[ip-1]==':') {
			*itemtype = LABEL;
			if (pril>5) printf("LABEL ");
		}
		else if (!strcmp(itembuf,"Problem")
			|| !strcmp(itembuf,"\\Problem") 
			|| !strcmp(itembuf,"Minimize")
			|| !strcmp(itembuf,"minimize")
			|| !strcmp(itembuf,"Maximize")
			|| !strcmp(itembuf,"maximize")
			|| !strcmp(itembuf,"Subject")
			|| !strcmp(itembuf,"subject")
			|| !strcmp(itembuf,"s.t.")
			|| !strcmp(itembuf,"Bounds")
			|| !strcmp(itembuf,"End")
			|| !strcmp(itembuf,"end")){
				*itemtype = KEYWORD;
				if (pril>5) printf("KEYWORD ");
			}
		else {
			*itemtype = WORD;
			if (pril>5) printf("WORD ");
		}
	}
	*ptr = ip;
	item[ip] = '\0';
	fflush(stdout);
	while ((ch=buf[np])==' ') incr(np);
	if ((ch=buf[np])=='\n') incr(np);
#ifdef DEBUG
	printf("<item: %1s>\n",item);
#endif

	return 0;
}


/** Accessing a variable by name */
/*
*  Identification of variables. We need a function that for a given
* string identifying a variable returns its internal number. The
* function |varnumber| is called with the pointer to the root of the
* red-black tree and a string. It returns the internal number of the
* corresponding variable, resp.\ generates a new number. Note, that we
* assume that there is space for a new variable in the respective arrays
* to avoid any need for performing reallocations here. The parameter
* |newvar| is nonzero if a new variable was encountered.
*/
int
varnumber(
		  int *T,
		  char *key,
		  int *newvar
		  )
{
	int curfather,w,x,y,z,root;
	int *rootptr;
	int i;
	char *compkey;

	*newvar = 1;
	curfather = RBNIL;
	root = *T;
	rootptr = T;
	while(*T!=RBNIL){
		curfather = *T;
		compkey = &globstr[vptr[*T]];
		if (strcmp(key,compkey)<0){
			T = &leftnode[*T];
		}
		else if (strcmp(key,compkey)>0){
			T = &rightnode[*T];
		}
		else {
			*newvar = 0;
			return varindex[*T];
		}
	}

	curnvars++;
	*T = curnvars;
	father[*T] = curfather;
	leftnode[*T] = RBNIL;
	rightnode[*T] = RBNIL;
	color[*T] = RED;
	varindex[*T] = curnvars-1;
	vptr[*T] = curnchars;
	globind[curnvars-1] = curnchars;
	i= 0;
	do{
		globstr[curnchars] = key[i];
		i++;
		curnchars++;
	}while(key[i]!='\0');
	globstr[curnchars] = key[i];
	curnchars++;
	if (father[*T]==RBNIL){
		root = *T;
		rootptr = T;
		color[root] = BLACK;
	}

	x = *T;
	while(x!=root&&color[father[x]]==RED){
		if (father[x]==leftnode[father[father[x]]]){
			y = rightnode[father[father[x]]];
			if (color[y]==RED){
				color[y] = BLACK;
				color[father[x]] = BLACK;
				x = father[father[x]];
				color[x] = RED;
			}
			else {
				if (x==rightnode[father[x]]){
					z = father[x];
					father[x] = father[z];
					leftnode[father[z]] = x;
					rightnode[z] = leftnode[x];
					father[leftnode[x]] = z;
					leftnode[x] = z;
					father[z] = x;
					x = z;
				}
				z = father[x];
				w = father[father[x]];
				leftnode[w] = rightnode[z];
				father[leftnode[w]] = w;
				father[z] = father[w];
				if (father[z]==RBNIL)
					*rootptr = z;
				if (leftnode[father[w]]==w)
					leftnode[father[w]] = z;
				else
					rightnode[father[w]] = z;
				father[x] = z;
				father[w] = z;
				leftnode[z] = x;
				rightnode[z] = w;
				color[w] = RED;
				color[z] = BLACK;
			}
		}
		else {
			y = leftnode[father[father[x]]];
			if (color[y]==RED){
				color[y] = BLACK;
				color[father[x]] = BLACK;
				x = father[father[x]];
				color[x] = RED;
			}
			else {
				if (x==leftnode[father[x]]){
					z = father[x];
					father[x] = father[z];
					rightnode[father[z]] = x;
					leftnode[z] = rightnode[x];
					father[rightnode[x]] = z;
					rightnode[x] = z;
					father[z] = x;
					x = z;
				}
				z = father[x];
				w = father[z];
				rightnode[w] = leftnode[z];
				father[rightnode[w]] = w;
				father[z] = father[w];
				if (father[z]==RBNIL)
					*rootptr = z;
				if (leftnode[father[w]]==w)
					leftnode[father[w]] = z;
				else
					rightnode[father[w]] = z;
				father[x] = z;
				father[w] = z;
				leftnode[z] = w;

				rightnode[z] = x;
				color[w] = RED;
				color[z] = BLACK;
			}
		}
	}
	color[*rootptr] = BLACK;

	return curnvars-1;
}


/** The function for reading an lp */
/*
*  Read the LP file. This function reads a problem in LP format. This
* function cannot detect every error in an LP file, so it is advisable
* to stick strictly to the formatting rules, e.g.\ to have labels for
* every row and for the objective function. Also note that the right
* hand side has to be a number. With |pril>=1| warnings and errors are
* printed, otherwise on error a nonzero value is returned and there is
* no information about warnings.
*/
int
alps_lpinput(
			 int pril,
			 int ignorevoidrows,
			 char *infilename,
			 char **problemname,
			 int *ncols, 
			 int *nrows, 
			 int *minmax,
			 int *nonz, 
			 int **matrowbeg, 
			 int **matrowcount, 
			 int **matrowind, 
			 double **matrowcoeff, 
			 int **matcolbeg, 
			 int **matcolcount, 
			 int **matcolind, 
			 double **matcolcoeff, 
			 double **rhs, 
			 char **csense,
			 double **obj, 
			 double **lb, 
			 double **ub,
			 char **colnamestr,
			 int **colnameind,
			 char **rownamestr,
			 int **rownameind,
			 int *nignoredrows,
			 int *ncolstrchars,
			 int *nrowstrchars
			 )
{
	// Local variables for lpinput

	int i,j,k,nitems;
	int itemtype,ip;
	double mult;
	double coef;
	char rowsense;
	char varname[200], rowname[200];
	char coefstr[100];
	char ch;
	double rowrhs, varlower, varupper;
	double *coefptr;
	double coefsave2;
	int varnr;
	int newvar;
	int void_row_read;
	int endfound;
	int upboundread,loboundread;



	if (pril>=3) printf("\nLPINPUT: %1s\n",infilename);
	// Initialize I/O
	/*
	*  Initializations and allocations. We allocate the circular buffer for
	* holding the input data, a smaller buffer for loading new input, and a
	* buffer for tokens.
	*/
	fp = fopen(infilename,"r");
	if (!fp) {
		if (pril) printf("File %1s not found!\n",infilename);
		return ALPS_LPREAD_NOFILE;
	}
	endfile = 0;

	lbuf = BUFFER_SIZE;
	ltbuf = TMP_BUFFER_SIZE;
	buf = (char *) malloc( lbuf * sizeof(char) );
	tmpbuf = (char *) malloc( ltbuf * sizeof(char) );
	itembuf = (char *) malloc( ltbuf * sizeof(char) );
	if (!buf || !tmpbuf || !itembuf) {
		if (pril) printf("Out of memory (1)!\n");
		return ALPS_LPREAD_NOMEM;
	}

	nitems = (int) fread(buf,1,lbuf,fp);
	if (!nitems) {
		if (pril) printf("Empty file!\n");
		return ALPS_LPREAD_EMPTYFILE;
	}
	np = 0;
	ep = nitems - 1;
	if (nitems<=lbuf && buf[nitems-1]==EOF) endfile = 1;


	// Allocate problem data
	/*
	*  We start with array sizes for a small problem and reallocate space
	* later if necessary.
	*/
	maxcurnvars = 10000;
	maxcurnrows = 5000;
	maxcurnonz = 50000;
	maxcurnchars = 10*maxcurnvars;
	maxcurrowchars = 10*maxcurnrows;
	*problemname = (char *) malloc( 500 * sizeof(char) );
	*matrowbeg = (int *) malloc( maxcurnrows * sizeof(int) );
	*matrowcount = (int *) malloc( maxcurnrows * sizeof(int) );
	*matrowind = (int *) malloc( maxcurnonz * sizeof(int) );
	*matrowcoeff = (double *) malloc( maxcurnonz * sizeof(double) );
	*matcolcount = (int *) malloc( maxcurnvars * sizeof(int) );
	*rhs = (double *) malloc( maxcurnrows * sizeof(double) );
	*csense = (char *) malloc( maxcurnrows * sizeof(char) );
	*obj = (double *) malloc( maxcurnvars * sizeof(double) );
	*lb = (double *) malloc( maxcurnvars * sizeof(double) );
	*ub = (double *) malloc( maxcurnvars * sizeof(double) );
	*colnamestr = (char *) malloc( maxcurnchars * sizeof(char) );
	*colnameind = (int *) malloc( maxcurnvars * sizeof(int) );
	*rownamestr = (char *) malloc( maxcurrowchars * sizeof(char) );
	*rownameind = (int *) malloc( maxcurnrows * sizeof(int) );
	if (!(*matrowbeg) || !(*matrowcount) || !(*matrowind)
		|| !(*matrowcoeff) || !(*matcolcount) || !(*rhs) || !(*csense)
		|| !(*problemname) || !(*obj) || !(*lb) || !(*ub) 
		|| !(*colnamestr) || !(*colnameind)
		|| !(*rownamestr) || !(*rownameind) ) {
			if (pril) printf("Out of memory (2)!\n");
			return ALPS_LPREAD_NOMEM;
		}
		(*problemname)[0] = '\0';
		curnvars = 0;
		curnrows = 0;
		curnonz = 0;
		curnchars = 0;
		currowchars = 0;
		globstr = *colnamestr;
		globind = *colnameind;
		(*nignoredrows) = 0;


		// Initialize name search

		vptr = (int *) malloc( (maxcurnvars+1) * sizeof(int) );
		father = (int *) malloc( (maxcurnvars+1) * sizeof(int) );
		leftnode = (int *) malloc( (maxcurnvars+1) * sizeof(int) );
		rightnode = (int *) malloc( (maxcurnvars+1) * sizeof(int) );
		varindex = (int *) malloc( (maxcurnvars+1) * sizeof(int) );
		color = (unsigned *) malloc( (maxcurnvars+1) * sizeof(unsigned) );
		if ( !vptr || !father || !leftnode || !rightnode
			|| !varindex || !color ) {
				if (pril) printf("Out of memory (6)!\n");
				return ALPS_LPREAD_NOMEM;
			}
			vptr[0] = -1;
			father[0] = 0;
			leftnode[0] = 0;
			rightnode[0] = 0;
			color[0] = BLACK;
			T = RBNIL;



			endfound = 0;
			do {
				getnextitem(pril,&itemtype,itembuf,&ip);
				if (pril>5) printf("%s\n",itembuf);

				switch (itemtype) {
	case KEYWORD:
		if (!strcmp(itembuf,"Problem")
			|| !strcmp(itembuf,"\\Problem")) {
				if (pril>=2) printf("Problem name....\n");
				// Read problem name
				/*
				*  Read problem name and objective function.
				*/
				getnextitem(pril,&itemtype,itembuf,&ip);
				if (strcmp(itembuf,"name:")) {
					if (pril) printf("Problem name format error!\n");
					return ALPS_LPREAD_NAMEINAVLID;
				}
				while ((ch=buf[np])==' ') incr(np);
				i = 0;
				while ((ch=buf[np])!=' ' && ch!='\n') {
					(*problemname)[++i] = buf[np];
					incr(np);
				}
				if (pril>5) printf("Problem name: %1s\n",*problemname);


			}
		else if (!strcmp(itembuf,"Minimize")
			|| !strcmp(itembuf,"minimize") ) {
				if (pril>=2) printf("Objective (MIN)....\n");
				*minmax = -1;
				// Read objective

				getnextitem(pril,&itemtype,itembuf,&ip);
				if (itemtype!=LABEL) {
				}
				else {
					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				mult = 1.0;
				if (itemtype==OPPLUS) {
					mult = 1.0;
					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				else if (itemtype==OPMINUS) {
					mult = -1.0;
					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				if (itemtype==WORD) {
					coef = 1.0;
				}
				else {
					sscanf(itembuf,"%lf",&coef);
					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				coef *= mult;
				if (itemtype!=WORD) {
					if (pril>=2) printf("Warning: void objective function!\n");
				}
				else {
					ogdf::strcpy(varname,200,itembuf);
					if (pril>=5) printf("%1s %.5lf\n",varname,coef);
					// Store objective function coefficient
					/*
					*  Store data into matrix structure. We generate the matrix first in row
					* format and at the end also in column format.
					*/
					varnr = varnumber(&T,varname,&newvar);
					if (pril>=4) printf("Obj %1s (%1d) = %.3lf\n",varname,varnr,coef);
					if (newvar) {
						(*lb)[varnr] = 0.0;
						(*ub)[varnr] = ALPS_LPREAD_INFINITY;
						(*matcolcount)[varnr] = 0;
						if (varnr>=maxcurnvars-1) { 
							// Reallocate problem data for variables

							if (pril>=2) printf("Reallocating column space....\n");
							maxcurnvars *= 2;
							*matcolcount = (int *) realloc(*matcolcount, maxcurnvars * sizeof(int) );
							*obj = (double *) realloc(*obj, maxcurnvars * sizeof(double) );
							*lb = (double *) realloc(*lb, maxcurnvars * sizeof(double) );
							*ub = (double *) realloc(*ub, maxcurnvars * sizeof(double) );
							*colnameind = (int *) realloc(*colnameind, maxcurnvars * sizeof(int) );
							if (!(*matrowbeg) || !(*matrowcount) || !(*obj)
								|| !(*lb) || !(*ub) || !(*colnameind) ) {
									if (pril) printf("Out of memory (3)!\n");
									return ALPS_LPREAD_NOMEM;
								}
								globind = *colnameind;
								vptr = (int *) realloc(vptr, (maxcurnvars+1) * sizeof(int) );
								father = (int *) realloc(father, (maxcurnvars+1) * sizeof(int) );
								leftnode = (int *) realloc(leftnode, (maxcurnvars+1) * sizeof(int) );
								rightnode = (int *) realloc(rightnode, (maxcurnvars+1) * sizeof(int) );
								varindex = (int *) realloc(varindex, (maxcurnvars+1) * sizeof(int) );
								color = (unsigned *) realloc(color, (maxcurnvars+1) * sizeof(unsigned) );
								if ( !vptr || !father || !leftnode || !rightnode
									|| !varindex || !color ) {
										if (pril) printf("Out of memory (3)!\n");
										return ALPS_LPREAD_NOMEM;
									}


						}
						if (curnchars>=maxcurnchars-20) { 
							// Reallocate space for names

							if (pril>=2) printf("Reallocating names space....\n");
							maxcurnchars *= 2;
							(*colnamestr) = (char *) realloc((*colnamestr), maxcurnchars * sizeof(char) );
							if (!(*colnamestr) ) {
								if (pril) printf("Out of memory (6)!\n");
								return ALPS_LPREAD_NOMEM;
							}
							globstr = *colnamestr;


						}
					}
					(*obj)[varnr] = coef;


				}

				getnextitem(pril,&itemtype,itembuf,&ip);
				while (itemtype==OPPLUS || itemtype==OPMINUS) {
					if (itemtype==OPPLUS) mult = 1.0;
					else  mult = -1.0;
					getnextitem(pril,&itemtype,itembuf,&ip);
					if (itemtype==WORD) {
						coef = 1.0;
					}
					else {
						sscanf(itembuf,"%lf",&coef);
						getnextitem(pril,&itemtype,itembuf,&ip);
					}
					if (itemtype!=WORD) {
						printf("Objective function error at %1s!\n",itembuf);
						return ALPS_LPREAD_INVALIDOBJ;
					}
					else {
						ogdf::strcpy(varname,200,itembuf);
					}
					coef *= mult;
					if (pril>=5) printf("%1s %.5lf\n",varname,coef);
					// Store objective function coefficient
					/*
					*  Store data into matrix structure. We generate the matrix first in row
					* format and at the end also in column format.
					*/
					varnr = varnumber(&T,varname,&newvar);
					if (pril>=4) printf("Obj %1s (%1d) = %.3lf\n",varname,varnr,coef);
					if (newvar) {
						(*lb)[varnr] = 0.0;
						(*ub)[varnr] = ALPS_LPREAD_INFINITY;
						(*matcolcount)[varnr] = 0;
						if (varnr>=maxcurnvars-1) { 
							// Reallocate problem data for variables

							if (pril>=2) printf("Reallocating column space....\n");
							maxcurnvars *= 2;
							*matcolcount = (int *) realloc(*matcolcount, maxcurnvars * sizeof(int) );
							*obj = (double *) realloc(*obj, maxcurnvars * sizeof(double) );
							*lb = (double *) realloc(*lb, maxcurnvars * sizeof(double) );
							*ub = (double *) realloc(*ub, maxcurnvars * sizeof(double) );
							*colnameind = (int *) realloc(*colnameind, maxcurnvars * sizeof(int) );
							if (!(*matrowbeg) || !(*matrowcount) || !(*obj)
								|| !(*lb) || !(*ub) || !(*colnameind) ) {
									if (pril) printf("Out of memory (3)!\n");
									return ALPS_LPREAD_NOMEM;
								}
								globind = *colnameind;
								vptr = (int *) realloc(vptr, (maxcurnvars+1) * sizeof(int) );
								father = (int *) realloc(father, (maxcurnvars+1) * sizeof(int) );
								leftnode = (int *) realloc(leftnode, (maxcurnvars+1) * sizeof(int) );
								rightnode = (int *) realloc(rightnode, (maxcurnvars+1) * sizeof(int) );
								varindex = (int *) realloc(varindex, (maxcurnvars+1) * sizeof(int) );
								color = (unsigned *) realloc(color, (maxcurnvars+1) * sizeof(unsigned) );
								if ( !vptr || !father || !leftnode || !rightnode
									|| !varindex || !color ) {
										if (pril) printf("Out of memory (3)!\n");
										return ALPS_LPREAD_NOMEM;
									}


						}
						if (curnchars>=maxcurnchars-20) { 
							// Reallocate space for names

							if (pril>=2) printf("Reallocating names space....\n");
							maxcurnchars *= 2;
							(*colnamestr) = (char *) realloc((*colnamestr), maxcurnchars * sizeof(char) );
							if (!(*colnamestr) ) {
								if (pril) printf("Out of memory (6)!\n");
								return ALPS_LPREAD_NOMEM;
							}
							globstr = *colnamestr;


						}
					}
					(*obj)[varnr] = coef;


					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				backup_buffer();


			}
		else if (!strcmp(itembuf,"Maximize")
			|| !strcmp(itembuf,"maximize") ) {
				if (pril>=2) printf("Objective (MAX)....\n");
				*minmax = 1;
				// Read objective

				getnextitem(pril,&itemtype,itembuf,&ip);
				if (itemtype!=LABEL) {
				}
				else {
					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				mult = 1.0;
				if (itemtype==OPPLUS) {
					mult = 1.0;
					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				else if (itemtype==OPMINUS) {
					mult = -1.0;
					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				if (itemtype==WORD) {
					coef = 1.0;
				}
				else {
					sscanf(itembuf,"%lf",&coef);
					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				coef *= mult;
				if (itemtype!=WORD) {
					if (pril>=2) printf("Warning: void objective function!\n");
				}
				else {
					ogdf::strcpy(varname,200,itembuf);
					if (pril>=5) printf("%1s %.5lf\n",varname,coef);
					// Store objective function coefficient
					/*
					*  Store data into matrix structure. We generate the matrix first in row
					* format and at the end also in column format.
					*/
					varnr = varnumber(&T,varname,&newvar);
					if (pril>=4) printf("Obj %1s (%1d) = %.3lf\n",varname,varnr,coef);
					if (newvar) {
						(*lb)[varnr] = 0.0;
						(*ub)[varnr] = ALPS_LPREAD_INFINITY;
						(*matcolcount)[varnr] = 0;
						if (varnr>=maxcurnvars-1) { 
							// Reallocate problem data for variables

							if (pril>=2) printf("Reallocating column space....\n");
							maxcurnvars *= 2;
							*matcolcount = (int *) realloc(*matcolcount, maxcurnvars * sizeof(int) );
							*obj = (double *) realloc(*obj, maxcurnvars * sizeof(double) );
							*lb = (double *) realloc(*lb, maxcurnvars * sizeof(double) );
							*ub = (double *) realloc(*ub, maxcurnvars * sizeof(double) );
							*colnameind = (int *) realloc(*colnameind, maxcurnvars * sizeof(int) );
							if (!(*matrowbeg) || !(*matrowcount) || !(*obj)
								|| !(*lb) || !(*ub) || !(*colnameind) ) {
									if (pril) printf("Out of memory (3)!\n");
									return ALPS_LPREAD_NOMEM;
								}
								globind = *colnameind;
								vptr = (int *) realloc(vptr, (maxcurnvars+1) * sizeof(int) );
								father = (int *) realloc(father, (maxcurnvars+1) * sizeof(int) );
								leftnode = (int *) realloc(leftnode, (maxcurnvars+1) * sizeof(int) );
								rightnode = (int *) realloc(rightnode, (maxcurnvars+1) * sizeof(int) );
								varindex = (int *) realloc(varindex, (maxcurnvars+1) * sizeof(int) );
								color = (unsigned *) realloc(color, (maxcurnvars+1) * sizeof(unsigned) );
								if ( !vptr || !father || !leftnode || !rightnode
									|| !varindex || !color ) {
										if (pril) printf("Out of memory (3)!\n");
										return ALPS_LPREAD_NOMEM;
									}


						}
						if (curnchars>=maxcurnchars-20) { 
							// Reallocate space for names

							if (pril>=2) printf("Reallocating names space....\n");
							maxcurnchars *= 2;
							(*colnamestr) = (char *) realloc((*colnamestr), maxcurnchars * sizeof(char) );
							if (!(*colnamestr) ) {
								if (pril) printf("Out of memory (6)!\n");
								return ALPS_LPREAD_NOMEM;
							}
							globstr = *colnamestr;


						}
					}
					(*obj)[varnr] = coef;


				}

				getnextitem(pril,&itemtype,itembuf,&ip);
				while (itemtype==OPPLUS || itemtype==OPMINUS) {
					if (itemtype==OPPLUS) mult = 1.0;
					else  mult = -1.0;
					getnextitem(pril,&itemtype,itembuf,&ip);
					if (itemtype==WORD) {
						coef = 1.0;
					}
					else {
						sscanf(itembuf,"%lf",&coef);
						getnextitem(pril,&itemtype,itembuf,&ip);
					}
					if (itemtype!=WORD) {
						printf("Objective function error at %1s!\n",itembuf);
						return ALPS_LPREAD_INVALIDOBJ;
					}
					else {
						ogdf::strcpy(varname,200,itembuf);
					}
					coef *= mult;
					if (pril>=5) printf("%1s %.5lf\n",varname,coef);
					// Store objective function coefficient
					/*
					*  Store data into matrix structure. We generate the matrix first in row
					* format and at the end also in column format.
					*/
					varnr = varnumber(&T,varname,&newvar);
					if (pril>=4) printf("Obj %1s (%1d) = %.3lf\n",varname,varnr,coef);
					if (newvar) {
						(*lb)[varnr] = 0.0;
						(*ub)[varnr] = ALPS_LPREAD_INFINITY;
						(*matcolcount)[varnr] = 0;
						if (varnr>=maxcurnvars-1) { 
							// Reallocate problem data for variables

							if (pril>=2) printf("Reallocating column space....\n");
							maxcurnvars *= 2;
							*matcolcount = (int *) realloc(*matcolcount, maxcurnvars * sizeof(int) );
							*obj = (double *) realloc(*obj, maxcurnvars * sizeof(double) );
							*lb = (double *) realloc(*lb, maxcurnvars * sizeof(double) );
							*ub = (double *) realloc(*ub, maxcurnvars * sizeof(double) );
							*colnameind = (int *) realloc(*colnameind, maxcurnvars * sizeof(int) );
							if (!(*matrowbeg) || !(*matrowcount) || !(*obj)
								|| !(*lb) || !(*ub) || !(*colnameind) ) {
									if (pril) printf("Out of memory (3)!\n");
									return ALPS_LPREAD_NOMEM;
								}
								globind = *colnameind;
								vptr = (int *) realloc(vptr, (maxcurnvars+1) * sizeof(int) );
								father = (int *) realloc(father, (maxcurnvars+1) * sizeof(int) );
								leftnode = (int *) realloc(leftnode, (maxcurnvars+1) * sizeof(int) );
								rightnode = (int *) realloc(rightnode, (maxcurnvars+1) * sizeof(int) );
								varindex = (int *) realloc(varindex, (maxcurnvars+1) * sizeof(int) );
								color = (unsigned *) realloc(color, (maxcurnvars+1) * sizeof(unsigned) );
								if ( !vptr || !father || !leftnode || !rightnode
									|| !varindex || !color ) {
										if (pril) printf("Out of memory (3)!\n");
										return ALPS_LPREAD_NOMEM;
									}


						}
						if (curnchars>=maxcurnchars-20) { 
							// Reallocate space for names

							if (pril>=2) printf("Reallocating names space....\n");
							maxcurnchars *= 2;
							(*colnamestr) = (char *) realloc((*colnamestr), maxcurnchars * sizeof(char) );
							if (!(*colnamestr) ) {
								if (pril) printf("Out of memory (6)!\n");
								return ALPS_LPREAD_NOMEM;
							}
							globstr = *colnamestr;


						}
					}
					(*obj)[varnr] = coef;


					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				backup_buffer();


			}
		else if (!strcmp(itembuf,"Subject")
			|| !strcmp(itembuf,"subject") 
			|| !strcmp(itembuf,"s.t.") ) {
				if (!strcmp(itembuf,"Subject")
					|| !strcmp(itembuf,"subject") ) {
						getnextitem(pril,&itemtype,itembuf,&ip);
						if (strcmp(itembuf,"To")
							&& strcmp(itembuf,"to")) {
								if (pril>=2) printf("Subject To format error\n");
								return ALPS_LPREAD_SUBJECTERROR;
							}
					}
					if (pril>=2) printf("Constraints section....\n");
					// Read constraints section
					/*
					*  Read the constraints.
					*/
					getnextitem(pril,&itemtype,itembuf,&ip);
					if (itemtype!=LABEL) {
						if (pril) printf("No row label!\n");
						return ALPS_LPREAD_NOROWLABEL;
					}
					curnrows = 0;
					while (itemtype==LABEL) {
						// Read next row
						/*
						*  We also allow void rows with zero or nonzero right hand sides, even
						* in the case of infeasibility. With positive |pril|, warning messages
						* are obtained in such cases. If the parameter |ignorevoidrows| is set
						* then these rows are ignored.
						*/
						void_row_read = 0;
						ogdf::strcpy(rowname,200,itembuf);
						// Start next row

						curnrows++;
						if (curnrows>=maxcurnrows-1) { 
							// Reallocate problem data for rows

							if (pril>=2) printf("Reallocating row space....\n");
							maxcurnrows *= 2;
							*matrowbeg = (int *) realloc(*matrowbeg, maxcurnrows * sizeof(int) );
							*matrowcount = (int *) realloc(*matrowcount, maxcurnrows * sizeof(int) );
							*rhs = (double *) realloc(*rhs, maxcurnrows * sizeof(double) );
							*csense = (char *) realloc(*csense, maxcurnrows * sizeof(char) );
							*rownameind = (int *) realloc(*rownameind, maxcurnrows * sizeof(int) );
							if ( !(*matrowbeg) || !(*matrowcount) || !(*rhs) 
								|| !(*csense) || !(*rownameind) ) {
									if (pril) printf("Out of memory (4)!\n");
									return ALPS_LPREAD_NOMEM;
								}


						}
						if (currowchars>=maxcurrowchars-30) { 
							// Reallocate space for row names

							if (pril>=2) printf("Reallocating row names space....\n");
							maxcurrowchars *= 2;
							(*rownamestr) = (char *) realloc((*rownamestr),maxcurrowchars * sizeof(char) );
							if (!(*rownamestr) ) {
								if (pril) printf("Out of memory (8)!\n");
								return ALPS_LPREAD_NOMEM;
							}


						}
						(*rownameind)[curnrows-1] = currowchars;
						i = 0;
						do {
							(*rownamestr)[currowchars] = rowname[i];
							i++;
							currowchars++;
						} while(rowname[i]!='\0');
						(*rownamestr)[currowchars-1] = '\0';
						(*matrowbeg)[curnrows-1] = curnonz;
						(*matrowcount)[curnrows-1] = 0;


						getnextitem(pril,&itemtype,itembuf,&ip);
						mult = 1.0;
						if (itemtype==OPPLUS) {
							mult = 1.0;
							getnextitem(pril,&itemtype,itembuf,&ip);
						}
						else if (itemtype==OPMINUS) {
							mult = -1.0;
							getnextitem(pril,&itemtype,itembuf,&ip);
						}
						if (itemtype==WORD) {
							coef = 1.0;
						}
						else if (itemtype==NUM){
							sscanf(itembuf,"%lf",&coef);
							getnextitem(pril,&itemtype,itembuf,&ip);
						}
						if (itemtype!=WORD) {
							if (itemtype==EQ 
								|| itemtype==LE 
								|| itemtype==GE ) {
									void_row_read = 1;
									goto void_row;
								}
						}
						else {
							ogdf::strcpy(varname,200,itembuf);
						}
						coef *= mult;
						// Store row coefficient

						varnr = varnumber(&T,varname,&newvar);
						if (pril>=4) printf("Row %1s (%1d) Var %1s (%1d) = %.3lf\n",
							rowname,curnrows-1,varname,varnr,coef);
						if (newvar) {
							(*lb)[varnr] = 0.0;
							(*ub)[varnr] = ALPS_LPREAD_INFINITY;
							(*obj)[varnr] = 0.0;
							(*matcolcount)[varnr] = 0;
							if (varnr>=maxcurnvars-1) { 
								// Reallocate problem data for variables

								if (pril>=2) printf("Reallocating column space....\n");
								maxcurnvars *= 2;
								*matcolcount = (int *) realloc(*matcolcount, maxcurnvars * sizeof(int) );
								*obj = (double *) realloc(*obj, maxcurnvars * sizeof(double) );
								*lb = (double *) realloc(*lb, maxcurnvars * sizeof(double) );
								*ub = (double *) realloc(*ub, maxcurnvars * sizeof(double) );
								*colnameind = (int *) realloc(*colnameind, maxcurnvars * sizeof(int) );
								if (!(*matrowbeg) || !(*matrowcount) || !(*obj)
									|| !(*lb) || !(*ub) || !(*colnameind) ) {
										if (pril) printf("Out of memory (3)!\n");
										return ALPS_LPREAD_NOMEM;
									}
									globind = *colnameind;
									vptr = (int *) realloc(vptr, (maxcurnvars+1) * sizeof(int) );
									father = (int *) realloc(father, (maxcurnvars+1) * sizeof(int) );
									leftnode = (int *) realloc(leftnode, (maxcurnvars+1) * sizeof(int) );
									rightnode = (int *) realloc(rightnode, (maxcurnvars+1) * sizeof(int) );
									varindex = (int *) realloc(varindex, (maxcurnvars+1) * sizeof(int) );
									color = (unsigned *) realloc(color, (maxcurnvars+1) * sizeof(unsigned) );
									if ( !vptr || !father || !leftnode || !rightnode
										|| !varindex || !color ) {
											if (pril) printf("Out of memory (3)!\n");
											return ALPS_LPREAD_NOMEM;
										}


							}
							if (curnchars>=maxcurnchars-20) { 
								// Reallocate space for names

								if (pril>=2) printf("Reallocating names space....\n");
								maxcurnchars *= 2;
								(*colnamestr) = (char *) realloc((*colnamestr), maxcurnchars * sizeof(char) );
								if (!(*colnamestr) ) {
									if (pril) printf("Out of memory (6)!\n");
									return ALPS_LPREAD_NOMEM;
								}
								globstr = *colnamestr;


							}
						}
						curnonz++;
						(*matrowind)[curnonz-1] = varnr;
						(*matrowcoeff)[curnonz-1] = coef;
						(*matrowcount)[curnrows-1]++;
						(*matcolcount)[varnr]++;
						if (curnonz>=maxcurnonz-1) { 
							// Reallocate problem data for nonzeros

							if (pril>=2) printf("Reallocating nonzero space....\n");
							maxcurnonz *= 2;
							*matrowind = (int *) realloc(*matrowind, maxcurnonz * sizeof(int) );
							*matrowcoeff = (double *) realloc(*matrowcoeff, maxcurnonz * sizeof(double) );
							if (!(*matrowind) || !(*matrowcoeff) ) {
								if (pril) printf("Out of memory (5)!\n");
								return ALPS_LPREAD_NOMEM;
							}


						}


						getnextitem(pril,&itemtype,itembuf,&ip);
						while (itemtype==OPPLUS || itemtype==OPMINUS) {
							coef = 1.0;
							coefptr = NULL;
							if (itemtype==OPPLUS) mult = 1.0;
							else  mult = -1.0;
							getnextitem(pril,&itemtype,itembuf,&ip);
							if (itemtype==WORD) {
								coef = 1.0;
								ogdf::strcpy(varname,200,itembuf);
							}
							else {
								coefsave2 = coef;
								coefptr = &coef;
								sscanf(itembuf,"%lf",&coef);
								ogdf::strcpy(coefstr,100,itembuf);
								getnextitem(pril,&itemtype,itembuf,&ip);
							}
							if (itemtype!=WORD) {
								if (allownumvars) {
									if (itemtype==NUM) {
										printf("WARNING: numerical varname %1s.\n",itembuf);
										if (coefptr) coef = *coefptr;
										ogdf::strcpy(varname,200,itembuf);
									}
									else if (itemtype==EQ || itemtype==LE || itemtype==GE
										|| itemtype==OPPLUS || itemtype==OPMINUS) {
											printf("WARNING: numerical varname %1s.\n",coefstr);
											if (coefptr) {
												ogdf::strcpy(varname,200,coefstr);
												coef = coefsave2;
											}
											else {
												printf("Constraint %1s error at %1s, type = %1d!\n",
													rowname,itembuf,itemtype);
												return ALPS_LPREAD_INVALIDROW;
											}
											backup_buffer();
										}
									else {
										printf("Constraint %1s error at %1s, type = %1d!\n",
											rowname,itembuf,itemtype);
										return ALPS_LPREAD_INVALIDROW;
									}
								}
								else {
									printf("Constraint %1s error at %1s, type = %1d!\n",
										rowname,itembuf,itemtype);
									return ALPS_LPREAD_INVALIDROW;
								}
							}
							else {
								ogdf::strcpy(varname,200,itembuf);
							}
							coef *= mult;
							// Store row coefficient

							varnr = varnumber(&T,varname,&newvar);
							if (pril>=4) printf("Row %1s (%1d) Var %1s (%1d) = %.3lf\n",
								rowname,curnrows-1,varname,varnr,coef);
							if (newvar) {
								(*lb)[varnr] = 0.0;
								(*ub)[varnr] = ALPS_LPREAD_INFINITY;
								(*obj)[varnr] = 0.0;
								(*matcolcount)[varnr] = 0;
								if (varnr>=maxcurnvars-1) { 
									// Reallocate problem data for variables

									if (pril>=2) printf("Reallocating column space....\n");
									maxcurnvars *= 2;
									*matcolcount = (int *) realloc(*matcolcount, maxcurnvars * sizeof(int) );
									*obj = (double *) realloc(*obj, maxcurnvars * sizeof(double) );
									*lb = (double *) realloc(*lb, maxcurnvars * sizeof(double) );
									*ub = (double *) realloc(*ub, maxcurnvars * sizeof(double) );
									*colnameind = (int *) realloc(*colnameind, maxcurnvars * sizeof(int) );
									if (!(*matrowbeg) || !(*matrowcount) || !(*obj)
										|| !(*lb) || !(*ub) || !(*colnameind) ) {
											if (pril) printf("Out of memory (3)!\n");
											return ALPS_LPREAD_NOMEM;
										}
										globind = *colnameind;
										vptr = (int *) realloc(vptr, (maxcurnvars+1) * sizeof(int) );
										father = (int *) realloc(father, (maxcurnvars+1) * sizeof(int) );
										leftnode = (int *) realloc(leftnode, (maxcurnvars+1) * sizeof(int) );
										rightnode = (int *) realloc(rightnode, (maxcurnvars+1) * sizeof(int) );
										varindex = (int *) realloc(varindex, (maxcurnvars+1) * sizeof(int) );
										color = (unsigned *) realloc(color, (maxcurnvars+1) * sizeof(unsigned) );
										if ( !vptr || !father || !leftnode || !rightnode
											|| !varindex || !color ) {
												if (pril) printf("Out of memory (3)!\n");
												return ALPS_LPREAD_NOMEM;
											}


								}
								if (curnchars>=maxcurnchars-20) { 
									// Reallocate space for names

									if (pril>=2) printf("Reallocating names space....\n");
									maxcurnchars *= 2;
									(*colnamestr) = (char *) realloc((*colnamestr), maxcurnchars * sizeof(char) );
									if (!(*colnamestr) ) {
										if (pril) printf("Out of memory (6)!\n");
										return ALPS_LPREAD_NOMEM;
									}
									globstr = *colnamestr;


								}
							}
							curnonz++;
							(*matrowind)[curnonz-1] = varnr;
							(*matrowcoeff)[curnonz-1] = coef;
							(*matrowcount)[curnrows-1]++;
							(*matcolcount)[varnr]++;
							if (curnonz>=maxcurnonz-1) { 
								// Reallocate problem data for nonzeros

								if (pril>=2) printf("Reallocating nonzero space....\n");
								maxcurnonz *= 2;
								*matrowind = (int *) realloc(*matrowind, maxcurnonz * sizeof(int) );
								*matrowcoeff = (double *) realloc(*matrowcoeff, maxcurnonz * sizeof(double) );
								if (!(*matrowind) || !(*matrowcoeff) ) {
									if (pril) printf("Out of memory (5)!\n");
									return ALPS_LPREAD_NOMEM;
								}


							}


							getnextitem(pril,&itemtype,itembuf,&ip);
						}

void_row:
						if (itemtype==LE)
							rowsense = '<';
						else if (itemtype==GE) 
							rowsense = '>';
						else if (itemtype==EQ) 
							rowsense = '=';
						else {
							if (pril) printf("Error in %1s (%1s,%1d)!\n", rowname,itembuf,itemtype);
							return ALPS_LPREAD_INVALIDROW;
						}
						getnextitem(pril,&itemtype,itembuf,&ip);
						if (itemtype==NUM) {
							sscanf(itembuf,"%lf",&rowrhs);
						}
						else if (itemtype==OPMINUS) {
							getnextitem(pril,&itemtype,itembuf,&ip);
							if (itemtype!=NUM) {
								if (pril) printf("Error in %1s (%1s,%1d)!\n", rowname,itembuf,itemtype);
								return ALPS_LPREAD_INVALIDROW;
							}
							else {
								sscanf(itembuf,"%lf",&rowrhs);
								rowrhs = -rowrhs;
							}
						}
						else if (itemtype==OPPLUS) {
							getnextitem(pril,&itemtype,itembuf,&ip);
							if (itemtype!=NUM) {
								if (pril) printf("Error in %1s (%1s,%1d)!\n", rowname,itembuf,itemtype);
								return ALPS_LPREAD_INVALIDROW;
							}
							else {
								sscanf(itembuf,"%lf",&rowrhs);
							}
						}
						else {
							if (pril) printf("Error in %1s (%1s,%1d)!\n", rowname,itembuf,itemtype);
							return ALPS_LPREAD_INVALIDROW;
						}
						// Store right hand side

						if (pril>=4) printf("%1s %c %.4lf\n",rowname,rowsense,rowrhs);
						(*rhs)[curnrows-1] = rowrhs;
						(*csense)[curnrows-1] = rowsense;


						getnextitem(pril,&itemtype,itembuf,&ip);
						if (void_row_read) {
							if (pril>=2) printf("WARNING: void row %1s.",rowname);
							if (ignorevoidrows) {
								if (pril>=2) printf(" Ignored.\n");
								currowchars = (*rownameind)[curnrows-1];
								curnrows--;
								(*nignoredrows)++;
							}
							else
								if (pril) printf("\n");
						}


					}
					backup_buffer();


			}
		else if (!strcmp(itembuf,"Bounds")) {
			if (pril>=2) printf("Bounds section....\n");
			// Read bounds section
			/*
			*  Read the bounds.
			*/
			getnextitem(pril,&itemtype,itembuf,&ip);
			while (itemtype!=KEYWORD && itemtype!=EOI) {
				upboundread = loboundread = 0;
				varlower = 0.0;
				varupper = ALPS_LPREAD_INFINITY;
				mult = 1.0;
				if (itemtype==OPMINUS) {
					mult = -1.0;
					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				if (itemtype==OPPLUS) {
					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				if (itemtype==NUM) {
					sscanf(itembuf,"%lf",&coef);
					coef *= mult;
					getnextitem(pril,&itemtype,itembuf,&ip);
					if (itemtype==LE) {
						varlower = coef;
						loboundread = 1;
					}
					else if (itemtype==GE) {
						varupper = coef;
						upboundread = 1;
					}
					else {
						if (pril) printf("Invalid bound input %1s!\n", itembuf);
						return ALPS_LPREAD_INVALIDBOUND;
					}
					getnextitem(pril,&itemtype,itembuf,&ip);
				}
				if (itemtype!=WORD) {
					if (pril) printf("Invalid bound input %1s!\n", itembuf);
					return ALPS_LPREAD_INVALIDBOUND;
				}
				else sscanf(itembuf,"%s",varname);
				mult = 1.0;
				getnextitem(pril,&itemtype,itembuf,&ip);
				if (itemtype==LE) {
					getnextitem(pril,&itemtype,itembuf,&ip);
					if (itemtype==OPMINUS) {
						mult = -1.0;
						getnextitem(pril,&itemtype,itembuf,&ip);
					}
					if (itemtype==OPPLUS) {
						getnextitem(pril,&itemtype,itembuf,&ip);
					}
					if (itemtype!=NUM) {
						if (pril) printf("Invalid bound input %1s!\n", itembuf);
						return ALPS_LPREAD_INVALIDBOUND;
					}
					else {
						sscanf(itembuf,"%lf",&coef);
						coef *= mult;
						varupper = coef;
						upboundread = 1;
					}
				}
				else if (itemtype==GE) {
					getnextitem(pril,&itemtype,itembuf,&ip);
					if (itemtype==OPMINUS) {
						mult = -1.0;
						getnextitem(pril,&itemtype,itembuf,&ip);
					}
					if (itemtype==OPPLUS) {
						getnextitem(pril,&itemtype,itembuf,&ip);
					}
					if (itemtype!=NUM) {
						if (pril) printf("Invalid bound input %1s!\n", itembuf);
						return ALPS_LPREAD_INVALIDBOUND;
					}
					else {
						sscanf(itembuf,"%lf",&coef);
						coef *= mult;
						varlower = coef;
						loboundread = 1;
					}
				}
				else if (itemtype==EQ) {
					getnextitem(pril,&itemtype,itembuf,&ip);
					if (itemtype==OPMINUS) {
						mult = -1.0;
						getnextitem(pril,&itemtype,itembuf,&ip);
					}
					if (itemtype==OPPLUS) {
						getnextitem(pril,&itemtype,itembuf,&ip);
					}
					if (itemtype!=NUM) {
						if (pril) printf("Invalid bound input %1s!\n", itembuf);
						return ALPS_LPREAD_INVALIDBOUND;
					}
					else {
						sscanf(itembuf,"%lf",&varupper);
						varupper *= mult;
						upboundread = 1;
						loboundread = 1;
					}
					varlower = varupper;
				}
				else if (!strcmp(itembuf,"Free")
					|| !strcmp(itembuf,"free") ){
						varlower = -ALPS_LPREAD_INFINITY;
						varupper = ALPS_LPREAD_INFINITY;
						upboundread = 1;
						loboundread = 1;
						if (pril>=2) printf("Free variable %1s\n",varname);
					}
				else {
					if (!upboundread) varupper = ALPS_LPREAD_INFINITY;
				}
				if (!upboundread && !loboundread) {
					if (pril) printf("Invalid bound input for %1s!\n", varname);
					return ALPS_LPREAD_INVALIDBOUND;
				}
				getnextitem(pril,&itemtype,itembuf,&ip);
				// Store bounds of variable

				varnr = varnumber(&T,varname,&newvar);
				if (pril>=5) printf("%.4le <= %1s <= %.4le\n",varlower,varname,varupper);
				(*lb)[varnr] = varlower;
				(*ub)[varnr] = varupper;
				if (newvar && (varnr>=maxcurnvars-1) ) { 
					// Reallocate problem data for variables

					if (pril>=2) printf("Reallocating column space....\n");
					maxcurnvars *= 2;
					*matcolcount = (int *) realloc(*matcolcount, maxcurnvars * sizeof(int) );
					*obj = (double *) realloc(*obj, maxcurnvars * sizeof(double) );
					*lb = (double *) realloc(*lb, maxcurnvars * sizeof(double) );
					*ub = (double *) realloc(*ub, maxcurnvars * sizeof(double) );
					*colnameind = (int *) realloc(*colnameind, maxcurnvars * sizeof(int) );
					if (!(*matrowbeg) || !(*matrowcount) || !(*obj)
						|| !(*lb) || !(*ub) || !(*colnameind) ) {
							if (pril) printf("Out of memory (3)!\n");
							return ALPS_LPREAD_NOMEM;
						}
						globind = *colnameind;
						vptr = (int *) realloc(vptr, (maxcurnvars+1) * sizeof(int) );
						father = (int *) realloc(father, (maxcurnvars+1) * sizeof(int) );
						leftnode = (int *) realloc(leftnode, (maxcurnvars+1) * sizeof(int) );
						rightnode = (int *) realloc(rightnode, (maxcurnvars+1) * sizeof(int) );
						varindex = (int *) realloc(varindex, (maxcurnvars+1) * sizeof(int) );
						color = (unsigned *) realloc(color, (maxcurnvars+1) * sizeof(unsigned) );
						if ( !vptr || !father || !leftnode || !rightnode
							|| !varindex || !color ) {
								if (pril) printf("Out of memory (3)!\n");
								return ALPS_LPREAD_NOMEM;
							}


				}
				if (curnchars>=maxcurnchars-20) { 
					// Reallocate space for names

					if (pril>=2) printf("Reallocating names space....\n");
					maxcurnchars *= 2;
					(*colnamestr) = (char *) realloc((*colnamestr), maxcurnchars * sizeof(char) );
					if (!(*colnamestr) ) {
						if (pril) printf("Out of memory (6)!\n");
						return ALPS_LPREAD_NOMEM;
					}
					globstr = *colnamestr;


				}


			}
			backup_buffer();


		}
		else if (!strcmp(itembuf,"End")
			|| !strcmp(itembuf,"end")) {
				endfound = 1;
				if (pril>=2) printf("End of lp input\n");
			}
		else { 
			if (pril>=2) printf("Invalid input! Stopped at %1s.\n",itembuf);
			return ALPS_LPREAD_INVALIDINPUT;
		}
		break;
	case EOI:
		if (pril>=2) printf("End of file \n");
		break;
	default:
		if (pril) printf("Invalid input! Stopped at %1s.\n",itembuf);
		return ALPS_LPREAD_INVALIDINPUT;
		break;
				}

			} while (itemtype!=EOI);

			if (!endfound) {
				if (pril>=2) printf("WARNING: Missing end.\n");
			}
			// Store matrix in column format
			/*
			*  We store the matrix in column format as well. This is easy because we
			* have already the column counts available. In order to save space we
			* reallocate all arrays.
			*/
			*obj = (double *) realloc(*obj, curnvars * sizeof(double) );
			*lb = (double *) realloc(*lb, curnvars * sizeof(double) );
			*ub = (double *) realloc(*ub, curnvars * sizeof(double) );
			*matrowbeg = (int *) realloc(*matrowbeg, curnrows * sizeof(int) );
			*matrowcount = (int *) realloc(*matrowcount, curnrows * sizeof(int) );
			*matrowind = (int *) realloc(*matrowind, curnonz * sizeof(int) );
			*matrowcoeff = (double *) realloc(*matrowcoeff, curnonz * sizeof(double) );
			*rhs = (double *) realloc(*rhs, curnrows * sizeof(double) );
			*csense = (char *) realloc(*csense, curnrows * sizeof(char) );
			if (!(*obj) || !(*lb) || !(*ub) 
				|| !(*matrowbeg) || !(*matrowcount) || !(*matrowind) || !(*matrowcoeff) 
				|| !(*rhs) || !(*csense) ) {
					if (pril) printf("Out of memory (7)!\n");
					return ALPS_LPREAD_NOMEM;
				}
				*matcolbeg = (int *) malloc(curnvars * sizeof(int) );
				*matcolind = (int *) malloc(curnonz * sizeof(int) );
				*matcolcoeff = (double *) malloc(curnonz * sizeof(double) );
				if (!(*matcolbeg) || !(*matcolind) || !(*matcolcoeff) ) {
					if (pril) printf("Out of memory (7)!\n");
					return ALPS_LPREAD_NOMEM;
				}
				(*matcolbeg)[0] = 0;
				for (j=1;j<curnvars;j++) 
					(*matcolbeg)[j] = (*matcolbeg)[j-1] + (*matcolcount)[j-1];
				for (i=0; i<curnrows; i++) {
					for (k=(*matrowbeg)[i];k<(*matrowbeg)[i]+(*matrowcount)[i];k++) {
						j = (*matrowind)[k];
						(*matcolind)[(*matcolbeg)[j]] = i;
						(*matcolcoeff)[(*matcolbeg)[j]] = (*matrowcoeff)[k];
						(*matcolbeg)[j]++;
					}
				}
				(*matcolbeg)[0] = 0;
				for (j=1;j<curnvars;j++) 
					(*matcolbeg)[j] = (*matcolbeg)[j-1] + (*matcolcount)[j-1];

				// Free temporary data
				ffree( (char**) &vptr);
				ffree( (char**) &father);
				ffree( (char**) &leftnode);
				ffree( (char**) &rightnode);
				ffree( (char**) &varindex);
				ffree( (char**) &color);

				*ncols = curnvars;
				*nrows = curnrows;
				*nonz = curnonz;
				*ncolstrchars = curnchars;
				*nrowstrchars = currowchars;
				return 0;
}

} // end namespace ogdf
