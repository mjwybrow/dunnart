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
 * Author(s): Michael Woodward
*/
#include <cstdio>
#include <vector>
#include "libdunnartcanvas/expand_grid.h"

namespace dunnart {

int dummy_loc;	//nasty hack, don't want to create a proper dummy node mechanism for now, so i'll just use the address of this int.

bool Grid::withinBounds(int i, int j)
{
	return ((i-oi) >=0 && (j-oj) >= 0 && (i-oi) < (int)data.size() && (j-oj) < (int)data[0].size());
}

bool Grid::isEmpty(int i, int j)
{
	//check if within bounds, if not, return true
	if (withinBounds(i, j))
	{
		if (data[i-oi][j-oj])
			return false;
		else
			return true;
	}
	else
		return true;
}

void Grid::insert_dummy_node(int i, int j)
{
	if (withinBounds(i, j))
	{
		data[i-oi][j-oj] = (ShapeObj*) (&dummy_loc);
	}
	else
	{
		resize(i, j);
		data[i-oi][j-oj] = (ShapeObj*) (&dummy_loc);
	}
}

bool Grid::is_dummy_node(int i, int j)
{
	return get(i, j) == (ShapeObj*) (&dummy_loc);
}

void Grid::insert(int i, int j, ShapeObj *shape)
{
	if (!isEmpty(i, j))
		printf("WARNING (expand_grid.cpp): insertion into occupied position\n");
	if (withinBounds(i, j))
	{
		data[i-oi][j-oj] = shape;
	}
	else
	{
		resize(i, j);
		data[i-oi][j-oj] = shape;
	}
	is[shape] = i;
	js[shape] = j;
}

//resize such that point i, j now exists
void Grid::resize(int newi, int newj)
{
	int oldrows, oldcols, newrows, newcols;
	
	oldrows = newrows = data.size();
	oldcols = newcols = data[0].size();
	int copyrows = 0;
	int copycols = 0;
	
	if (oi > newi)
	{
		copyrows = oi-newi;
		newrows += copyrows;
		oi -= copyrows;
	}
	else if ((int) data.size() <= newi-oi)
	{
		newrows += (newi-oi)-((int)data.size()) + 1;
	}
	if (oj > newj)
	{
		copycols = oj-newj;
		newcols += copycols;
		oj -= copycols;
	}
	else if ((int)data[0].size() <= newj-oj)
	{
		newcols += (newj-oj)-((int)data[0].size())+1;
	}
	
	//resize
	if (oldrows != newrows)
	{
		data.resize(newrows);
	}
	for (unsigned int i=0; i<data.size(); i++)
	{
		data[i].resize(newcols);
	}
	
	//copy
	if (copyrows > 0 || copycols > 0)
	{
		for (int i = oldrows - 1; i>=0; i--)
		{
			for (int j = oldcols - 1; j>=0; j--)
			{
				data[i+copyrows][j+copycols] = data[i][j];
				if (i<copyrows || j < copycols)
					data[i][j] = NULL;
			}
		}
	}
}

bool Grid::insert_row(int after_i)
{
	if (after_i < oi || after_i >= oi + (int)data.size()-1)
		return false;
	
	unsigned after_i2 = (unsigned) (after_i - oi);
	data.resize(data.size() + 1);
	data[data.size()-1].resize(data[0].size());
	for (int i = (int) data.size()-1; i>(int)after_i2+1; i--)
	{
		for (unsigned int j=0; j<data[i].size(); j++)
		{
			data[i][j] = data[i-1][j];
		}
	}
	for (unsigned int  j=0; j<data[after_i2+1].size(); j++)
	{
		data[after_i2+1][j] = NULL;
	}
	
	return true;
}

bool Grid::insert_col(int after_j)
{
	if (after_j < oj || after_j >= oj + (int)data[0].size() -1)
		return false;
	
	unsigned after_j2 = (unsigned) (after_j - oj);
	for (unsigned int i=0; i<data.size(); i++)
	{
		data[i].resize(data[i].size()+1);
		for (int j=(int)data[i].size()-1; j>(int)after_j2+1; j--)
		{
			data[i][j] = data[i][j-1];
		}
	}
	for (unsigned int  i=0; i<data.size(); i++)
	{
		data[i][after_j2+1] = NULL;
	}
	return true;
}

int Grid::get_min_i(void)
{
	return oi;
}

int Grid::get_min_j(void)
{
	return oj;
}

int Grid::get_max_i(void)
{
	return oi+data.size()-1;
}

int Grid::get_max_j(void)
{
	return oj+data[0].size()-1;
}

bool Grid::find(int *fi, int *fj, ShapeObj *shape)
{
	for (unsigned int i=0; i<data.size(); i++)
		for (unsigned int j=0; j<data[i].size(); j++)
		{
			if (data[i][j] == shape)
			{
				*fi = i;
				*fj = j;
				return true;
			}
		}
	return false;
}

void Grid::print(void)
{
	for(unsigned int i = 0; i< data.size(); i++)
	{
		for (unsigned int j=0; j<data[i].size(); j++)
		{
			if (data[i][j] != NULL && data[i][j] != (ShapeObj*)(&dummy_loc))
				printf("1 ");
			else
				printf("0 ");
		}
		printf("\n");
	}
}

ShapeObj *Grid::get(int i, int j)
{
	return data[i-oi][j-oj];
}

bool Grid::get_grid_coords(ShapeObj *shape, int *pi, int *pj)
{
	*pi = is[shape];
	*pj = js[shape];
	return true;	//should do a proper check to see if we found it.
}

void Grid::move(ShapeObj* shape, int newi, int newj)
{
	int oldi, oldj;
	get_grid_coords(shape, &oldi, &oldj);
	data[oldi-oi][oldj-oj] = NULL;
	insert(newi, newj, shape);
}

}
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

