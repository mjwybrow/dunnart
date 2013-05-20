/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2007  Michael Woodward
 * Copyright (C) 2008  Monash University
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA  02110-1301, USA.
 * 
 *
 * Author(s): Michael Woodward
 *            Michael Wybrow  <http://michael.wybrow.info/>
*/

#include "libdunnartcanvas/canvas.h"

#ifndef NOGRAPHVIZ
/**
** see header file for documentation.
*/

#include <sstream>
#include <map>
#include <queue>
#include <deque>

#include <gvc.h>

#include "libdunnartcanvas/align3.h"
#include "libdunnartcanvas/shape.h"
#include "libdunnartcanvas/undo.h"
#include "libdunnartcanvas/expand_grid.h"
#include "libdunnartcanvas/canvasitem.h"
#include "libdunnartcanvas/oldcanvas.h"
#include "libdunnartcanvas/connector.h"
#include "libdunnartcanvas/graphlayout.h"
#include "libdunnartcanvas/canvasview.h"
#include "libdunnartcanvas/visibility.h"

#include "libavoid/geomtypes.h"
#include "libavoid/router.h"
using Avoid::Point;

namespace dunnart {


int count; 
const int MAX_ROWS = 41;
const int MAX_COLS = 39;
bool DO_ALIGNING_STEP = false;

enum Direction { UP, RIGHT, DOWN, LEFT };

//in this new algorithm i'll want to remove previ and prevj I think, but i'll leave that till later.
class GridPoint {
public:
    int i;
    int j;
    int previ;
    int prevj;

    GridPoint(){}
    GridPoint(int ci, int cj, int pi, int pj)
    {
        i = ci;
        j = cj;
        previ = pi;
        prevj = pj;
    }
    GridPoint(int ci, int cj) //, int pi, int pj
    {
        i = ci;
        j = cj;
        //previ = pi;
        //prevj = pj;
    }
    GridPoint(const GridPoint &p)
    {
        i = p.i;
        j = p.j;
        //previ = p.previ;
        //prevj = p.prevj;
    }

