#ifndef STATUS_H
#define STATUS_H

#include <stddef.h>	// size_t
#include <string.h>	// strdup()
#include "global.h"	// ssize_t (if needed)

/* Sets error status accordingly */
#define errstat(status)				\
	if ((status) == ERR_INTERNAL) {	\
		errln = __LINE__;			\
		errfile = __FILE__;			\
	}								\
	errstat = (status)

/* Indicates invalid syntax */
#define invsynt(expr, pos)			\
	errpos = (pos);					\
	if(errstr)						\
		free(errstr);				\
	if (!(errstr = strdup(expr)))	\
		errstat(ERR_INTERNAL)

/* Ensures error string is freed after program termination
 * Returns nonzero value on failure */
#define initstat()	atexit(deststat)

#define ERR_INTERNAL	1
#define ERR_INVFLAG		2
#define ERR_INVDEC		3
#define ERR_SYNTAX		4
#define ERR_OVERFLOW	5
#define ERR_MISSOPER	6
#define ERR_DIVZERO		7
#define ERR_MODULO		8
#define ERR_EVENROOT	9
#define ERR_INPUTSIZE	10

extern int errstat;		// Error status
extern char *errfile;	// File in which error occured
extern int errln;		// Line at which internal error occured
extern char *errstr;	// String containing invalid syntax
extern size_t errpos;	// Index in errstr where syntax is invalid

/* Error string destructor */
extern void deststat(void);

/* Prints message according to error status */
extern void pstatus(void);

#endif // #ifndef STATUS_H