/*
 * Dunnart - Constraint-based Diagram Editor
 *
 * Copyright (C) 2003-2007  Michael Wybrow
 * Copyright (C) 2006-2008  Monash University
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
 * Author(s): Michael Wybrow  <http://michael.wybrow.info/>
*/

#ifndef SHARED_H
#define SHARED_H

#include <QtGlobal>
#include <cassert>

#ifndef NDEBUG
    #define SELECTIVE_DEBUG
#endif

#ifndef NDEBUG
  //#define DPRINTF_DEBUG
#endif

#ifdef DPRINTF_DEBUG
  #include <cstdarg>
#endif


namespace dunnart {


#define MAX_PATH_LEN 200
#define MAX_NAME_LEN 20

extern char runtime_path[MAX_PATH_LEN + 1];
extern char filename[MAX_PATH_LEN + 1];


#ifdef DPRINTF_DEBUG

inline void d_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stdout, fmt, ap);
    va_end(ap);
}
#else
inline void d_printf(const char *fmt, ...)
{
    // Fix unused variable warnings:
    Q_UNUSED (fmt);
}
#endif


}
#endif
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

