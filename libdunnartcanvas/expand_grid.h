/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2007  Michael Woodward
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
*/

#ifndef EXPANDGRID_H_
#define EXPANDGRID_H_

#include <vector>
#include <map>

namespace dunnart {

class ShapeObj;

class Grid
{
    public:
        Grid()
        {
           //init to 1x1 grid
           oi = 0;
           oj = 0;
           data.resize(1);
           data[0].resize(1);
        }
        Grid(int num_rows, int num_cols)
        {
            oi = 0;
            oj = 0;
            data.resize(num_rows);
            for (int i = 0; i<num_rows; i++)
                data[i].resize(num_cols);
        }
        bool isEmpty(int i, int j);
        bool withinBounds(int i, int j);
        void insert(int i, int j, ShapeObj* uml);
        void insert_dummy_node(int i, int j);
        bool is_dummy_node(int i, int j);
        bool insert_row(int after_i);
        bool insert_col(int after_j);
        void resize(int i, int j);
        int get_min_i(void);
        int get_min_j(void);
        int get_max_i(void);
        int get_max_j(void);
        bool find(int *fi, int *fj, ShapeObj *uml);
        void print(void);
        ShapeObj *get(int i, int j);
        bool get_grid_coords(ShapeObj* uml, int *pi, int *pj);    //warning, may not be compatible with expanding grid.
        void move(ShapeObj* uml, int newi, int newj);
    private:
        int oi, oj;
        std::vector<std::vector<ShapeObj*> > data;
        std::map<ShapeObj*, int> is;
        std::map<ShapeObj*, int> js;
};

}
#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