    bool operator==(const GridPoint &other) const {
        return this->i == other.i && this->j == other.j;    //don't compare previ and prevj... currently doesn't matter, may be undesirable anyway
    }
    bool operator<(const GridPoint &other) const {
        return this->i < other.i || (this->i == other.i && this->j < other.j);
    }

};


bool on_both_segments(double xs, double ys, int p[2][2][2])
{
    return (   xs >= qMin(p[0][0][0], p[0][1][0]) && xs <= qMax(p[0][0][0], p[0][1][0])
               && ys >= qMin(p[0][0][1], p[0][1][1]) && ys <= qMax(p[0][0][1], p[0][1][1])
               && xs >= qMin(p[1][0][0], p[1][1][0]) && xs <= qMax(p[1][0][0], p[1][1][0])
               && ys >= qMin(p[1][0][1], p[1][1][1]) && ys <= qMax(p[1][0][1], p[1][1][1])
               );
}

//as above but point must not be at the end of both segments.
bool on_both_segments2(double xs, double ys, int p[2][2][2])
{
    return on_both_segments(xs, ys, p) &&
               ((   xs > qMin(p[0][0][0], p[0][1][0]) && xs < qMax(p[0][0][0], p[0][1][0])
               || ys > qMin(p[0][0][1], p[0][1][1]) && ys < qMax(p[0][0][1], p[0][1][1]))
               || (xs > qMin(p[1][0][0], p[1][1][0]) && xs < qMax(p[1][0][0], p[1][1][0])
               || ys > qMin(p[1][0][1], p[1][1][1]) && ys < qMax(p[1][0][1], p[1][1][1]))
                   );

}

//determines if the lines defined by c intersect.
//if common_node is true, intersections can only be accepted if the lines are parallel.
//otherwise, the intersection could be caused by two connections coming out from the centre of the same shape.
//[edge][point][coord]
bool intersection(int p[2][2][2], bool common_node)
{
    double m1;
    double m2;
    double c1;
    double c2;
    double xs;
    double ys;

    if (count == 3)
    {
        printf("(%d, %d)->(%d,%d), (%d,%d)->(%d,%d): ", p[0][0][0], p[0][0][1], p[0][1][0], p[0][1][1], p[1][0][0], p[1][0][1], p[1][1][0], p[1][1][1]);
        if (p[0][0][0] == 2 && p[0][0][1] == 6 && p[0][1][0] == 2 && p[0][1][1] == 5 && p[1][0][0] == 4 && p[1][0][1] == 8 && p[1][1][0] == 1 && p[1][1][1] == 6)
        {
            printf("this is the one...\n");
        }
    }
    if (p[0][1][0] - p[0][0][0] == 0)
    {
        if (p[1][1][0] - p[1][0][0] == 0)    //both lines vertical
        {

            //intersection only if same line and segments overlap
            bool result =  (p[0][0][0] == p[1][0][0]) && on_both_segments2(p[0][0][0], p[0][0][1], p)
            || on_both_segments2(p[0][1][0], p[0][1][1], p)
            || on_both_segments2(p[1][0][0], p[1][0][1], p)
            || on_both_segments2(p[1][1][0], p[1][1][1], p);

            /*((p[0][0][1] >= qMin(p[1][0][1], p[1][1][1]) && p[0][0][1] <= qMax(p[1][0][1], p[1][1][1]))
                                                          || (p[0][1][1] >= qMin(p[1][0][1], p[1][1][1]) && p[0][1][1] <= qMax(p[1][0][1], p[1][1][1]))
                                                          ||(p[1][0][1] >= qMin(p[0][0][1], p[0][1][1]) && p[1][0][1] <= qMax(p[0][0][1], p[0][1][1]))
                                                          || (p[1][1][1] >= qMin(p[0][0][1], p[0][1][1]) && p[1][1][1] <= qMax(p[0][0][1], p[0][1][1])));*/
            if (count==3)
                printf("%s(both vert)\n", result?"TRUE":"FALSE");
            return result;
        }
        else
        {
            //first line vertical
            xs = p[0][1][0];
            m2 = (p[1][1][1] - p[1][0][1])/(double)(p[1][1][0] - p[1][0][0]);
            c2 = p[1][0][1] - m2*p[1][0][0];    //c = y - mx
            ys = m2 * xs + c2;
        }
    }
    else if (p[1][1][0] - p[1][0][0] == 0)
    {
        //second line vertical;
        xs = p[1][1][0];
        m1 = (p[0][1][1] - p[0][0][1])/(double)(p[0][1][0] - p[0][0][0]);
        c1 = p[0][0][1] - m1*p[0][0][0];    //c = y - mx
        ys = m1 * xs + c1;
    }
    else
    {
        //neither line vertical, normal case.
        m1 = (p[0][1][1] - p[0][0][1])/(double)(p[0][1][0] - p[0][0][0]);
        m2 = (p[1][1][1] - p[1][0][1])/(double)(p[1][1][0] - p[1][0][0]);
        c1 = p[0][0][1] - m1*p[0][0][0];    //c = y - mx
        c2 = p[1][0][1] - m2*p[1][0][0];    //c = y - mx

        if (m1 == m2)    //parallel lines
        {
            if (count==3 && m1==0 && m2==0)
                printf("...parallel horizontal lines...");
            if (c1==c2)
            {
                if (count==3 && m1==0 && m2==0)
                    printf("...same parallels horizontal lines...");
                //parallel lines... intersect if any end of a segment lies on the other segment
                bool result = on_both_segments2(p[0][0][0], p[0][0][1], p)
                              || on_both_segments2(p[0][1][0], p[0][1][1], p)
                              || on_both_segments2(p[1][0][0], p[1][0][1], p)
                              || on_both_segments2(p[1][1][0], p[1][1][1], p);
                if (count==3) printf("%s(parallel same lines)\n", result?"TRUE":"FALSE");
                return result;
            }
            else
            {
                if (count==3) printf("FALSE (different parallel lines)\n");
                return false;
            }
        }
        xs = (c2-c1)/(m1-m2);
        ys = m1*xs+c1;
    }

    //if this point is reached, the two lines intersection, simply check whether the intersection is on both line segments
    bool result = on_both_segments(xs, ys, p) && !common_node;
    if (count==3)
        printf("%s\n", (result ? "TRUE" : "FALSE"));
    return result;
}


//return the number of edge crossings edges of node n are related in if node n is at i,j
int edge_crossings(Canvas *canvas, Agnode_t *n, int i, int j, Agraph_t* gv, std::map<Agnode_t*, ShapeObj*> &dunnart_nodes, std::map<ShapeObj*, Agnode_t*> &gv_nodes, Grid &grid)
{
    if (count == 3)
        printf("%s!\n", dunnart_nodes[n]->getName());

    //outer/inner edge   start/end    i/j
    int p[2][2][2];

    //these always hold the grid points of node n
    //i becomes y, j becomes x
    p[1][0][1] = i;
    p[1][0][0] = j;

    int crossings = 0;

    QList<CanvasItem *> canvas_items = canvas->items();
    for (int i = 0; i < canvas_items.size(); ++i)
    {  
                Connector *conn = dynamic_cast<Connector *>(canvas_items.at(i));
        if (conn)
        {  
            std::pair<ShapeObj *, ShapeObj *> attachees = 
                    conn->getAttachedShapes();

            //ignore connectors that come off n in the outer loop
            //if (gv_nodes[attachees.first] != n &&
            //    gv_nodes[attachees.second] != n)
            //{
            bool common_with_n = (gv_nodes[attachees.first] == n || gv_nodes[attachees.second] == n);

            grid.get_grid_coords(attachees.first, &p[0][0][1], &p[0][0][0]);
            grid.get_grid_coords(attachees.second, &p[0][1][1], &p[0][1][0]);

            Agedge_t *e;
            Agnode_t *next;
            for (e = agfstout(gv, n); e; e = agnxtout(gv, e)) {
                next = (e->head == n ? e->tail : e->head);
                //grid points of the node connected to node n
                grid.get_grid_coords(dunnart_nodes[next], &p[1][1][1], &p[1][1][0]);

                bool common_with_next = (gv_nodes[attachees.first] == next || gv_nodes[attachees.second] == next);
                if (!(common_with_n && common_with_next))
                {
                    if (intersection(p, common_with_n || common_with_next))
                        crossings++;
                }
            }
            for (e = agfstin(gv, n); e; e = agnxtin(gv, e)) {
                next = (e->head == n ? e->tail : e->head);
                //grid points of the node connected to node n
                grid.get_grid_coords(dunnart_nodes[next], &p[1][1][1], &p[1][1][0]);
                bool common_with_next = (gv_nodes[attachees.first] == next || gv_nodes[attachees.second] == next);
                if (!(common_with_n && common_with_next))
                {
                    if (intersection(p, common_with_n || common_with_next))
                        crossings++;
                }
            }
            //}
        }
    }
    return crossings;
}


#if 0
// ------  Unused -------

static bool within_bounds(int gridi, int gridj)
{
    return gridi > 0 && gridj > 0 && gridi < MAX_ROWS && gridj < MAX_COLS;
}

static void get_feasible_directions(std::vector<GridPoint> &feasible_directions, Grid &grid, int gridi, int gridj, int diri, int dirj)
{
    if (grid.isEmpty(gridi+diri, gridj+dirj))
        feasible_directions.push_back(GridPoint(gridi+diri, gridj+dirj, gridi, gridj));
    if (grid.isEmpty(gridi+dirj, gridj+diri))
        feasible_directions.push_back(GridPoint(gridi+dirj, gridj+diri, gridi, gridj));
    if (grid.isEmpty(gridi-dirj, gridj-diri))
        feasible_directions.push_back(GridPoint(gridi-dirj, gridj-diri, gridi, gridj));
    if (grid.isEmpty(gridi-diri, gridj-dirj))
        feasible_directions.push_back(GridPoint(gridi-diri, gridj-dirj, gridi, gridj));

    //diagonals
    if (grid.isEmpty(gridi+1, gridj+1))
        feasible_directions.push_back(GridPoint(gridi+1, gridj+1, gridi, gridj));
    if (grid.isEmpty(gridi-1, gridj+1))
        feasible_directions.push_back(GridPoint(gridi-1, gridj+1, gridi, gridj));
    if (grid.isEmpty(gridi-1, gridj-1))
        feasible_directions.push_back(GridPoint(gridi-1, gridj-1, gridi, gridj));
    if (grid.isEmpty(gridi+1, gridj-1))
        feasible_directions.push_back(GridPoint(gridi+1, gridj-1, gridi, gridj));
}
#endif


class SortByDegree {
public:
    SortByDegree(std::map<Agnode_t*, int> &degrees) : node_degrees(degrees) {}
    bool operator() (Agnode_t* p1, Agnode_t* p2) const {
        return node_degrees[p1] > node_degrees[p2];    //reverse order
    }
private:
    std::map<Agnode_t*, int> &node_degrees;
};

class SortByCoord {
public:
    SortByCoord(Grid &grid, Direction dir) : _grid(grid), _dir(dir) {}
    bool operator() (ShapeObj* p1, ShapeObj* p2) const {
        int ni1, nj1, ni2, nj2;
        _grid.get_grid_coords(p1, &ni1, &nj1);
        _grid.get_grid_coords(p2, &ni2, &nj2);
        return (_dir == LEFT || _dir == RIGHT) ? (ni1 < ni2) : (nj1 < nj2);
    }
private:
    Grid &_grid;
    Direction _dir;
};

/*
//not really necessary?
class SortByScore {
public:
    SortByDegree(std::map<std::pair<Agnode_t*, GridPoint>, double> &placement_scores) : scores(placement_scores) {}
    bool operator() (std::pair<Agnode_t*, GridPoint> p1, std::pair<Agnode_t*, GridPoint> p2) const {
        return scores[p1] < scores[p2];
    }
private:
    std::map<std::pair<Agnode_t*, GridPoint>, double> &scores;
};
*/


#if 0
// ------  Unused -------

//list nodes
static void list_max_degree_nodes(std::vector<Agnode_t*> &nodes, int places, Agraph_t* gv, Agnode_t* curr, std::map<Agnode_t*, bool> &marked, std::map<Agnode_t*, int> &degrees)
{
    Agnode_t* next;
    Agedge_t *e;

    //find the connected unmarked node with the highest degree
    for (e = agfstout(gv, curr); e; e = agnxtout(gv, e)) {
      next = (e->head == curr ? e->tail : e->head);
      if (!marked[next])
      {
          nodes.resize(nodes.size()+1);
          nodes[nodes.size()-1] = next;
          //nodes.push_back(next);
      }
    }
    for (e = agfstin(gv, curr); e; e = agnxtin(gv, e)) {
      next = (e->head == curr ? e->tail : e->head);
      if (!marked[next])
      {
          nodes.resize(nodes.size()+1);
          nodes[nodes.size()-1] = next;
          //nodes.push_back(next);
      }
    }

    //sort nodes by degree.
    std::sort(nodes.begin(), nodes.end(), SortByDegree(degrees));

    // keep first 'places' nodes, plus any of the same degree to the placesth node.
    int min_degree = -1;
    for (unsigned int i = 0; i<nodes.size(); i++)
    {
        if (min_degree >= 0)
            if (degrees[nodes[i]] != min_degree)
            {
                nodes.resize(i);
                break;
            }
        else
        {
            --places;
            if (places == 0)
                min_degree = degrees[nodes[i]];
        }
    }

    //only keep the first n, plus any of those of equal degree to the nth.

    //sort
    /*sort(nodes.begin(), nodes.end(), SortByDegree);

    typedef map<Agnode_t*, int, DegreeOrder> StringStringMap;

    if (max_degree == 0)
    return NULL;
    else
    return max_degree_node;*/
}
#endif


static void list_unmarked_nodes(std::vector<Agnode_t*> &nodes, Agraph_t* gv, Agnode_t* curr, std::map<Agnode_t*, bool> &marked)
{
    Agnode_t* next;
    Agedge_t *e;

    //find the connected unmarked node with the highest degree
    for (e = agfstout(gv, curr); e; e = agnxtout(gv, e)) {
        next = (e->head == curr ? e->tail : e->head);
        if (!marked[next])
        {
            nodes.resize(nodes.size()+1);
            nodes[nodes.size()-1] = next;
            //nodes.push_back(next);
        }
    }
    for (e = agfstin(gv, curr); e; e = agnxtin(gv, e)) {
        next = (e->head == curr ? e->tail : e->head);
        if (!marked[next])
        {
            nodes.resize(nodes.size()+1);
            nodes[nodes.size()-1] = next;
            //nodes.push_back(next);
        }
    }
}


/* 
procedure:
 1. get feasible directions: number = places
 2. get nodes (places is input)
 3. create node, direction pairs
 4. map those pairs to a score
 5. modify scores to reflect 'regret'
 6. choose best score, place.
 7. remove pairs no longer valid... remove from the nodes and places vectors. 
 8. Goto 4

std::map<std::pair<Agnode_t*, GridPoint>, double> scores;

//std::vector<std::pair<Agnode_t*, GridPoint>> placements;
for (int i = 0; i<nodes.size(); i++)
{
    for (int j = 0; j<places.size(); j++)
    {
        //std::pair<Agnode_t*, GridPoint> placement(nodes[i], places[j]);
        //placements.push_back(std::make_pair(nodes[i], places[j]));
        //placements.push_back(placement);
        scores[std::make_pair<Agnode_t*, GridPoint>(nodes[i], places[j])] = score(placement, ....);
    }
}
*/

//regret might not be needed in this version if there was a mechanism for rotating around to find better arrangements... but I think regret might be easier.
//in this version of regret, the main challenge is if a direction
//unfortunately, this version won't work at all. it assumes all combinations of directions and nodes, which is wrong.
//need to rewrite it, but how?
static double get_regret(Agnode_t *node, Direction dir, std::map<std::pair<Agnode_t*, Direction>, double> &scores, std::vector<Direction> &directions, std::vector<Agnode_t*> &nodes)
{
    //for all directions whose best node is node, add the difference between the best and second best in that direction.

    //loops through scores...ignore any with direction == dir.
    //now have to handle the case when regret doesn't make sense. if I use a few more variables I should be able to get it to work...

    double regret = 0;
    std::map<Direction, Agnode_t*> best;
    std::map<Direction, Agnode_t*> second_best;
    std::map<Direction, bool> seen;
    std::map<Direction, double> best_score;
    std::map<Direction, double> second_best_score;

    seen[UP] = false; seen[DOWN] = false; seen[LEFT] = false; seen[RIGHT] = false;

    for (std::map<std::pair<Agnode_t*, Direction>, double>::iterator m = scores.begin(); m != scores.end(); ++m)
    {
        if (m->first.second == dir)
            continue;

        if (!seen[m->first.second])
        {
            seen[m->first.second] = true;
            best[m->first.second] = m->first.first;
            best_score[m->first.second] = scores[m->first];
            second_best_score[m->first.second] = 200;
        }
        else
        {
            if (scores[m->first] < best_score[m->first.second])
            {
                second_best[m->first.second] = best[m->first.second];
                second_best_score[m->first.second] = best_score[m->first.second];
                best_score[m->first.second] = scores[m->first];
                best[m->first.second] = m->first.first;
            }
            else if (scores[m->first] < second_best_score[m->first.second])
            {
                second_best_score[m->first.second] = scores[m->first];
                second_best[m->first.second] = m->first.first;
            }
        }
    }

    for (unsigned int i = 0; i<directions.size(); i++)
    {
        if (directions[i] == dir)
            continue;
        if (seen[directions[i]] && best[directions[i]] == node)
            regret += best_score[directions[i]] - best_score[directions[i]];
    }
    return regret;
    /*



        double this_score = m->second;
        printf("Score for %s (%s): %f", dunnart_nodes[m->first.first]->getName(),
               (m->first.second == DOWN ? "DOWN" : (m->first.second == UP ? "UP" : (m->first.second == RIGHT ? "RIGHT" : "LEFT"))), this_score);
        //don't do this if there is no choice or no other nodes
        //if (directions.size() > 1 && nodes.size() > 1)
        //{
        this_score += get_regret(m->first.first, m->first.second, scores, directions, nodes);
            printf(", %f with regret\n", this_score);

    for (unsigned int i = 0; i<directions.size(); i++)
    {
        if (directions[i] == dir)
            continue;

        Agnode_t* best = nodes[0];
        double best_score = scores[std::make_pair<Agnode_t*, Direction>(nodes[0], directions[i])];
        Agnode_t* second_best;
        double second_best_score = 200;    //this determines what happens when there is NO second best. I anticipate this could happen quite a bit.
        for (unsigned int j = 1; j<nodes.size(); j++)
        {
            double this_score = scores[std::make_pair<Agnode_t*, Direction>(nodes[j], directions[i])];
            if (this_score < best_score)
            {
                second_best = best;
                second_best_score = best_score;
                best_score = scores[std::make_pair<Agnode_t*, Direction>(nodes[j], directions[i])];
                best = nodes[j];
            }
            else if (this_score < second_best_score)
            {
                second_best_score = this_score;
                second_best = nodes[j];
            }
        }

        if (best == node)
        {
            regret += second_best_score - best_score;
        }
    }
    return regret;
    */
/*
    for (unsigned int i = 0; i<nodes.size(); i++)
    {
        if (nodes[i] == node)    //ignore the node we are talking about placing in the regret calculation
            continue;

        GridPoint best = places[0];
        double best_score = scores[std::make_pair<Agnode_t*, GridPoint>(nodes[i], places[0])];
        GridPoint second_best;
        double second_best_score = 1e8;
        for (unsigned int j = 1; j<places.size(); j++)
        {
            double this_score = scores[std::make_pair<Agnode_t*, GridPoint>(nodes[i], places[j])];
            if (this_score < best_score)
            {
                second_best = best;
                second_best_score = best_score;
                best_score = scores[std::make_pair<Agnode_t*, GridPoint>(nodes[i], places[j])];    //QUESTION: does this use of make pair again work?
                best = places[j];
            }
            else if (this_score < second_best_score)
            {
                second_best_score = this_score;
                second_best = places[j];
            }
        }
        if (best.i == placement.i && best.j == placement.j)
        {
            regret += second_best_score - best_score;
        }
    }

    return regret;
    //return regret < 100 ? 0 : regret;
 */
}

/*
for (std::map<std::pair<Agnode_t*, GridPoint>, double>::iterator m = scores.begin(); m != scores.end(); ++m)
{
    //don't do this if there is no choice.
    m->second += regret(m->first, scores);

    sumX += m->second.x;
    sumY += m->second.y;
}

//two alternatives? create a map, OR, call the score function inside the sorting function
//map means yet another data structure, but less computations of objective function.

for(int i  0; i<placements.size(); i++)
{

}
*/

#if 0
// Unused

//eek... be careful... diri is actually the y direction.
static double get_dist(ShapeObj *curr, ShapeObj *next, int diri, int dirj)
{
    double x1, y1, w1, h1;
    double x2, y2, w2, h2;
    curr->getPosAndSize(x1, y1, w1, h1);
    next->getPosAndSize(x2, y2, w2, h2);

    double r = sqrt(pow(x1 + w1/2 + dirj*(50 + w1/2 + w2/2) - x2, 2) + pow(y1 + h1/2 + diri*(50 + h1/2 + h2/2) - y2,2));
    return r;
}


//inefficient, fix this up later
static double get_grid_dist(int si, int sj, Agnode_t* dst, std::map<Agnode_t*, ShapeObj*> &dunnart_nodes, Grid &grid)
{
    //i know... i bet its double placements...
    ShapeObj *u = dunnart_nodes[dst];

    int di, dj;

    //undefined behaviour if there is a problem with finding them in the grid.
    if (!grid.find(&di, &dj, u))
        printf("WARNING (align2.cpp): problem finding class in grid... overwritten?\n");

    return sqrt(pow(di - si, 2) + pow(dj - sj, 2));
}

//sum grid dist: look at gviz connected nodes. look for marked BUT NOT curr. if they are marked, need to know where in the grid. hmmm.
//do it the slow hack way then fix it up.
static double sum_grid_dist(Agraph_t* gv, Agnode_t* curr, Agnode_t* next, int gridi, int gridj, 
                std::map<Agnode_t*, bool> &marked, std::map<Agnode_t*, ShapeObj*> &dunnart_nodes, Grid &grid)
{
    Agnode_t* connected;
    Agedge_t* e;
    double sum = 0;
    for (e = agfstout(gv, next); e; e = agnxtout(gv, e)) {
        connected = (e->head == next ? e->tail : e->head);
        if (marked[connected] && connected != curr)
        {
            //printf(dunnart_nodes[connected]->getName());
            sum += get_grid_dist(gridi, gridj, connected, dunnart_nodes, grid);
        }
    }
    for (e = agfstin(gv, next); e; e = agnxtin(gv, e)) {
        connected = (e->head == next ? e->tail : e->head);
        if (marked[connected] && connected != curr)
        {
            //printf(dunnart_nodes[connected]->getName());
            sum += get_grid_dist(gridi, gridj, connected, dunnart_nodes, grid);
        }
    }
    return sum;
}
#endif

static double get_score(Agnode_t* curr, Agnode_t* next, std::map<Agnode_t*, ShapeObj*> &dunnart_nodes, Grid &grid,
                        Agraph_t* gv, std::map<Agnode_t*, bool> &marked, std::map<Agnode_t*, int> &degrees, Direction dir, int currentcrossings, int newcrossings)
{
    //return
    /*
     double x = 100000.0/degrees[next]
    + 10000*(abs(nextP.i -nextP.previ) + abs(nextP.j - nextP.prevj))    //penalizes diagonal placements.
    + 1000*sum_grid_dist(gv, curr, next, nextP.i, nextP.j, marked, dunnart_nodes, grid)
    + get_dist(dunnart_nodes[curr], dunnart_nodes[next],nextP.i - nextP.previ, nextP.j - nextP.prevj);
     */

    int ci, cj, ni, nj;
    grid.get_grid_coords(dunnart_nodes[curr], &ci, &cj);
    grid.get_grid_coords(dunnart_nodes[next], &ni, &nj);
    double score;
    //crossings
    score = (newcrossings == currentcrossings) ? 1000 : 1000/(double)(newcrossings-currentcrossings);
    score += 100/degrees[next];
    if (dir == UP || dir == DOWN)
    {
        score += 10 * abs(cj - nj);
    }
    else if (dir == LEFT || dir == RIGHT)
    {
        score += 10 * abs(ci - ni);
    }
    return score;
    //double x = 100000.0/degrees[next];
    //double y = 10000*degrees[next]*(abs(nextP.i -nextP.previ) + abs(nextP.j - nextP.prevj));    //penalizes diagonal placements, especially those of degree > 1
    //double z = 1000*sum_grid_dist(gv, curr, next, nextP.i, nextP.j, marked, dunnart_nodes, grid) ;
    //double t = get_dist(dunnart_nodes[curr], dunnart_nodes[next],nextP.i - nextP.previ, nextP.j - nextP.prevj);
    //if ((x + y + z + t) > 1e8)
    //{
        //printf("!%s!", dunnart_nodes[curr]->getName());
        //fflush(stdout);
        //printf(grid[nextP.i][nextP.j]->get_cl      ass_name());
        //printf("!");
    //    sum_grid_dist(gv, curr, next, nextP.i, nextP.j, marked, dunnart_nodes, grid);
    //}
    //return x + y + z + t;
}

#if 0
// Unused

static Agnode_t* get_max_degree_node(
    Agraph_t* gv,
    std::map<Agnode_t*, ShapeObj*> &dunnart_nodes,
    std::map<Agnode_t*, int> &degrees,
    std::map<Agnode_t*, bool> &marked,
    Agnode_t *curr,
    int diri, int dirj)
{
    Agnode_t* max_degree_node = NULL;
    Agnode_t* next;
    Agedge_t *e;
    int max_degree = 0;

    //find the connected unmarked node with the highest degree
    for (e = agfstout(gv, curr); e; e = agnxtout(gv, e)) {
        next = (e->head == curr ? e->tail : e->head);
        if (!marked[next])
        {
            if (degrees[next]> max_degree)
            {
                max_degree = degrees[next];
                max_degree_node = next;
            }
        }
    }
    for (e = agfstin(gv, curr); e; e = agnxtin(gv, e)) {
        next = (e->head == curr ? e->tail : e->head);
        if (!marked[next])
        {
            if (degrees[next]> max_degree)
            {
                max_degree = degrees[next];
                max_degree_node = next;
            }
        }
    }

    if (max_degree == 0)
        return NULL;
    else
        return max_degree_node;
}
#endif


bool vertically_aligned(Grid &grid, int row, int col, int num_rows, std::map<Agnode_t*, bool> &marked, std::map<ShapeObj*, Agnode_t*> &gv_nodes)
{
    //aligned if in either direction the first class found (if any) is marked
    bool aligned = false;
    for (int i=row+1; i<num_rows; i++)
    {
        if (!grid.isEmpty(i, col))
        {
            if (marked[gv_nodes[grid.get(i, col)]])
                aligned = true;
            break;
        }
    }
    for (int i = row-1; i>=0; i--)
    {
        if (!grid.isEmpty(i, col))
        {
            if (marked[gv_nodes[grid.get(i, col)]])
                aligned = true;
            break;
        }
    }
    return true;
}

/*
//think this one hasn't been changed from vert aligned properly...
bool horizontally_aligned(Grid &grid, int row, int col, int num_cols, std::map<Agnode_t*, bool> &marked, std::map<ShapeObj*, Agnode_t*> &gv_nodes)
{
    //aligned if in either direction the first class found (if any) is marked
    bool aligned = false;
    for (int i=row+1; i<num_rows; i++)
    {
        if (!grid.isEmpty(i, col))
        {
            if (marked[gv_nodes[grid.get(i, col)]])
                aligned = true;
            break;
        }
    }
    for (int i = row-1; i>=0; i--)
    {
        if (!grid.isEmpty(i, col))
        {
            if (marked[gv_nodes[grid.get(i, col)]])
                aligned = true;
            break;
        }
    }
    return true;
}
*/

//dunnart_nodes no longer needs to be passed I think....
void insert_in_order(Agnode_t* node, Grid &grid, std::deque<Agnode_t*> &Q, std::map<Agnode_t*, int> &degrees, std::map<ShapeObj*, Agnode_t*> &gv_nodes, std::map<Agnode_t*, ShapeObj*> &dunnart_nodes)
{
    int k;
    //grid.get_grid_coords(dunnart_nodes[node], &ni, &nj);
    for (k = Q.size(); k> 0; k--)
    {
        //not quite finished. loop cond might be not quite right (might not want to step in i before checking...)
        //very close though.
        if (degrees[Q[k-1]] >= degrees[node])
            break;
    }
    Q.insert(Q.begin() + k, node);
}

//gets the list of directions in which there is currently no aligned class.
void get_alignment_directions(Grid &grid, std::vector<Direction> &directions, Agnode_t* curr, int ci, int cj, Agraph_t* gv, std::map<Agnode_t*, ShapeObj*> &dunnart_nodes, std::map<Agnode_t*, bool> &marked, std::map<ShapeObj*, Agnode_t*> &gv_nodes, std::map<Agnode_t*, int> &degrees, std::deque<Agnode_t*> &Q)
{
    Agedge_t *e;
    Agnode_t *next;
    int ni, nj;
    bool rem_up = false;
    bool rem_right = false;
    bool rem_down = false;
    bool rem_left = false;

    for (e = agfstout(gv, curr); e; e = agnxtout(gv, e)) {
        next = (e->head == curr ? e->tail : e->head);
        grid.get_grid_coords(dunnart_nodes[next], &ni, &nj);
        if (ni == ci && nj > cj)
        {
            rem_right = true;
            if (!marked[next])
            {
                marked[next] = true;
                insert_in_order(next, grid, Q, degrees, gv_nodes, dunnart_nodes);
            }
        }
        if (ni == ci && nj < cj)
        {
            rem_left= true;
            if (!marked[next])
            {
                marked[next] = true;
                insert_in_order(next, grid, Q, degrees, gv_nodes, dunnart_nodes);
            }
        }
        if (nj == cj && ni > ci)
        {
            rem_down = true;
            if (!marked[next])
            {
                marked[next] = true;
                insert_in_order(next, grid, Q, degrees, gv_nodes, dunnart_nodes);
            }
        }
        if (nj == cj && ni < ci)
        {
            rem_up = true;
            if (!marked[next])
            {
                marked[next] = true;
                insert_in_order(next, grid, Q, degrees, gv_nodes, dunnart_nodes);
            }
        }
    }
    for (e = agfstin(gv, curr); e; e = agnxtin(gv, e)) {
        next = (e->head == curr ? e->tail : e->head);
        grid.get_grid_coords(dunnart_nodes[next], &ni, &nj);
        if (ni == ci && nj > cj)
        {
            rem_right = true;
            if (!marked[next])
            {
                marked[next] = true;
                insert_in_order(next, grid, Q, degrees, gv_nodes, dunnart_nodes);
            }
        }
        if (ni == ci && nj < cj)
        {
            rem_left= true;
            if (!marked[next])
            {
                marked[next] = true;
                insert_in_order(next, grid, Q, degrees, gv_nodes, dunnart_nodes);
            }
        }
        if (nj == cj && ni > ci)
        {
            rem_down = true;
            if (!marked[next])
            {
                marked[next] = true;
                insert_in_order(next, grid, Q, degrees, gv_nodes, dunnart_nodes);
            }
        }
        if (nj == cj && ni < ci)
        {
            rem_up = true;
            if (!marked[next])
            {
                marked[next] = true;
                insert_in_order(next, grid, Q, degrees, gv_nodes, dunnart_nodes);
            }
        }
    }

    directions.resize((rem_up ? 0:1) + (rem_right ? 0:1) + (rem_down ? 0:1) + (rem_left ? 0:1));
    int i = 0;
    if (!rem_up)
        directions[i++] = UP;
    if (!rem_right)
        directions[i++] = RIGHT;
    if (!rem_down)
        directions[i++] = DOWN;
    if (!rem_left)
        directions[i++] = LEFT;
}

//after get direction, we want to get all nodes, all remaining directions, and make pairs out of any where node in the correct quadrant.

//then we want to score them all

//then we do regret but the other way round and try not to penalise missing directions excessively.

//the next trick is to actually perform the move...

//then to consider post processing sort of steps


static void rec_align(
    Canvas *canvas,
    std::deque<Agnode_t*> &Q,
    Agraph_t* gv,
    std::map<ShapeObj*, Agnode_t*> &gv_nodes,
    std::map<Agnode_t*, ShapeObj*> &dunnart_nodes,
    std::map<Agnode_t*, int> &degrees,
    std::map<Agnode_t*, bool> &marked,
    Grid &grid,
    Agnode_t *curr,
    std::map<Agnode_t*, bool> &aligned
)
{
    //Agnode_t *curr = gv_nodes[grid.get(gridi, gridj)];
    int gridi, gridj;
    grid.get_grid_coords(dunnart_nodes[curr], &gridi, &gridj);
    printf(">>Placing %s...\n", dunnart_nodes[curr]->getName());
    //printf("Placing (%d, %d)\n", gridi, gridj);

    std::vector<Direction> directions;
    //get_alignment_directions(grid, directions, curr, gridi, gridj, gv, dunnart_nodes, marked, gv_nodes, degrees, Q);
    directions.resize(4);
    directions[0] = UP;
    directions[1] = RIGHT;
    directions[2] = DOWN;
    directions[3] = LEFT;

    //printf("done..\n");
    //return;

    std::vector<Agnode_t*> nodes;
    list_unmarked_nodes(nodes, gv, curr, marked);

    while (directions.size() > 0 && nodes.size() > 0)
    {
         // 3, 4
        std::map<std::pair<Agnode_t*, Direction>, double> scores;

        for (unsigned int i = 0; i<nodes.size(); i++)
        {
            int ci, cj;
            grid.get_grid_coords(dunnart_nodes[nodes[i]], &ci, &cj);
            int currentcrossings = edge_crossings(canvas, nodes[i],
                    ci, cj, gv, dunnart_nodes, gv_nodes, grid);
            int newcrossings;

            for (unsigned int j = 0; j<directions.size(); j++)
            {
                //add code here to make sure node is in the correct quadrant for direction...
                //shouldn't be too hard
                //int ni=-1, nj=-1;
                //grid.get_grid_coords(dunnart_nodes[nodes[i]], &ni, &nj);
                //if (ni == -1 || nj == -1)
                //{
                //    printf("Error: couldn't get coords :/\n");
                //}

                if(directions[j] == UP && ci <= gridi
                   || directions[j] == RIGHT && cj >= gridj
                   || directions[j] == DOWN && ci >= gridi
                   || directions[j] == LEFT && cj <= gridj)
                {
                    //work out placement that would result from new direction
                    int ni, nj;
                    if (directions[j] == UP || directions[j] == DOWN)
                        {
                        ni = ci;
                        nj = gridj;
                        if (ci == gridi)
                            ni = (directions[j] == UP ? gridi-1 : gridi+1);
                        }
                        else
                        {
                        nj = cj;
                        ni = gridi;
                        if (cj == gridj)
                            nj = (directions[j] == LEFT ? gridj-1 : gridj+1);
                        }


                    newcrossings = edge_crossings(canvas, nodes[i], ni, nj, gv, dunnart_nodes, gv_nodes, grid);
                    printf("%s [(x,y)] (%d, %d)->(%d, %d), crossings %d -> %d\n", dunnart_nodes[nodes[i]]->getName(), cj, ci, nj, ni, currentcrossings, newcrossings);
                    if (newcrossings <= currentcrossings)
                        {
                        scores[std::make_pair<Agnode_t*, Direction>(nodes[i], directions[j])] =
                            get_score(curr, nodes[i], dunnart_nodes, grid, gv, marked, degrees, directions[j], currentcrossings, newcrossings);
                        }
                }
                //doesn't work with code to see if we should be stopping, i suspect.
                //else    //issue with code further down means implicitly score is set to 0. don't want that. make it a huge number instead.
                //{
                //    scores[std::make_pair<Agnode_t*, Direction>(nodes[i], directions[j])] = 1e10;
                //}
            }
        }

        // 5, 6
        if (scores.size() < 1)
            break;

        double best_score = 1.0e30;
        std::pair<Agnode_t*, Direction> best_placement;
        for (std::map<std::pair<Agnode_t*, Direction>, double>::iterator m = scores.begin(); m != scores.end(); ++m)
        {
            double this_score = m->second;
            printf("Score for %s (%s): %f", dunnart_nodes[m->first.first]->getName(),
                   (m->first.second == DOWN ? "DOWN" : (m->first.second == UP ? "UP" : (m->first.second == RIGHT ? "RIGHT" : "LEFT"))), this_score);
            //don't do this if there is no choice or no other nodes
            //if (directions.size() > 1 && nodes.size() > 1)
            //{
                this_score += get_regret(m->first.first, m->first.second, scores, directions, nodes);
                printf(", %f with regret\n", this_score);
            //}
            //else
            //    printf("\n");

            if (this_score < best_score)
            {
                best_score = m->second;
                best_placement = m->first;
            }
            //else
            //    printf("<%f, %f>", this_score, best_score);
        }

        //printf("done\n");
        //return;

        //make placement...
        /*
        printf("==%s placed at (%d, %d), (%d, %d) from %s\n", dunnart_nodes[best_placement.first]->getName(),
               best_placement.second.i, best_placement.second.j,
               best_placement.second.i - best_placement.second.previ,
               best_placement.second.j - best_placement.second.prevj, dunnart_nodes[curr]->getName());
         */
        printf("==Aligning %s %s from %s\n",
               dunnart_nodes[best_placement.first]->getName(),
               (best_placement.second == DOWN ? "DOWN" : (best_placement.second == UP ? "UP" : (best_placement.second == RIGHT ? "RIGHT" : "LEFT"))),
               dunnart_nodes[curr]->getName());

        //placement generally just involves translating in one grid dimension. For now, assume no other class is encountered.
        //look at direction (best_placement.second), and node (best_placement.first), get grid coords for node, make the coord same as that for curr, and change the other one if there is an overlap
        int ni, nj;
        int newi, newj;
        grid.get_grid_coords(dunnart_nodes[best_placement.first], &ni, &nj);
        if (best_placement.second == UP || best_placement.second == DOWN)
        {
            newi = ni == gridi ? (best_placement.second == UP ? gridi-1: gridi+1) : ni;
            newj = gridj;
        }
        else
        {
            newi = gridi;
            newj = nj == gridj ? (best_placement.second == LEFT ? gridj-1: gridj+1) : nj;
        }
        if (!grid.isEmpty(newi, newj))
        {
            //XXX: this definitely needs to change. this assumes that there is nothing in the way of the node that was in the way.
            grid.move(grid.get(newi, newj), newi == ni ? newi : (newi > ni ? newi+1 : newi-1),
                      newj == nj ? newj : (newj > nj ? newj+1 : newj-1));
        }
        grid.move(dunnart_nodes[best_placement.first], newi, newj);

        //ok, so we have just moved from ni to newi and nj to newj
        //we want to calculate for all unmarked connected nodes of best_placement.first which ones will be in a different quadrant due to this,
        //and translate them accordingly.
        //quadrant moved: ci>ni != ci>newi || cj>nj != cj>newj
        //connected nodes... get the loop
        Agedge_t *e;
        Agnode_t *next;
        int nexti, nextj;

        for (e = agfstout(gv, best_placement.first); e; e = agnxtout(gv, e)) {
            next = (e->head == best_placement.first ? e->tail : e->head);
            if (!marked[next])
            {
                grid.get_grid_coords(dunnart_nodes[next], &nexti, &nextj);
                if (nexti>ni != nexti>newi || nextj>nj != nextj>newj)
                {
                    grid.move(dunnart_nodes[next], nexti+(newi-ni), nextj+(newj-nj));
                }
            }
        }
        for (e = agfstin(gv, best_placement.first); e; e = agnxtin(gv, e)) {
            next = (e->head == best_placement.first ? e->tail : e->head);
            if (!marked[next])
            {
                grid.get_grid_coords(dunnart_nodes[next], &nexti, &nextj);
                if (nexti>ni != nexti>newi || nextj>nj != nextj>newj)
                {
                    grid.move(dunnart_nodes[next], nexti+(newi-ni), nextj+(newj-nj));
                }
            }
        }


//        grid.insert(best_placement.second.i, best_placement.second.j, dunnart_nodes[best_placement.first]);
        //might do this for all nodes at the start.

        if (marked[best_placement.first])
        {
            printf("inserting marked node 1!\n");
        }
        marked[best_placement.first] = TRUE;
        insert_in_order(best_placement.first, grid, Q, degrees, gv_nodes, dunnart_nodes);
        /*
        int k;
        for (k = Q.size(); k> 0; k--)
        {
            //not quite finished. loop cond might be not quite right (might not want to step in i before checking...)
            //very close though.
            if (degrees[gv_nodes[grid.get(Q[k-1].i, Q[k-1].j)]] >= degrees[best_placement.first])
                break;
        }
        Q.insert(Q.begin() + k, GridPoint(ni, nj));
         */
        //Q.push_back(best_placement.second);

        //places.erase(find(places.begin(), places.end(), best_placement.second));
        //nodes.erase(find(nodes.begin(), nodes.end(), best_placement.first));
        //meh! this won't work will it
        //places.erase(best_placement.second);
        //        int n1 = places.size();
        //        int n2 = nodes.size();
        for (unsigned int i = 0; i< directions.size(); i++)
        {
            if (directions[i] == best_placement.second)
            {
                for (unsigned int j = i+1; j<directions.size(); j++)
                {
                    directions[j-1] = directions[j];
                }
                directions.resize(directions.size() - 1);
                break;
            }
        }

        for (unsigned int i = 0; i< nodes.size(); i++)
        {
            if (nodes[i] == best_placement.first)
            {
                for (unsigned int j = i+1; j<nodes.size(); j++)
                {
                    nodes[j-1] = nodes[j];
                }
                nodes.resize(nodes.size() - 1);
                break;
            }
        }
        //nodes.erase(best_placement.first);
    }

    /* ok, in here, I want to add an aligning step as I sketched out before */

    //problem... not quite working yet. its moving around classes i'd prefer it didn't touch
    //The decision of when to move and when not to is a tricky one.
    //certainly don't unless they are all aligned... but when are they aligned...? possibly a new map for aligned in this way? but that won't fully solve the problem, surely.
    //there's a lot to think about there, I have to leave it

    if (DO_ALIGNING_STEP)
    {

    std::vector<Direction> dirs;
    //dirs.resize(1); //4
    dirs.resize(4);
    dirs[0] = LEFT;
    dirs[1] = UP;
    dirs[2] = RIGHT;
    dirs[3] = DOWN;
    std::vector<ShapeObj*> nodes_in_dir;
    for (unsigned int i = 0; i < dirs.size(); i++)
    {
        nodes_in_dir.resize(0);
        //get_nodes_in_dir(nodes_in_dir, gv);

        //this gets all in that direction. in practice, I don't want them all, I want those that I choose to align in this direction
        //I think I want to align in the direction with the most nodes first.... maybe I could have a vector of nodes in dirs...
        //I'll try it on Menu first, with just left.
        Agedge_t* e;
        Agnode_t* next;
        int ni, nj;
        for (e = agfstout(gv, curr); e; e = agnxtout(gv, e)) {
            next = (e->head == curr ? e->tail : e->head);
            grid.get_grid_coords(dunnart_nodes[next], &ni, &nj);
            if (!aligned[next])
            {
                if (dirs[i] == UP && ni < gridi
                    || dirs[i] == DOWN && ni > gridi
                    || dirs[i] == RIGHT && nj > gridj
                    || dirs[i] == LEFT && nj < gridj)
                {
                    nodes_in_dir.resize(nodes_in_dir.size()+1);
                    nodes_in_dir[nodes_in_dir.size()-1] = dunnart_nodes[next];
                }
            }
        }
        for (e = agfstin(gv, curr); e; e = agnxtin(gv, e)) {
            next = (e->head == curr ? e->tail : e->head);
            grid.get_grid_coords(dunnart_nodes[next], &ni, &nj);
            if (!aligned[next])
            {
                if (dirs[i] == UP && ni < gridi
                    || dirs[i] == DOWN && ni > gridi
                    || dirs[i] == RIGHT && nj > gridj
                    || dirs[i] == LEFT && nj < gridj)
                {
                    nodes_in_dir.resize(nodes_in_dir.size()+1);
                    nodes_in_dir[nodes_in_dir.size()-1] = dunnart_nodes[next];
                }
            }
        }

        if (nodes_in_dir.size() == 0)
            continue;

        std::sort(nodes_in_dir.begin(), nodes_in_dir.end(), SortByCoord(grid, dirs[i]));
        //align (hmm)

        //compact.. only works if they are first aligned... unfortunately alignment is a bit of a sticking point at the moment.
        //alignment fits in with the problem of knowing which ones to get... if I can get that right I have something much more powerful.
        int lasti, lastj;
        for (unsigned int k = 0; k<nodes_in_dir.size(); k++)
        {
            if (k==0)
            {
                grid.get_grid_coords(nodes_in_dir[k], &lasti, &lastj);
            }
            else
            {
                int ci, cj;
                grid.get_grid_coords(nodes_in_dir[k], &ci, &cj);
                if ((dirs[i] == LEFT || dirs[i] == RIGHT) ? (cj == lastj && ci > (lasti+1)) : (ci == lasti && cj > (lastj+1)))
                {
                    //found a gap. move everything into it to fill it up.
                    int l = k-1;
                    while (l >= 0)
                        {
                        grid.move(nodes_in_dir[l], (dirs[i] == LEFT || dirs[i] == RIGHT) ? ci-(k-l) : ci,
                                  (dirs[i] == LEFT || dirs[i] == RIGHT) ? cj : cj-(k-l));
                        l--;
                        }
                }
                lasti = ci;
                lastj = cj;
            }
        }

        continue;

        //balance
        //look at size vs lasti/lastj
        int firsti, firstj;
        grid.get_grid_coords(nodes_in_dir[0], &firsti, &firstj);
        int imbalance = static_cast<int>((dirs[i] == LEFT || dirs[i] == RIGHT) ?
                (lasti - (gridi+ceil(nodes_in_dir.size()/2.0))) : 
                (lastj - (gridj+ceil(nodes_in_dir.size()/2.0))));
        if (imbalance != 0)
        {
            //reduce imbalance if classes are in the way
            for (int k = imbalance; k != 0; imbalance > 0 ? k-- : k++)
            {
                if (dirs[i] == LEFT || dirs[i] == RIGHT)
                {
                    if (imbalance > 0)
                        {
                        if (!grid.isEmpty(firsti - k, firstj))
                            imbalance = k-1;
                        }
                        else
                        {
                        if (!grid.isEmpty(lasti - imbalance, firstj))
                            imbalance = k-1;
                        }
                }
                else
                {
                    if (imbalance > 0)
                        {
                        if (!grid.isEmpty(firsti, firstj - k))
                            imbalance = k-1;
                        }
                        else
                        {
                        if (!grid.isEmpty(firsti, lastj - imbalance))
                            imbalance = k-1;
                        }
                }
            }

            if (imbalance == 0)
                continue;

            //now, do the shifting. simply shift by -imbalance in the correct order
            for (int k = (imbalance > 0 ? 0 : nodes_in_dir.size() -1);
                 (imbalance > 0 ? k< (int) nodes_in_dir.size() : k>= 0);
                 (imbalance > 0 ? k++ : k--))
            {
                int ci, cj;
                grid.get_grid_coords(nodes_in_dir[k], &ci, &cj);
                grid.move(nodes_in_dir[k],
                          (dirs[i] == LEFT || dirs[i] == RIGHT) ? ci - imbalance : ci,
                          (dirs[i] == LEFT || dirs[i] == RIGHT) ? cj - imbalance : cj);
            }
        }
    }
    } // end of if DO_ALIGNING_STEP
    /* now add all the rest that haven't been touched */
    nodes.resize(0);
    list_unmarked_nodes(nodes, gv, curr, marked);

    for (int i = 0; i< (int) nodes.size(); i++)
    {
        int k;
        for (k = Q.size(); k> 0; k--)
        {
            if (degrees[Q[k-1]] >= degrees[nodes[i]])
                break;
        }
        //int ni, nj;
        if (marked[nodes[i]])
        {
            printf("inserting marked node 2!\n");
        }
        marked[nodes[i]] = TRUE;
        //grid.get_grid_coords(dunnart_nodes[nodes[i]], &ni, &nj);
        Q.insert(Q.begin() + k, nodes[i]);
    }


    //mark everything as aligned.
    Agedge_t*e;
    Agnode_t* next;
    for (e = agfstout(gv, curr); e; e = agnxtout(gv, e)) {
        next = (e->head == curr ? e->tail : e->head);
        aligned[next] = TRUE;
    }
    for (e = agfstin(gv, curr); e; e = agnxtin(gv, e)) {
        next = (e->head == curr ? e->tail : e->head);
        aligned[next] = TRUE;
    }

    /*
procedure:
     1. get feasible directions: number = places
     2. get nodes (places is input)
     3. create node, direction pairs
     4. map those pairs to a score
     5. modify scores to reflect 'regret'
     6. choose best score, place.
     7. remove pairs no longer valid... remove from the nodes and places vectors.
     8. Goto 4
     */
    /*
    std::vector<GridPoint> places;
    std::vector<Agnode_t*> nodes;

    get_feasible_directions(places, grid, gridi, gridj, diri, dirj);    //1

    //first, check whether we actually want to place the node here or do some expanding
    Agnode_t* next;
    Agedge_t *e;

    unsigned int nodes_to_place = 0;
    //find the connected unmarked node with the highest degree
    for (e = agfstout(gv, curr); e; e = agnxtout(gv, e)) {
        next = (e->head == curr ? e->tail : e->head);
        if (!marked[next])
        {
            nodes_to_place++;
        }
    }
    for (e = agfstin(gv, curr); e; e = agnxtin(gv, e)) {
        next = (e->head == curr ? e->tail : e->head);
        if (!marked[next])
        {
            nodes_to_place++;
        }
    }

    if (nodes_to_place > 7 && (gridi != 0 || gridj != 0))
    {
        printf("PROBLEM: NODE DEGREE TOO HIGH\n");
    }
    if (nodes_to_place > places.size() && nodes_to_place <= 7 && false)    //simple expansion won't do any good if nodes to place is more than 7
    {
        //insert a dummy node... bit of a hack atm
        grid.insert_dummy_node(gridi, gridj);

        for (unsigned int i = 0; i < Q.size(); i++)
        {
            if (grid.isEmpty(Q[i].i, Q[i].j))
            {
                printf("problem: even before expansion empty grid position in queue\n");
            }
        }

        int expandi = 0;
        int expandj = 0;
        bool expandedi = false;
        bool expandedj = false;

        grid.print();
        printf("\n");
        //insert a row/column/both so that a placement is possible
        if (gridi != previ)
        {
            if (gridi > previ)
            {
                expandedi = grid.insert_row(gridi);
                expandi = 1;
            }
            else
            {
                expandedi = grid.insert_row(gridi-1);
                expandi = -1;
            }

            //expandi = gridi-previ;
            //if (abs(expandi) > 1)
            //    expandi = gridi-previ > 0 ? 1 : -1;

        }
        if (gridj != prevj)
        {
            if (gridj > prevj)
            {
                expandedj = grid.insert_col(gridj);
                expandj = 1;
            }
            else
            {
                expandedj = grid.insert_col(gridj-1);
                expandj = -1;
            }
        }

        //modify gridpoints in the queue to reflect the repositioning.
        grid.print();

        for (unsigned int i = 0; i < Q.size(); i++)
        {
            if (expandedi)
            {
                if (Q[i].i > gridi + (expandi > 0 ? 0 : expandi))
                    Q[i].i++;
                if (Q[i].previ > gridi + (expandi > 0 ? 0 : expandi))
                    Q[i].previ++;
            }
            if (expandedj)
            {
                if (Q[i].j > gridj + (expandj > 0 ? 0 : expandi))
                    Q[i].j++;
                if (Q[i].prevj > gridj + (expandj > 0 ? 0 : expandi))
                    Q[i].prevj++;
            }

            if (grid.isEmpty(Q[i].i, Q[i].j))
            {
                printf("problem: empty grid position in queue\n");
            }
        }



        //insert at new position
        grid.insert(gridi+expandi, gridj+expandj, dunnart_nodes[curr]);

        //make sure the node at the new position will be the very next to be considered, then finish.

        //previ and prevj set to the prev node, for positioning purposes.
        Q.push_front(GridPoint(gridi+expandi, gridj+expandj, previ, prevj)); //push front instead of back.
        return;
    }

    list_max_degree_nodes(nodes, places.size(), gv, curr, marked, degrees); //2

    while (places.size() > 0 && nodes.size() > 0)
    {
         // 3, 4
        std::map<std::pair<Agnode_t*, GridPoint>, double> scores;

        for (unsigned int i = 0; i<nodes.size(); i++)
        {
            if (strcmp(dunnart_nodes[nodes[i]]->getName(), "Class C") == 0)
            {
                printf("found it\n");
            }

            for (unsigned int j = 0; j<places.size(); j++)
            {
                //std::pair<Agnode_t*, GridPoint> placement(nodes[i], places[j]);
                //placements.push_back(std::make_pair(nodes[i], places[j]));
                //placements.push_back(placement);
                scores[std::make_pair<Agnode_t*, GridPoint>(nodes[i], places[j])] =
                    get_score(curr, nodes[i], dunnart_nodes, grid, gv, marked, degrees, places[j]);
            }
        }

        // 5, 6
        double best_score = 1.0e30;
        std::pair<Agnode_t*, GridPoint> best_placement;
        for (std::map<std::pair<Agnode_t*, GridPoint>, double>::iterator m = scores.begin(); m != scores.end(); ++m)
        {
            double this_score = m->second;
            printf("Score for %s (%d, %d): %f", dunnart_nodes[m->first.first]->getName(),
                   m->first.second.i - m->first.second.previ, m->first.second.j - m->first.second.prevj, this_score);
            //don't do this if there is no choice or no other nodes
            if (places.size() > 1 && nodes.size() > 1)
            {
                this_score += get_regret(m->first.first, m->first.second, scores, places, nodes);
                printf(", %f with regret\n", this_score);
            }
            else
                printf("\n");

            if (this_score < best_score)
            {
                best_score = m->second;
                best_placement = m->first;
            }
            //else
            //    printf("<%f, %f>", this_score, best_score);
        }
        //printf("!");

        //make placement
        printf("==%s placed at (%d, %d), (%d, %d) from %s\n", dunnart_nodes[best_placement.first]->getName(),
               best_placement.second.i, best_placement.second.j,
               best_placement.second.i - best_placement.second.previ,
               best_placement.second.j - best_placement.second.prevj, dunnart_nodes[curr]->getName());
        //printf("==%s placed (%d, %d) from node\n", dunnart_nodes[best_placement.first]->getName(), best_placement.second.i - best_placement.second.previ,
        //       best_placement.second.j - best_placement.second.prevj);

        grid.insert(best_placement.second.i, best_placement.second.j, dunnart_nodes[best_placement.first]);
        marked[best_placement.first] = TRUE;
        Q.push_back(best_placement.second);

        //places.erase(find(places.begin(), places.end(), best_placement.second));
        //nodes.erase(find(nodes.begin(), nodes.end(), best_placement.first));
        //meh! this won't work will it
        //places.erase(best_placement.second);
//        int n1 = places.size();
//        int n2 = nodes.size();
        for (unsigned int i = 0; i< places.size(); i++)
        {
            if (places[i] == best_placement.second)
            {
                for (unsigned int j = i+1; j<places.size(); j++)
                {
                    places[j-1] = places[j];
                }
                places.resize(places.size() - 1);
                break;
            }
        }

        for (unsigned int i = 0; i< nodes.size(); i++)
        {
            if (nodes[i] == best_placement.first)
            {
                for (unsigned int j = i+1; j<nodes.size(); j++)
                {
                    nodes[j-1] = nodes[j];
                }
                nodes.resize(nodes.size() - 1);
                break;
            }
        }
        //nodes.erase(best_placement.first);
    }
    */


    //while ((next_node = get_next_node(gv, dunnart_nodes, degrees, marked, curr, 0, 0)) != NULL && !no_space)
    //{

        /*
        int besti = 0;
        int bestj = 0;
        double best_sum_grid_dist = 10000;
        double closest_distance = 10000;

        //sum grid dist: look at gviz connected nodes. look for marked BUT NOT curr. if they are marked, need to know where in the grid. hmmm.
        //do it the slow hack way then fix it up.
        if (within_bounds(gridi+diri, gridj+dirj) && grid[gridi+diri][gridj+dirj]==NULL)
        {
            int grid_dist = sum_grid_dist(gv, curr, max_degree_node, gridi+diri, gridj+dirj, marked, dunnart_nodes, grid);
            if (grid_dist < best_sum_grid_dist)
            {
                besti = diri;
                bestj = dirj;
                best_sum_grid_dist = grid_dist;
                closest_distance = get_dist(dunnart_nodes[curr], dunnart_nodes[max_degree_node], besti, bestj);
                //rec_straight = true; rec_left = false; rec_right = false; rec_backward = false;
            }
            else if (grid_dist == best_sum_grid_dist)
            {
                int dist = get_dist(dunnart_nodes[curr], dunnart_nodes[max_degree_node], diri, dirj);
                if (dist < closest_distance)
                {
                    besti = diri;
                    bestj = dirj;
                    closest_distance = dist;
                    //rec_straight = true; rec_left = false; rec_right = false; rec_backward = false;
                }
            }
        }
        if (within_bounds(gridi+dirj, gridj+diri)  && grid[gridi+dirj][gridj+diri]==NULL)
        {
            int grid_dist = sum_grid_dist(gv, curr, max_degree_node, gridi+dirj, gridj+diri, marked, dunnart_nodes, grid);
            if (grid_dist < best_sum_grid_dist)
            {
                besti = dirj;
                bestj = diri;
                best_sum_grid_dist = grid_dist;
                closest_distance = get_dist(dunnart_nodes[curr], dunnart_nodes[max_degree_node], besti, bestj);
                //rec_left = true; rec_straight = false; rec_right = false; rec_backward = false;
            }
            else if (grid_dist == best_sum_grid_dist)
            {
                int dist = get_dist(dunnart_nodes[curr], dunnart_nodes[max_degree_node], dirj, diri);
                if (dist < closest_distance)
                {
                    besti = dirj;
                    bestj = diri;
                    closest_distance = dist;
                    //rec_left = true; rec_straight = false; rec_right = false; rec_backward = false;
                }
            }
        }
        if (within_bounds(gridi-dirj, gridj-diri) && grid[gridi-dirj][gridj-diri]==NULL)
        {
            int grid_dist = sum_grid_dist(gv, curr, max_degree_node, gridi-diri, gridj-dirj, marked, dunnart_nodes, grid);
            if (grid_dist < best_sum_grid_dist)
            {
                besti = -dirj;
                bestj = -diri;
                best_sum_grid_dist = grid_dist;
                closest_distance = get_dist(dunnart_nodes[curr], dunnart_nodes[max_degree_node], besti, bestj);
                //rec_right = true; rec_straight = false; rec_left = false; rec_backward = false;
            }
            else if (grid_dist == best_sum_grid_dist)
            {
                int dist = get_dist(dunnart_nodes[curr], dunnart_nodes[max_degree_node], -dirj, -diri);
                if (dist < closest_distance)
                {
                    besti = -dirj;
                    bestj = -diri;
                    closest_distance = dist;
                    //rec_right = true; rec_straight = false; rec_left = false; rec_backward = false;
                }
            }
        }
        if (within_bounds(gridi-diri, gridj-dirj) && grid[gridi-diri][gridj-dirj]==NULL)
        {
            int grid_dist = sum_grid_dist(gv, curr, max_degree_node, gridi-diri, gridj-dirj, marked, dunnart_nodes, grid);
            if (grid_dist < best_sum_grid_dist)
            {
                besti = -diri;
                bestj = -dirj;
                best_sum_grid_dist = grid_dist;
                closest_distance = get_dist(dunnart_nodes[curr], dunnart_nodes[max_degree_node], besti, bestj);
                //rec_backward = true; rec_straight = false; rec_left = false; rec_right = false;
            }
            else if (grid_dist == best_sum_grid_dist)
            {
                int dist = get_dist(dunnart_nodes[curr], dunnart_nodes[max_degree_node], -diri, -dirj);
                if (dist < closest_distance)
                {
                    besti = -diri;
                    bestj = -dirj;
                    closest_distance = dist;
                    //rec_backward = true; rec_straight = false; rec_left = false; rec_right = false;
                }
            }
        }

        //use besti, bestj instead!!
        //problem: diagonal alignment, how to handle that? Ideally it should happen.... at the end.
        //so... either go in two passes, or have two queues. Two queues is the sensible idea I think.

        //no_space = !(rec_backward || rec_straight || rec_left || rec_right);
        no_space = (besti == 0 && bestj == 0);

        */
        /*
        if (!no_space)
        {
            grid[gridi+besti][gridj+bestj] = dunnart_nodes[max_degree_node];
            marked[max_degree_node] = TRUE;
            Q.push(GridPoint(gridi+besti, gridj+bestj, gridi, gridj));
        }
        */
        /*
        if (rec_straight)
        {
            grid[gridi+diri][gridj+dirj] = dunnart_nodes[max_degree_node];
            marked[max_degree_node] = TRUE;
            rec_straight2 = true;
        }
        if (rec_right)
        {
            grid[gridi-dirj][gridj-diri] = dunnart_nodes[max_degree_node];
            marked[max_degree_node] = TRUE;
            rec_right2 = true;
        }
        if (rec_left)
        {
            grid[gridi+dirj][gridj+diri] = dunnart_nodes[max_degree_node];
            marked[max_degree_node] = TRUE;
            rec_left2 = true;
        }
        if (rec_backward)
        {
            grid[gridi-diri][gridj-dirj] = dunnart_nodes[max_degree_node];
            marked[max_degree_node] = TRUE;
            rec_backward2 = true;
        }
         */
    //}
    /*
    if (rec_straight2)
        rec_align(gv, gv_nodes, dunnart_nodes, degrees, marked, newPos, grid, gridi+diri, gridj+dirj, gridi, gridj);
    if (rec_right2)
        rec_align(gv, gv_nodes, dunnart_nodes, degrees, marked, newPos, grid, gridi-dirj, gridj-diri, gridi, gridj);
    if (rec_left2)
        rec_align(gv, gv_nodes, dunnart_nodes, degrees, marked, newPos, grid, gridi+dirj, gridj+diri, gridi, gridj);      
    if (rec_backward2)
        rec_align(gv, gv_nodes, dunnart_nodes, degrees, marked, newPos, grid, gridi-diri, gridj-dirj, gridi, gridj); 
*/
}

/*
static void position_rest(
                      Agraph_t* gv,
                      std::map<ShapeObj*, Agnode_t*> &gv_nodes,
                      std::map<Agnode_t*, ShapeObj*> &dunnart_nodes,
                      std::map<Agnode_t*, int> &degrees,
                      std::map<Agnode_t*, bool> &marked,
                      std::map<ShapeObj*, Point> &newPos,
                      ShapeObj* grid[MAX_ROWS][MAX_COLS],
                      int gridi, int gridj
                          )
{
    Agnode_t *curr = gv_nodes[grid[gridi][gridj]];
    Agnode_t *max_degree_node = NULL;

    if (within_bounds(gridi+1, gridj+1) && grid[gridi+1][gridj+1]==NULL &&
        (max_degree_node = get_max_degree_node(gv, dunnart_nodes, degrees, marked, curr, 1, 1)))
        {
            grid[gridi+1][gridj+1] = dunnart_nodes[max_degree_node];
            marked[max_degree_node] = TRUE;

            //set the current dunnart node to be a certain distance the appropriate side of the previous dunnart node.
            double x, y, w, h;
            double x1, y1, w1, h1;
            grid[gridi][gridj]->getPosAndSize(x, y, w, h);
            dunnart_nodes[max_degree_node]->getPosAndSize(x1, y1, w1, h1);
            x1 = (newPos[grid[gridi][gridj]].x+w/2) + (1)*(50+w/2+w1/2) - w1/2;
            y1 = (newPos[grid[gridi][gridj]].y+h/2) + (1)*(50+h/2+h1/2) - h1/2;
            newPos[dunnart_nodes[max_degree_node]] = Point(x1, y1);
        }
    if (within_bounds(gridi+1, gridj-1)  && grid[gridi+1][gridj-1]==NULL &&
        (max_degree_node = get_max_degree_node(gv, dunnart_nodes, degrees, marked, curr,1,-1)))
    {
        grid[gridi+1][gridj-1] = dunnart_nodes[max_degree_node];
        marked[max_degree_node] = TRUE;

        double x, y, w, h;
        double x1, y1, w1, h1;
        grid[gridi][gridj]->getPosAndSize(x, y, w, h);
        dunnart_nodes[max_degree_node]->getPosAndSize(x1, y1, w1, h1);
        x1 = (newPos[grid[gridi][gridj]].x+w/2) + (-1)*(50+w/2+w1/2) - w1/2;
        y1 = (newPos[grid[gridi][gridj]].y+h/2) + (1)*(50+h/2+h1/2) - h1/2;
        newPos[dunnart_nodes[max_degree_node]] = Point(x1, y1);
    }
    if (within_bounds(gridi-1, gridj-1) && grid[gridi-1][gridj-1]==NULL &&
        (max_degree_node = get_max_degree_node(gv, dunnart_nodes, degrees, marked, curr,-1,-1)))
    {
        grid[gridi-1][gridj-1] = dunnart_nodes[max_degree_node];
        marked[max_degree_node] = TRUE;

        //set the current dunnart node to be a certain distance the appropriate side of the previous dunnart node.
        double x, y, w, h;
        double x1, y1, w1, h1;
        grid[gridi][gridj]->getPosAndSize(x, y, w, h);
        dunnart_nodes[max_degree_node]->getPosAndSize(x1, y1, w1, h1);
        x1 = (x+w/2) + (-1)*(50+w/2+w1/2) - w1/2;
        y1 = (y+h/2) + (-1)*(50+h/2+h1/2) - h1/2;
        newPos[dunnart_nodes[max_degree_node]] = Point(x1, y1);
    }
    if (within_bounds(gridi-1, gridj+1) && grid[gridi-1][gridj+1]==NULL &&
        (max_degree_node = get_max_degree_node(gv, dunnart_nodes, degrees, marked, curr, -1, 1)))
    {
        grid[gridi-1][gridj+1] = dunnart_nodes[max_degree_node];
        marked[max_degree_node] = TRUE;

        //set the current dunnart node to be a certain distance the appropriate side of the previous dunnart node.
        double x, y, w, h;
        double x1, y1, w1, h1;
        grid[gridi][gridj]->getPosAndSize(x, y, w, h);
        dunnart_nodes[max_degree_node]->getPosAndSize(x1, y1, w1, h1);
        x1 = (newPos[grid[gridi][gridj]].x+w/2) + (1)*(50+w/2+w1/2) - w1/2;
        y1 = (newPos[grid[gridi][gridj]].y+h/2) + (-1)*(50+h/2+h1/2) - h1/2;
        newPos[dunnart_nodes[max_degree_node]] = Point(x1, y1);
    }
}
*/

static bool no_edge_or_common_neighbour(Agraph_t* gv, std::map<ShapeObj*, Agnode_t*> &gv_nodes,
            ShapeObj *node1,   ShapeObj *node2)
{
    if (agfindedge(gv, gv_nodes[node1], gv_nodes[node2]) == NULL)
    {
        //check if the nodes have neighbours in common
        Agedge_t *e;
        Agedge_t *f;
        Agnode_t *n1;
        Agnode_t* n2;
        for (e = agfstout(gv, gv_nodes[node1]); e; e = agnxtout(gv, e)) {
            n1 = (e->head == gv_nodes[node1] ? e->tail : e->head);

            for (f = agfstout(gv, gv_nodes[node2]); f; f = agnxtout(gv, f)) {
                n2 = (f->head == gv_nodes[node2] ? f->tail : f->head);
                if (n1 == n2)
                    return false;
            }
            for (f = agfstin(gv, gv_nodes[node2]); f; f = agnxtin(gv, f)) {
                n2 = (f->head == gv_nodes[node2] ? f->tail : f->head);
                if (n1 == n2)
                    return false;
            }
        }
        for (e = agfstin(gv, gv_nodes[node1]); e; e = agnxtin(gv, e)) {
            n1 = (e->head == gv_nodes[node1] ? e->tail : e->head);

            for (f = agfstout(gv, gv_nodes[node2]); f; f = agnxtout(gv, f)) {
                n2 = (f->head == gv_nodes[node2] ? f->tail : f->head);
                if (n1 == n2)
                    return false;
            }
            for (f = agfstin(gv, gv_nodes[node2]); f; f = agnxtin(gv, f)) {
                n2 = (f->head == gv_nodes[node2] ? f->tail : f->head);
                if (n1 == n2)
                    return false;
            }
        }
        return true;    //didn't find a common neighbour
    }
    else
        return false;
}


//assumes all classes are contracted for now. I could automatically contract them if necessary.
void autoAlignSelection(Canvas *canvas) {
    GVC_t *gvc = gvContext();
    Agraph_t* gv = agopen((char *) "g", AGRAPH);

    std::map<ShapeObj*, Agnode_t*> gv_nodes;
    std::map<Agnode_t*, ShapeObj*> dunnart_nodes;
    std::map<Agnode_t*, int> degrees;
    std::map<Agnode_t*, bool> marked;
    std::map<Agnode_t*, bool> aligned;
    std::map<ShapeObj*, Point> newPos;

    double sumX = 0;
    double sumY = 0;
    int num_nodes  = 0;
    Point old_graph_centre;
    double max_width, max_height, minX, maxX, minY, maxY;
    bool first = true;
    double x, y, w, h;

    //create graphviz graph, calculate centre point, max height width X Y, min X Y as we go.
    ConnMultiset allConnMs;
    QList<CanvasItem *> canvas_selection = canvas->selectedItems();

    foreach (CanvasItem *co, canvas_selection)
    {
        ShapeObj *shape = dynamic_cast<ShapeObj *> (co);
        if (shape)
        {
            std::ostringstream s; s << shape->get_ID();
            char ss[s.str().length()+1];
            strcpy(ss,s.str().c_str());
            Agnode_t *n;
            gv_nodes[shape] = n = agnode(gv,ss);
            dunnart_nodes[n] = shape;
            double x, y, w, h;
            shape->getPosAndSize(x,y,w,h);
            sumX += x;
            sumY += y;
            ++num_nodes;
            newPos[shape] = Point(x, y);

            shape->getPosAndSize(x,y,w,h);
            if (first)
            {
                first = false;
                max_width = w;
                max_height = h;
                minX = x;
                maxX = x + w;
                minY = y;
                maxY = y+h;
            }
            max_width = qMax(max_width, w);
            max_height = qMax(max_height, h);
            maxX = qMax(maxX, x+w);
            maxY = qMax(maxY, y+h);
            minX = qMin(minX, x);
            minY = qMin(minY, y);
        
            ConnMultiset connMs = shape->getConnMultiset();
            allConnMs.insert(connMs.begin(), connMs.end());
        }
    }

    ConnMultiset::iterator prev = allConnMs.begin();
    ConnMultiset::iterator current = allConnMs.begin();
    for (; current != allConnMs.end(); ++current)
    {
        if (current != prev)
        {
            if (*current == *prev)
            {
                // The two connector endpoints are both connected to
                // shapes within the selection, add it to the set of 
                // objects being considered.
                std::pair<ShapeObj *, ShapeObj *> attachees = 
                        (*current)->getAttachedShapes();
                agedge(gv, gv_nodes[dynamic_cast<ShapeObj*>(attachees.first)], 
                           gv_nodes[dynamic_cast<ShapeObj*>(attachees.second)]);
            }
        }
        prev = current;
    }
 
    old_graph_centre.x = sumX/num_nodes;
    old_graph_centre.y = sumY/num_nodes;

    //create a bit of a buffer around nodes.
    max_width += 10;
    max_height += 10;

    int num_rows = static_cast<int>(ceil((maxY - minY) / max_height) + 2);
    int num_cols = static_cast<int>(ceil((maxX - minX) / max_width) + 2);
    minY -= max_height;
    minX -= max_width;

    //create a grid with this many rows and cols... not really necessary with min
    Grid grid(num_rows, num_cols);

    /* calculate degrees, assign to grid as we go */
    Agnode_t *n;
    Agedge_t *e;

    int max_degree = 0;
    Agnode_t *max_degree_node;
    int closest_row, closest_col;

    for (n= agfstnode(gv); n; n = agnxtnode(gv, n)) {
        marked[n] = FALSE;
        aligned[n] = FALSE;
        int deg = 0;
        for (e = agfstout(gv, n); e; e = agnxtout(gv, e)) {
            deg++;
        }
        for (e = agfstin(gv, n); e; e = agnxtin(gv, e)) {
            deg++;
        }
        degrees[n] = deg;
        if (deg > max_degree)
        {
            max_degree = deg;
            max_degree_node = n;
        }

        //assign to grid.
        dunnart_nodes[n]->getPosAndSize(x,y,w,h);
        closest_row = static_cast<int>(round((y-minY)/max_height));
        closest_col = static_cast<int>(round((x-minX)/max_width));
        if (grid.isEmpty(closest_row, closest_col))
        {
            grid.insert(closest_row, closest_col, dunnart_nodes[n]);
        }
        else
        {
            //'push' idea.
            double x2, y2, w2, h2;
            grid.get(closest_row,closest_col)->getPosAndSize(x2,y2,w2,h2);
            //push_vertically if there is no overlap in height, otherwise push horizontally.
            bool push_vertically = y+h < y2 || y > y2+h2;
            //should always be room in positive direction, but not always negative direction.
            bool room_neg = false;
            if (push_vertically)
            {
                for (int k = closest_row; k >=0; k--)
                    if (grid.isEmpty(k, closest_col))
                        room_neg = true;
            }
            else
            {
                for (int k = closest_col; k >=0; k--)
                    if (grid.isEmpty(closest_row, k))
                        room_neg = true;
            }
            bool new_class_closer;
            if (push_vertically)
            {
                new_class_closer = std::abs(round((y-minY)/max_height) - (y-minY)/max_height) < std::abs(round((y2-minY)/max_height) - (y2-minY)/max_height);
            }
            else
            {
                new_class_closer = std::abs(round((x-minX)/max_width) - (x-minX)/max_width) < std::abs(round((x2-minX)/max_width) - (x2-minX)/max_width);
            }
            int k = closest_row;
            int l = closest_col;
            bool new_class_pos_direction = push_vertically ? y2<y : x2<x;
            //push left if room in the negative direction AND the the other class is closer than the class in that direction
            //if (room_neg && (new_class_pos_direction && new_class_closer || !new_class_pos_direction && !new_class_closer))

            printf("room_neg: %d, push_vertically: %d, new_class_closer: %d, new_class_pos_direction: %d\n", room_neg, push_vertically, new_class_closer, new_class_pos_direction);

            if (room_neg && new_class_pos_direction == new_class_closer)    //simplifed from the line above
            {
                //push the class in the negative direction negatively
                ShapeObj *place = dunnart_nodes[n];
                ShapeObj *replaced;

                //if existing was in the neg direction, place new at curr position, else at first neg position
                for(push_vertically ? (k = (y2<y ? closest_row : closest_row-1)) : (l = (x2<x ? closest_col : closest_col-1));
                    push_vertically ? k>=0 : l>=0;
                    push_vertically ? k-- : l--)
                {
                    if (grid.isEmpty(k, l))
                        {
                        grid.insert(k,l, place);
                        break;
                        }
                        else
                        {
                        replaced = grid.get(k, l);
                        grid.insert(k, l, place);
                        place = replaced;
                        }
                }
            }
            else
            {
                //push the class in the positive direction positively
                ShapeObj *place = dunnart_nodes[n];
                ShapeObj *replaced;
                //if existing was in the neg direction, place new at first pos position, else at current position
                for(push_vertically ? (k = (y2 < y ? closest_row+1:closest_row)) : (l = (x2<x ? closest_col+1 : closest_col));
                    push_vertically ? k<num_rows : l<num_cols;
                    push_vertically ? k++ : l++)
                {
                    if (grid.isEmpty(k, l))
                        {
                        grid.insert(k,l, place);

                        break;
                        }
                        else
                        {
                        replaced = grid.get(k, l);
                        grid.insert(k, l, place);
                        place = replaced;
                        }
                }
            }
        }
    }
#if 0
    if (selection.size() == 1)
    {
        max_degree_node = gv_nodes[static_cast<ShapeObj *>(selection.front())];
    }
#endif
    canvas->deselectAll();

    //test connector crossings...
    for (int i = 0; i<num_rows; i++)
        for (int j=0; j<num_cols; j++)
        {
            if (!grid.isEmpty(i, j))
            {
                int ec = edge_crossings(canvas, gv_nodes[grid.get(i, j)],
                        i, j, gv, dunnart_nodes, gv_nodes, grid);
                printf("Crossings for %s: %d\n", grid.get(i,j)->getName(), ec);
            }
        }

    /* delete any existing alignment guides */
    QList<CanvasItem *> canvas_items = canvas->items();
    foreach (CanvasItem *ci, canvas_items)
    {  
        Guideline *guide = dynamic_cast<Guideline *> (ci);
        if (guide)
        {
            shape_select(guide);
            //guide->deleteObject(false);
        }
    }
    canvas->deleteSelection();
    canvas->deselectAll();

    //ShapeObj *grid[MAX_ROWS][MAX_COLS];
    /*
    for (int i = 0; i<MAX_ROWS; i++)
    {
        for (int j = 0; j<MAX_COLS; j++)
        {
            grid[i][j] = NULL;
        }
    }
    */

    //disable things? (like auto graph layout, etc.. maybe enable non-overlap etc)

    //make necessary moves.
    //grid.insert(0, 0, dunnart_nodes[max_degree_node]);
    //newPos[dunnart_nodes[max_degree_node]] = Point(sumX/num_nodes, sumY/num_nodes);    //starting node should be positioned in the graph centre.

    std::deque<Agnode_t*> Q;
    //int fi, fj;
    //grid.get_grid_coords(dunnart_nodes[max_degree_node], &fi, &fj);
    //GridPoint currP = GridPoint(fi, fj);
    Agnode_t* curr;
    marked[max_degree_node] = TRUE;
    Q.push_back(max_degree_node);

    count = 0;

    while (!Q.empty() && count++ < 80)
    {
        curr = Q.front();
        Q.pop_front();
        rec_align(canvas, Q, gv, gv_nodes, dunnart_nodes, degrees, marked, grid, curr, aligned);
        if (Q.empty())
        {
            //scan the diagram for unconnected parts
            max_degree = 0;
            max_degree_node = NULL;
            for (n= agfstnode(gv); n; n = agnxtnode(gv, n))
            {
                if (!marked[n])
                {
                    int deg = 0;
                    for (e = agfstout(gv, n); e; e = agnxtout(gv, e)) {
                        deg++;
                        }
                    for (e = agfstin(gv, n); e; e = agnxtin(gv, e)) {
                        deg++;
                        }
                    degrees[n] = deg;
                    if (deg > max_degree)
                        {
                        max_degree = deg;
                        max_degree_node = n;
                        }
                }
            }
            if (max_degree_node != NULL)
            {
                marked[max_degree_node] = TRUE;
                Q.push_back(max_degree_node);
            }
        }
    }

    //marked[max_degree_node] = TRUE;
    //We want to do aligning in a breadth-first order. We want to align nodes horizontally or vertically if possible.
    //Other nodes will be positioned in any remaining space afterwards (currently this means diagonally, but I might extend it to search for the nearest gap if that isn't enough)
    /*
    std::deque<GridPoint> Q;
    std::deque<GridPoint> SecondPass;
    GridPoint currP = GridPoint(0, 0, 0, 0);
    Q.push_back(currP);

    while (!Q.empty())
    {
        currP = Q.front();
        Q.pop_front();
        rec_align(Q, gv, gv_nodes, dunnart_nodes, degrees, marked, newPos, grid, currP.i, currP.j, currP.previ, currP.prevj);
        SecondPass.push_back(currP);
    }
    */

    //calculate new_graph_centre, translate all nodes to move new centre to old centre
    /*
    Point translate;
    sumX = 0;
    sumY = 0;

        //std::map<ShapeObj*, Point> newPos;

    for (std::map<ShapeObj*, Point>::iterator m = newPos.begin(); m != newPos.end(); ++m)
    {
        sumX += m->second.x;
        sumY += m->second.y;
    }

    translate.x = old_graph_centre.x - sumX/num_nodes;
    translate.y = old_graph_centre.y - sumY/num_nodes;
     */

    //animate into position using FRAMES steps. Use simple connector routing to make things clearer.
    const int FRAMES = 50;
    std::map<ShapeObj*, Point> incr;


    /*
    for (std::map<Agnode_t*, ShapeObj*>::iterator m = dunnart_nodes.begin(); m != dunnart_nodes.end(); ++m)
        {
            double x, y, w, h;
            m->second->getPosAndSize(x,y,w,h);
            newPos[m->second].x += translate.x;
            newPos[m->second].y += translate.y;
            incr[m->second] = Point((x-newPos[m->second].x)/FRAMES, (y-newPos[m->second].y)/FRAMES);
        }
     */

    for (int i = 0; i<num_rows; i++)
        for (int j=0; j<num_cols; j++)
        {
            if (!grid.isEmpty(i, j))
            {
                ShapeObj *shape = grid.get(i,j);
                shape->getPosAndSize(x,y,w,h);
                newPos[shape].x = (minX + j*max_width + (max_width-w)/2);
                newPos[shape].y = (minY + i*max_height + (max_height-h)/2);
                incr[shape] = Point((x-newPos[shape].x)/FRAMES,(y-newPos[shape].y)/FRAMES);
            }
        }

    for (int i = FRAMES-1; i>=0; --i)
    {
        for (std::map<Agnode_t*, ShapeObj*>::iterator m = dunnart_nodes.begin(); m != dunnart_nodes.end(); ++m)
        {
            m->second->setPos(
                    newPos[m->second].x + i*incr[m->second].x + HANDLE_PADDING,
                    newPos[m->second].y + i*incr[m->second].y + HANDLE_PADDING,
                    false);
        }

        //reroute connectors using simple routing.
        bool lastSimpleRouting = canvas->router()->SimpleRouting;
        canvas->router()->SimpleRouting = true;
        reroute_connectors(canvas);
        canvas->router()->SimpleRouting = lastSimpleRouting;
#if 0
        mathematicallyRepositionLabels();
#endif
    }

    grid.print();

    //now make necessary alignments

    //horizontal.
    int selections = 0;
    ShapeObj *last_node;

    //std::vector<std::vector<ShapeObj*> > &grid_data = grid.get_data();
    for (int i = grid.get_min_i(); i<=grid.get_max_i(); i++)
    {
        last_node = NULL;
        for (int j = grid.get_min_j(); j<=grid.get_max_j(); j++)
        {
            /*
             //disbled.. gaps in grid no longer a big deal
            if (grid.isEmpty(i, j))
            {
                if (selections > 1)
                {
                    CanvasItemList sel_copy = selection;
                    createAlignment(ALIGN_MIDDLE, sel_copy);
                }
                deselectAll();
                selections = 0;
            }*/
            if (!grid.isEmpty(i, j))
            {
                //class found but it isn't connected to the previous one or connected through a command class so start over
                if (selections > 0 && no_edge_or_common_neighbour(gv, gv_nodes, last_node, grid.get(i, j)))
                {
                    if (selections > 1)
                        {
                        CanvasItemList sel_copy = canvas->selectedItems();
                        createAlignment(ALIGN_MIDDLE, sel_copy);
                        }
                    canvas->deselectAll();
                    selections = 0;
                }
                shape_select(grid.get(i, j));
                last_node = grid.get(i, j);
                selections++;
            }
        }
        if (selections > 1)
        {
            CanvasItemList sel_copy = canvas->selectedItems();
            createAlignment(ALIGN_MIDDLE, sel_copy);
        }
        canvas->deselectAll();
        selections = 0;
    }

    //vertical.
    selections = 0;
    for (int j = grid.get_min_j(); j<=grid.get_max_j(); j++)
    {
        last_node = NULL;
        for (int i = grid.get_min_i(); i<=grid.get_max_i(); i++)
        {
            /*
             //disabled... gaps in the grid no longer a big deal.
            if (grid.isEmpty(i, j))
            {
                if (selections > 1)
                {
                    CanvasItemList sel_copy = selection;
                    createAlignment(ALIGN_CENTER, sel_copy);
                }
                deselectAll();
                selections = 0;
            }*/
            if (!grid.isEmpty(i, j))
            {
                //class found but it isn't connected to the previous one so start over
                if (selections > 0 && no_edge_or_common_neighbour(gv, gv_nodes, last_node, grid.get(i, j)))
                {
                    if (selections > 1)
                        {
                        CanvasItemList sel_copy = canvas->selectedItems();
                        createAlignment(ALIGN_CENTER, sel_copy);
                        }
                    canvas->deselectAll();
                    selections = 0;
                }
                shape_select(grid.get(i, j));
                last_node = grid.get(i, j);
                selections++;
            }
        }
        if (selections > 1)
        {
            CanvasItemList sel_copy = canvas->selectedItems();
            createAlignment(ALIGN_CENTER, sel_copy);
        }
        canvas->deselectAll();
        selections = 0;
    }


    Actions& actions = canvas->getActions();
    actions.clear();
    //add_undo_record(DELTA_MOVE, guide);   
    // Relayout.
    canvas->interrupt_graph_layout();

    agclose(gv);
    gvFreeContext(gvc);
}

}
#else
namespace dunnart {

void autoAlignSelection(Canvas *canvas)
{
    Q_UNUSED (canvas)
    // Do nothing.
    return;
}

}
#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

