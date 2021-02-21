#ifndef STATUS_H
#define STATUS_H

#include <stddef.h>	// size_t
#include <stdlib.h>	// atexit()
#include <string.h>	// strdup()
#include "global.h"	// ssize_t

/* Sets error status accordingly */
#define setstat(stat)				\
	{								\
		ErrLn = __LINE__;			\
		ErrFile = __FILE__;			\
		ErrStat = stat;				\
	}

enum ErrorStatus {ERR_INTERNAL = 1, ERR_INVFLAG, ERR_INVARG, ERR_INVDEC, ERR_SYNTAX, ERR_OVERFLOW,
				  ERR_MISSOPER, ERR_DIVZERO, ERR_MODULO, ERR_IMAGINARY, ERR_INPUTSIZE};

extern char *ErrFile;	// File in which error occured
extern char *ErrStr;	// String containing invalid syntax
extern int ErrLn;		// Line at which internal error occured
extern int ErrStat;		// Error status
extern size_t ErrPos;	// Index in ErrStr where syntax is invalid

/* Frees error string buffer */
extern void clrstat(void);

/* Prints message according to error status */
extern void pstatus(void);

/* Sets invalid str and position
 * Pass string as NULL to omit */
extern void setinv(const char *str, size_t pos);

#endif // #ifndef STATUS_H