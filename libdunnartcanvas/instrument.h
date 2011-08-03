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

#ifndef INSTRUMENT_H
#define INSTRUMENT_H


//#define INSTRUMENT


#ifdef INSTRUMENT
  #include <cstdarg>
  #include <ctime>
#endif

namespace dunnart {


#ifdef INSTRUMENT


extern FILE *ins_fp;


inline void ins_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    int secs = time(NULL) - base_time;
    int hr = secs / 3600;
    int mn = (secs % 3600) / 60;
    int sc = (secs % 3600) % 60;
    fprintf(ins_fp, "%02d:%02d:%02d - ", hr, mn, sc);
    vfprintf(ins_fp, fmt, ap);
    va_end(ap);
    fprintf(ins_fp, "\n");
    fflush(ins_fp);
}


inline void ins_init(const QString& filenameStr)
{
    char *filename = filenameStr.toLatin1().data();
    char logfile[strlen(filename) + 5];
    sprintf(logfile, "%s.log", filename);
    ins_fp = fopen(logfile, "w");
    if (!ins_fp)
    {
        qFatal("Cannot open log file `%s' for writing.\n", logfile);
    }
    ins_printf("File Opened");
}


inline void ins_end(void)
{
    if (ins_fp)
    {
        ins_printf("File Closed");
        fclose(ins_fp);
        ins_fp = NULL;
    }
}


#else

inline void ins_init(const QString& filenameStr)
{
    Q_UNUSED (filenameStr)
}


inline void ins_end(void)
{
}


inline void ins_printf(const char *fmt, ...)
{
    Q_UNUSED (fmt)
}

#endif


}
#endif

// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

