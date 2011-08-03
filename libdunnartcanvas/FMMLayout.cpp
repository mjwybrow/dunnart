/*
 * $Revision: 1.6 $
 * 
 * last checkin:
 *   $Author: gutwenger $ 
 *   $Date: 2007-11-09 12:12:41 +0100 (Fr, 09 Nov 2007) $ 
 ***************************************************************/
 
/** \file
 * \brief Implementation of Fast Multipole Multilevel Method (FM^3).
 * 
 * \author Stefan Hachul
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

#include <QtGlobal>
#include "libdunnartcanvas/FMMLayout.h"

#include <src/energybased/mathExtension.h>
#include <src/energybased/numexcept.h>
#include <src/energybased/MAARPacking.h>
#include <src/energybased/Edge.h>
#include <time.h>
#include <ogdf/basic/simple_graph_alg.h>
#include <ogdf/basic/basic.h>

#include <ogdf/internal/energybased/NodeAttributes.h>
#include <ogdf/internal/energybased/EdgeAttributes.h>
#include <src/energybased/Rectangle.h>

namespace ogdf {

const Graph::NodeType FMMLayout::FIXED = Graph::dummy;
const Graph::NodeType FMMLayout::UNFIXED = Graph::vertex;

FMMLayout::FMMLayout() 
{
  initialize_all_options();   
}

void FMMLayout::call(GraphAttributes &AG, char* ps_file)
{
    call(AG);
    create_postscript_drawing(AG,ps_file);
}
void FMMLayout::call(GraphAttributes &AG)
{
  const Graph &G = AG.constGraph();
  EdgeArray<double> edgelength(G);
  edge e;
  forall_edges(e,G)
    edgelength[e] = 1.0;
  call(AG,edgelength);
}


//--------------------------- most important functions --------------------------------

void FMMLayout :: call(GraphAttributes &GA, const EdgeArray<double> &edgeLength) {
  double t_total;
  const Graph &G = GA.constGraph();
  NodeArray<NodeAttributes> A(G);       //stores the attributes of the nodes (given by L)
  EdgeArray<EdgeAttributes> E(G);       //stores the edge attributes of G
  Graph G_reduced;                      //stores a undirected simple and loopfree copy 
                                        //of G
  EdgeArray<EdgeAttributes> E_reduced;  //stores the edge attributes of G_reduced
  NodeArray<NodeAttributes> A_reduced;  //stores the node attributes of G_reduced 

  if(G.numberOfNodes() > 1)
    {
      GA.clearAllBends();//all are edges straight line
      if(useHighLevelOptions())
        update_low_level_options_due_to_high_level_options_settings();
      import_NodeAttributes(G,GA,A);
      import_EdgeAttributes(G,edgeLength,E);
      
      //times(&t_total);
      usedTime(t_total);
      max_integer_position = pow(2.0,maxIntPosExponent());
      init_ind_ideal_edgelength(G,A,E);  
      make_simple_loopfree(G,A,E,G_reduced,A_reduced,E_reduced);
      call_DIVIDE_ET_IMPERA_step(G_reduced,A_reduced,E_reduced);
      //time_total = get_time(t_total);
      time_total = usedTime(t_total);
            
      export_NodeAttributes(G_reduced,A_reduced,GA);
    }
  else //trivial cases
    { 
      if(G.numberOfNodes() == 1 )
        {
          node v = G.firstNode();
          GA.x(v) = 0;
          GA.y(v) = 0;
        } 
    }
}
void FMMLayout :: call_DIVIDE_ET_IMPERA_step(Graph& G,NodeArray<NodeAttributes>& A, EdgeArray<EdgeAttributes>& E)
{
    int i;
    NodeArray<int> component(G); //holds for each node the index of its component
    number_of_components = connectedComponents(G,component);//calculate components of G
    Graph* G_sub = new Graph[number_of_components];  
    NodeArray<NodeAttributes>* A_sub = new NodeArray<NodeAttributes>[number_of_components]; 
    EdgeArray<EdgeAttributes>* E_sub = new EdgeArray<EdgeAttributes>[number_of_components];
    create_maximum_connected_subGraphs(G,A,E,G_sub,A_sub,E_sub,component);

    for(i = 0; i < number_of_components;i++) {
        call_step_for_subGraph(G_sub[i],A_sub[i],E_sub[i]);
    }

    // need to pass positions of subgraphs back to original graph
    node v_orig;
    forall_nodes(v_orig,G) {
        node v_sub = A[v_orig].get_subgraph_node();      
        A[v_orig].set_position(A_sub[component[v_orig]][v_sub].get_position());   
    }

    delete_all_subGraphs(G_sub,A_sub,E_sub);
}
void FMMLayout :: call_step_for_subGraph(Graph& G,NodeArray<NodeAttributes>& 
                             A,EdgeArray<EdgeAttributes>& E)
{ 
  update_boxlength_and_cornercoordinate(G,A);
  call_FORCE_CALCULATION_step(G,A,E);   
}


void FMMLayout :: call_FORCE_CALCULATION_step(Graph& G,NodeArray<NodeAttributes>&A,
                     EdgeArray<EdgeAttributes>& E)
{
 const int ITERBOUND = 10000;//needed to guarantee termination if 
                             //stopCriterion() == scThreshold
 if(G.numberOfNodes() > 1)
  {
   int iter = 1;
   const int max_iter = 30;
   double actforcevectorlength = threshold() + 1;

   NodeArray<DPoint> F_rep(G); //stores rep. forces 
   NodeArray<DPoint> F_attr(G); //stores attr. forces 
   NodeArray<DPoint> F (G); //stores resulting forces 
   NodeArray<DPoint> last_node_movement(G);//stores the force vectors F of the last
                                           //iterations (needed to avoid oscillations) 

   set_average_ideal_edgelength(G,E);//needed for easy scaling of the forces
   //printf("average_ideal_edgelength = %f\n",average_ideal_edgelength);
   make_initialisations_for_rep_calc_classes(G,A,F_rep);

   while( ((stopCriterion() == scFixedIterations)&&(iter <= max_iter)) ||
          ((stopCriterion() == scThreshold)&&(actforcevectorlength >= threshold())&&
           (iter <= ITERBOUND)) ||
          ((stopCriterion() == scFixedIterationsOrThreshold)&&(iter <= max_iter) &&
           (actforcevectorlength >= threshold())) )
    {//while
     calculate_forces(G,A,E,F,F_attr,F_rep,last_node_movement,iter,0);
     if(stopCriterion() != scFixedIterations)
       actforcevectorlength = get_average_forcevector_length(G,F);
     iter++;
    }//while

   call_POSTPROCESSING_step(G,A,E,F,F_attr,F_rep,last_node_movement);
 
   deallocate_memory_for_rep_calc_classes(); 
 }
}


inline void FMMLayout :: call_POSTPROCESSING_step(Graph& G,NodeArray<NodeAttributes>& A,
                         EdgeArray<EdgeAttributes>& E,NodeArray
                         <DPoint>& F,NodeArray<DPoint>& F_attr, 
                         NodeArray<DPoint>& F_rep,NodeArray<DPoint>
                         & last_node_movement)
{
    int i;

    for(i = 1; i<= 10; i++) {
        calculate_forces(G,A,E,F,F_attr,F_rep,last_node_movement,i,1);
    }

    for(i = 1; i<= fineTuningIterations(); i++) {
        calculate_forces(G,A,E,F,F_attr,F_rep,last_node_movement,i,2);
    }
}
  

//------------------------- functions for pre/post-processing -------------------------

void FMMLayout :: initialize_all_options()
{
    //setting high level optionss
    useHighLevelOptions(false); pageFormat(pfSquare); unitEdgeLength(100); 
    newInitialPlacement(false); qualityVersusSpeed(qvsBeautifulAndFast);

    //setting low level options
    //setting general options
    randSeed(100);edgeLengthMeasurement(elmBoundingCircle);
    allowedPositions(apInteger);maxIntPosExponent(40);

    //setting options for the force calculation step
    forceModel(fmFruchtermanReingold);springStrength(1);repForcesStrength(1);
    repulsiveForcesCalculation(rfcNMM);
    //repulsiveForcesCalculation(rfcExact);
    stopCriterion(scFixedIterationsOrThreshold);
    threshold(0.01);fixedIterations(30);forceScalingFactor(0.05);
    coolTemperature(false);coolValue(0.99);
   
    //setting options for postprocessing
    fineTuningIterations(0);
    fineTuneScalar(0.2);adjustPostRepStrengthDynamically(false);
    postSpringStrength(1.0);postStrengthOfRepForces(1);

    //setting options for different repulsive force calculation methods
    frGridQuotient(2); 
    nmTreeConstruction(rtcSubtreeBySubtree);nmSmallCell(scfIteratively);
    nmParticlesInLeaves(25); nmPrecision(4);   
}


void FMMLayout :: update_low_level_options_due_to_high_level_options_settings()
{
  double uel = unitEdgeLength();
  bool nip = newInitialPlacement();
  QualityVsSpeed qvs = qualityVersusSpeed();

  //update
  initialize_all_options();
  useHighLevelOptions(true);
  unitEdgeLength(uel);
  newInitialPlacement(nip);
  qualityVersusSpeed(qvs);

  if(qualityVersusSpeed() == qvsGorgeousAndEfficient)
    {
      fixedIterations(60);
      fineTuningIterations(40);
      nmPrecision(6);
    }
  else if(qualityVersusSpeed() == qvsBeautifulAndFast)
    {
      fixedIterations(30);
      fineTuningIterations(20);
      nmPrecision(4);
    }
  else //qualityVersusSpeed() == qvsNiceAndIncredibleSpeed
    {
      fixedIterations(15);
      fineTuningIterations(10);
      nmPrecision(2);
    }
}


inline void FMMLayout :: import_NodeAttributes(const Graph& G, GraphAttributes& GA,
                        NodeArray<NodeAttributes>& A)
{
 node v;
 DPoint position;

 forall_nodes(v,G)
   {
     position.m_x = GA.x(v);
     position.m_y = GA.y(v);
     A[v].set_NodeAttributes(GA.width(v),GA.height(v),position,NULL,NULL); 
     Graph::NodeType isFixed = UNFIXED;
     if(GA.attributes()&GraphAttributes::nodeType) {
         isFixed = GA.type(v);
     }
     A[v].set_type(isFixed);
   }
}


inline void FMMLayout :: import_EdgeAttributes(const Graph& G, const EdgeArray<double>& 
                        edgeLength, EdgeArray<EdgeAttributes>& E)
{
 edge e;
 double length;

 forall_edges(e,G)
    {
      if(edgeLength[e] > 0) //no negative edgelength allowed
    length = edgeLength[e];
      else 
    length = 1;
     
      E[e].set_EdgeAttributes(length,NULL,NULL);
    }
}


inline void FMMLayout :: init_ind_ideal_edgelength(const Graph& G,NodeArray
                            <NodeAttributes>& 
                            A,EdgeArray<EdgeAttributes>& E)
{
 edge e;

 if (edgeLengthMeasurement() == elmMidpoint)
   forall_edges(e,G)
    E[e].set_length(E[e].get_length() * unitEdgeLength());
  
 else //(edgeLengthMeasurement() == elmBoundingCircle)
  {
   set_radii(G,A);
   forall_edges(e,G)
     E[e].set_length(E[e].get_length() * unitEdgeLength() + radius[e->source()] 
             + radius[e->target()]);
  }
}


inline void FMMLayout :: set_radii(const Graph& G,NodeArray<NodeAttributes>& A)
{
 node v;
 radius.init(G);
 double w,h;
 forall_nodes(v,G)
 {
  w = A[v].get_width()/2;
  h = A[v].get_height()/2;
  radius[v] = sqrt(w*w+ h*h);
 }
}


inline void FMMLayout :: export_NodeAttributes(Graph& G_reduced, NodeArray
                        <NodeAttributes>& 
                        A_reduced,GraphAttributes& GA)
{
  node v_copy;
  forall_nodes(v_copy,G_reduced)
  {
    GA.x(A_reduced[v_copy].get_original_node()) =  A_reduced[v_copy].get_position().m_x;
    GA.y(A_reduced[v_copy].get_original_node()) =  A_reduced[v_copy].get_position().m_y;
  }
}


void FMMLayout :: make_simple_loopfree(const Graph& G,NodeArray<NodeAttributes>& 
                       A, EdgeArray<EdgeAttributes>E, Graph& G_reduced,
                       NodeArray<NodeAttributes>& A_reduced,EdgeArray
                       <EdgeAttributes>& E_reduced)
{
  node u_orig,v_orig,v_reduced; 
  edge e_reduced,e_orig;

  //create the reduced Graph G_reduced and save in A/E links to node/edges of G_reduced 
  //create G_reduced as a copy of G without selfloops!

  G_reduced.clear();
  forall_nodes(v_orig,G)
     A[v_orig].set_copy_node(G_reduced.newNode());   
  forall_edges(e_orig,G)
    {
      u_orig = e_orig->source();
      v_orig = e_orig->target();
      if(u_orig != v_orig)
    E[e_orig].set_copy_edge(G_reduced.newEdge (A[u_orig].get_copy_node(),
                            A[v_orig].get_copy_node()));
      else 
        E[e_orig].set_copy_edge(NULL);//mark this edge as deleted
    }

  //remove parallel (and reversed) edges from G_reduced
  EdgeArray<double> new_edgelength(G_reduced); 
  List<edge> S;
  S.clear();
  delete_parallel_edges(G,E,G_reduced,S,new_edgelength);

  //make A_reduced, E_reduced valid for G_reduced
  A_reduced.init(G_reduced);
  E_reduced.init(G_reduced);

  //import information for A_reduced, E_reduced and links to the original nodes/edges
  //of the copy nodes/edges
  forall_nodes(v_orig,G)
    {
      v_reduced = A[v_orig].get_copy_node();      
      A_reduced[v_reduced].set_NodeAttributes(A[v_orig].get_width(), A[v_orig].
                          get_height(),A[v_orig].get_position(),
                          v_orig,NULL);      
      A_reduced[v_reduced].set_type(A[v_orig].get_type());
    } 
  forall_edges(e_orig,G)
    {
      e_reduced = E[e_orig].get_copy_edge();
      if(e_reduced != NULL) 
    E_reduced[e_reduced].set_EdgeAttributes(E[e_orig].get_length(),e_orig,NULL);
    } 

  //update edgelength of copy edges in G_reduced associated with a set of parallel 
  //edges in G
  update_edgelength(S,new_edgelength,E_reduced);  
}


void FMMLayout :: delete_parallel_edges(const Graph& G,EdgeArray<EdgeAttributes>& E,
                     Graph& G_reduced,List<edge>& S,EdgeArray<double>
                     & new_edgelength)
{       
  EdgeMaxBucketFunc MaxSort;
  EdgeMinBucketFunc MinSort;           
  ListIterator<Edge> EdgeIterator;
  edge e_act,e_save;
  Edge f_act;
  List<Edge> sorted_edges;  
  EdgeArray<edge> original_edge (G_reduced); //helping array
  int save_s_index,save_t_index,act_s_index,act_t_index;
  int counter = 1;
  Graph* Graph_ptr = &G_reduced;

  //save the original edges for each edge in G_reduced
  forall_edges(e_act,G)
    if(E[e_act].get_copy_edge() != NULL) //e_act is no self_loops
      original_edge[E[e_act].get_copy_edge()] = e_act;
 
  forall_edges(e_act,G_reduced)
    {
      f_act.set_Edge(e_act,Graph_ptr); 
      sorted_edges.pushBack(f_act);
    }
    
  sorted_edges.bucketSort(0,G_reduced.numberOfNodes()-1,MaxSort);
  sorted_edges.bucketSort(0,G_reduced.numberOfNodes()-1,MinSort);

  //now parallel edges are consecutive in sorted_edges
  for(EdgeIterator = sorted_edges.begin();EdgeIterator.valid();++EdgeIterator) 
   {//for
     e_act = (*EdgeIterator).get_edge();
     act_s_index = e_act->source()->index();
     act_t_index = e_act->target()->index();

     if(EdgeIterator != sorted_edges.begin())
      {//if
        if( (act_s_index == save_s_index && act_t_index == save_t_index) ||
        (act_s_index == save_t_index && act_t_index == save_s_index) )
     {         
           if(counter == 1) //first parallel edge
        {
             S.pushBack(e_save);
             new_edgelength[e_save] = E[original_edge[e_save]].get_length() +
                                  E[original_edge[e_act]].get_length();
        }
           else //more then two parallel edges
          new_edgelength[e_save] +=E[original_edge[e_act]].get_length();

           E[original_edge[e_act]].set_copy_edge(NULL); //mark copy of edge as deleted
           G_reduced.delEdge(e_act);                    //delete copy edge in G_reduced
           counter++;
         }
        else 
     { 
           if (counter > 1)
             { 
           new_edgelength[e_save]/=counter; 
           counter = 1;
         }
       save_s_index = act_s_index;
       save_t_index = act_t_index;
           e_save = e_act;     
         }         
      }//if
     else //first edge
      {
    save_s_index = act_s_index;
    save_t_index = act_t_index;
        e_save = e_act;        
      }   
   }//for

 //treat special case (last edges were multiple edges)
 if(counter >1) 
   new_edgelength[e_save]/=counter;
}

  
inline void FMMLayout :: update_edgelength(List<edge>& S,EdgeArray<double>& 
                        new_edgelength,EdgeArray<EdgeAttributes>& 
                        E_reduced)
{
 edge e;
 while (!S.empty())
   {
     e = S.popFrontRet();
     E_reduced[e].set_length(new_edgelength[e]);
   }
}

inline double FMMLayout :: get_post_rep_force_strength(int n)
{
  mathExtension M;  
  return M.min(0.2,400.0/double(n)); 
}


//------------------------- functions for divide et impera step -----------------------

void FMMLayout :: create_maximum_connected_subGraphs(Graph& G,NodeArray<NodeAttributes>&
                A, EdgeArray<EdgeAttributes>&E,Graph G_sub[], NodeArray
                <NodeAttributes> A_sub[], EdgeArray<EdgeAttributes> 
                E_sub[],NodeArray<int>& component)
{
  node u_orig,v_orig,v_sub; 
  edge e_sub,e_orig;
  int i;

  //create the subgraphs and save links to subgraph nodes/edges in A
  forall_nodes(v_orig,G)
     A[v_orig].set_subgraph_node(G_sub[component[v_orig]].newNode());   
  forall_edges(e_orig,G)
    {
      u_orig = e_orig->source();
      v_orig = e_orig->target();
      E[e_orig].set_subgraph_edge( G_sub[component[u_orig]].newEdge
        (A[u_orig].get_subgraph_node(),A[v_orig].get_subgraph_node()));
    }

  //make A_sub,E_sub valid for the subgraphs
  for(i = 0; i< number_of_components;i++)
    {
      A_sub[i].init(G_sub[i]);
      E_sub[i].init(G_sub[i]);
    }

  //import information for A_sub,E_sub and links to the original nodes/edges
  //of the subGraph nodes/edges

  forall_nodes(v_orig,G)
    {
      v_sub = A[v_orig].get_subgraph_node();      
      A_sub[component[v_orig]][v_sub].set_NodeAttributes(A[v_orig].get_width(),
                                         A[v_orig].get_height(),A[v_orig].get_position(),
                     v_orig,NULL);      
      A_sub[component[v_orig]][v_sub].set_type(A[v_orig].get_type());
    } 
  forall_edges(e_orig,G)
    {
      e_sub = E[e_orig].get_subgraph_edge(); 
      v_orig = e_orig->source();   
      E_sub[component[v_orig]][e_sub].set_EdgeAttributes(E[e_orig].get_length(),
                             e_orig,NULL);
    }

}


inline void FMMLayout :: calculate_bounding_rectangles_of_components(List<Rectangle>& R,
                    Graph G_sub[],NodeArray<NodeAttributes> A_sub[])
{
 int i;
 Rectangle r;
 R.clear();

 for(i=0;i<number_of_components;i++)
  {
   r = calculate_bounding_rectangle(G_sub[i],A_sub[i],i);
   R.pushBack(r); 
  }
}


Rectangle FMMLayout :: calculate_bounding_rectangle(Graph& G,NodeArray<NodeAttributes>& 
                             A, int componenet_index)
{
 mathExtension M;
 Rectangle r;
 node v;
 double x_min,x_max,y_min,y_max,act_x_min,act_x_max,act_y_min,act_y_max;
 double max_boundary;//the maximum of half of the width and half of the height of
                     //each node; (needed to be able to tipp rectangles over without
                     //having access to the height and width of each node) 

 forall_nodes(v,G)
  {  
   max_boundary = M.max(A[v].get_width()/2, A[v].get_height()/2); 
   if(v == G.firstNode())
     { 
       x_min = A[v].get_x() - max_boundary;
       x_max = A[v].get_x() + max_boundary;
       y_min = A[v].get_y() - max_boundary;
       y_max = A[v].get_y() + max_boundary;
     }
   else
     {
       act_x_min = A[v].get_x() - max_boundary;
       act_x_max = A[v].get_x() + max_boundary;
       act_y_min = A[v].get_y() - max_boundary;
       act_y_max = A[v].get_y() + max_boundary;
       if(act_x_min < x_min) x_min = act_x_min;
       if(act_x_max > x_max) x_max = act_x_max;
       if(act_y_min < y_min) y_min = act_y_min;
       if(act_y_max > y_max) y_max = act_y_max;
     }
  }
    
 r.set_rectangle(x_max-x_min,y_max-y_min,x_min,y_min,componenet_index);
 return r;
}


inline void FMMLayout :: delete_all_subGraphs(Graph G_sub[], NodeArray<NodeAttributes> 
                     A_sub[],EdgeArray<EdgeAttributes> E_sub[])
{
 delete [] G_sub;
 delete [] A_sub;
 delete [] E_sub;
}



//-------------------------- functions for force calculation ---------------------------

inline void FMMLayout :: calculate_forces(Graph& G, NodeArray<NodeAttributes>& A,
                       EdgeArray<EdgeAttributes>& E,NodeArray<DPoint>
                       & F,NodeArray<DPoint>& F_attr, NodeArray
                       <DPoint>& F_rep,NodeArray<DPoint>& 
                       last_node_movement,int iter,int 
                       fine_tuning_step)
{
    //printf("calculate_forces: fine_tuning=%d\n",fine_tuning_step);
    calculate_attractive_forces(G,A,E,F_attr);
    calculate_repulsive_forces(G,A,F_rep);
    add_attr_rep_forces(G,A,F_attr,F_rep,F,iter,fine_tuning_step);
    prevent_oscillations(G,F,last_node_movement,iter);
    move_nodes(G,A,F); 
    update_boxlength_and_cornercoordinate(G,A);
}


inline void FMMLayout :: init_F(Graph& G, NodeArray<DPoint>& F)
{
 DPoint nullpoint (0,0);
 node v;
 forall_nodes(v,G)
  F[v] = nullpoint;
}


inline void FMMLayout:: make_initialisations_for_rep_calc_classes(Graph& G,NodeArray
                  <NodeAttributes> &A, NodeArray<DPoint>& F_rep)
{
    // Avoid unused variable warnings:
    Q_UNUSED (A);
    Q_UNUSED (F_rep);

  if(repulsiveForcesCalculation() == rfcExact)
     FR.make_initialisations(boxlength,down_left_corner,frGridQuotient());
  else if(repulsiveForcesCalculation() == rfcGridApproximation)
     FR.make_initialisations(boxlength,down_left_corner,frGridQuotient());
  else //(repulsiveForcesCalculation() == rfcNMM
    NM.make_initialisations(G,boxlength,down_left_corner,
                nmParticlesInLeaves(),nmPrecision(),
                nmTreeConstruction(),nmSmallCell());
}


inline void FMMLayout :: calculate_repulsive_forces(Graph &G, NodeArray<NodeAttributes> 
                             &A, NodeArray<DPoint>& F_rep)
{
 if(repulsiveForcesCalculation() == rfcExact )
    FR.calculate_exact_repulsive_forces(G,A,F_rep);
 else if(repulsiveForcesCalculation() == rfcGridApproximation )
    FR.calculate_approx_repulsive_forces(G,A,F_rep);
 else //repulsiveForcesCalculation() == rfcNMM
    NM.calculate_repulsive_forces(G,A,F_rep);
}


inline void FMMLayout :: deallocate_memory_for_rep_calc_classes()
{
  if(repulsiveForcesCalculation() == rfcNMM)
    NM.deallocate_memory();
}


void FMMLayout :: calculate_attractive_forces(Graph& G,NodeArray<NodeAttributes> & A, 
                     EdgeArray<EdgeAttributes> & E,NodeArray
                     <DPoint>& F_attr)
{
    mathExtension M;
    numexcept N;
    edge e;
    node u,v;
    double norm_v_minus_u,scalar;
    DPoint vector_v_minus_u,f_u;
    DPoint nullpoint (0,0);

    //initialisation 
    init_F(G,F_attr); 

    //calculation
    forall_edges (e,G) {
        u = e->source();
        v = e->target();
        vector_v_minus_u  = A[v].get_position() - A[u].get_position() ; 
        norm_v_minus_u = M.norm(vector_v_minus_u);
        if(vector_v_minus_u == nullpoint) {
            f_u = nullpoint;
        } else if(!N.f_near_machine_precision(norm_v_minus_u,f_u)) {
            scalar = f_attr_scalar(norm_v_minus_u,E[e].get_length())/norm_v_minus_u;
            f_u.m_x = scalar * vector_v_minus_u.m_x;
            f_u.m_y = scalar * vector_v_minus_u.m_y;
        }

        F_attr[v] = F_attr[v] - f_u;
        F_attr[u] = F_attr[u] + f_u;
    }//for
}


double FMMLayout :: f_attr_scalar (double d,double ind_ideal_edge_length)
{
  mathExtension M;
  double s;

  if(forceModel() == fmFruchtermanReingold)  
     s =  d*d/(ind_ideal_edge_length*ind_ideal_edge_length*ind_ideal_edge_length);
  else if (forceModel() == fmEades)
    {
      double c = 10;
      if (d == 0) 
        s = -1e10;
      else 
        s =  c * M.Log2(d/ind_ideal_edge_length) /(ind_ideal_edge_length);
    }
  else if (forceModel() == fmNew)
    {
      double c =  M.Log2(d/ind_ideal_edge_length);
      if (d > 0) 
    s =  c * d * d /
        (ind_ideal_edge_length * ind_ideal_edge_length * ind_ideal_edge_length); 
      else 
    s = -1e10;
    }
  else cout <<" Error FMMLayout:: f_attr_scalar"<<endl;
   
 return s;
} 
      

void FMMLayout :: add_attr_rep_forces(Graph& G, NodeArray<NodeAttributes>& A, NodeArray<DPoint>& F_attr,NodeArray
                       <DPoint>& F_rep,NodeArray<DPoint>& F,int iter,int
                       fine_tuning_step)
{
 mathExtension M;
 numexcept N;
 node v;
 DPoint f,force;
 DPoint nullpoint (0,0);
 double norm_f,scalar;
 double act_spring_strength,act_rep_force_strength;
 
 //set cool_factor
 if(coolTemperature() == false)
   cool_factor = 1.0;
 else if((coolTemperature() == true) && (fine_tuning_step == 0))
  {
   if(iter == 1)
     cool_factor = coolValue();
   else 
     cool_factor *= coolValue();
  }

 if(fine_tuning_step == 1)
   cool_factor /= 10.0; //decrease the temperature rapidly
 else if (fine_tuning_step == 2)
  {
    if(iter <= fineTuningIterations() -5)
      cool_factor = fineTuneScalar(); //decrease the temperature rapidly
    else
      cool_factor = (fineTuneScalar()/10.0);          
  }

 //set the values for the spring strength and strength of the rep. force field
 if(fine_tuning_step <= 1)//usual case
   {
     act_spring_strength = springStrength();
     act_rep_force_strength = repForcesStrength(); 
   }
 else if(!adjustPostRepStrengthDynamically())
   {
     act_spring_strength = postSpringStrength();
     act_rep_force_strength = postStrengthOfRepForces();
   }
 else //adjustPostRepStrengthDynamically())
   {
     act_spring_strength = postSpringStrength();
     act_rep_force_strength = get_post_rep_force_strength(G.numberOfNodes());
   }
 //printf("FMM: act_spring_strength = %f\n",act_spring_strength);
 //printf("act_rep_force_strength = %f\n",act_rep_force_strength);

 forall_nodes(v,G)
  {
     if(A[v].get_type()==FIXED) {
         force = nullpoint;
     } else {
       f.m_x = act_spring_strength * F_attr[v].m_x + act_rep_force_strength * F_rep[v].m_x;
       f.m_y = act_spring_strength * F_attr[v].m_y + act_rep_force_strength * F_rep[v].m_y;
       f.m_x = average_ideal_edgelength * average_ideal_edgelength * f.m_x;
       f.m_y = average_ideal_edgelength * average_ideal_edgelength * f.m_y;

       norm_f = M.norm(f);
       if(f == nullpoint)
         force = nullpoint;
       else if(N.f_near_machine_precision(norm_f,force))
         restrict_force_to_comp_box(force);
       else
         {
           scalar = M.min (norm_f * cool_factor * forceScalingFactor(),
                   max_radius(iter))/norm_f;
           force.m_x = scalar * f.m_x;
           force.m_y = scalar * f.m_y;  
         }
     }
   F[v] = force;
  } 
}


inline void FMMLayout :: move_nodes(Graph& G,NodeArray<NodeAttributes>& A,NodeArray<DPoint>& F)
{        
 node v;

 forall_nodes(v,G) {
     if(A[v].get_type()==UNFIXED) {
       A[v].set_position(A[v].get_position() + F[v]);
     } else {
            //cout << "FMM:fixed["<<v->index()<<"]@ "<<A[v].get_position().m_x<<","<<A[v].get_position().m_y<<endl;
     }
 }
}


void FMMLayout :: update_boxlength_and_cornercoordinate(Graph& G,NodeArray
                             <NodeAttributes>&A)
{
    mathExtension M;
    node v;
    double xmin,xmax,ymin,ymax;
    DPoint midpoint;


    v = G.firstNode();
    midpoint = A[v].get_position();
    xmin = xmax = midpoint.m_x;
    ymin = ymax = midpoint.m_y;

    forall_nodes(v,G) {
        midpoint = A[v].get_position();
        if (midpoint.m_x < xmin ) xmin = midpoint.m_x;
        if (midpoint.m_x > xmax ) xmax = midpoint.m_x;
        if (midpoint.m_y < ymin ) ymin = midpoint.m_y;
        if (midpoint.m_y > ymax ) ymax = midpoint.m_y;
    }    

    //set down_left_corner and boxlength

    down_left_corner.m_x = floor(xmin - 1);
    down_left_corner.m_y = floor(ymin - 1);
    boxlength = ceil(M.max(ymax-ymin, xmax-xmin) *1.01 + 2);

    //exception handling: all nodes have same x and y coordinate
    if(boxlength <= 2 )
    {
        boxlength = G.numberOfNodes()* 20;
        down_left_corner.m_x = floor(xmin) - (boxlength/2);
        down_left_corner.m_y = floor(ymin) - (boxlength/2);
    }   


    //export the boxlength and down_left_corner values to the rep. calc. classes

    if(repulsiveForcesCalculation() == rfcExact || repulsiveForcesCalculation() == rfcGridApproximation) {
        FR.update_boxlength_and_cornercoordinate(boxlength,down_left_corner);
    } else { //repulsiveForcesCalculation() == rfcNMM
        NM.update_boxlength_and_cornercoordinate(boxlength,down_left_corner);
    }
}


inline double FMMLayout :: max_radius(int iter)
{ //a really easy function
  if(iter == 1)
    return boxlength/1000;  
  else
    return boxlength/5; 
}

 
inline void FMMLayout :: set_average_ideal_edgelength(Graph& G,EdgeArray
                               <EdgeAttributes>& E)
{
 double averagelength = 0;
 edge e;

 if(G.numberOfEdges() > 0)
   {
    forall_edges(e,G)
       averagelength += E[e].get_length();
    average_ideal_edgelength = averagelength/G.numberOfEdges();
   }
 else
  average_ideal_edgelength = 50;
}


inline double FMMLayout :: get_average_forcevector_length (Graph& G,NodeArray<DPoint>& F)
{
  mathExtension M;
  double lengthsum = 0;
  node v;
  forall_nodes(v,G)
    lengthsum += M.norm(F[v]);
  lengthsum /=G.numberOfNodes();
  return lengthsum;
}


void FMMLayout :: prevent_oscillations(Graph& G, NodeArray<DPoint>& F,NodeArray<DPoint>& 
                 last_node_movement,int iter)
{

 mathExtension M;
 const double pi_times_1_over_6 = 0.52359878;
 const double pi_times_2_over_6 = 2 * pi_times_1_over_6;
 const double pi_times_3_over_6 = 3 * pi_times_1_over_6;
 const double pi_times_4_over_6 = 4 * pi_times_1_over_6;
 const double pi_times_5_over_6 = 5 * pi_times_1_over_6;
 const double pi_times_7_over_6 = 7 * pi_times_1_over_6;
 const double pi_times_8_over_6 = 8 * pi_times_1_over_6;
 const double pi_times_9_over_6 = 9 * pi_times_1_over_6;
 const double pi_times_10_over_6 = 10 * pi_times_1_over_6;
 const double pi_times_11_over_6 = 11 * pi_times_1_over_6;

 DPoint nullpoint (0,0);
 double fi; //angle in [0,2pi) measured counterclockwise 
 double norm_old,norm_new,quot_old_new;

 if (iter > 1) //usual case
  {//if1
   node v;
   forall_nodes(v,G)
    {
      DPoint force_new (F[v].m_x,F[v].m_y); 
      DPoint force_old (last_node_movement[v].m_x,last_node_movement[v].m_y);
      norm_new = M.norm(F[v]);
      norm_old  = M.norm(last_node_movement[v]);
      if ((norm_new > 0) && (norm_old > 0))
       {//if2  
      quot_old_new =  norm_old / norm_new;
 
         //prevent oscillations
     fi = angle(nullpoint,force_old,force_new);
     if(((fi <= pi_times_1_over_6)||(fi >= pi_times_11_over_6))&& 
        ((norm_new > (norm_old*2.0))) )
       {
         F[v].m_x = quot_old_new * 2.0 * F[v].m_x;
         F[v].m_y = quot_old_new * 2.0 * F[v].m_y;
       }
         else if ((fi >= pi_times_1_over_6)&&(fi <= pi_times_2_over_6)&& 
          (norm_new > (norm_old*1.5) ) )
       {
         F[v].m_x = quot_old_new * 1.5 * F[v].m_x;
         F[v].m_y = quot_old_new * 1.5 * F[v].m_y;
       }
         else if ((fi >= pi_times_2_over_6)&&(fi <= pi_times_3_over_6)&& 
          (norm_new > (norm_old)) )
       {
         F[v].m_x = quot_old_new * F[v].m_x;
         F[v].m_y = quot_old_new * F[v].m_y;
       }
         else if ((fi >= pi_times_3_over_6)&&(fi <= pi_times_4_over_6)&& 
          (norm_new > (norm_old*0.66666666)) )
       {
         F[v].m_x = quot_old_new * 0.66666666 * F[v].m_x;
         F[v].m_y = quot_old_new * 0.66666666 * F[v].m_y;
       }
     else if ((fi >= pi_times_4_over_6)&&(fi <= pi_times_5_over_6)&& 
          (norm_new > (norm_old*0.5)) )
       {
         F[v].m_x = quot_old_new * 0.5 * F[v].m_x;
         F[v].m_y = quot_old_new * 0.5 * F[v].m_y;
       }
     else if ((fi >= pi_times_5_over_6)&&(fi <= pi_times_7_over_6)&& 
          (norm_new > (norm_old*0.33333333)) )
       {
         F[v].m_x = quot_old_new * 0.33333333 * F[v].m_x;
         F[v].m_y = quot_old_new * 0.33333333 * F[v].m_y;
       }
     else if ((fi >= pi_times_7_over_6)&&(fi <= pi_times_8_over_6)&& 
          (norm_new > (norm_old*0.5)) )
       {
         F[v].m_x = quot_old_new * 0.5 * F[v].m_x;
         F[v].m_y = quot_old_new * 0.5 * F[v].m_y;
       }
     else if ((fi >= pi_times_8_over_6)&&(fi <= pi_times_9_over_6)&& 
          (norm_new > (norm_old*0.66666666)) )
       {
         F[v].m_x = quot_old_new * 0.66666666 * F[v].m_x;
         F[v].m_y = quot_old_new * 0.66666666 * F[v].m_y;
       }
     else if ((fi >= pi_times_9_over_6)&&(fi <= pi_times_10_over_6)&& 
          (norm_new > (norm_old)) )
       {
         F[v].m_x = quot_old_new * F[v].m_x;
         F[v].m_y = quot_old_new * F[v].m_y;
       }
     else if ((fi >= pi_times_10_over_6)&&(fi <= pi_times_11_over_6)&& 
          (norm_new > (norm_old*1.5) ) )
       {
         F[v].m_x = quot_old_new * 1.5 * F[v].m_x;
         F[v].m_y = quot_old_new * 1.5 * F[v].m_y;
       }
       }//if2 
      last_node_movement[v]= F[v];    
    }
  }//if1
 else if (iter == 1)
   init_last_node_movement(G,F,last_node_movement);  
}


double FMMLayout :: angle(DPoint& P,DPoint& Q, DPoint& R)
{
  double PI = 3.1415927;
  double dx1 = Q.m_x - P.m_x; 
  double dy1 = Q.m_y - P.m_y; 
  double dx2 = R.m_x - P.m_x; 
  double dy2 = R.m_y - P.m_y; 
  double fi;//the angle

  if ((dx1 == 0 && dy1 == 0) || (dx2 == 0 && dy2 == 0)) 
     cout<<"Multilevel::angle()"<<endl;
  
  double norm  = (dx1*dx1+dy1*dy1)*(dx2*dx2+dy2*dy2);
  double cosfi = (dx1*dx2+dy1*dy2) / sqrt(norm);

  if (cosfi >=  1.0 ) fi = 0;
  if (cosfi <= -1.0 ) fi = PI;
  else
    {
      fi = acos(cosfi);
      if (dx1*dy2 < dy1*dx2) fi = -fi;
      if (fi < 0) fi += 2*PI; 
    }
  return fi;
}


inline void FMMLayout :: init_last_node_movement(Graph& G, NodeArray<DPoint>& F,
                          NodeArray<DPoint>& last_node_movement)
{
 node v;
 forall_nodes(v,G)
    last_node_movement[v]= F[v];
}  


inline void FMMLayout :: restrict_force_to_comp_box(DPoint& force)
{
 double x_min = down_left_corner.m_x;
 double x_max = down_left_corner.m_x+boxlength;
 double y_min = down_left_corner.m_y;
 double y_max = down_left_corner.m_y+boxlength;
 if (force.m_x < x_min )
     force.m_x = x_min;
 else if (force.m_x > x_max )
     force.m_x = x_max;
 if (force.m_y < y_min )
     force.m_y = y_min;
 else if (force.m_y > y_max )
     force.m_y = y_max;
}


//------------------ functions for analytic information --------------------------------

/*inline clock_t FMMLayout :: get_time(tms t)
{
  tms now;
  times(&now);
  clock_t dif = now.tms_utime - t.tms_utime;
  return dif;
}
*/

