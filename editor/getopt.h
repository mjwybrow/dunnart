/*****************************************************************************
* getopt.h - competent and free getopt library.
* Mark K. Kim (mark@cbreak.org)
* http://www.cbreak.org/
*
* This is a free software you can use, modify, and include in your own
* program(s).  However, by using this software in any aforementioned manner,
* you agree to relieve the author of this software (me, Mark K. Kim,)
* from any liability.  Please take a special note that I do not claim
* this software to be fit for any purpose, though such meaning may be
* implied in the rest of the software.
*/
#ifndef GETOPT_H_
#define GETOPT_H_


#ifdef __cplusplus
extern "C" {
#endif


extern char* mj_optarg;
extern int mj_optind;
extern int mj_opterr;
extern int mj_optopt;

int mj_getopt(int argc, char** argv, char* optstr);


#ifdef __cplusplus
}
#endif


#endif /* GETOPT_H_ */


/* vim:ts=3
*/
// vim: filetype=cpp ts=4 sw=4 et tw=0 wm=0 cindent

