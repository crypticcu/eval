#ifndef STATUS_H
#define STATUS_H

#include <stddef.h>	// size_t
#include <string.h>	// strdup()
#include "global.h"	// ssize_t (if needed)

#define errstat(status)				\
	if ((status) == ERR_INTERNAL) {	\
		errln = __LINE__;			\
		errfile = __FILE__;			\
	}								\
	errstat = (status)

#define invsynt(expr, pos)	\
	errpos = (pos);			\
	if(errstr)				\
		free(errstr);		\
	errstr = strdup(expr)

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

extern int errstat;

/* For internal errors */
extern char *errfile;
extern int errln;

/* For syntax errors */
extern char *errstr;
extern size_t errpos;

/* Prints message according to error status */
extern void pstatus(void);

/* Ensures error string is freed after program termination
 * Returns nonzero value on failure */
extern int initstat(void);

#endif /* #ifndef STATUS_H */