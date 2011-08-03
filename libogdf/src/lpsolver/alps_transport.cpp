/*
 * $Revision: 1.1 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2008-11-27 03:02:28 +1100 (Thu, 27 Nov 2008) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of a network simplex algorithm for
 * transportation problems. This is used for a variant of the 
 * scaling procedure.
 * 
 * \author Matthias ELf
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

#include "alps_transport.h"  /* Main data structures used by the following funcions  */

#include "stdio.h"
#include "math.h"
#include "alps.h" /* Definitions of |ALPS_ZEROEPS| , ... */

namespace ogdf {

/*
*  Function |optimize_transport|. This function performs the
* optimization process of the network-simplex algorithm.
*/

int optimize_transport(BASIS *basis)
{
	/** Declare |optimize_transport| scalars */

	int i_enter,j_enter;
	int arcindex;
	int halfnodes;
	int subroot;
	int prev;
	int orient;		
	int status;
	double deltadual;		


	/** Declare |optimize_transport| arrays */

	int *family;
	int *pred;
	int *brother;
	int *son;




	status=init_basis(basis);  /* construct initial basis */
	if (status) return status; 

	/** Define simplifications for |optimize_transport| */

	family = basis->family;
	pred = basis->pred;
	brother = basis->brother;
	son = basis->son;



	halfnodes=basis->no_nodes/2;

	deltadual=find_entering_arc(basis,&i_enter,&j_enter,&arcindex); 

	while(i_enter>=0){		/* there is an entering arc  */

		basis->pivot++;		

#ifdef PRINT
		printf("+-----------------------------------+\n");
		printf("|            pivot %3d              |\n",basis->pivot);
		printf("+-----------------------------------+\n");
#endif

		subroot = find_cycle(basis,i_enter,j_enter,arcindex,&orient);

#ifdef PRINT
		printf("\nentering arc : %d  --> %d \n",i_enter,j_enter);
		printf("    arcindex : %d\n",arcindex);
		printf("     redcost : %8.4lf",deltadual);
#endif

		/* update duals in the smaller subtrees */
		if(family[subroot]<=halfnodes)
			update_dual(basis,subroot,(orient>0)?deltadual:-deltadual);
		else { /* separate trees, update duals, then reconnect */
			basis->rerooted++;
			prev = pred[subroot];
			/* remove root from successors of prev */
			son[prev]= brother[subroot];

			update_dual(basis,0,(orient>0)?-deltadual:deltadual);

			brother[subroot]=son[prev];      /* insert root to successors of prev */
			son[prev]=subroot;
		}

#ifdef PRINT    
		print_basis(basis);
#endif 

		deltadual=find_entering_arc(basis,&i_enter,&j_enter,&arcindex);
	}

	status=check_solution(basis);

	return status;  
}

/*
*  Function |find_entering_arc|. The following function scans the list
* of arcs for arcs with negative reduced cost. At each iteration it
* scans a sequence of arcs and, among all admissible arcs found, it
* selects the one with the smallest negative reduced cost. At the next
* pivot and if no admissible arc is found in the current sequence, the
* next sequence is scanned. The length of a sequence is 450.
*/

double find_entering_arc(BASIS *basis,int *i_enter, int *j_enter, int *arcindex)
{
	/** Declare |find_entering_arc| scalars */

	int i;
	int m,n;
	static int jj=0;
	static int kk=0;		       
	int iter,seqiter,seqsize;
	int tmp;
	double redcost;
	double redcost_best;


	/** Declare |find_entering_arc| arrays */

	int *colbeg;
	int *colcount;
	int *colind;
	int no_nodes;
	int no_arcs;
	double *cost;
	double *dual;


	/** Define simplifications for |find_entering_arc| */

	m=basis->m;
	n=basis->n;
	colbeg=basis->colbeg;
	colcount=basis->colcount;
	colind=basis->colind;
	cost=basis->cost;
	no_arcs=basis->no_arcs;
	no_nodes=basis->no_nodes;
	dual=basis->dual;



	redcost_best=-ALPS_ZEROEPS;

	iter=0;
	seqiter=0;
	seqsize=450;
	*i_enter=-1;
	*j_enter=-1;

	for( ; ; ) {
		if(kk==no_arcs){
			jj=0;
			kk=0;
		} else if (kk==(colbeg[jj]+colcount[jj])) jj++;
		while(jj<n){
			while(kk<(colbeg[jj]+colcount[jj])){
				i = colind[kk];
				redcost = cost[kk]-dual[i]+dual[tmp=m+jj];
				if (redcost<redcost_best)
				{
					redcost_best=redcost;
					*i_enter=i;
					*j_enter=tmp;
					*arcindex=kk;
				}
				kk++;
				if(++seqiter==seqsize)
					if(*i_enter>=0) {
						return redcost_best;
					} else {
						seqiter=0;
					}
					if(++iter==no_arcs) return redcost_best;
			}
			jj++;
		}
	}
}



/*
*  Function |find_cycle|. The following function searches the
* basis-cycle formed by the entering arc |i_enter|-|j_enter|, computes
* the leaving arc and performs the update of the basis-tree.
*/

int find_cycle(BASIS *basis, int i_enter, int j_enter, int arcindex, int *orient)
{ 
	/** Declare |find_cycle| scalars */

	int joint;			/* lowest common ancester of i and j */
	int i,j;			/* nodes in path from i (j) to joint  */
	int i_minflow, j_minflow;	/* maximal flow change in path from i (j) to joint */
	int i_min, j_min;		/* nodes of most restrictive arcs in path from i (j) to joint */
	int i_family, j_family;	/* number of successors of nodes i and j */
	int tmp;			 
	int newfamily,subtree;     
	int prev,next;                
	int newflow,prevflow;
	int newindex,previndex;
	int neworient,prevorient;
	int minnode;			/* deeper node of the blocking arc */
	int ret;			/* return value */


	/** Declare |find_cycle| arrays */

	int *family;
	int *reverse;
	int *pred;
	int *son;
	int *brother;
	int *upward;
	int *flow;


	/** Declare simplifications for |find_cycle| */

	family=basis->family;
	reverse=basis->reverse;
	pred=basis->pred;
	son=basis->son;
	brother=basis->brother;
	upward=basis->upward;
	flow=basis->flow;



	/** Compute pivot-cycle and flow-change */

	i_minflow = j_minflow = ALPS_INT_INFINITY; /* start with maximum flow */
	i_min=j_min=-1;
	i=i_enter;
	j=j_enter;

	/* try to augment flow  */

	while(i!=j){
		i_family=family[i];
		j_family=family[j];
		while(i_family!=j_family){
			while(i_family<j_family){ 
				if(upward[i]){		/* arc at i points upward */
					if((tmp=flow[i])<i_minflow){
						i_minflow=tmp;
						i_min=i;
					}
				}
				i=pred[i];		/* i ascends */
				i_family=family[i];
			}
			while(i_family>j_family){ /* j must ascend */
				if(!(upward[j]))	/* arc at j points downward */
					if((tmp=flow[j])<=j_minflow){
						j_minflow=tmp;
						j_min=j;
					}
					j=pred[j];		/* j ascends */
					j_family=family[j];
			}
		}
		if(i!=j){			/* i and j must ascend */
			if(upward[i])		/* arc at i points upward*/
				if((tmp=flow[i])<i_minflow){
					i_minflow=tmp;
					i_min=i;
				}
				if(!(upward[j]))		/* arc at j points downward*/
					if((tmp=flow[j])<=j_minflow){
						j_minflow=tmp;
						j_min=j;
					}
					i=pred[i];		/* i ascends */
					j=pred[j];		/* j ascends */
		}
	}
	joint=i; 




#ifdef PRINT
	printf("------ result Find-Cycle ------\n");
	printf("joint : %d\n\n",joint);
	printf("    i_min = %d         i_minflow : %8d \n",i_min,i_minflow);
	printf("    j_min = %d         j_minflow : %8d \n",j_min,j_minflow);
#endif

	/* Update of the basis tree */

	if (!i_minflow){
		basis->degpivot++;

#ifdef PRINT
		printf(" ******** degenerated  pivot ******** \n");
#endif

		/** Basis tree update (degenerated case / no flow change) */

		ret=i_enter;
		*orient=1;			/* |i_enter|--|j_enter| is new tree-arc */
		i=i_enter;			
		j=j_enter;
		minnode=i_min;
		newflow=0;			/* flow still equals 0 */
		newindex=arcindex;
		neworient=1;		/* new arc poit upward */
		newfamily=subtree=family[minnode];

		do{				/* update the tree */

			prev=pred[i];		/* update pred */
			pred[i]=j;		

			prevflow=flow[i];		/* update flow */
			flow[i]=newflow;

			previndex=reverse[i];	/* update reverse */
			reverse[i]=newindex;

			prevorient=upward[i];	/* update upward */
			upward[i]=neworient;

			if ((tmp=son[prev]) == i){/* remove i from the successors of prev */
				son[prev]=brother[i];
			} else{
				while((next=brother[tmp])!=i) tmp=next;
				brother[tmp]=brother[i];
			}

			brother[i]=son[j];	/* add i to the successors of j */
			son[j]=i;

			tmp=family[i];		/* update family */
			family[i]=newfamily;
			newfamily=subtree-tmp;

			j=i;			/* moving up  */
			i=prev;
			newflow=prevflow;
			newindex= previndex;
			neworient=!prevorient;
		}while(j!=minnode);

		for( i=prev ; i!=joint ; i=pred[i])	/* update family */
			family[i]-=subtree;
		for( i=j_enter ; i!=joint ; i=pred[i])
			family[i]+=subtree;
		return ret;			/* return root of new subtree  */


	}

	if ( i_minflow < j_minflow ) {
		/** Basis tree update (leaving arc is above i / insert |i_enter|--|j_enter|) */


		ret=i_enter;
		*orient=1;
		i=i_enter;			
		j=j_enter;
		minnode=i_min;
		newflow=i_minflow;		/* increasing flow to iminflow */
		newindex=arcindex;
		neworient=1;		/* new arc point upward */
		newfamily=subtree=family[minnode];

		do{				/* update the tree */

			prev=pred[i];		/* update pred */
			pred[i]=j;

			prevflow=flow[i];		/* update flow */
			flow[i]=newflow;

			previndex=reverse[i];	/* update reverse */
			reverse[i]=newindex;

			prevorient=upward[i];
			upward[i]=neworient;

			if ((tmp=son[prev]) == i){/* remove i from the successors of prev */
				son[prev]=brother[i];
			} else{
				while((next=brother[tmp])!=i) tmp=next;
				brother[tmp]=brother[i];
			}

			brother[i]=son[j];	/* add i to the successors of j */
			son[j]=i;

			tmp=family[i];		/* update family */
			family[i]=newfamily;
			newfamily=subtree-tmp;

			j=i;			/* moving up */
			i=prev;
			if (prevorient){
				newflow=prevflow - i_minflow;
			}else{
				newflow=prevflow + i_minflow;
			}
			newindex = previndex;
			neworient=!prevorient;
		}while(j!=minnode);

		/* continue flow update of cycle from |i_min| to joint */
		while(i!=joint){
			if(upward[i]){
				flow[i]-=i_minflow;
			}else{
				flow[i]+=i_minflow;
			}
			family[i]-=subtree;
			i=pred[i];
		}

		/* update from |j_enter| to joint */
		j=j_enter;
		while(j!=joint){
			if (upward[j]){
				flow[j]+=i_minflow;
			}else{
				flow[j]-=i_minflow;
			}
			family[j]+=subtree;
			j=pred[j];
		}


	} else {
		/** Basis tree update (leaving arc is above j / insert |j_enter|--|i_enter|) */


		ret=j_enter;
		*orient=-1;
		j=i_enter;			
		i=j_enter;
		minnode=j_min;
		newflow=j_minflow;		/* increasing flow to iminflow */
		newindex = arcindex;
		neworient= 0;		/* new arc points downward */
		newfamily=subtree=family[minnode];

		do{				/* update the tree */

			prev=pred[i];		/* update pred */
			pred[i]=j;		

			prevflow=flow[i];		/* update flow */
			flow[i]=newflow;

			previndex=reverse[i];	/* update reverse */
			reverse[i]=newindex;

			prevorient=upward[i];
			upward[i]=neworient;

			if ((tmp=son[prev]) == i){/* remove i from the successors of prev */
				son[prev]=brother[i];
			} else{
				while((next=brother[tmp])!=i) tmp=next;
				brother[tmp]=brother[i];
			}

			brother[i]=son[j];	/* add i to the successors of j */
			son[j]=i;

			tmp=family[i];		/* update family */
			family[i]=newfamily;
			newfamily=subtree-tmp;

			j=i;			/* moving up  */
			i=prev;
			if (prevorient){
				newflow=prevflow + j_minflow;
			}else{
				newflow=prevflow - j_minflow;
			}
			newindex = previndex;
			neworient= !prevorient;
		}while(j!=minnode);

		/* continue flow update of cycle from |j_min| to joint */
		while(i!=joint){
			if(upward[i]){
				flow[i]+=j_minflow;
			}else{
				flow[i]-=j_minflow;
			}
			family[i]-=subtree;
			i=pred[i];
		}

		/* update from |i_enter| to joint */
		i=i_enter;
		while(i!=joint){
			if (upward[i]){
				flow[i]-=j_minflow;
			}else{
				flow[i]+=j_minflow;
			}
			family[i]+=subtree;
			i=pred[i];
		}



	}
	return ret; /* return root of new subtree */
}

/*
*  Function |update_dual|. The following function scans the subtree
* rooted at |root| and performs the update of the dual variables by
* adding |deltadual|.
*/

void update_dual(BASIS *basis, int root, double deltadual)
{
	int j,k;
	int *son;
	int *pred;
	int *brother;
	double *dual;

	pred=basis->pred;
	brother=basis->brother;
	son=basis->son;
	dual=basis->dual;

	k=root;
	if (son[k]<0) {		/* subtree has only one node */
		dual[k]+=deltadual;
		return; 
	};

	for( ; ; ) {			/* scanning subtree rooted at node */
		do{				/* descent in subtree */
			do{
				dual[k]+=deltadual;	/* updating duals */
				j=k;
				k=son[k];
			} while (k>=0);
			k=brother[j];
		} while (k>=0); 
		do {			/* ascent in subtree  */
			k=pred[j];
			if (k==root) return;
			j=k;
			k=brother[k];
		} while (k<0);
	}
}


/*
*  Function |check_solution|. The following function scans the
* basis-tree and checks if there are artificial arcs with nonzero flow.
* It also computes the dual variables of the cycle free solution of the
* transportation problem after removing artificial arcs.
*/

int check_solution(BASIS *basis)
{

	/** Declare |check_solution| scalars */


	int j,k;
	int artificial;
	double bigM;
	double deltadual;


	/** Declare |check_solution| arrays */

	int *son;
	int *pred;
	int *brother;
	int *reverse;
	int *upward;
	int *flow;
	double *dual;


	/** Define simpilfications for |check_solution| */

	bigM=basis->bigM;
	pred=basis->pred;
	brother=basis->brother;
	son=basis->son;
	dual=basis->dual;
	flow=basis->flow;
	reverse=basis->reverse;
	upward=basis->upward;
	artificial=basis->artificial;




	k=0;			/* start at the root 0 */
	deltadual=-dual[0];

	if (son[k]<0) {		/* subtree has only one node */
		dual[k]+=deltadual;
		return 0;			/* solution correct */
	};

	for( ; ; ) {			/* scanning subtree rooted at node */
		do{				/* descent in subtree */
			do{

				if (reverse[k]==artificial){ 
					if (flow[k]>0){
						printf("solution contains artificial arcs with positive flow !!! \n");
						return 1;		/* incorrect solution */
					} 
					deltadual+=(upward[k])?-bigM:bigM;
				}	
				dual[k]+=deltadual;	/* updating duals */
				j=k;
				k=son[k];
			} while (k>=0);

			if (reverse[j]==artificial){
				deltadual+=(upward[j])?bigM:-bigM;
			}

			k=brother[j];

		} while (k>=0); 
		do {			/* ascent in subtree  */
			if (reverse[j]==artificial){
				deltadual+=(upward[j])?bigM:-bigM;
			}
			k=pred[j];
			if (k==0) return 0;
			j=k;
			k=brother[k];
		} while (k<0);
		if (reverse[j]==artificial){
			deltadual+=(upward[j])?bigM:-bigM;
		}
	}
}

/*
*  Function |init_basis|. This function constructs the initial basis.
*/

int init_basis(BASIS *basis)
{
	/** Declare |init_basis| scalars */

	int i,j,jj,k;
	int artificial;
	int no_nodes;
	double bigM;


	/** Declare |init_basis| arrays */

	int *pred;
	int *son;
	int *brother;
	int *family;
	int *reverse;
	int *upward;
	int *flow;
	int *demand;
	double *dual; 



	/* statistics */
	basis->pivot=0;
	basis->rerooted=0;
	basis->degpivot=0;

	/* set bigM */
	basis->bigM = bigM = 1.0 + (basis->no_nodes-1)/2.0*basis->maxcost;

	no_nodes = basis->no_nodes = basis->m + basis->n; /* number of arcs */
	artificial = basis->artificial = basis->no_arcs; /* index of artificial arcs */
	demand=basis->demand;

	basis->cost[artificial]=bigM;	/* set cost of artificial arcs  */

	/* initialization of the basis-tree */
	basis->pred    = pred    = (int*) malloc(sizeof(int)*no_nodes);
	basis->son     = son     = (int*) malloc(sizeof(int)*no_nodes);
	basis->brother = brother = (int*) malloc(sizeof(int)*no_nodes);
	basis->family  = family  = (int*) malloc(sizeof(int)*no_nodes);
	basis->reverse = reverse = (int*) malloc(sizeof(int)*no_nodes);
	basis->upward  = upward  = (int*) malloc(sizeof(int)*no_nodes);
	basis->dual    = dual    = (double*) malloc(sizeof(double)*no_nodes);
	basis->flow    = flow    = (int*) malloc(sizeof(int)*no_nodes);

	if (!pred || !son || !brother || !family || !reverse || !upward 
		|| !dual || !flow )
		/** run out of memory */

	{
		fprintf (stderr, "run out of memory\n");
		return(ALPS_RUNOUTOFMEMORY);
	}





	pred[0]=-1;
	son[0]=1;
	brother[0]=-1;
	flow[0]=0;
	reverse[0]=-1;
	upward[0]=0;

	for(i=1;i<basis->m;i++){
		flow[i]=demand[i];
		pred[i]=0;
		son[i]=-1;
		family[i]=1;
		brother[i]=i+1;
		reverse[i]=artificial;
	}
	brother[basis->m-1]=-1;

	for(j=0;j<basis->n;j++){

		k=basis->colbeg[j];
		i=basis->colind[k];

		jj=basis->m+j;

		/* insert jj to the successors of i */
		brother[jj]=son[i];
		pred[jj]=i;
		son[i]=jj; 
		son[jj]=-1;

		family[i]++;
		flow[i]+=demand[jj]; 

		family[jj]=1;
		reverse[jj]=k;
		upward[jj]=0;
		flow[jj]=demand[jj];
	}

	for(i=1;i<basis->m;i++){
		if (flow[i]<=0) {
			flow[i]*=-1;
			upward[i]=1;
		}else{
			upward[i]=0;
		} 
	}

	family[0]=basis->no_nodes;
	basis->no_artificial=basis->m-1;
	flow[0]=-1;

	init_dual(basis);

	return 0;     /* initialization ok */
}


/*
*  Function |init_dual|. The following function scans the basis-tree and
* initializes the dual variables
*/

void init_dual(BASIS *basis)
{
	int j,k;
	int *son;
	int *pred;
	int *brother;
	int *upward;
	int *reverse;
	double *dual;
	double *cost;

	pred=basis->pred;
	brother=basis->brother;
	son=basis->son;
	dual=basis->dual;
	cost=basis->cost;
	upward=basis->upward;
	reverse=basis->reverse;

	k=0;
	dual[k]=0.0;  
	if (son[k]<0){ 
		return;		/* subtree has only one node */
	} else {
		k=son[k];
	}

	for( ; ; ) {			/* scanning the basis-tree */
		do{				/* descent in subtree */
			do{

				/* initializing duals */
				if (upward[k]) dual[k] = dual[pred[k]]+ cost[reverse[k]]; 
				else  dual[k] = dual[pred[k]]-cost[reverse[k]];

				j=k;
				k=son[k];
			} while (k>=0);
			k=brother[j];
		} while (k>=0); 
		do {			/* ascent in subtree  */
			k=pred[j];
			if (k==0) return;
			j=k;
			k=brother[k];
		} while (k<0);
	}
}


/*
*  Functions for debugging I/O.
*/

void print_basis(BASIS *basis)
/* prints basis data */
{
	int i;
	int no_nodes;
	int *pred;
	int *son;
	int *brother;
	int *family;
	int *upward;
	int *reverse;
	int *flow;
	double *dual;

	no_nodes = basis->no_nodes;
	pred = basis->pred;
	son = basis->son;
	brother = basis->brother;
	family = basis->family;
	upward = basis->upward;
	reverse = basis->reverse;
	flow = basis->flow;
	dual = basis->dual;

	printf("\nbasis :\n\n");
	printf("+------+-------+-----+---------+--------+-----+---------+----------+--------------+\n"); 
	printf("| node |  pred | son | brother | family | upw | reverse |   flow   |      dual    |\n"); 
	printf("+------+-------+-----+---------+--------+-----+---------+----------+--------------+\n"); 
	for(i=0;i<no_nodes;i++)
		printf("| %3d  |  %3d  | %3d |   %3d   |   %3d  | %2d  |   %3d   | %8d | %12.4lf |\n"
		,i,pred[i],son[i],brother[i],family[i],upward[i],reverse[i],flow[i],dual[i]);
	printf("+------+-------+-----+---------+--------+-----+---------+----------+--------------+\n"); 
} 


void print_data(BASIS *basis)
{
	int i,j,k;
	int artificial;
	int no_nodes;
	int no_arcs;
	int m,n;

	int *demand;
	double *cost;

	artificial=basis->artificial;
	no_nodes=basis->no_nodes;
	no_arcs=basis->no_arcs;
	m=basis->m;
	n=basis->n;
	demand=basis->demand;
	cost=basis->cost;

	printf("+------+----------+\n");
	printf("| node |  demand  |\n");
	printf("+------+----------+\n");
	for(i=0;i<no_nodes;i++) 
		printf("|  %3d | %8d |\n",i,demand[i]); 
	printf("+------+----------+\n");


	printf("\n---- network-data ----\n");
	printf("#nodes : %d\n",no_nodes);
	printf("#arcs  : %d\n",no_arcs);
	printf("artificial  : %d\n\n",artificial);
	printf("+-----+-------------+----------+\n");
	printf("|     |   i  ->  j  |   cost   |\n"); 
	printf("+-----+-------------+----------+\n");

	for (j=0;j<n;j++) {
		for (k=basis->colbeg[j];k<basis->colbeg[j]+basis->colcount[j];k++) {
			i = basis->colind[k];
			printf("| %3d | %3d  -> %3d | %8.4lf |\n",k,i,m+j,cost[k]);
		}
	}
	printf("+-----+-------------+----------+\n\n");      
}

} // end namespace ogdf