double FMMLayout :: getCpuTime()
{
    return time_total;
  //return (double(time_total)/sysconf(_SC_CLK_TCK));
}

void FMMLayout :: create_postscript_drawing(GraphAttributes& AG, char* ps_file)
{
    ofstream out_fmmm (ps_file,ios::out);
    if (!ps_file) cout<<ps_file<<" could not be opened !"<<endl; 
    const Graph& G = AG.constGraph();
    node v; 
    edge e;
    double x_min = AG.x(G.firstNode());
    double x_max = x_min;
    double y_min = AG.y(G.firstNode());
    double y_max = y_min;
    double max_dist;
    mathExtension M;
    double scale_factor;

    forall_nodes(v,G)
    {
        x_min = min(AG.x(v),x_min);
        x_max = max(AG.x(v),x_max);
        y_min = min(AG.y(v),y_min);
        y_max = max(AG.y(v),y_max);
    }
    //printf("width=%f, height=%f\n",x_max-x_min, y_max-y_min);
    max_dist = M.max(x_max -x_min,y_max-y_min);
    scale_factor = 500.0/max_dist;

  out_fmmm<<"%!PS-Adobe-2.0 "<<endl;
  out_fmmm<<"%%Pages:  1 "<<endl;
  out_fmmm<<"% %BoundingBox: "<<x_min<<" "<<x_max<<" "<<y_min<<" "<<y_max<<endl;
  out_fmmm<<"%%EndComments "<<endl;
  out_fmmm<<"%%"<<endl;
  out_fmmm<<"%% Circle"<<endl;
  out_fmmm<<"/ellipse_dict 4 dict def"<<endl;
  out_fmmm<<"/ellipse {"<<endl;
  out_fmmm<<"  ellipse_dict"<<endl;
  out_fmmm<<"  begin"<<endl;
  out_fmmm<<"   newpath"<<endl;
  out_fmmm<<"   /yrad exch def /xrad exch def /ypos exch def /xpos exch def"<<endl;
  out_fmmm<<"   matrix currentmatrix"<<endl;
  out_fmmm<<"   xpos ypos translate"<<endl;
  out_fmmm<<"   xrad yrad scale"<<endl;
  out_fmmm<<"  0 0 1 0 360 arc"<<endl;
  out_fmmm<<"  setmatrix"<<endl;
  out_fmmm<<"  closepath"<<endl;
  out_fmmm<<" end"<<endl;
  out_fmmm<<"} def"<<endl;
  out_fmmm<<"%% Nodes"<<endl;
  out_fmmm<<"/v { "<<endl;
  out_fmmm<<" /y exch def"<<endl;
  out_fmmm<<" /x exch def"<<endl;
  out_fmmm<<"1.000 1.000 0.894 setrgbcolor"<<endl;
  out_fmmm<<"x y 10.0 10.0 ellipse fill"<<endl;
  out_fmmm<<"0.000 0.000 0.000 setrgbcolor"<<endl;
  out_fmmm<<"x y 10.0 10.0 ellipse stroke"<<endl;
  out_fmmm<<"} def"<<endl;
  out_fmmm<<"%% Edges"<<endl;
  out_fmmm<<"/e { "<<endl;
  out_fmmm<<" /b exch def"<<endl;
  out_fmmm<<" /a exch def"<<endl;
  out_fmmm<<" /y exch def"<<endl;
  out_fmmm<<" /x exch def"<<endl;
  out_fmmm<<"x y moveto a b lineto stroke"<<endl;
  out_fmmm<<"} def"<<endl;
  out_fmmm<<"%% "<<endl;
  out_fmmm<<"%% INIT "<<endl;
  out_fmmm<<"20  200 translate"<<endl;
  out_fmmm<<scale_factor<<"  "<<scale_factor<<"  scale "<<endl;
  out_fmmm<<"1 setlinewidth "<<endl;
  out_fmmm<<"%%BeginProgram "<<endl;
  forall_edges(e,G)
      out_fmmm<<AG.x(e->source())<<" "<<AG.y(e->source())<<" "
	      <<AG.x(e->target())<<" "<<AG.y(e->target())<<" e"<<endl;
  forall_nodes(v,G)
      out_fmmm<<AG.x(v)<<" "<<AG.y(v) <<" v"<<endl;
  out_fmmm<<"%%EndProgram "<<endl;
  out_fmmm<<"showpage "<<endl;
  out_fmmm<<"%%EOF "<<endl;
}

} //end namespace ogdf
// vim: filetype=cpp:cindent:expandtab:shiftwidth=4:tabstop=4:softtabstop=4 :

